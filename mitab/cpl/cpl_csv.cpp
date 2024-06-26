/******************************************************************************
 * $Id: cpl_csv.cpp 10646 2007-01-18 02:38:10Z warmerdam $
 *
 * Project:  CPL - Common Portability Library
 * Purpose:  CSV (comma separated value) file access.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 1999, Frank Warmerdam
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ****************************************************************************/

#include "cpl_csv.h"

#include "cpl_conv.h"
#include "cpl_multiproc.h"

CPL_CVSID("$Id: cpl_csv.cpp 10646 2007-01-18 02:38:10Z warmerdam $");

CPL_C_START
const char *GDALDefaultCSVFilename(const char *pszBasename);
CPL_C_END

/* ==================================================================== */
/*      The CSVTable is a persistant set of info about an open CSV      */
/*      table.  While it doesn't currently maintain a record index,     */
/*      or in-memory copy of the table, it could be changed to do so    */
/*      in the future.                                                  */
/* ==================================================================== */
using CSVTable = struct ctb {
    FILE *fp;

    struct ctb *psNext;

    char *pszFilename;

    char **papszFieldNames;

    char **papszRecFields;

    int iLastLine;

    /* Cache for whole file */
    int nLineCount;
    char **papszLines;
    int *panLineIndex;
    char *pszRawData;
};

/* It would likely be better to share this list between threads, but
   that will require some rework. */

/************************************************************************/
/*                             CSVAccess()                              */
/*                                                                      */
/*      This function will fetch a handle to the requested table.       */
/*      If not found in the ``open table list'' the table will be       */
/*      opened and added to the list.  Eventually this function may     */
/*      become public with an abstracted return type so that            */
/*      applications can set options about the table.  For now this     */
/*      isn't done.                                                     */
/************************************************************************/

static CSVTable *CSVAccess(const char *pszFilename)

{
    CSVTable *psTable;
    FILE *fp;

    /* -------------------------------------------------------------------- */
    /*      Fetch the table, and allocate the thread-local pointer to it    */
    /*      if there isn't already one.                                     */
    /* -------------------------------------------------------------------- */
    CSVTable **ppsCSVTableList;

    ppsCSVTableList = (CSVTable **)CPLGetTLS(CTLS_CSVTABLEPTR);
    if (ppsCSVTableList == nullptr) {
        ppsCSVTableList = (CSVTable **)CPLCalloc(1, sizeof(CSVTable *));
        CPLSetTLS(CTLS_CSVTABLEPTR, ppsCSVTableList, TRUE);
    }

    /* -------------------------------------------------------------------- */
    /*      Is the table already in the list.                               */
    /* -------------------------------------------------------------------- */
    for (psTable = *ppsCSVTableList; psTable != nullptr; psTable = psTable->psNext) {
        if (EQUAL(psTable->pszFilename, pszFilename)) {
            /*
             * Eventually we should consider promoting to the front of
             * the list to accelerate frequently accessed tables.
             */

            return (psTable);
        }
    }

    /* -------------------------------------------------------------------- */
    /*      If not, try to open it.                                         */
    /* -------------------------------------------------------------------- */
    fp = VSIFOpen(pszFilename, "rb");
    if (fp == nullptr)
        return nullptr;

    /* -------------------------------------------------------------------- */
    /*      Create an information structure about this table, and add to    */
    /*      the front of the list.                                          */
    /* -------------------------------------------------------------------- */
    psTable = (CSVTable *)CPLCalloc(sizeof(CSVTable), 1);

    psTable->fp = fp;
    psTable->pszFilename = CPLStrdup(pszFilename);
    psTable->psNext = *ppsCSVTableList;

    *ppsCSVTableList = psTable;

    /* -------------------------------------------------------------------- */
    /*      Read the table header record containing the field names.        */
    /* -------------------------------------------------------------------- */
    psTable->papszFieldNames = CSVReadParseLine(fp);

    return (psTable);
}

/************************************************************************/
/*                            CSVDeaccess()                             */
/************************************************************************/

void CSVDeaccess(const char *pszFilename)

{
    CSVTable *psLast, *psTable;

    /* -------------------------------------------------------------------- */
    /*      Fetch the table, and allocate the thread-local pointer to it    */
    /*      if there isn't already one.                                     */
    /* -------------------------------------------------------------------- */
    CSVTable **ppsCSVTableList;

    ppsCSVTableList = (CSVTable **)CPLGetTLS(CTLS_CSVTABLEPTR);
    if (ppsCSVTableList == nullptr)
        return;

    /* -------------------------------------------------------------------- */
    /*      A NULL means deaccess all tables.                               */
    /* -------------------------------------------------------------------- */
    if (pszFilename == nullptr) {
        while (*ppsCSVTableList != nullptr) CSVDeaccess((*ppsCSVTableList)->pszFilename);

        return;
    }

    /* -------------------------------------------------------------------- */
    /*      Find this table.                                                */
    /* -------------------------------------------------------------------- */
    psLast = nullptr;
    for (psTable = *ppsCSVTableList;
         psTable != nullptr && !EQUAL(psTable->pszFilename, pszFilename);
         psTable = psTable->psNext) {
        psLast = psTable;
    }

    if (psTable == nullptr) {
        CPLDebug("CPL_CSV", "CPLDeaccess( %s ) - no match.", pszFilename);
        return;
    }

    /* -------------------------------------------------------------------- */
    /*      Remove the link from the list.                                  */
    /* -------------------------------------------------------------------- */
    if (psLast != nullptr)
        psLast->psNext = psTable->psNext;
    else
        *ppsCSVTableList = psTable->psNext;

    /* -------------------------------------------------------------------- */
    /*      Free the table.                                                 */
    /* -------------------------------------------------------------------- */
    if (psTable->fp != nullptr)
        VSIFClose(psTable->fp);

    CSLDestroy(psTable->papszFieldNames);
    CSLDestroy(psTable->papszRecFields);
    CPLFree(psTable->pszFilename);
    CPLFree(psTable->panLineIndex);
    CPLFree(psTable->pszRawData);
    CPLFree(psTable->papszLines);

    CPLFree(psTable);

    CPLReadLine(nullptr);
}

/************************************************************************/
/*                            CSVSplitLine()                            */
/*                                                                      */
/*      Tokenize a CSV line into fields in the form of a string         */
/*      list.  This is used instead of the CPLTokenizeString()          */
/*      because it provides correct CSV escaping and quoting            */
/*      semantics.                                                      */
/************************************************************************/

static char **CSVSplitLine(const char *pszString)

{
    char **papszRetList = nullptr;
    char *pszToken;
    int nTokenMax, nTokenLen;

    pszToken = (char *)CPLCalloc(10, 1);
    nTokenMax = 10;

    while (pszString != nullptr && *pszString != '\0') {
        int bInString = FALSE;

        nTokenLen = 0;

        /* Try to find the next delimeter, marking end of token */
        for (; *pszString != '\0'; pszString++) {
            /* End if this is a delimeter skip it and break. */
            if (!bInString && *pszString == ',') {
                pszString++;
                break;
            }

            if (*pszString == '"') {
                if (!bInString || pszString[1] != '"') {
                    bInString = !bInString;
                    continue;
                } /* doubled quotes in string resolve to one quote */

                pszString++;
            }

            if (nTokenLen >= nTokenMax - 2) {
                nTokenMax = nTokenMax * 2 + 10;
                pszToken = (char *)CPLRealloc(pszToken, nTokenMax);
            }

            pszToken[nTokenLen] = *pszString;
            nTokenLen++;
        }

        pszToken[nTokenLen] = '\0';
        papszRetList = CSLAddString(papszRetList, pszToken);

        /* If the last token is an empty token, then we have to catch
         * it now, otherwise we won't reenter the loop and it will be lost.
         */
        if (*pszString == '\0' && *(pszString - 1) == ',') {
            papszRetList = CSLAddString(papszRetList, "");
        }
    }

    if (papszRetList == nullptr)
        papszRetList = (char **)CPLCalloc(sizeof(char *), 1);

    CPLFree(pszToken);

    return papszRetList;
}

/************************************************************************/
/*                          CSVFindNextLine()                           */
/*                                                                      */
/*      Find the start of the next line, while at the same time zero    */
/*      terminating this line.  Take into account that there may be     */
/*      newline indicators within quoted strings, and that quotes       */
/*      can be escaped with a backslash.                                */
/************************************************************************/

static char *CSVFindNextLine(char *pszThisLine)

{
    int nQuoteCount = 0, i;

    for (i = 0; pszThisLine[i] != '\0'; i++) {
        if (pszThisLine[i] == '\"' && (i == 0 || pszThisLine[i - 1] != '\\'))
            nQuoteCount++;

        if ((pszThisLine[i] == 10 || pszThisLine[i] == 13) && (nQuoteCount % 2) == 0)
            break;
    }

    while (pszThisLine[i] == 10 || pszThisLine[i] == 13) pszThisLine[i++] = '\0';

    if (pszThisLine[i] == '\0')
        return nullptr;

    return pszThisLine + i;
}

/************************************************************************/
/*                             CSVIngest()                              */
/*                                                                      */
/*      Load entire file into memory and setup index if possible.       */
/************************************************************************/

static void CSVIngest(const char *pszFilename)

{
    CSVTable *psTable = CSVAccess(pszFilename);
    int nFileLen, i, nMaxLineCount, iLine = 0;
    char *pszThisLine;

    if (psTable->pszRawData != nullptr)
        return;

    /* -------------------------------------------------------------------- */
    /*      Ingest whole file.                                              */
    /* -------------------------------------------------------------------- */
    VSIFSeek(psTable->fp, 0, SEEK_END);
    nFileLen = VSIFTell(psTable->fp);
    VSIRewind(psTable->fp);

    psTable->pszRawData = (char *)CPLMalloc(nFileLen + 1);
    if ((int)VSIFRead(psTable->pszRawData, 1, nFileLen, psTable->fp) != nFileLen) {
        CPLFree(psTable->pszRawData);
        psTable->pszRawData = nullptr;

        CPLError(CE_Failure, CPLE_FileIO, "Read of file %s failed.", psTable->pszFilename);
        return;
    }

    psTable->pszRawData[nFileLen] = '\0';

    /* -------------------------------------------------------------------- */
    /*      Get count of newlines so we can allocate line array.            */
    /* -------------------------------------------------------------------- */
    nMaxLineCount = 0;
    for (i = 0; i < nFileLen; i++) {
        if (psTable->pszRawData[i] == 10)
            nMaxLineCount++;
    }

    psTable->papszLines = (char **)CPLCalloc(sizeof(char *), nMaxLineCount);

    /* -------------------------------------------------------------------- */
    /*      Build a list of record pointers into the raw data buffer        */
    /*      based on line terminators.  Zero terminate the line             */
    /*      strings.                                                        */
    /* -------------------------------------------------------------------- */
    /* skip header line */
    pszThisLine = CSVFindNextLine(psTable->pszRawData);

    while (pszThisLine != nullptr && iLine < nMaxLineCount) {
        psTable->papszLines[iLine++] = pszThisLine;
        pszThisLine = CSVFindNextLine(pszThisLine);
    }

    psTable->nLineCount = iLine;

    /* -------------------------------------------------------------------- */
    /*      Allocate and populate index array.  Ensure they are in          */
    /*      ascending order so that binary searches can be done on the      */
    /*      array.                                                          */
    /* -------------------------------------------------------------------- */
    psTable->panLineIndex = (int *)CPLMalloc(sizeof(int) * psTable->nLineCount);
    for (i = 0; i < psTable->nLineCount; i++) {
        psTable->panLineIndex[i] = atoi(psTable->papszLines[i]);

        if (i > 0 && psTable->panLineIndex[i] < psTable->panLineIndex[i - 1]) {
            CPLFree(psTable->panLineIndex);
            psTable->panLineIndex = nullptr;
            break;
        }
    }

    psTable->iLastLine = -1;

    /* -------------------------------------------------------------------- */
    /*      We should never need the file handle against, so close it.      */
    /* -------------------------------------------------------------------- */
    VSIFClose(psTable->fp);
    psTable->fp = nullptr;
}

/************************************************************************/
/*                          CSVReadParseLine()                          */
/*                                                                      */
/*      Read one line, and return split into fields.  The return        */
/*      result is a stringlist, in the sense of the CSL functions.      */
/************************************************************************/

char **CSVReadParseLine(FILE *fp)

{
    const char *pszLine;
    char *pszWorkLine;
    char **papszReturn;

    CPLAssert(fp != NULL);
    if (fp == nullptr)
        return (nullptr);

    pszLine = CPLReadLine(fp);
    if (pszLine == nullptr)
        return (nullptr);

    /* -------------------------------------------------------------------- */
    /*      If there are no quotes, then this is the simple case.           */
    /*      Parse, and return tokens.                                       */
    /* -------------------------------------------------------------------- */
    if (strchr(pszLine, '\"') == nullptr)
        return CSVSplitLine(pszLine);

    /* -------------------------------------------------------------------- */
    /*      We must now count the quotes in our working string, and as      */
    /*      long as it is odd, keep adding new lines.                       */
    /* -------------------------------------------------------------------- */
    pszWorkLine = CPLStrdup(pszLine);

    while (TRUE) {
        int i, nCount = 0;

        for (i = 0; pszWorkLine[i] != '\0'; i++) {
            if (pszWorkLine[i] == '\"' && (i == 0 || pszWorkLine[i - 1] != '\\'))
                nCount++;
        }

        if (nCount % 2 == 0)
            break;

        pszLine = CPLReadLine(fp);
        if (pszLine == nullptr)
            break;

        pszWorkLine = (char *)CPLRealloc(pszWorkLine, strlen(pszWorkLine) + strlen(pszLine) + 2);
        strcat(pszWorkLine, "\n");  // This gets lost in CPLReadLine().
        strcat(pszWorkLine, pszLine);
    }

    papszReturn = CSVSplitLine(pszWorkLine);

    CPLFree(pszWorkLine);

    return papszReturn;
}

/************************************************************************/
/*                             CSVCompare()                             */
/*                                                                      */
/*      Compare a field to a search value using a particular            */
/*      criteria.                                                       */
/************************************************************************/

static int CSVCompare(const char *pszFieldValue, const char *pszTarget,
                      CSVCompareCriteria eCriteria)

{
    if (eCriteria == CC_ExactString) {
        return (strcmp(pszFieldValue, pszTarget) == 0);
    }
    if (eCriteria == CC_ApproxString) {
        return (EQUAL(pszFieldValue, pszTarget));
    }
    if (eCriteria == CC_Integer) {
        return (atoi(pszFieldValue) == atoi(pszTarget));
    }

    return FALSE;
}

/************************************************************************/
/*                            CSVScanLines()                            */
/*                                                                      */
/*      Read the file scanline for lines where the key field equals     */
/*      the indicated value with the suggested comparison criteria.     */
/*      Return the first matching line split into fields.               */
/************************************************************************/

char **CSVScanLines(FILE *fp, int iKeyField, const char *pszValue, CSVCompareCriteria eCriteria)

{
    char **papszFields = nullptr;
    int bSelected = FALSE, nTestValue;

    CPLAssert(pszValue != NULL);
    CPLAssert(iKeyField >= 0);
    CPLAssert(fp != NULL);

    nTestValue = atoi(pszValue);

    while (!bSelected) {
        papszFields = CSVReadParseLine(fp);
        if (papszFields == nullptr)
            return (nullptr);

        if (CSLCount(papszFields) < iKeyField + 1) {
            /* not selected */
        } else if (eCriteria == CC_Integer && atoi(papszFields[iKeyField]) == nTestValue) {
            bSelected = TRUE;
        } else {
            bSelected = CSVCompare(papszFields[iKeyField], pszValue, eCriteria);
        }

        if (!bSelected) {
            CSLDestroy(papszFields);
            papszFields = nullptr;
        }
    }

    return (papszFields);
}

/************************************************************************/
/*                        CSVScanLinesIndexed()                         */
/*                                                                      */
/*      Read the file scanline for lines where the key field equals     */
/*      the indicated value with the suggested comparison criteria.     */
/*      Return the first matching line split into fields.               */
/************************************************************************/

static char **CSVScanLinesIndexed(CSVTable *psTable, int nKeyValue)

{
    int iTop, iBottom, iMiddle, iResult = -1;

    CPLAssert(psTable->panLineIndex != NULL);

    /* -------------------------------------------------------------------- */
    /*      Find target record with binary search.                          */
    /* -------------------------------------------------------------------- */
    iTop = psTable->nLineCount - 1;
    iBottom = 0;

    while (iTop >= iBottom) {
        iMiddle = (iTop + iBottom) / 2;
        if (psTable->panLineIndex[iMiddle] > nKeyValue)
            iTop = iMiddle - 1;
        else if (psTable->panLineIndex[iMiddle] < nKeyValue)
            iBottom = iMiddle + 1;
        else {
            iResult = iMiddle;
            break;
        }
    }

    if (iResult == -1)
        return nullptr;

    /* -------------------------------------------------------------------- */
    /*      Parse target line, and update iLastLine indicator.              */
    /* -------------------------------------------------------------------- */
    psTable->iLastLine = iResult;

    return CSVSplitLine(psTable->papszLines[iResult]);
}

/************************************************************************/
/*                        CSVScanLinesIngested()                        */
/*                                                                      */
/*      Read the file scanline for lines where the key field equals     */
/*      the indicated value with the suggested comparison criteria.     */
/*      Return the first matching line split into fields.               */
/************************************************************************/

static char **CSVScanLinesIngested(CSVTable *psTable, int iKeyField, const char *pszValue,
                                   CSVCompareCriteria eCriteria)

{
    char **papszFields = nullptr;
    int bSelected = FALSE, nTestValue;

    CPLAssert(pszValue != NULL);
    CPLAssert(iKeyField >= 0);

    nTestValue = atoi(pszValue);

    /* -------------------------------------------------------------------- */
    /*      Short cut for indexed files.                                    */
    /* -------------------------------------------------------------------- */
    if (iKeyField == 0 && eCriteria == CC_Integer && psTable->panLineIndex != nullptr)
        return CSVScanLinesIndexed(psTable, nTestValue);

    /* -------------------------------------------------------------------- */
    /*      Scan from in-core lines.                                        */
    /* -------------------------------------------------------------------- */
    while (!bSelected && psTable->iLastLine + 1 < psTable->nLineCount) {
        psTable->iLastLine++;
        papszFields = CSVSplitLine(psTable->papszLines[psTable->iLastLine]);

        if (CSLCount(papszFields) < iKeyField + 1) {
            /* not selected */
        } else if (eCriteria == CC_Integer && atoi(papszFields[iKeyField]) == nTestValue) {
            bSelected = TRUE;
        } else {
            bSelected = CSVCompare(papszFields[iKeyField], pszValue, eCriteria);
        }

        if (!bSelected) {
            CSLDestroy(papszFields);
            papszFields = nullptr;
        }
    }

    return (papszFields);
}

/************************************************************************/
/*                            CSVScanFile()                             */
/*                                                                      */
/*      Scan a whole file using criteria similar to above, but also     */
/*      taking care of file opening and closing.                        */
/************************************************************************/

char **CSVScanFile(const char *pszFilename, int iKeyField, const char *pszValue,
                   CSVCompareCriteria eCriteria)

{
    CSVTable *psTable;

    /* -------------------------------------------------------------------- */
    /*      Get access to the table.                                        */
    /* -------------------------------------------------------------------- */
    CPLAssert(pszFilename != NULL);

    if (iKeyField < 0)
        return nullptr;

    psTable = CSVAccess(pszFilename);
    if (psTable == nullptr)
        return nullptr;

    CSVIngest(pszFilename);

    /* -------------------------------------------------------------------- */
    /*      Does the current record match the criteria?  If so, just        */
    /*      return it again.                                                */
    /* -------------------------------------------------------------------- */
    if (iKeyField >= 0 && iKeyField < CSLCount(psTable->papszRecFields) &&
        CSVCompare(pszValue, psTable->papszRecFields[iKeyField], eCriteria)) {
        return psTable->papszRecFields;
    }

    /* -------------------------------------------------------------------- */
    /*      Scan the file from the beginning, replacing the ``current       */
    /*      record'' in our structure with the one that is found.           */
    /* -------------------------------------------------------------------- */
    psTable->iLastLine = -1;
    CSLDestroy(psTable->papszRecFields);

    if (psTable->pszRawData != nullptr)
        psTable->papszRecFields = CSVScanLinesIngested(psTable, iKeyField, pszValue, eCriteria);
    else {
        VSIRewind(psTable->fp);
        CPLReadLine(psTable->fp); /* throw away the header line */

        psTable->papszRecFields = CSVScanLines(psTable->fp, iKeyField, pszValue, eCriteria);
    }

    return (psTable->papszRecFields);
}

/************************************************************************/
/*                           CPLGetFieldId()                            */
/*                                                                      */
/*      Read the first record of a CSV file (rewinding to be sure),     */
/*      and find the field with the indicated name.  Returns -1 if      */
/*      it fails to find the field name.  Comparison is case            */
/*      insensitive, but otherwise exact.  After this function has      */
/*      been called the file pointer will be positioned just after      */
/*      the first record.                                               */
/************************************************************************/

int CSVGetFieldId(FILE *fp, const char *pszFieldName)

{
    char **papszFields;
    int i;

    CPLAssert(fp != NULL && pszFieldName != NULL);

    VSIRewind(fp);

    papszFields = CSVReadParseLine(fp);
    for (i = 0; papszFields != nullptr && papszFields[i] != nullptr; i++) {
        if (EQUAL(papszFields[i], pszFieldName)) {
            CSLDestroy(papszFields);
            return i;
        }
    }

    CSLDestroy(papszFields);

    return -1;
}

/************************************************************************/
/*                         CSVGetFileFieldId()                          */
/*                                                                      */
/*      Same as CPLGetFieldId(), except that we get the file based      */
/*      on filename, rather than having an existing handle.             */
/************************************************************************/

int CSVGetFileFieldId(const char *pszFilename, const char *pszFieldName)

{
    CSVTable *psTable;
    int i;

    /* -------------------------------------------------------------------- */
    /*      Get access to the table.                                        */
    /* -------------------------------------------------------------------- */
    CPLAssert(pszFilename != NULL);

    psTable = CSVAccess(pszFilename);
    if (psTable == nullptr)
        return -1;

    /* -------------------------------------------------------------------- */
    /*      Find the requested field.                                       */
    /* -------------------------------------------------------------------- */
    for (i = 0; psTable->papszFieldNames != nullptr && psTable->papszFieldNames[i] != nullptr;
         i++) {
        if (EQUAL(psTable->papszFieldNames[i], pszFieldName)) {
            return i;
        }
    }

    return -1;
}

/************************************************************************/
/*                         CSVScanFileByName()                          */
/*                                                                      */
/*      Same as CSVScanFile(), but using a field name instead of a      */
/*      field number.                                                   */
/************************************************************************/

char **CSVScanFileByName(const char *pszFilename, const char *pszKeyFieldName, const char *pszValue,
                         CSVCompareCriteria eCriteria)

{
    int iKeyField;

    iKeyField = CSVGetFileFieldId(pszFilename, pszKeyFieldName);
    if (iKeyField == -1)
        return nullptr;

    return (CSVScanFile(pszFilename, iKeyField, pszValue, eCriteria));
}

/************************************************************************/
/*                            CSVGetField()                             */
/*                                                                      */
/*      The all-in-one function to fetch a particular field value       */
/*      from a CSV file.  Note this function will return an empty       */
/*      string, rather than NULL if it fails to find the desired        */
/*      value for some reason.  The caller can't establish that the     */
/*      fetch failed.                                                   */
/************************************************************************/

const char *CSVGetField(const char *pszFilename, const char *pszKeyFieldName,
                        const char *pszKeyFieldValue, CSVCompareCriteria eCriteria,
                        const char *pszTargetField)

{
    CSVTable *psTable;
    char **papszRecord;
    int iTargetField;

    /* -------------------------------------------------------------------- */
    /*      Find the table.                                                 */
    /* -------------------------------------------------------------------- */
    psTable = CSVAccess(pszFilename);
    if (psTable == nullptr)
        return "";

    /* -------------------------------------------------------------------- */
    /*      Find the correct record.                                        */
    /* -------------------------------------------------------------------- */
    papszRecord = CSVScanFileByName(pszFilename, pszKeyFieldName, pszKeyFieldValue, eCriteria);

    if (papszRecord == nullptr)
        return "";

    /* -------------------------------------------------------------------- */
    /*      Figure out which field we want out of this.                     */
    /* -------------------------------------------------------------------- */
    iTargetField = CSVGetFileFieldId(pszFilename, pszTargetField);
    if (iTargetField < 0)
        return "";

    if (iTargetField >= CSLCount(papszRecord))
        return "";

    return (papszRecord[iTargetField]);
}

/************************************************************************/
/*                       GDALDefaultCSVFilename()                       */
/************************************************************************/

const char *GDALDefaultCSVFilename(const char *pszBasename)

{
    static CPL_THREADLOCAL char szPath[512];
    FILE *fp = nullptr;
    const char *pszResult;
    static CPL_THREADLOCAL int bCSVFinderInitialized = FALSE;

    pszResult = CPLFindFile("epsg_csv", pszBasename);

    if (pszResult != nullptr)
        return pszResult;

    if (!bCSVFinderInitialized) {
        bCSVFinderInitialized = TRUE;

        if (CPLGetConfigOption("GEOTIFF_CSV", nullptr) != nullptr)
            CPLPushFinderLocation(CPLGetConfigOption("GEOTIFF_CSV", nullptr));

        if (CPLGetConfigOption("GDAL_DATA", nullptr) != nullptr)
            CPLPushFinderLocation(CPLGetConfigOption("GDAL_DATA", nullptr));

        pszResult = CPLFindFile("epsg_csv", pszBasename);

        if (pszResult != nullptr)
            return pszResult;
    }

    if ((fp = fopen("csv/horiz_cs.csv", "rt")) != nullptr) {
        sprintf(szPath, "csv/%s", pszBasename);
    } else {
#ifdef GDAL_PREFIX
#ifdef MACOSX_FRAMEWORK
        sprintf(szPath, GDAL_PREFIX "/Resources/epsg_csv/%s", pszBasename);
#else
        sprintf(szPath, GDAL_PREFIX "/share/epsg_csv/%s", pszBasename);
#endif
#else
        sprintf(szPath, "/usr/local/share/epsg_csv/%s", pszBasename);
#endif
        if ((fp = fopen(szPath, "rt")) == nullptr)
            strcpy(szPath, pszBasename);
    }

    if (fp != nullptr)
        fclose(fp);

    return (szPath);
}

/************************************************************************/
/*                            CSVFilename()                             */
/*                                                                      */
/*      Return the full path to a particular CSV file.  This will       */
/*      eventually be something the application can override.           */
/************************************************************************/

static const char *(*pfnCSVFilenameHook)(const char *) = nullptr;

const char *CSVFilename(const char *pszBasename)

{
    if (pfnCSVFilenameHook == nullptr)
        return GDALDefaultCSVFilename(pszBasename);

    return (pfnCSVFilenameHook(pszBasename));
}

/************************************************************************/
/*                         SetCSVFilenameHook()                         */
/*                                                                      */
/*      Applications can use this to set a function that will           */
/*      massage CSV filenames.                                          */
/************************************************************************/

/**
 * Override CSV file search method.
 *
 * @param CSVFileOverride The pointer to a function which will return the
 * full path for a given filename.
  *

This function allows an application to override how the GTIFGetDefn() and related function find the
CSV (Comma Separated Value) values required. The pfnHook argument should be a pointer to a function
that will take in a CSV filename and return a full path to the file. The returned string should be
to an internal static buffer so that the caller doesn't have to free the result.

<b>Example:</b><br>

The listgeo utility uses the following override function if the user
specified a CSV file directory with the -t commandline switch (argument
put into CSVDirName).  <p>

<pre>

    ...


    SetCSVFilenameHook( CSVFileOverride );

    ...


static const char *CSVFileOverride( const char * pszInput )

{
    static char         szPath[1024];

#ifdef WIN32
    sprintf( szPath, "%s\\%s", CSVDirName, pszInput );
#else
    sprintf( szPath, "%s/%s", CSVDirName, pszInput );
#endif

    return( szPath );
}
</pre>

*/

CPL_C_START
void SetCSVFilenameHook(const char *(*pfnNewHook)(const char *))

{
    pfnCSVFilenameHook = pfnNewHook;
}
CPL_C_END
