/**********************************************************************
 * $Id: mitab_imapinfofile.cpp,v 1.24 2007/06/21 14:00:23 dmorissette Exp $
 *
 * Name:     mitab_imapinfo
 * Project:  MapInfo mid/mif Tab Read/Write library
 * Language: C++
 * Purpose:  Implementation of the IMapInfoFile class, super class of
 *           of MIFFile and TABFile
 * Author:   Daniel Morissette, dmorissette@dmsolutions.ca
 *
 **********************************************************************
 * Copyright (c) 1999-2001, Daniel Morissette
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************
 *
 * $Log: mitab_imapinfofile.cpp,v $
 * Revision 1.24  2007/06/21 14:00:23  dmorissette
 * Added missing cast in isspace() calls to avoid failed assertion on Windows
 * (MITAB bug 1737, GDAL ticket 1678))
 *
 * Revision 1.23  2007/06/12 14:43:19  dmorissette
 * Use iswspace instead of sispace in IMapInfoFile::SmartOpen() (bug 1737)
 *
 * Revision 1.22  2007/06/12 13:52:37  dmorissette
 * Added IMapInfoFile::SetCharset() method (bug 1734)
 *
 * Revision 1.21  2005/05/19 21:10:50  fwarmerdam
 * changed to use OGRLayers spatial filter support
 *
 * Revision 1.20  2005/05/19 15:27:00  jlacroix
 * Implement a method to set the StyleString of a TABFeature.
 * This is done via the ITABFeaturePen, Brush and Symbol classes.
 *
 * Revision 1.19  2004/06/30 20:29:04  dmorissette
 * Fixed refs to old address danmo@videotron.ca
 *
 * Revision 1.18  2003/12/19 07:55:55  fwarmerdam
 * treat 3D features as 2D on write
 *
 * Revision 1.17  2001/09/14 19:14:43  warmerda
 * added attribute query support
 *
 * Revision 1.16  2001/09/14 03:23:55  warmerda
 * Substantial upgrade to support spatial queries using spatial indexes
 *
 * Revision 1.15  2001/07/03 23:11:21  daniel
 * Test for NULL geometries if spatial filter enabled in GetNextFeature().
 *
 * Revision 1.14  2001/03/09 04:16:02  daniel
 * Added TABSeamless for reading seamless TAB files
 *
 * Revision 1.13  2001/02/27 19:59:05  daniel
 * Enabled spatial filter in IMapInfoFile::GetNextFeature(), and avoid
 * unnecessary feature cloning in GetNextFeature() and GetFeature()
 *
 * Revision 1.12  2001/02/06 22:03:24  warmerda
 * fixed memory leak of whole features in CreateFeature
 *
 * Revision 1.11  2001/01/23 21:23:42  daniel
 * Added projection bounds lookup table, called from TABFile::SetProjInfo()
 *
 * Revision 1.10  2001/01/22 16:03:58  warmerda
 * expanded tabs
 *
 * Revision 1.9  2000/11/30 20:27:56  warmerda
 * make variable length string fields 254 wide, not 255
 *
 * Revision 1.8  2000/02/28 03:11:35  warmerda
 * fix support for zero width fields
 *
 * Revision 1.7  2000/02/02 20:14:03  warmerda
 * made safer when encountering geometryless features
 *
 * Revision 1.6  2000/01/26 18:17:35  warmerda
 * added CreateField method
 *
 * Revision 1.5  2000/01/15 22:30:44  daniel
 * Switch to MIT/X-Consortium OpenSource license
 *
 * Revision 1.4  2000/01/11 19:06:25  daniel
 * Added support for conversion of collections in CreateFeature()
 *
 * Revision 1.3  1999/12/14 02:14:50  daniel
 * Added static SmartOpen() method + TABView support
 *
 * Revision 1.2  1999/11/08 19:15:44  stephane
 * Add headers method
 *
 * Revision 1.1  1999/11/08 04:17:27  stephane
 * First Revision
 *
 **********************************************************************/

#include "mitab.h"
#include "mitab_utils.h"

#ifdef __HP_aCC
#include <wchar.h> /* iswspace() */
#else
#include <cwctype> /* iswspace() */
#endif

/**********************************************************************
 *                   IMapInfoFile::IMapInfoFile()
 *
 * Constructor.
 **********************************************************************/
IMapInfoFile::IMapInfoFile() {
    m_nCurFeatureId = 0;
    m_poCurFeature = nullptr;
    m_bBoundsSet = FALSE;
    m_pszCharset = nullptr;
}

/**********************************************************************
 *                   IMapInfoFile::~IMapInfoFile()
 *
 * Destructor.
 **********************************************************************/
IMapInfoFile::~IMapInfoFile() {
    if (m_poCurFeature) {
        delete m_poCurFeature;
        m_poCurFeature = nullptr;
    }

    CPLFree(m_pszCharset);
    m_pszCharset = nullptr;
}

/**********************************************************************
 *                   IMapInfoFile::SmartOpen()
 *
 * Use this static method to automatically open any flavour of MapInfo
 * dataset.  This method will detect the file type, create an object
 * of the right type, and open the file.
 *
 * Call GetFileClass() on the returned object if you need to find out
 * its exact type.  (To access format-specific methods for instance)
 *
 * Returns the new object ptr. , or NULL if the open failed.
 **********************************************************************/
IMapInfoFile *IMapInfoFile::SmartOpen(const char *pszFname, GBool bTestOpenNoError /*=FALSE*/) {
    IMapInfoFile *poFile = nullptr;
    int nLen = 0;

    if (pszFname)
        nLen = strlen(pszFname);

    if (nLen > 4 && (EQUAL(pszFname + nLen - 4, ".MIF") || EQUAL(pszFname + nLen - 4, ".MID"))) {
        /*-------------------------------------------------------------
         * MIF/MID file
         *------------------------------------------------------------*/
        poFile = new MIFFile;
    } else if (nLen > 4 && EQUAL(pszFname + nLen - 4, ".TAB")) {
        /*-------------------------------------------------------------
         * .TAB file ... is it a TABFileView or a TABFile?
         * We have to read the .tab header to find out.
         *------------------------------------------------------------*/
        FILE *fp;
        const char *pszLine;
        char *pszAdjFname = CPLStrdup(pszFname);
        GBool bFoundFields = FALSE, bFoundView = FALSE, bFoundSeamless = FALSE;

        TABAdjustFilenameExtension(pszAdjFname);
        fp = VSIFOpen(pszAdjFname, "r");
        while (fp && (pszLine = CPLReadLine(fp)) != nullptr) {
            while (isspace((unsigned char)*pszLine)) pszLine++;
            if (EQUALN(pszLine, "Fields", 6))
                bFoundFields = TRUE;
            else if (EQUALN(pszLine, "create view", 11))
                bFoundView = TRUE;
            else if (EQUALN(pszLine, "\"\\IsSeamless\" = \"TRUE\"", 21))
                bFoundSeamless = TRUE;
        }

        if (bFoundView)
            poFile = new TABView;
        else if (bFoundFields && bFoundSeamless)
            poFile = new TABSeamless;
        else if (bFoundFields)
            poFile = new TABFile;

        if (fp)
            VSIFClose(fp);

        CPLFree(pszAdjFname);
    }

    /*-----------------------------------------------------------------
     * Perform the open() call
     *----------------------------------------------------------------*/
    if (poFile && poFile->Open(pszFname, "r", bTestOpenNoError) != 0) {
        delete poFile;
        poFile = nullptr;
    }

    if (!bTestOpenNoError && poFile == nullptr) {
        CPLError(CE_Failure, CPLE_FileIO, "%s could not be opened as a MapInfo dataset.", pszFname);
    }

    return poFile;
}

/**********************************************************************
 *                   IMapInfoFile::GetNextFeature()
 *
 * Standard OGR GetNextFeature implementation.  This methode is used
 * to retreive the next OGRFeature.
 **********************************************************************/
OGRFeature *IMapInfoFile::GetNextFeature() {
    OGRFeature *poFeatureRef;
    OGRGeometry *poGeom;
    int nFeatureId;

    while ((nFeatureId = GetNextFeatureId(m_nCurFeatureId)) != -1) {
        poFeatureRef = GetFeatureRef(nFeatureId);
        if (poFeatureRef == nullptr)
            return nullptr;
        if ((m_poFilterGeom == nullptr ||
             ((poGeom = poFeatureRef->GetGeometryRef()) != nullptr && FilterGeometry(poGeom))) &&
            (m_poAttrQuery == nullptr || m_poAttrQuery->Evaluate(poFeatureRef))) {
            // Avoid cloning feature... return the copy owned by the class
            CPLAssert(poFeatureRef == m_poCurFeature);
            m_poCurFeature = nullptr;
            return poFeatureRef;
        }
    }
    return nullptr;
}

/**********************************************************************
 *                   IMapInfoFile::CreateFeature()
 *
 * Standard OGR CreateFeature implementation.  This methode is used
 * to create a new feature in current dataset
 **********************************************************************/
OGRErr IMapInfoFile::CreateFeature(OGRFeature *poFeature) {
    TABFeature *poTABFeature;
    OGRGeometry *poGeom;
    OGRwkbGeometryType eGType;
    OGRErr eErr;
    TABPoint *poTABPointFeature = nullptr;
    TABRegion *poTABRegionFeature = nullptr;
    TABPolyline *poTABPolylineFeature = nullptr;

    /*-----------------------------------------------------------------
     * MITAB won't accept new features unless they are in a type derived
     * from TABFeature... so we have to do our best to map to the right
     * feature type based on the geometry type.
     *----------------------------------------------------------------*/
    poGeom = poFeature->GetGeometryRef();
    if (poGeom != nullptr)
        eGType = poGeom->getGeometryType();
    else
        eGType = wkbNone;

    switch (wkbFlatten(eGType)) {
        /*-------------------------------------------------------------
         * POINT
         *------------------------------------------------------------*/
        case wkbPoint:
            poTABFeature = new TABPoint(poFeature->GetDefnRef());
            if (poFeature->GetStyleString()) {
                poTABPointFeature = (TABPoint *)poTABFeature;
                poTABPointFeature->SetSymbolFromStyleString(poFeature->GetStyleString());
            }
            break;
        /*-------------------------------------------------------------
         * REGION
         *------------------------------------------------------------*/
        case wkbPolygon:
        case wkbMultiPolygon:
            poTABFeature = new TABRegion(poFeature->GetDefnRef());
            if (poFeature->GetStyleString()) {
                poTABRegionFeature = (TABRegion *)poTABFeature;
                poTABRegionFeature->SetPenFromStyleString(poFeature->GetStyleString());

                poTABRegionFeature->SetBrushFromStyleString(poFeature->GetStyleString());
            }
            break;
        /*-------------------------------------------------------------
         * LINE/PLINE/MULTIPLINE
         *------------------------------------------------------------*/
        case wkbLineString:
        case wkbMultiLineString:
            poTABFeature = new TABPolyline(poFeature->GetDefnRef());
            if (poFeature->GetStyleString()) {
                poTABPolylineFeature = (TABPolyline *)poTABFeature;
                poTABPolylineFeature->SetPenFromStyleString(poFeature->GetStyleString());
            }
            break;
        /*-------------------------------------------------------------
         * Collection types that are not directly supported... convert
         * to multiple features in output file through recursive calls.
         *------------------------------------------------------------*/
        case wkbGeometryCollection:
        case wkbMultiPoint: {
            OGRErr eStatus = OGRERR_NONE;
            int i;
            auto *poColl = (OGRGeometryCollection *)poGeom;
            OGRFeature *poTmpFeature = poFeature->Clone();

            for (i = 0; eStatus == OGRERR_NONE && i < poColl->getNumGeometries(); i++) {
                poTmpFeature->SetGeometry(poColl->getGeometryRef(i));
                eStatus = CreateFeature(poTmpFeature);
            }
            delete poTmpFeature;
            return eStatus;
        } break;
        /*-------------------------------------------------------------
         * Unsupported type.... convert to MapInfo geometry NONE
         *------------------------------------------------------------*/
        case wkbUnknown:
        default:
            poTABFeature = new TABFeature(poFeature->GetDefnRef());
            break;
    }

    if (poGeom != nullptr)
        poTABFeature->SetGeometryDirectly(poGeom->clone());

    for (int i = 0; i < poFeature->GetDefnRef()->GetFieldCount(); i++) {
        poTABFeature->SetField(i, poFeature->GetRawFieldRef(i));
    }

    if (SetFeature(poTABFeature) > -1)
        eErr = OGRERR_NONE;
    else
        eErr = OGRERR_FAILURE;

    delete poTABFeature;

    return eErr;
}

/**********************************************************************
 *                   IMapInfoFile::GetFeature()
 *
 * Standard OGR GetFeature implementation.  This methode is used
 * to get the wanted (nFeatureId) feature, a NULL value will be
 * returned on error.
 **********************************************************************/
OGRFeature *IMapInfoFile::GetFeature(long nFeatureId) {
    OGRFeature *poFeatureRef;

    poFeatureRef = GetFeatureRef(nFeatureId);
    if (poFeatureRef) {
        // Avoid cloning feature... return the copy owned by the class
        CPLAssert(poFeatureRef == m_poCurFeature);
        m_poCurFeature = nullptr;

        return poFeatureRef;
    }
    return nullptr;
}

/************************************************************************/
/*                            CreateField()                             */
/*                                                                      */
/*      Create a native field based on a generic OGR definition.        */
/************************************************************************/

OGRErr IMapInfoFile::CreateField(OGRFieldDefn *poField, int bApproxOK)

{
    TABFieldType eTABType;
    int nWidth = poField->GetWidth();

    if (poField->GetType() == OFTInteger) {
        eTABType = TABFInteger;
        if (nWidth == 0)
            nWidth = 12;
    } else if (poField->GetType() == OFTReal) {
        eTABType = TABFFloat;
        if (nWidth == 0)
            nWidth = 32;
    } else if (poField->GetType() == OFTString) {
        eTABType = TABFChar;
        if (nWidth == 0)
            nWidth = 254;
        else
            nWidth = MIN(254, nWidth);
    } else {
        CPLError(CE_Failure, CPLE_AppDefined,
                 "IMapInfoFile::CreateField() called with unsupported field"
                 " type %d.\n"
                 "Note that Mapinfo files don't support list field types.\n",
                 poField->GetType());

        return OGRERR_FAILURE;
    }

    if (AddFieldNative(poField->GetNameRef(), eTABType, nWidth, poField->GetPrecision()) > -1)
        return OGRERR_NONE;

    return OGRERR_FAILURE;
}

/**********************************************************************
 *                   IMapInfoFile::SetCharset()
 *
 * Set the charset for the tab header.
 *
 *
 * Returns 0 on success, -1 on error.
 **********************************************************************/
int IMapInfoFile::SetCharset(const char *pszCharset) {
    if (pszCharset && strlen(pszCharset) > 0) {
        CPLFree(m_pszCharset);
        m_pszCharset = CPLStrdup(pszCharset);
    }
    return 0;
}
