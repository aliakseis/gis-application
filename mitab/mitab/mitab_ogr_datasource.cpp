/**********************************************************************
 * $Id: mitab_ogr_datasource.cpp,v 1.12 2007/03/22 20:01:36 dmorissette Exp $
 *
 * Name:     mitab_ogr_datasource.cpp
 * Project:  MapInfo Mid/Mif, Tab ogr support
 * Language: C++
 * Purpose:  Implementation of OGRTABDataSource.
 * Author:   Stephane Villeneuve, stephane.v@videotron.ca
 *           and Frank Warmerdam, warmerdam@pobox.com
 *
 **********************************************************************
 * Copyright (c) 1999, 2000, Stephane Villeneuve
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
 * $Log: mitab_ogr_datasource.cpp,v $
 * Revision 1.12  2007/03/22 20:01:36  dmorissette
 * Added SPATIAL_INDEX_MODE=QUICK creation option (MITAB bug 1669)
 *
 * Revision 1.11  2006/01/27 14:27:35  fwarmerdam
 * fixed ogr bounds setting problems (bug 1198)
 *
 * Revision 1.10  2005/09/20 04:40:02  fwarmerdam
 * fixed CPLReadDir memory leak
 *
 * Revision 1.9  2004/10/15 01:52:30  fwarmerdam
 * Modified CreateLayer() to use  -1000,-1000,1000,1000 bounds for GEOGCS
 * much like in mitab_bounds.cpp.  This ensures that geographic files in
 * the range 0-360 works as well as -180 to 180.
 *
 * Revision 1.8  2004/07/07 15:42:46  fwarmerdam
 * fixed up some single layer creation issues
 *
 * Revision 1.7  2004/02/27 21:06:03  fwarmerdam
 * Better support for "single file" creation ... don't allow other layers to
 * be created.  But *do* single file to satisfy the first layer creation request
 * made.  Also, allow creating a datasource "on" an existing directory.
 *
 * Revision 1.6  2003/03/21 14:20:49  warmerda
 * fixed email
 *
 * Revision 1.5  2002/02/08 16:52:16  warmerda
 * added support for FORMAT=MIF option for creating layers
 *
 * Revision 1.4  2001/02/06 22:13:54  warmerda
 * fixed memory leak in OGRTABDataSource::CreateLayer()
 *
 * Revision 1.3  2001/01/22 16:03:58  warmerda
 * expanded tabs
 *
 * Revision 1.2  2000/07/04 01:46:23  warmerda
 * Avoid warnings on unused arguments.
 *
 * Revision 1.1  2000/01/26 18:17:09  warmerda
 * New
 *
 **********************************************************************/

#include "mitab_ogr_driver.h"

/*=======================================================================
 *                 OGRTABDataSource
 *
 * We need one single OGRDataSource/Driver set of classes to handle all
 * the MapInfo file types.  They all deal with the IMapInfoFile abstract
 * class.
 *=====================================================================*/

/************************************************************************/
/*                         OGRTABDataSource()                           */
/************************************************************************/

OGRTABDataSource::OGRTABDataSource()

{
    m_pszName = nullptr;
    m_pszDirectory = nullptr;
    m_nLayerCount = 0;
    m_papoLayers = nullptr;
    m_papszOptions = nullptr;
    m_bCreateMIF = FALSE;
    m_bSingleFile = FALSE;
    m_bSingleLayerAlreadyCreated = FALSE;
    m_bQuickSpatialIndexMode = FALSE;
}

/************************************************************************/
/*                         ~OGRTABDataSource()                          */
/************************************************************************/

OGRTABDataSource::~OGRTABDataSource()

{
    CPLFree(m_pszName);
    CPLFree(m_pszDirectory);

    for (int i = 0; i < m_nLayerCount; i++) delete m_papoLayers[i];

    CPLFree(m_papoLayers);
    CSLDestroy(m_papszOptions);
}

/************************************************************************/
/*                               Create()                               */
/*                                                                      */
/*      Create a new dataset (directory or file).                       */
/************************************************************************/

int OGRTABDataSource::Create(const char *pszName, char **papszOptions)

{
    VSIStatBuf sStat;
    const char *pszOpt;

    CPLAssert(m_pszName == NULL);

    m_pszName = CPLStrdup(pszName);
    m_papszOptions = CSLDuplicate(papszOptions);

    if ((pszOpt = CSLFetchNameValue(papszOptions, "FORMAT")) != nullptr && EQUAL(pszOpt, "MIF"))
        m_bCreateMIF = TRUE;
    else if (EQUAL(CPLGetExtension(pszName), "mif") || EQUAL(CPLGetExtension(pszName), "mid"))
        m_bCreateMIF = TRUE;

    if ((pszOpt = CSLFetchNameValue(papszOptions, "SPATIAL_INDEX_MODE")) != nullptr &&
        EQUAL(pszOpt, "QUICK"))
        m_bQuickSpatialIndexMode = TRUE;

    /* -------------------------------------------------------------------- */
    /*      Create a new empty directory.                                   */
    /* -------------------------------------------------------------------- */
    if (strlen(CPLGetExtension(pszName)) == 0) {
        if (VSIStat(pszName, &sStat) == 0) {
            if (!VSI_ISDIR(sStat.st_mode)) {
                CPLError(CE_Failure, CPLE_OpenFailed,
                         "Attempt to create dataset named %s,\n"
                         "but that is an existing file.\n",
                         pszName);
                return FALSE;
            }
        } else {
            if (VSIMkdir(pszName, 0755) != 0) {
                CPLError(CE_Failure, CPLE_AppDefined, "Unable to create directory %s.\n", pszName);
                return FALSE;
            }
        }

        m_pszDirectory = CPLStrdup(pszName);
    }

    /* -------------------------------------------------------------------- */
    /*      Create a new single file.                                       */
    /* -------------------------------------------------------------------- */
    else {
        IMapInfoFile *poFile;

        if (m_bCreateMIF)
            poFile = new MIFFile;
        else
            poFile = new TABFile;

        if (poFile->Open(pszName, "wb", FALSE) != 0) {
            delete poFile;
            return FALSE;
        }

        m_nLayerCount = 1;
        m_papoLayers = (IMapInfoFile **)CPLMalloc(sizeof(void *));
        m_papoLayers[0] = poFile;

        m_pszDirectory = CPLStrdup(CPLGetPath(pszName));
        m_bSingleFile = TRUE;
    }

    return TRUE;
}

/************************************************************************/
/*                                Open()                                */
/*                                                                      */
/*      Open an existing file, or directory of files.                   */
/************************************************************************/

int OGRTABDataSource::Open(const char *pszName, int bTestOpen)

{
    VSIStatBuf stat;

    CPLAssert(m_pszName == NULL);

    m_pszName = CPLStrdup(pszName);

    /* -------------------------------------------------------------------- */
    /*      Is this a file or directory?                                    */
    /* -------------------------------------------------------------------- */
    if (VSIStat(pszName, &stat) != 0 || (!VSI_ISDIR(stat.st_mode) && !VSI_ISREG(stat.st_mode))) {
        if (!bTestOpen) {
            CPLError(CE_Failure, CPLE_OpenFailed,
                     "%s is not a file or directory.\n"
                     "Unable to open as a Mapinfo dataset.\n",
                     pszName);
        }

        return FALSE;
    }

    /* -------------------------------------------------------------------- */
    /*      If it is a file, try to open as a Mapinfo file.                 */
    /* -------------------------------------------------------------------- */
    if (VSI_ISREG(stat.st_mode)) {
        IMapInfoFile *poFile;

        poFile = IMapInfoFile::SmartOpen(pszName, bTestOpen);
        if (poFile == nullptr)
            return FALSE;

        m_nLayerCount = 1;
        m_papoLayers = (IMapInfoFile **)CPLMalloc(sizeof(void *));
        m_papoLayers[0] = poFile;

        m_pszDirectory = CPLStrdup(CPLGetPath(pszName));
    }

    /* -------------------------------------------------------------------- */
    /*      Otherwise, we need to scan the whole directory for files        */
    /*      ending in .tab or .mif.                                         */
    /* -------------------------------------------------------------------- */
    else {
        char **papszFileList = CPLReadDir(pszName);

        m_pszDirectory = CPLStrdup(pszName);

        for (int iFile = 0; papszFileList != nullptr && papszFileList[iFile] != nullptr; iFile++) {
            IMapInfoFile *poFile;
            const char *pszExtension = CPLGetExtension(papszFileList[iFile]);
            char *pszSubFilename;

            if (!EQUAL(pszExtension, "tab") && !EQUAL(pszExtension, "mif"))
                continue;

            pszSubFilename =
                CPLStrdup(CPLFormFilename(m_pszDirectory, papszFileList[iFile], nullptr));

            poFile = IMapInfoFile::SmartOpen(pszSubFilename, bTestOpen);
            CPLFree(pszSubFilename);

            if (poFile == nullptr) {
                CSLDestroy(papszFileList);
                return FALSE;
            }

            m_nLayerCount++;
            m_papoLayers =
                (IMapInfoFile **)CPLRealloc(m_papoLayers, sizeof(void *) * m_nLayerCount);
            m_papoLayers[m_nLayerCount - 1] = poFile;
        }

        CSLDestroy(papszFileList);

        if (m_nLayerCount == 0) {
            if (!bTestOpen)
                CPLError(CE_Failure, CPLE_OpenFailed, "No mapinfo files found in directory %s.\n",
                         m_pszDirectory);

            return FALSE;
        }
    }

    return TRUE;
}

/************************************************************************/
/*                           GetLayerCount()                            */
/************************************************************************/

int OGRTABDataSource::GetLayerCount()

{
    if (m_bSingleFile && !m_bSingleLayerAlreadyCreated)
        return 0;

    return m_nLayerCount;
}

/************************************************************************/
/*                              GetLayer()                              */
/************************************************************************/

OGRLayer *OGRTABDataSource::GetLayer(int iLayer)

{
    if (iLayer < 0 || iLayer >= GetLayerCount())
        return nullptr;

    return m_papoLayers[iLayer];
}

/************************************************************************/
/*                            CreateLayer()                             */
/************************************************************************/

OGRLayer *OGRTABDataSource::CreateLayer(const char *pszLayerName, OGRSpatialReference *poSRSIn,
                                        OGRwkbGeometryType /* eGeomTypeIn */,
                                        char ** /* papszOptions */)

{
    IMapInfoFile *poFile;
    char *pszFullFilename;

    /* -------------------------------------------------------------------- */
    /*      If it's a single file mode file, then we may have already       */
    /*      instantiated the low level layer.   We would just need to       */
    /*      reset the coordinate system and (potentially) bounds.           */
    /* -------------------------------------------------------------------- */
    if (m_bSingleFile) {
        if (m_bSingleLayerAlreadyCreated) {
            CPLError(CE_Failure, CPLE_AppDefined,
                     "Unable to create new layers in this single file dataset.");
            return nullptr;
        }

        m_bSingleLayerAlreadyCreated = TRUE;

        poFile = (IMapInfoFile *)m_papoLayers[0];
    }

    /* -------------------------------------------------------------------- */
    /*      We need to initially create the file, and add it as a layer.    */
    /* -------------------------------------------------------------------- */
    else {
        if (m_bCreateMIF) {
            pszFullFilename = CPLStrdup(CPLFormFilename(m_pszDirectory, pszLayerName, "mif"));

            poFile = new MIFFile;
        } else {
            pszFullFilename = CPLStrdup(CPLFormFilename(m_pszDirectory, pszLayerName, "tab"));

            poFile = new TABFile;
        }

        if (poFile->Open(pszFullFilename, "wb", FALSE) != 0) {
            CPLFree(pszFullFilename);
            delete poFile;
            return FALSE;
        }

        m_nLayerCount++;
        m_papoLayers = (IMapInfoFile **)CPLRealloc(m_papoLayers, sizeof(void *) * m_nLayerCount);
        m_papoLayers[m_nLayerCount - 1] = poFile;

        CPLFree(pszFullFilename);
    }

    /* -------------------------------------------------------------------- */
    /*      Assign the coordinate system (if provided) and set              */
    /*      reasonable bounds.                                              */
    /* -------------------------------------------------------------------- */
    if (poSRSIn != nullptr)
        poFile->SetSpatialRef(poSRSIn);

    if (!poFile->IsBoundsSet() && !m_bCreateMIF) {
        if (poSRSIn != nullptr && poSRSIn->GetRoot() != nullptr &&
            EQUAL(poSRSIn->GetRoot()->GetValue(), "GEOGCS"))
            poFile->SetBounds(-1000, -1000, 1000, 1000);
        else
            poFile->SetBounds(-30000000, -15000000, 30000000, 15000000);
    }

    if (m_bQuickSpatialIndexMode && poFile->SetQuickSpatialIndexMode() != 0) {
        CPLError(CE_Warning, CPLE_AppDefined, "Setting Quick Spatial Index Mode failed.");
    }

    return poFile;
}

/************************************************************************/
/*                           TestCapability()                           */
/************************************************************************/

int OGRTABDataSource::TestCapability(const char *pszCap)

{
    if (EQUAL(pszCap, ODsCCreateLayer))
        return !m_bSingleFile || !m_bSingleLayerAlreadyCreated;

    return FALSE;
}
