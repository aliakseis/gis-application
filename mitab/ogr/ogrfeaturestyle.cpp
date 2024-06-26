/******************************************************************************
 * $Id: ogrfeaturestyle.cpp 11399 2007-05-02 20:10:00Z warmerdam $
 *
 * Project:  OpenGIS Simple Features Reference Implementation
 * Purpose:  Feature Representation string API
 * Author:   Stephane Villeneuve, stephane.v@videotron.ca
 *
 ******************************************************************************
 * Copyright (c) 2000-2001, Stephane Villeneuve
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

#include "cpl_conv.h"
#include "cpl_string.h"
#include "ogr_feature.h"
#include "ogr_featurestyle.h"

CPL_CVSID("$Id: ogrfeaturestyle.cpp 11399 2007-05-02 20:10:00Z warmerdam $");

CPL_C_START
void OGRFeatureStylePuller() {}
CPL_C_END

/****************************************************************************/
/*                Class Parameter (used in the String)                      */
/*                                                                          */
/*      The order of all parameter MUST be the same than in the definition  */
/****************************************************************************/
OGRStyleParamId asStylePen[] = {{OGRSTPenColor, "c", FALSE, OGRSTypeString},
                                {OGRSTPenWidth, "w", TRUE, OGRSTypeDouble},
                                {OGRSTPenPattern, "p", TRUE, OGRSTypeString},
                                {OGRSTPenId, "id", FALSE, OGRSTypeString},
                                {OGRSTPenPerOffset, "dp", TRUE, OGRSTypeDouble},
                                {OGRSTPenCap, "cap", FALSE, OGRSTypeString},
                                {OGRSTPenJoin, "j", FALSE, OGRSTypeString},
                                {OGRSTPenPriority, "l", FALSE, OGRSTypeInteger}};

OGRStyleParamId asStyleBrush[] = {{OGRSTBrushFColor, "fc", FALSE, OGRSTypeString},
                                  {OGRSTBrushBColor, "bc", FALSE, OGRSTypeString},
                                  {OGRSTBrushId, "id", FALSE, OGRSTypeString},
                                  {OGRSTBrushAngle, "a", TRUE, OGRSTypeDouble},
                                  {OGRSTBrushSize, "s", TRUE, OGRSTypeDouble},
                                  {OGRSTBrushDx, "dx", TRUE, OGRSTypeDouble},
                                  {OGRSTBrushDy, "dy", TRUE, OGRSTypeDouble},
                                  {OGRSTBrushPriority, "l", FALSE, OGRSTypeInteger}};

OGRStyleParamId asStyleSymbol[] = {{OGRSTSymbolId, "id", FALSE, OGRSTypeString},
                                   {OGRSTSymbolAngle, "a", FALSE, OGRSTypeDouble},
                                   {OGRSTSymbolColor, "c", FALSE, OGRSTypeString},
                                   {OGRSTSymbolSize, "s", TRUE, OGRSTypeDouble},
                                   {OGRSTSymbolDx, "dx", TRUE, OGRSTypeDouble},
                                   {OGRSTSymbolDy, "dy", TRUE, OGRSTypeDouble},
                                   {OGRSTSymbolStep, "ds", TRUE, OGRSTypeDouble},
                                   {OGRSTSymbolPerp, "dp", TRUE, OGRSTypeDouble},
                                   {OGRSTSymbolOffset, "di", TRUE, OGRSTypeDouble},
                                   {OGRSTSymbolPriority, "l", FALSE, OGRSTypeInteger},
                                   {OGRSTSymbolFontName, "f", FALSE, OGRSTypeString}};

OGRStyleParamId asStyleLabel[] = {{OGRSTLabelFontName, "f", FALSE, OGRSTypeString},
                                  {OGRSTLabelSize, "s", TRUE, OGRSTypeDouble},
                                  {OGRSTLabelTextString, "t", FALSE, OGRSTypeString},
                                  {OGRSTLabelAngle, "a", FALSE, OGRSTypeDouble},
                                  {OGRSTLabelFColor, "c", FALSE, OGRSTypeString},
                                  {OGRSTLabelBColor, "b", FALSE, OGRSTypeString},
                                  {OGRSTLabelPlacement, "m", FALSE, OGRSTypeString},
                                  {OGRSTLabelAnchor, "p", FALSE, OGRSTypeInteger},
                                  {OGRSTLabelDx, "dx", TRUE, OGRSTypeDouble},
                                  {OGRSTLabelDy, "dy", TRUE, OGRSTypeDouble},
                                  {OGRSTLabelPerp, "dp", TRUE, OGRSTypeDouble},
                                  {OGRSTLabelBold, "bo", FALSE, OGRSTypeBoolean},
                                  {OGRSTLabelItalic, "it", FALSE, OGRSTypeBoolean},
                                  {OGRSTLabelUnderline, "un", FALSE, OGRSTypeBoolean},
                                  {OGRSTLabelPriority, "l", FALSE, OGRSTypeInteger},
                                  {OGRSTLabelStrikeout, "st", FALSE, OGRSTypeBoolean},
                                  {OGRSTLabelStretch, "w", FALSE, OGRSTypeDouble},
                                  {OGRSTLabelAdjHor, "ah", FALSE, OGRSTypeString},
                                  {OGRSTLabelAdjVert, "av", FALSE, OGRSTypeString},
                                  {OGRSTLabelHColor, "h", FALSE, OGRSTypeString}};

OGRStyleParamId asStyleVector[] = {{OGRSTVectorId, "id", FALSE, OGRSTypeString},
                                   {OGRSTVectorNoCompress, "nc", FALSE, OGRSTypeInteger},
                                   {OGRSTVectorSprain, "sp", FALSE, OGRSTypeInteger},
                                   {OGRSTVectorNoSlope, "ns", FALSE, OGRSTypeInteger},
                                   {OGRSTVectorMirroring, "m", FALSE, OGRSTypeInteger},
                                   {OGRSTVectorCentering, "c", FALSE, OGRSTypeInteger},
                                   {OGRSTVectorPriority, "l", FALSE, OGRSTypeInteger}};

/* ======================================================================== */
/* OGRStyleMgr                                                              */
/* ======================================================================== */

/****************************************************************************/
/*             OGRStyleMgr::OGRStyleMgr(OGRStyleTable *poDataSetStyleTable) */
/*                                                                          */
/****************************************************************************/
OGRStyleMgr::OGRStyleMgr(OGRStyleTable *poDataSetStyleTable) {
    m_poDataSetStyleTable = poDataSetStyleTable;
    m_pszStyleString = nullptr;
}

/****************************************************************************/
/*             OGRStyleMgr::~OGRStyleMgr()                                  */
/*                                                                          */
/****************************************************************************/
OGRStyleMgr::~OGRStyleMgr() {
    if (m_pszStyleString)
        CPLFree(m_pszStyleString);
}

/****************************************************************************/
/*      GBool OGRStyleMgr::SetFeatureStyleString(OGRFeature *poFeature,     */
/*                                       char *pszStyleString,              */
/*                                       GBool bNoMatching)                 */
/*      Set the gived representation to the feature,                        */
/*      if bNoMatching == TRUE, don't try to find it in the styletable      */
/*      otherwize, we will use the name defined in the styletable           */
/****************************************************************************/
GBool OGRStyleMgr::SetFeatureStyleString(OGRFeature *poFeature, const char *pszStyleString,
                                         GBool bNoMatching) {
    const char *pszName;
    if (poFeature == FALSE)
        return FALSE;

    if (pszStyleString == nullptr)
        poFeature->SetStyleString("");
    else if (bNoMatching == TRUE)
        poFeature->SetStyleString(pszStyleString);
    else if ((pszName = GetStyleName(pszStyleString)) != nullptr)
        poFeature->SetStyleString(pszName);
    else
        poFeature->SetStyleString(pszStyleString);

    return TRUE;
}

/****************************************************************************/
/*            const char *OGRStyleMgr::InitFromFeature(OGRFeature *)        */
/*                                                                          */
/****************************************************************************/
const char *OGRStyleMgr::InitFromFeature(OGRFeature *poFeature) {
    CPLFree(m_pszStyleString);
    m_pszStyleString = nullptr;

    if (poFeature)
        InitStyleString(poFeature->GetStyleString());
    else
        m_pszStyleString = nullptr;

    return m_pszStyleString;
}
/****************************************************************************/
/*            GBool OGRStyleMgr::InitStyleString(char *pszStyleString)      */
/*                                                                          */
/****************************************************************************/
GBool OGRStyleMgr::InitStyleString(const char *pszStyleString) {
    CPLFree(m_pszStyleString);
    m_pszStyleString = nullptr;

    if (pszStyleString && pszStyleString[0] == '@')
        m_pszStyleString = CPLStrdup(GetStyleByName(pszStyleString));
    else
        m_pszStyleString = nullptr;

    if (m_pszStyleString == nullptr && pszStyleString)
        m_pszStyleString = CPLStrdup(pszStyleString);

    return TRUE;
}
/****************************************************************************/
/*      const char *OGRStyleMgr::GetStyleName(const char *pszStyleString)   */
/*                                                                          */
/****************************************************************************/
const char *OGRStyleMgr::GetStyleName(const char *pszStyleString) {
    // SECURITY:  the unit and the value for all parameter should be the same,
    // a text comparaison is executed .

    const char *pszStyle;

    if (pszStyleString)
        pszStyle = pszStyleString;
    else
        pszStyle = m_pszStyleString;

    if (pszStyle) {
        if (m_poDataSetStyleTable)
            return m_poDataSetStyleTable->GetStyleName(pszStyle);
    }
    return nullptr;
}
/****************************************************************************/
/*      const char *OGRStyleMgr::GetStyleByName(const char *pszStyleName)   */
/*                                                                          */
/****************************************************************************/
const char *OGRStyleMgr::GetStyleByName(const char *pszStyleName) {
    if (m_poDataSetStyleTable) {
        return m_poDataSetStyleTable->Find(pszStyleName);
    }
    return nullptr;
}

/****************************************************************************/
/*            GBool OGRStyleMgr::AddStyle(char *pszStyleName,               */
/*                                   char *pszStyleString)                  */
/*                                                                          */
/****************************************************************************/
GBool OGRStyleMgr::AddStyle(const char *pszStyleName, const char *pszStyleString) {
    const char *pszStyle;

    if (pszStyleString)
        pszStyle = pszStyleString;
    else
        pszStyle = m_pszStyleString;

    if (m_poDataSetStyleTable) {
        return m_poDataSetStyleTable->AddStyle(pszStyleName, pszStyle);
    }
    return FALSE;
}
/****************************************************************************/
/*            const char *OGRStyleMgr::GetStyleString(OGRFeature *)         */
/*                                                                          */
/****************************************************************************/
const char *OGRStyleMgr::GetStyleString(OGRFeature *poFeature) {
    if (poFeature == nullptr)
        return m_pszStyleString;

    return InitFromFeature(poFeature);
}

GBool OGRStyleMgr::AddPart(const char *pszPart) {
    char *pszTmp;
    if (pszPart) {
        if (m_pszStyleString) {
            pszTmp = CPLStrdup(CPLSPrintf("%s;%s", m_pszStyleString, pszPart));
            CPLFree(m_pszStyleString);
            m_pszStyleString = pszTmp;
        } else {
            pszTmp = CPLStrdup(CPLSPrintf("%s", pszPart));
            CPLFree(m_pszStyleString);
            m_pszStyleString = pszTmp;
        }
        return TRUE;
    }

    return FALSE;
}

/****************************************************************************/
/*            GBool OGRStyleMgr::AddPart(OGRStyleTool *)                    */
/*            Add a new part in the current style                           */
/****************************************************************************/
GBool OGRStyleMgr::AddPart(OGRStyleTool *poStyleTool) {
    char *pszTmp;
    if (poStyleTool) {
        if (m_pszStyleString) {
            pszTmp =
                CPLStrdup(CPLSPrintf("%s;%s", m_pszStyleString, poStyleTool->GetStyleString()));
            CPLFree(m_pszStyleString);
            m_pszStyleString = pszTmp;
        } else {
            pszTmp = CPLStrdup(CPLSPrintf("%s", poStyleTool->GetStyleString()));
            CPLFree(m_pszStyleString);
            m_pszStyleString = pszTmp;
        }
        return TRUE;
    }

    return FALSE;
}

/****************************************************************************/
/*            int OGRStyleMgr::GetPartCount(const char *pszStyleString)     */
/*            return the number of part in the stylestring                  */
/****************************************************************************/
int OGRStyleMgr::GetPartCount(const char *pszStyleString) {
    const char *pszPart;
    int nPartCount = 1;
    const char *pszString;
    const char *pszStrTmp;

    if (pszStyleString != nullptr)
        pszString = pszStyleString;
    else
        pszString = m_pszStyleString;

    if (pszString == nullptr)
        return 0;

    pszStrTmp = pszString;
    while ((pszPart = strstr(pszStrTmp, ";")) != nullptr) {
        pszStrTmp = &pszPart[1];
        nPartCount++;
    }
    return nPartCount;
}

/****************************************************************************/
/*            OGRStyleTool *OGRStyleMgr::GetPart(int hPartId,               */
/*                                 const char *pszStyleString)              */
/*                                                                          */
/*     Return a StyleTool of the type of the wanted part, could return NULL */
/****************************************************************************/
OGRStyleTool *OGRStyleMgr::GetPart(int hPartId, const char *pszStyleString) {
    char **papszStyleString;
    const char *pszStyle;
    const char *pszString;
    OGRStyleTool *poStyleTool;

    if (pszStyleString)
        pszStyle = pszStyleString;
    else
        pszStyle = m_pszStyleString;

    if (pszStyle == nullptr)
        return nullptr;

    papszStyleString = CSLTokenizeString2(
        pszStyle, ";", CSLT_HONOURSTRINGS | CSLT_PRESERVEQUOTES | CSLT_PRESERVEESCAPES);

    pszString = CSLGetField(papszStyleString, hPartId);

    if (pszString || strlen(pszString) > 0) {
        poStyleTool = CreateStyleToolFromStyleString(pszString);
        if (poStyleTool)
            poStyleTool->SetStyleString(pszString);
        CSLDestroy(papszStyleString);
        return poStyleTool;
    }
    CSLDestroy(papszStyleString);
    return nullptr;
}

/****************************************************************************/
/* OGRStyleTool *CreateStyleToolFromStyleString(const char *pszStyleString) */
/*                                                                          */
/* create a Style tool from the gived StyleString, it should contain only a */
/* part of a StyleString                                                    */
/****************************************************************************/
OGRStyleTool *OGRStyleMgr::CreateStyleToolFromStyleString(const char *pszStyleString) {
    char **papszToken = CSLTokenizeString2(
        pszStyleString, "();", CSLT_HONOURSTRINGS | CSLT_PRESERVEQUOTES | CSLT_PRESERVEESCAPES);
    OGRStyleTool *poStyleTool;

    if (CSLCount(papszToken) < 2)
        poStyleTool = nullptr;
    else if (EQUAL(papszToken[0], "PEN"))
        poStyleTool = new OGRStylePen();
    else if (EQUAL(papszToken[0], "BRUSH"))
        poStyleTool = new OGRStyleBrush();
    else if (EQUAL(papszToken[0], "SYMBOL"))
        poStyleTool = new OGRStyleSymbol();
    else if (EQUAL(papszToken[0], "LABEL"))
        poStyleTool = new OGRStyleLabel();
    else if (EQUAL(papszToken[0], "VECTOR"))
        poStyleTool = new OGRStyleVector();
    else
        poStyleTool = nullptr;

    CSLDestroy(papszToken);

    return poStyleTool;
}

/* ======================================================================== */
/*                OGRStyleTable                                             */
/*     Object Used to manage and store a styletable                         */
/* ======================================================================== */

/****************************************************************************/
/*              OGRStyleTable::OGRStyleTable()                              */
/*                                                                          */
/****************************************************************************/
OGRStyleTable::OGRStyleTable() { m_papszStyleTable = nullptr; }

/****************************************************************************/
/*          OGRStyleTable::~OGRStyleTable()                                 */
/*                                                                          */
/****************************************************************************/
OGRStyleTable::~OGRStyleTable() { Clear(); }

/****************************************************************************/
/*                void OGRStyleTable::Clear()                               */
/*                                                                          */
/****************************************************************************/
void OGRStyleTable::Clear() {
    if (m_papszStyleTable)
        CSLDestroy(m_papszStyleTable);
    m_papszStyleTable = nullptr;
}

/****************************************************************************/
/*    const char *OGRStyleTable::GetStyleName(const char *pszStyleString)   */
/*                                                                          */
/*    return the Name of a gived stylestring otherwise NULL                 */
/****************************************************************************/
const char *OGRStyleTable::GetStyleName(const char *pszStyleString) {
    int i;
    const char *pszStyleStringBegin;
    static char *pszName = nullptr;
    char *pszTmp;

    if (pszName)
        CPLFree(pszName);

    pszName = nullptr;

    for (i = 0; i < CSLCount(m_papszStyleTable); i++) {
        pszStyleStringBegin = strstr(m_papszStyleTable[i], ":");

        if (pszStyleStringBegin && EQUAL(&pszStyleStringBegin[1], pszStyleString)) {
            pszName = CPLStrdup(m_papszStyleTable[i]);
            pszTmp = strstr(pszName, ":");
            if (pszTmp)
                pszTmp[0] = '\0';
            break;
        }
    }

    return pszName;
}

/****************************************************************************/
/*            GBool OGRStyleTable::AddStyle(char *pszName,                  */
/*                                          char *pszStyleString)           */
/*                                                                          */
/*   Add a new style in the table, no comparaison will be done on the       */
/*   Style string, only on the name, TRUE success, FALSE error              */
/****************************************************************************/
GBool OGRStyleTable::AddStyle(const char *pszName, const char *pszStyleString) {
    int nPos;
    const char *pszNewString = nullptr;

    if (pszName && pszStyleString) {
        if ((nPos = IsExist(pszName)) != -1)
            return FALSE;

        pszNewString = CPLSPrintf("%s:%s", pszName, pszStyleString);

        m_papszStyleTable = CSLAddString(m_papszStyleTable, pszNewString);
        return TRUE;
    }
    return FALSE;
}

/****************************************************************************/
/*            GBool OGRStyleTable::RemoveStyle(char *pszName)               */
/*                                                                          */
/*    Remove the gived style in the table based on the name, return TRUE    */
/*    on success otherwise FALSE                                            */
/****************************************************************************/
GBool OGRStyleTable::RemoveStyle(const char *pszName) {
    int nPos;
    if ((nPos = IsExist(pszName)) != -1) {
        m_papszStyleTable = CSLRemoveStrings(m_papszStyleTable, nPos, 1, nullptr);
        return TRUE;
    }
    return FALSE;
}

/****************************************************************************/
/*            GBool OGRStyleTable::ModifyStyle(char *pszName,               */
/*                                             char *pszStyleString)        */
/*                                                                          */
/*    Modify the gived style, if the style doesn't exist, it will be added  */
/*    return TRUE on success otherwise return FALSE                         */
/****************************************************************************/
GBool OGRStyleTable::ModifyStyle(const char *pszName, const char *pszStyleString) {
    if (pszName == nullptr || pszStyleString == nullptr)
        return FALSE;

    RemoveStyle(pszName);
    return AddStyle(pszName, pszStyleString);
}

/****************************************************************************/
/*            GBool OGRStyleTable::SaveStyleTable(char *)                   */
/*                                                                          */
/*    Save the StyleTable in the gived file, return TRUE on success         */
/*    otherwise return FALSE                                                */
/****************************************************************************/
GBool OGRStyleTable::SaveStyleTable(const char *pszFilename) {
    if (pszFilename == nullptr)
        return FALSE;

    if (CSLSave(m_papszStyleTable, pszFilename) == 0)
        return FALSE;

    return TRUE;
}

/****************************************************************************/
/*            GBool OGRStyleTable::LoadStyleTable(char *)                   */
/*                                                                          */
/*            Read the Style table from a file, return TRUE on success      */
/*            otherwise return FALSE                                        */
/****************************************************************************/
GBool OGRStyleTable::LoadStyleTable(const char *pszFilename) {
    if (pszFilename == nullptr)
        return FALSE;

    CSLDestroy(m_papszStyleTable);

    m_papszStyleTable = CSLLoad(pszFilename);

    if (m_papszStyleTable == nullptr)
        return FALSE;

    return TRUE;
}

/****************************************************************************/
/*             const char *OGRStyleTable::Find(const char *pszName)         */
/*                                                                          */
/*             return the StyleString based on the gived name,              */
/*             otherwise return NULL                                        */
/****************************************************************************/
const char *OGRStyleTable::Find(const char *pszName) {
    const char *pszDash = nullptr;
    const char *pszOutput = nullptr;

    int nPos;
    if ((nPos = IsExist(pszName)) != -1) {
        pszOutput = CSLGetField(m_papszStyleTable, nPos);

        pszDash = strstr(pszOutput, ":");

        if (pszDash)
            return &pszDash[1];
    }
    return nullptr;
}

/****************************************************************************/
/*              OGRStyleTable::Print(FILE *fpOut)                           */
/*                                                                          */
/****************************************************************************/
void OGRStyleTable::Print(FILE *fpOut) {
    VSIFPrintf(fpOut, "#OFS-Version: 1.0\n");
    VSIFPrintf(fpOut, "#StyleField: style\n");
    if (m_papszStyleTable) {
        CSLPrint(m_papszStyleTable, fpOut);
    }
}

/****************************************************************************/
/*             GBool OGRStyleTable::IsExist(const char *pszName)            */
/*                                                                          */
/*   return a index of the style in the table otherwise return -1           */
/****************************************************************************/
int OGRStyleTable::IsExist(const char *pszName) {
    int i;
    const char *pszNewString;

    if (pszName == nullptr)
        return -1;

    pszNewString = CPLSPrintf("%s:", pszName);

    for (i = 0; i < CSLCount(m_papszStyleTable); i++) {
        if (strstr(m_papszStyleTable[i], pszNewString) != nullptr) {
            return i;
        }
    }

    return -1;
}

/************************************************************************/
/*                               Clone()                                */
/************************************************************************/

/**
 * Duplicate style table.
 *
 * The newly created style table is owned by the caller, and will have it's
 * own reference to the OGRStyleTable.
 *
 * This method is the same as the C function OGR_F_Clone().
 *
 * @return new feature, exactly matching this feature.
 */

OGRStyleTable *OGRStyleTable::Clone()

{
    auto *poNew = new OGRStyleTable();

    poNew->m_papszStyleTable = CSLDuplicate(m_papszStyleTable);

    return poNew;
}

/****************************************************************************/
/*                          OGRStyleTool::OGRStyleTool()                    */
/*                                                                          */
/****************************************************************************/
OGRStyleTool::OGRStyleTool(OGRSTClassId eClassId) {
    m_eClassId = eClassId;
    m_dfScale = 1.0;
    m_eUnit = OGRSTUMM;
    m_pszStyleString = nullptr;
    m_bModified = FALSE;
    m_bParsed = FALSE;
}

/****************************************************************************/
/*                       OGRStyleTool::~OGRStyleTool()                      */
/*                                                                          */
/****************************************************************************/
OGRStyleTool::~OGRStyleTool() { CPLFree(m_pszStyleString); }

/****************************************************************************/
/*      void OGRStyleTool::SetStyleString(const char *pszStyleString)       */
/*                                                                          */
/****************************************************************************/
void OGRStyleTool::SetStyleString(const char *pszStyleString) {
    m_pszStyleString = CPLStrdup(pszStyleString);
}

/****************************************************************************/
/*const char *OGRStyleTool::GetStyleString( OGRStyleParamId *pasStyleParam ,*/
/*                          OGRStyleValue *pasStyleValue, int nSize)        */
/*                                                                          */
/****************************************************************************/
const char *OGRStyleTool::GetStyleString(OGRStyleParamId *pasStyleParam,
                                         OGRStyleValue *pasStyleValue, int nSize) {
    if (IsStyleModified()) {
        int i;
        GBool bFound;
        const char *pszClass;
        // FIXME: we should use CPLString instead of static buffer:
        char szCurrent[8192];
        szCurrent[0] = '\0';

        CPLFree(m_pszStyleString);

        switch (GetType()) {
            case OGRSTCPen:
                pszClass = "PEN(";
                break;
            case OGRSTCBrush:
                pszClass = "BRUSH(";
                break;
            case OGRSTCSymbol:
                pszClass = "SYMBOL(";
                break;
            case OGRSTCLabel:
                pszClass = "LABEL(";
                break;
            case OGRSTCVector:
                pszClass = "VECTOR(";
                break;
            default:
                pszClass = "UNKNOWN(";
        }

        strcat(szCurrent, pszClass);

        bFound = FALSE;
        for (i = 0; i < nSize; i++) {
            if (pasStyleValue[i].bValid == FALSE)
                continue;

            if (bFound)
                strcat(szCurrent, ",");
            bFound = TRUE;

            strcat(szCurrent, pasStyleParam[i].pszToken);
            switch (pasStyleParam[i].eType) {
                case OGRSTypeString:
                    strcat(szCurrent, ":");
                    strcat(szCurrent, pasStyleValue[i].pszValue);
                    break;
                case OGRSTypeDouble:
                    strcat(szCurrent, CPLSPrintf(":%f", pasStyleValue[i].dfValue));
                    break;
                case OGRSTypeInteger:
                    strcat(szCurrent, CPLSPrintf(":%d", pasStyleValue[i].nValue));
                    break;
                default:
                    break;
            }
            if (pasStyleParam[i].bGeoref)
                switch (pasStyleValue[i].eUnit) {
                    case OGRSTUGround:
                        strcat(szCurrent, "g");
                        break;
                    case OGRSTUPixel:
                        strcat(szCurrent, "px");
                        break;
                    case OGRSTUPoints:
                        strcat(szCurrent, "pt");
                        break;
                    case OGRSTUCM:
                        strcat(szCurrent, "cm");
                        break;
                    case OGRSTUInches:
                        strcat(szCurrent, "in");
                        break;
                    case OGRSTUMM:
                        // strcat(szCurrent,"mm");
                    default:
                        break;  // imp
                }
        }
        strcat(szCurrent, ")");

        m_pszStyleString = CPLStrdup(szCurrent);

        m_bModified = FALSE;
    }

    return m_pszStyleString;
}

/************************************************************************/
/*                          GetRGBFromString()                          */
/************************************************************************/

GBool OGRStyleTool::GetRGBFromString(const char *pszColor, int &nRed, int &nGreen, int &nBlue,
                                     int &nTransparance) {
    int nCount = 0;

    nTransparance = 255;

    // FIXME: should we really use sscanf here?
    if (pszColor)
        nCount = sscanf(pszColor, "#%2x%2x%2x%2x", &nRed, &nGreen, &nBlue, &nTransparance);

    if (nCount >= 3)
        return TRUE;

    return FALSE;
}

/************************************************************************/
/*                           GetSpecificId()                            */
/*                                                                      */
/*      return -1, if the wanted type is not found, ex:                 */
/*      if you want ogr-pen value, pszWanted should be ogr-pen(case     */
/*      sensitive)                                                      */
/************************************************************************/

int OGRStyleTool::GetSpecificId(const char *pszId, const char *pszWanted) {
    const char *pszRealWanted = pszWanted;
    const char *pszFound;
    int nValue = -1;

    if (pszWanted == nullptr || strlen(pszWanted) == 0)
        pszRealWanted = "ogr-pen";

    if (pszId == nullptr)
        return -1;

    if ((pszFound = strstr(pszId, pszRealWanted)) != nullptr) {
        // We found the string, it could be no value after it, use default one
        nValue = 0;

        if (pszFound[strlen(pszRealWanted)] == '-')
            nValue = atoi(&pszFound[strlen(pszRealWanted) + 1]);
    }

    return nValue;
}

/************************************************************************/
/*                              GetType()                               */
/************************************************************************/
OGRSTClassId OGRStyleTool::GetType() { return m_eClassId; }

/************************************************************************/
/*                              SetUnit()                               */
/************************************************************************/
void OGRStyleTool::SetUnit(OGRSTUnitId eUnit, double dfScale) {
    m_dfScale = dfScale;
    m_eUnit = eUnit;
}

/************************************************************************/
/*                               Parse()                                */
/************************************************************************/
GBool OGRStyleTool::Parse(OGRStyleParamId *pasStyle, OGRStyleValue *pasValue, int nCount) {
    char **papszToken;   // Token to contains StyleString Type and content
    char **papszToken2;  // Token that will contains StyleString elements

    OGRSTUnitId eLastUnit;

    if (IsStyleParsed() == TRUE)
        return TRUE;

    StyleParsed();

    if (m_pszStyleString == nullptr)
        return FALSE;

    // Tokenize the String to get the Type and the content
    // Example: Type(elem1:val2,elem2:val2)
    papszToken = CSLTokenizeString2(
        m_pszStyleString, "()", CSLT_HONOURSTRINGS | CSLT_PRESERVEQUOTES | CSLT_PRESERVEESCAPES);

    if (CSLCount(papszToken) > 2 || CSLCount(papszToken) == 0) {
        CSLDestroy(papszToken);
        CPLError(CE_Failure, CPLE_AppDefined, "Error in the format of the StyleTool %s\n",
                 m_pszStyleString);
        return FALSE;
    }

    // Tokenize the content of the StyleString to get paired components in it.
    papszToken2 = CSLTokenizeString2(
        papszToken[1], ",", CSLT_HONOURSTRINGS | CSLT_PRESERVEQUOTES | CSLT_PRESERVEESCAPES);

    // Valid that we have the right StyleString for this feature type.
    switch (GetType()) {
        case OGRSTCPen:
            if (!EQUAL(papszToken[0], "PEN")) {
                CPLError(CE_Failure, CPLE_AppDefined,
                         "Error in the Type of StyleTool %s should be a PEN Type\n", papszToken[0]);
                CSLDestroy(papszToken);
                CSLDestroy(papszToken2);
                return FALSE;
            }
            break;
        case OGRSTCBrush:
            if (!EQUAL(papszToken[0], "BRUSH")) {
                CPLError(CE_Failure, CPLE_AppDefined,
                         "Error in the Type of StyleTool %s should be a BRUSH Type\n",
                         papszToken[0]);
                CSLDestroy(papszToken);
                CSLDestroy(papszToken2);
                return FALSE;
            }
            break;
        case OGRSTCSymbol:
            if (!EQUAL(papszToken[0], "SYMBOL")) {
                CPLError(CE_Failure, CPLE_AppDefined,
                         "Error in the Type of StyleTool %s should be a SYMBOL Type\n",
                         papszToken[0]);
                CSLDestroy(papszToken);
                CSLDestroy(papszToken2);
                return FALSE;
            }
            break;
        case OGRSTCLabel:
            if (!EQUAL(papszToken[0], "LABEL")) {
                CPLError(CE_Failure, CPLE_AppDefined,
                         "Error in the Type of StyleTool %s should be a LABEL Type\n",
                         papszToken[0]);
                CSLDestroy(papszToken);
                CSLDestroy(papszToken2);
                return FALSE;
            }
            break;
        case OGRSTCVector:
            if (!EQUAL(papszToken[0], "VECTOR")) {
                CPLError(CE_Failure, CPLE_AppDefined,
                         "Error in the Type of StyleTool %s should be a VECTOR Type\n",
                         papszToken[0]);
                CSLDestroy(papszToken);
                CSLDestroy(papszToken2);
                return FALSE;
            }
            break;
        default:
            CPLError(CE_Failure, CPLE_AppDefined,
                     "Error in the Type of StyleTool, Type undetermined\n");
            CSLDestroy(papszToken);
            CSLDestroy(papszToken2);
            return FALSE;
            break;
    }

    ////////////////////////////////////////////////////////////////////////
    // Here we will loop on each element in the StyleString. If it's
    // a valid element, we will add it in the StyleTool with
    // SetParamStr().
    //
    // It's important to note that the SetInternalUnit...() is use to update
    // the unit of the StyleTool param (m_eUnit).
    // See OGRStyleTool::SetParamStr().
    // There's a StyleTool unit (m_eUnit), which is the output unit, and each
    // parameter of the style have its own unit value (the input unit). Here we
    // set m_eUnit to the input unit and in SetParamStr(), we will use this
    // value to set the input unit. Then after the loop we will reset m_eUnit
    // to it's original value. (Yes it's a side effect / black magic)
    //
    // The pasStyle variable is a global variable passed in argument to the
    // function. See at the top of this file the four OGRStyleParamId
    // variable. They are used to register the valid parameter of each
    // StyleTool.
    ////////////////////////////////////////////////////////////////////////

    // Save Scale and output Units because the parsing code will alter
    // the values
    eLastUnit = m_eUnit;
    double dSavedScale = m_dfScale;
    int i, nElements = CSLCount(papszToken2);

    for (i = 0; i < nElements; i++) {
        char **papszStylePair = CSLTokenizeString2(papszToken2[i], ":", CSLT_HONOURSTRINGS);
        int j, nTokens = CSLCount(papszStylePair);

        if (nTokens < 1 || nTokens > 2) {
            CPLError(CE_Warning, CPLE_AppDefined, "Error in the StyleTool String %s",
                     m_pszStyleString);
            CPLError(CE_Warning, CPLE_AppDefined, "Malformed element #%d (\"%s\") skipped", i,
                     papszToken2[i]);
            CSLDestroy(papszStylePair);
            continue;
        }

        for (j = 0; j < nCount; j++) {
            if (EQUAL(pasStyle[j].pszToken, papszStylePair[0])) {
                if (nTokens == 2 && pasStyle[j].bGeoref == TRUE)
                    SetInternalInputUnitFromParam(papszStylePair[1]);

                // Set either the actual value of style parameter or "1"
                // for boolean parameters which do not have values.
                // "1" means that boolean parameter is present in the style
                // string.
                OGRStyleTool::SetParamStr(pasStyle[j], pasValue[j],
                                          (nTokens == 2) ? papszStylePair[1] : "1");

                break;
            }
        }

        CSLDestroy(papszStylePair);
    }

    m_eUnit = eLastUnit;
    m_dfScale = dSavedScale;

    CSLDestroy(papszToken2);
    CSLDestroy(papszToken);

    return TRUE;
}

/************************************************************************/
/*                   SetInternalInputUnitFromParam()                    */
/************************************************************************/

void OGRStyleTool::SetInternalInputUnitFromParam(char *pszString) {
    char *pszUnit;

    if (pszString == nullptr)
        return;
    pszUnit = strstr(pszString, "g");
    if (pszUnit) {
        SetUnit(OGRSTUGround);
        pszUnit[0] = '\0';
        return;
    }
    pszUnit = strstr(pszString, "px");
    if (pszUnit) {
        SetUnit(OGRSTUPixel);
        pszUnit[0] = '\0';
        return;
    }
    pszUnit = strstr(pszString, "pt");
    if (pszUnit) {
        SetUnit(OGRSTUPoints);
        pszUnit[0] = '\0';
        return;
    }
    pszUnit = strstr(pszString, "mm");
    if (pszUnit) {
        SetUnit(OGRSTUMM);
        pszUnit[0] = '\0';
        return;
    }
    pszUnit = strstr(pszString, "cm");
    if (pszUnit) {
        SetUnit(OGRSTUCM);
        pszUnit[0] = '\0';
        return;
    }
    pszUnit = strstr(pszString, "in");
    if (pszUnit) {
        SetUnit(OGRSTUInches);
        pszUnit[0] = '\0';
        return;
    }

    SetUnit(OGRSTUMM);
}

/************************************************************************/
/*                          ComputeWithUnit()                           */
/************************************************************************/
double OGRStyleTool::ComputeWithUnit(double dfValue, OGRSTUnitId eInputUnit) {
    OGRSTUnitId eOutputUnit = GetUnit();

    double dfNewValue = dfValue;  // dfValue in  Meter;

    if (eOutputUnit == eInputUnit)
        return dfValue;

    switch (eInputUnit) {
        case OGRSTUGround:
            dfNewValue = dfValue / m_dfScale;
            break;
        case OGRSTUPixel:
            dfNewValue = dfValue / (72.0 * 39.37);
            break;
        case OGRSTUPoints:
            dfNewValue = dfValue / (72.0 * 39.37);
            break;
        case OGRSTUMM:
            dfNewValue = 0.001 * dfValue;
            break;
        case OGRSTUCM:
            dfNewValue = 0.01 * dfValue;
            break;
        case OGRSTUInches:
            dfNewValue = dfValue / 39.37;
            break;
        default:
            break;  // imp
    }

    switch (eOutputUnit) {
        case OGRSTUGround:
            dfNewValue *= m_dfScale;
            break;
        case OGRSTUPixel:
            dfNewValue *= (72.0 * 39.37);
            break;
        case OGRSTUPoints:
            dfNewValue *= (72.0 * 39.37);
            break;
        case OGRSTUMM:
            dfNewValue *= 1000.0;
            break;
        case OGRSTUCM:
            dfNewValue *= 100.0;
            break;
        case OGRSTUInches:
            dfNewValue *= 39.37;
            break;
        default:
            break;  // imp
    }
    return dfNewValue;
}

/************************************************************************/
/*                          ComputeWithUnit()                           */
/************************************************************************/
int OGRStyleTool::ComputeWithUnit(int nValue, OGRSTUnitId eUnit) {
    return (int)ComputeWithUnit((double)nValue, eUnit);
}

/************************************************************************/
/*                            GetParamStr()                             */
/************************************************************************/
const char *OGRStyleTool::GetParamStr(OGRStyleParamId &sStyleParam, OGRStyleValue &sStyleValue,
                                      GBool &bValueIsNull) {
    if (!Parse()) {
        bValueIsNull = TRUE;
        return nullptr;
    }

    bValueIsNull = !sStyleValue.bValid;

    if (bValueIsNull == TRUE)
        return nullptr;

    switch (sStyleParam.eType) {
            // if sStyleParam.bGeoref == TRUE , need to convert to output value;
        case OGRSTypeString:
            return sStyleValue.pszValue;
        case OGRSTypeDouble:
            if (sStyleParam.bGeoref)
                return CPLSPrintf("%f", ComputeWithUnit(sStyleValue.dfValue, sStyleValue.eUnit));
            else
                return CPLSPrintf("%f", sStyleValue.dfValue);

        case OGRSTypeInteger:
            if (sStyleParam.bGeoref)
                return CPLSPrintf("%d", ComputeWithUnit(sStyleValue.nValue, sStyleValue.eUnit));
            else
                return CPLSPrintf("%d", sStyleValue.nValue);
        case OGRSTypeBoolean:
            return CPLSPrintf("%d", sStyleValue.nValue);
        default:
            bValueIsNull = TRUE;
            return nullptr;
    }
}

/****************************************************************************/
/*    int OGRStyleTool::GetParamNum(OGRStyleParamId sStyleParam ,           */
/*                               OGRStyleValue sStyleValue,                 */
/*                               GBool &bValueIsNull)                       */
/*                                                                          */
/****************************************************************************/
int OGRStyleTool::GetParamNum(OGRStyleParamId &sStyleParam, OGRStyleValue &sStyleValue,
                              GBool &bValueIsNull) {
    return (int)GetParamDbl(sStyleParam, sStyleValue, bValueIsNull);
}

/****************************************************************************/
/*       double OGRStyleTool::GetParamDbl(OGRStyleParamId sStyleParam ,     */
/*                               OGRStyleValue sStyleValue,                 */
/*                               GBool &bValueIsNull)                       */
/*                                                                          */
/****************************************************************************/
double OGRStyleTool::GetParamDbl(OGRStyleParamId &sStyleParam, OGRStyleValue &sStyleValue,
                                 GBool &bValueIsNull) {
    if (!Parse()) {
        bValueIsNull = TRUE;
        return 0;
    }

    bValueIsNull = !sStyleValue.bValid;

    if (bValueIsNull == TRUE)
        return 0;

    switch (sStyleParam.eType) {
            // if sStyleParam.bGeoref == TRUE , need to convert to output value;
        case OGRSTypeString:
            if (sStyleParam.bGeoref)
                return ComputeWithUnit(atof(sStyleValue.pszValue), sStyleValue.eUnit);
            else
                return atof(sStyleValue.pszValue);
        case OGRSTypeDouble:
            if (sStyleParam.bGeoref)
                return ComputeWithUnit(sStyleValue.dfValue, sStyleValue.eUnit);
            else
                return sStyleValue.dfValue;
        case OGRSTypeInteger:
            if (sStyleParam.bGeoref)
                return (double)ComputeWithUnit(sStyleValue.nValue, sStyleValue.eUnit);
            else
                return (double)sStyleValue.nValue;
        case OGRSTypeBoolean:
            return (double)sStyleValue.nValue;
        default:
            bValueIsNull = TRUE;
            return 0;
    }
}

/****************************************************************************/
/*      void OGRStyleTool::SetParamStr(OGRStyleParamId &sStyleParam ,       */
/*                             OGRStyleValue &sStyleValue,                  */
/*                             const char *pszParamString)                  */
/*                                                                          */
/****************************************************************************/
void OGRStyleTool::SetParamStr(OGRStyleParamId &sStyleParam, OGRStyleValue &sStyleValue,
                               const char *pszParamString) {
    Parse();
    StyleModified();
    sStyleValue.bValid = TRUE;
    sStyleValue.eUnit = GetUnit();
    switch (sStyleParam.eType) {
            // if sStyleParam.bGeoref == TRUE , need to convert to output value;
        case OGRSTypeString:
            sStyleValue.pszValue = CPLStrdup(pszParamString);
            break;
        case OGRSTypeDouble:
            sStyleValue.dfValue = atof(pszParamString);
            break;
        case OGRSTypeInteger:
        case OGRSTypeBoolean:
            sStyleValue.nValue = atoi(pszParamString);
            break;
        default:
            sStyleValue.bValid = FALSE;
            break;
    }
}

/****************************************************************************/
/*    void OGRStyleTool::SetParamNum(OGRStyleParamId &sStyleParam ,         */
/*                             OGRStyleValue &sStyleValue,                  */
/*                             int nParam)                                  */
/*                                                                          */
/****************************************************************************/
void OGRStyleTool::SetParamNum(OGRStyleParamId &sStyleParam, OGRStyleValue &sStyleValue,
                               int nParam) {
    Parse();
    StyleModified();
    sStyleValue.bValid = TRUE;
    sStyleValue.eUnit = GetUnit();
    switch (sStyleParam.eType) {
            // if sStyleParam.bGeoref == TRUE , need to convert to output value;
        case OGRSTypeString:
            sStyleValue.pszValue = CPLStrdup(CPLSPrintf("%d", nParam));
            break;
        case OGRSTypeDouble:
            sStyleValue.dfValue = (double)nParam;
            break;
        case OGRSTypeInteger:
        case OGRSTypeBoolean:
            sStyleValue.nValue = nParam;
            break;
        default:
            sStyleValue.bValid = FALSE;
            break;
    }
}

/****************************************************************************/
/*      void OGRStyleTool::SetParamDbl(OGRStyleParamId &sStyleParam ,       */
/*                             OGRStyleValue &sStyleValue,                  */
/*                             double dfParam)                              */
/*                                                                          */
/****************************************************************************/
void OGRStyleTool::SetParamDbl(OGRStyleParamId &sStyleParam, OGRStyleValue &sStyleValue,
                               double dfParam) {
    Parse();
    StyleModified();
    sStyleValue.bValid = TRUE;
    sStyleValue.eUnit = GetUnit();
    switch (sStyleParam.eType) {
            // if sStyleParam.bGeoref == TRUE , need to convert to output value;
        case OGRSTypeString:
            sStyleValue.pszValue = CPLStrdup(CPLSPrintf("%f", dfParam));
            break;
        case OGRSTypeDouble:
            sStyleValue.dfValue = dfParam;
            break;
        case OGRSTypeInteger:
        case OGRSTypeBoolean:
            sStyleValue.nValue = (int)dfParam;
            break;
        default:
            sStyleValue.bValid = FALSE;
            break;
    }
}

/* ======================================================================== */
/*                OGRStylePen                                               */
/*       Specific parameter (Set/Get) for the StylePen                      */
/* ======================================================================== */

/****************************************************************************/
/*                      OGRStylePen::OGRStylePen()                          */
/*                                                                          */
/****************************************************************************/
OGRStylePen::OGRStylePen() : OGRStyleTool(OGRSTCPen) {
    m_pasStyleValue = (OGRStyleValue *)CPLCalloc(OGRSTPenLast, sizeof(OGRStyleValue));
}

/****************************************************************************/
/*                      OGRStylePen::~OGRStylePen()                         */
/*                                                                          */
/****************************************************************************/
OGRStylePen::~OGRStylePen() {
    for (int i = 0; i < OGRSTPenLast; i++) {
        if (m_pasStyleValue[i].pszValue != nullptr) {
            CPLFree(m_pasStyleValue[i].pszValue);
            m_pasStyleValue[i].pszValue = nullptr;
        }
    }

    CPLFree(m_pasStyleValue);
}

/************************************************************************/
/*                         OGRStylePen::Parse()                         */
/************************************************************************/
GBool OGRStylePen::Parse()

{
    return OGRStyleTool::Parse(asStylePen, m_pasStyleValue, (int)OGRSTPenLast);
}

/************************************************************************/
/*                            GetParamStr()                             */
/************************************************************************/
const char *OGRStylePen::GetParamStr(OGRSTPenParam eParam, GBool &bValueIsNull) {
    return OGRStyleTool::GetParamStr(asStylePen[eParam], m_pasStyleValue[eParam], bValueIsNull);
}

/************************************************************************/
/*                            GetParamNum()                             */
/************************************************************************/
int OGRStylePen::GetParamNum(OGRSTPenParam eParam, GBool &bValueIsNull) {
    return OGRStyleTool::GetParamNum(asStylePen[eParam], m_pasStyleValue[eParam], bValueIsNull);
}

/************************************************************************/
/*                            GetParamDbl()                             */
/************************************************************************/
double OGRStylePen::GetParamDbl(OGRSTPenParam eParam, GBool &bValueIsNull) {
    return OGRStyleTool::GetParamDbl(asStylePen[eParam], m_pasStyleValue[eParam], bValueIsNull);
}

/************************************************************************/
/*                            SetParamStr()                             */
/************************************************************************/

void OGRStylePen::SetParamStr(OGRSTPenParam eParam, const char *pszParamString) {
    OGRStyleTool::SetParamStr(asStylePen[eParam], m_pasStyleValue[eParam], pszParamString);
}

/************************************************************************/
/*                            SetParamNum()                             */
/************************************************************************/
void OGRStylePen::SetParamNum(OGRSTPenParam eParam, int nParam) {
    OGRStyleTool::SetParamNum(asStylePen[eParam], m_pasStyleValue[eParam], nParam);
}

/************************************************************************/
/*                            SetParamDbl()                             */
/************************************************************************/
void OGRStylePen::SetParamDbl(OGRSTPenParam eParam, double dfParam) {
    OGRStyleTool::SetParamDbl(asStylePen[eParam], m_pasStyleValue[eParam], dfParam);
}

/************************************************************************/
/*                           GetStyleString()                           */
/************************************************************************/
const char *OGRStylePen::GetStyleString() {
    return OGRStyleTool::GetStyleString(asStylePen, m_pasStyleValue, (int)OGRSTPenLast);
}

/****************************************************************************/
/*                      OGRStyleBrush::OGRStyleBrush()                      */
/*                                                                          */
/****************************************************************************/
OGRStyleBrush::OGRStyleBrush() : OGRStyleTool(OGRSTCBrush) {
    m_pasStyleValue = (OGRStyleValue *)CPLCalloc(OGRSTBrushLast, sizeof(OGRStyleValue));
}

/****************************************************************************/
/*                      OGRStyleBrush::~OGRStyleBrush()                     */
/*                                                                          */
/****************************************************************************/
OGRStyleBrush::~OGRStyleBrush() {
    for (int i = 0; i < OGRSTBrushLast; i++) {
        if (m_pasStyleValue[i].pszValue != nullptr) {
            CPLFree(m_pasStyleValue[i].pszValue);
            m_pasStyleValue[i].pszValue = nullptr;
        }
    }

    CPLFree(m_pasStyleValue);
}

/************************************************************************/
/*                               Parse()                                */
/************************************************************************/
GBool OGRStyleBrush::Parse() {
    return OGRStyleTool::Parse(asStyleBrush, m_pasStyleValue, (int)OGRSTBrushLast);
}

/************************************************************************/
/*                            GetParamStr()                             */
/************************************************************************/
const char *OGRStyleBrush::GetParamStr(OGRSTBrushParam eParam, GBool &bValueIsNull) {
    return OGRStyleTool::GetParamStr(asStyleBrush[eParam], m_pasStyleValue[eParam], bValueIsNull);
}

/************************************************************************/
/*                            GetParamNum()                             */
/************************************************************************/
int OGRStyleBrush::GetParamNum(OGRSTBrushParam eParam, GBool &bValueIsNull) {
    return OGRStyleTool::GetParamNum(asStyleBrush[eParam], m_pasStyleValue[eParam], bValueIsNull);
}

/************************************************************************/
/*                            GetParamDbl()                             */
/************************************************************************/
double OGRStyleBrush::GetParamDbl(OGRSTBrushParam eParam, GBool &bValueIsNull) {
    return OGRStyleTool::GetParamDbl(asStyleBrush[eParam], m_pasStyleValue[eParam], bValueIsNull);
}

/************************************************************************/
/*                            SetParamStr()                             */
/************************************************************************/
void OGRStyleBrush::SetParamStr(OGRSTBrushParam eParam, const char *pszParamString) {
    OGRStyleTool::SetParamStr(asStyleBrush[eParam], m_pasStyleValue[eParam], pszParamString);
}

/************************************************************************/
/*                            SetParamNum()                             */
/************************************************************************/
void OGRStyleBrush::SetParamNum(OGRSTBrushParam eParam, int nParam) {
    OGRStyleTool::SetParamNum(asStyleBrush[eParam], m_pasStyleValue[eParam], nParam);
}

/************************************************************************/
/*                            SetParamDbl()                             */
/************************************************************************/
void OGRStyleBrush::SetParamDbl(OGRSTBrushParam eParam, double dfParam) {
    OGRStyleTool::SetParamDbl(asStyleBrush[eParam], m_pasStyleValue[eParam], dfParam);
}

/************************************************************************/
/*                           GetStyleString()                           */
/************************************************************************/
const char *OGRStyleBrush::GetStyleString() {
    return OGRStyleTool::GetStyleString(asStyleBrush, m_pasStyleValue, (int)OGRSTBrushLast);
}

/****************************************************************************/
/*                      OGRStyleSymbol::OGRStyleSymbol()                    */
/****************************************************************************/
OGRStyleSymbol::OGRStyleSymbol() : OGRStyleTool(OGRSTCSymbol) {
    m_pasStyleValue = (OGRStyleValue *)CPLCalloc(OGRSTSymbolLast, sizeof(OGRStyleValue));
}

/****************************************************************************/
/*                      OGRStyleSymbol::~OGRStyleSymbol()                   */
/*                                                                          */
/****************************************************************************/
OGRStyleSymbol::~OGRStyleSymbol() {
    for (int i = 0; i < OGRSTSymbolLast; i++) {
        if (m_pasStyleValue[i].pszValue != nullptr) {
            CPLFree(m_pasStyleValue[i].pszValue);
            m_pasStyleValue[i].pszValue = nullptr;
        }
    }

    CPLFree(m_pasStyleValue);
}

/************************************************************************/
/*                               Parse()                                */
/************************************************************************/
GBool OGRStyleSymbol::Parse() {
    return OGRStyleTool::Parse(asStyleSymbol, m_pasStyleValue, (int)OGRSTSymbolLast);
}

/************************************************************************/
/*                            GetParamStr()                             */
/************************************************************************/
const char *OGRStyleSymbol::GetParamStr(OGRSTSymbolParam eParam, GBool &bValueIsNull) {
    return OGRStyleTool::GetParamStr(asStyleSymbol[eParam], m_pasStyleValue[eParam], bValueIsNull);
}
/************************************************************************/
/*                            GetParamNum()                             */
/************************************************************************/
int OGRStyleSymbol::GetParamNum(OGRSTSymbolParam eParam, GBool &bValueIsNull) {
    return OGRStyleTool::GetParamNum(asStyleSymbol[eParam], m_pasStyleValue[eParam], bValueIsNull);
}
/************************************************************************/
/*                            GetParamDbl()                             */
/************************************************************************/
double OGRStyleSymbol::GetParamDbl(OGRSTSymbolParam eParam, GBool &bValueIsNull) {
    return OGRStyleTool::GetParamDbl(asStyleSymbol[eParam], m_pasStyleValue[eParam], bValueIsNull);
}

/************************************************************************/
/*                            SetParamStr()                             */
/************************************************************************/
void OGRStyleSymbol::SetParamStr(OGRSTSymbolParam eParam, const char *pszParamString) {
    OGRStyleTool::SetParamStr(asStyleSymbol[eParam], m_pasStyleValue[eParam], pszParamString);
}

/************************************************************************/
/*                            SetParamNum()                             */
/************************************************************************/
void OGRStyleSymbol::SetParamNum(OGRSTSymbolParam eParam, int nParam) {
    OGRStyleTool::SetParamNum(asStyleSymbol[eParam], m_pasStyleValue[eParam], nParam);
}

/************************************************************************/
/*                            SetParamDbl()                             */
/************************************************************************/
void OGRStyleSymbol::SetParamDbl(OGRSTSymbolParam eParam, double dfParam) {
    OGRStyleTool::SetParamDbl(asStyleSymbol[eParam], m_pasStyleValue[eParam], dfParam);
}
/************************************************************************/
/*                           GetStyleString()                           */
/************************************************************************/
const char *OGRStyleSymbol::GetStyleString() {
    return OGRStyleTool::GetStyleString(asStyleSymbol, m_pasStyleValue, (int)OGRSTSymbolLast);
}

/****************************************************************************/
/*                      OGRStyleLabel::OGRStyleLabel()                      */
/*                                                                          */
/****************************************************************************/
OGRStyleLabel::OGRStyleLabel() : OGRStyleTool(OGRSTCLabel) {
    m_pasStyleValue = (OGRStyleValue *)CPLCalloc(OGRSTLabelLast, sizeof(OGRStyleValue));
}

/****************************************************************************/
/*                      OGRStyleLabel::~OGRStyleLabel()                     */
/*                                                                          */
/****************************************************************************/
OGRStyleLabel::~OGRStyleLabel() {
    for (int i = 0; i < OGRSTLabelLast; i++) {
        if (m_pasStyleValue[i].pszValue != nullptr) {
            CPLFree(m_pasStyleValue[i].pszValue);
            m_pasStyleValue[i].pszValue = nullptr;
        }
    }

    CPLFree(m_pasStyleValue);
}

/************************************************************************/
/*                               Parse()                                */
/************************************************************************/
GBool OGRStyleLabel::Parse() {
    return OGRStyleTool::Parse(asStyleLabel, m_pasStyleValue, (int)OGRSTLabelLast);
}

/************************************************************************/
/*                            GetParamStr()                             */
/************************************************************************/
const char *OGRStyleLabel::GetParamStr(OGRSTLabelParam eParam, GBool &bValueIsNull) {
    return OGRStyleTool::GetParamStr(asStyleLabel[eParam], m_pasStyleValue[eParam], bValueIsNull);
}
/************************************************************************/
/*                            GetParamNum()                             */
/************************************************************************/
int OGRStyleLabel::GetParamNum(OGRSTLabelParam eParam, GBool &bValueIsNull) {
    return OGRStyleTool::GetParamNum(asStyleLabel[eParam], m_pasStyleValue[eParam], bValueIsNull);
}
/************************************************************************/
/*                            GetParamDbl()                             */
/************************************************************************/
double OGRStyleLabel::GetParamDbl(OGRSTLabelParam eParam, GBool &bValueIsNull) {
    return OGRStyleTool::GetParamDbl(asStyleLabel[eParam], m_pasStyleValue[eParam], bValueIsNull);
}
/************************************************************************/
/*                            SetParamStr()                             */
/************************************************************************/
void OGRStyleLabel::SetParamStr(OGRSTLabelParam eParam, const char *pszParamString) {
    OGRStyleTool::SetParamStr(asStyleLabel[eParam], m_pasStyleValue[eParam], pszParamString);
}
/************************************************************************/
/*                            SetParamNum()                             */
/************************************************************************/
void OGRStyleLabel::SetParamNum(OGRSTLabelParam eParam, int nParam) {
    OGRStyleTool::SetParamNum(asStyleLabel[eParam], m_pasStyleValue[eParam], nParam);
}

/************************************************************************/
/*                            SetParamDbl()                             */
/************************************************************************/
void OGRStyleLabel::SetParamDbl(OGRSTLabelParam eParam, double dfParam) {
    OGRStyleTool::SetParamDbl(asStyleLabel[eParam], m_pasStyleValue[eParam], dfParam);
}
/************************************************************************/
/*                           GetStyleString()                           */
/************************************************************************/
const char *OGRStyleLabel::GetStyleString() {
    return OGRStyleTool::GetStyleString(asStyleLabel, m_pasStyleValue, (int)OGRSTLabelLast);
}

/****************************************************************************/
/*                      OGRStyleVector::OGRStyleVector()                    */
/*                                                                          */
/****************************************************************************/
OGRStyleVector::OGRStyleVector() : OGRStyleTool(OGRSTCVector) {
    m_pasStyleValue = (OGRStyleValue *)CPLCalloc(OGRSTVectorLast, sizeof(OGRStyleValue));
}

/****************************************************************************/
/*                      OGRStyleVector::~OGRStyleVector()                   */
/*                                                                          */
/****************************************************************************/
OGRStyleVector::~OGRStyleVector() {
    for (int i = 0; i < OGRSTVectorLast; i++) {
        if (m_pasStyleValue[i].pszValue != nullptr) {
            CPLFree(m_pasStyleValue[i].pszValue);
            m_pasStyleValue[i].pszValue = nullptr;
        }
    }

    CPLFree(m_pasStyleValue);
}

/************************************************************************/
/*                               Parse()                                */
/************************************************************************/
GBool OGRStyleVector::Parse() {
    return OGRStyleTool::Parse(asStyleVector, m_pasStyleValue, (int)OGRSTVectorLast);
}

/************************************************************************/
/*                            GetParamStr()                             */
/************************************************************************/
const char *OGRStyleVector::GetParamStr(OGRSTVectorParam eParam, GBool &bValueIsNull) {
    return OGRStyleTool::GetParamStr(asStyleVector[eParam], m_pasStyleValue[eParam], bValueIsNull);
}
/************************************************************************/
/*                            GetParamNum()                             */
/************************************************************************/
int OGRStyleVector::GetParamNum(OGRSTVectorParam eParam, GBool &bValueIsNull) {
    return OGRStyleTool::GetParamNum(asStyleVector[eParam], m_pasStyleValue[eParam], bValueIsNull);
}
/************************************************************************/
/*                            GetParamDbl()                             */
/************************************************************************/
double OGRStyleVector::GetParamDbl(OGRSTVectorParam eParam, GBool &bValueIsNull) {
    return OGRStyleTool::GetParamDbl(asStyleVector[eParam], m_pasStyleValue[eParam], bValueIsNull);
}
/************************************************************************/
/*                            SetParamStr()                             */
/************************************************************************/
void OGRStyleVector::SetParamStr(OGRSTVectorParam eParam, const char *pszParamString) {
    OGRStyleTool::SetParamStr(asStyleVector[eParam], m_pasStyleValue[eParam], pszParamString);
}
/************************************************************************/
/*                            SetParamNum()                             */
/************************************************************************/
void OGRStyleVector::SetParamNum(OGRSTVectorParam eParam, int nParam) {
    OGRStyleTool::SetParamNum(asStyleVector[eParam], m_pasStyleValue[eParam], nParam);
}

/************************************************************************/
/*                            SetParamDbl()                             */
/************************************************************************/
void OGRStyleVector::SetParamDbl(OGRSTVectorParam eParam, double dfParam) {
    OGRStyleTool::SetParamDbl(asStyleVector[eParam], m_pasStyleValue[eParam], dfParam);
}
/************************************************************************/
/*                           GetStyleString()                           */
/************************************************************************/
const char *OGRStyleVector::GetStyleString() {
    return OGRStyleTool::GetStyleString(asStyleVector, m_pasStyleValue, (int)OGRSTVectorLast);
}
