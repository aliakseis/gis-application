/******************************************************************************
 * $Id: ogrpoint.cpp 10646 2007-01-18 02:38:10Z warmerdam $
 *
 * Project:  OpenGIS Simple Features Reference Implementation
 * Purpose:  The Point geometry class.
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

#include <cassert>

#include "ogr_geometry.h"
#include "ogr_p.h"

CPL_CVSID("$Id: ogrpoint.cpp 10646 2007-01-18 02:38:10Z warmerdam $");

/************************************************************************/
/*                              OGRPoint()                              */
/************************************************************************/

/**
 * Create a (0,0) point.
 */

OGRPoint::OGRPoint()

{
    x = 0;
    y = 0;
    z = 0;
}

/************************************************************************/
/*                              OGRPoint()                              */
/*                                                                      */
/*      Initialize point to value.                                      */
/************************************************************************/

OGRPoint::OGRPoint(double xIn, double yIn, double zIn)

{
    x = xIn;
    y = yIn;
    z = zIn;
    nCoordDimension = 3;
}

/************************************************************************/
/*                              OGRPoint()                              */
/*                                                                      */
/*      Initialize point to value.                                      */
/************************************************************************/

OGRPoint::OGRPoint(double xIn, double yIn)

{
    x = xIn;
    y = yIn;
    z = 0.0;
    nCoordDimension = 2;
}

/************************************************************************/
/*                             ~OGRPoint()                              */
/************************************************************************/

OGRPoint::~OGRPoint()

    = default;

/************************************************************************/
/*                               clone()                                */
/*                                                                      */
/*      Make a new object that is a copy of this object.                */
/************************************************************************/

OGRGeometry *OGRPoint::clone() const

{
    auto *poNewPoint = new OGRPoint(x, y, z);

    poNewPoint->assignSpatialReference(getSpatialReference());
    poNewPoint->setCoordinateDimension(nCoordDimension);

    return poNewPoint;
}

/************************************************************************/
/*                               empty()                                */
/************************************************************************/
void OGRPoint::empty()

{
    x = y = z = 0.0;
}

/************************************************************************/
/*                            getDimension()                            */
/************************************************************************/

int OGRPoint::getDimension() const

{
    return 0;
}

/************************************************************************/
/*                          getGeometryType()                           */
/************************************************************************/

OGRwkbGeometryType OGRPoint::getGeometryType() const

{
    if (nCoordDimension < 3)
        return wkbPoint;

    return wkbPoint25D;
}

/************************************************************************/
/*                          getGeometryName()                           */
/************************************************************************/

const char *OGRPoint::getGeometryName() const

{
    return "POINT";
}

/************************************************************************/
/*                            flattenTo2D()                             */
/************************************************************************/

void OGRPoint::flattenTo2D()

{
    z = 0;
    nCoordDimension = 2;
}

/************************************************************************/
/*                       setCoordinateDimension()                       */
/************************************************************************/

void OGRPoint::setCoordinateDimension(int nNewDimension)

{
    nCoordDimension = nNewDimension;

    if (nCoordDimension == 2)
        z = 0;
}

/************************************************************************/
/*                              WkbSize()                               */
/*                                                                      */
/*      Return the size of this object in well known binary             */
/*      representation including the byte order, and type information.  */
/************************************************************************/

int OGRPoint::WkbSize() const

{
    if (nCoordDimension != 3)
        return 21;

    return 29;
}

/************************************************************************/
/*                           importFromWkb()                            */
/*                                                                      */
/*      Initialize from serialized stream in well known binary          */
/*      format.                                                         */
/************************************************************************/

OGRErr OGRPoint::importFromWkb(unsigned char *pabyData, int nSize)

{
    OGRwkbByteOrder eByteOrder;

    if (nSize < 21 && nSize != -1)
        return OGRERR_NOT_ENOUGH_DATA;

    /* -------------------------------------------------------------------- */
    /*      Get the byte order byte.                                        */
    /* -------------------------------------------------------------------- */
    eByteOrder = DB2_V72_FIX_BYTE_ORDER((OGRwkbByteOrder)*pabyData);
    assert(eByteOrder == wkbXDR || eByteOrder == wkbNDR);

    /* -------------------------------------------------------------------- */
    /*      Get the geometry feature type.  For now we assume that          */
    /*      geometry type is between 0 and 255 so we only have to fetch     */
    /*      one byte.                                                       */
    /* -------------------------------------------------------------------- */
    OGRwkbGeometryType eGeometryType;
    int bIs3D;

    if (eByteOrder == wkbNDR) {
        eGeometryType = (OGRwkbGeometryType)pabyData[1];
        bIs3D = pabyData[4] & 0x80 || pabyData[2] & 0x80;
    } else {
        eGeometryType = (OGRwkbGeometryType)pabyData[4];
        bIs3D = pabyData[1] & 0x80 || pabyData[3] & 0x80;
    }

    assert(eGeometryType == wkbPoint);

    /* -------------------------------------------------------------------- */
    /*      Get the vertex.                                                 */
    /* -------------------------------------------------------------------- */
    memcpy(&x, pabyData + 5, 16);

    if (OGR_SWAP(eByteOrder)) {
        CPL_SWAPDOUBLE(&x);
        CPL_SWAPDOUBLE(&y);
    }

    if (bIs3D) {
        memcpy(&z, pabyData + 5 + 16, 8);
        if (OGR_SWAP(eByteOrder)) {
            CPL_SWAPDOUBLE(&z);
        }
        nCoordDimension = 3;
    } else {
        z = 0;
        nCoordDimension = 2;
    }

    return OGRERR_NONE;
}

/************************************************************************/
/*                            exportToWkb()                             */
/*                                                                      */
/*      Build a well known binary representation of this object.        */
/************************************************************************/

OGRErr OGRPoint::exportToWkb(OGRwkbByteOrder eByteOrder, unsigned char *pabyData) const

{
    /* -------------------------------------------------------------------- */
    /*      Set the byte order.                                             */
    /* -------------------------------------------------------------------- */
    pabyData[0] = DB2_V72_UNFIX_BYTE_ORDER((unsigned char)eByteOrder);

    /* -------------------------------------------------------------------- */
    /*      Set the geometry feature type.                                  */
    /* -------------------------------------------------------------------- */
    GUInt32 nGType = getGeometryType();

    if (eByteOrder == wkbNDR)
        nGType = CPL_LSBWORD32(nGType);
    else
        nGType = CPL_MSBWORD32(nGType);

    memcpy(pabyData + 1, &nGType, 4);

    /* -------------------------------------------------------------------- */
    /*      Copy in the raw data.                                           */
    /* -------------------------------------------------------------------- */
    memcpy(pabyData + 5, &x, 16);

    if (nCoordDimension == 3) {
        memcpy(pabyData + 5 + 16, &z, 8);
    }

    /* -------------------------------------------------------------------- */
    /*      Swap if needed.                                                 */
    /* -------------------------------------------------------------------- */
    if (OGR_SWAP(eByteOrder)) {
        CPL_SWAPDOUBLE(pabyData + 5);
        CPL_SWAPDOUBLE(pabyData + 5 + 8);

        if (nCoordDimension == 3)
            CPL_SWAPDOUBLE(pabyData + 5 + 16);
    }

    return OGRERR_NONE;
}

/************************************************************************/
/*                           importFromWkt()                            */
/*                                                                      */
/*      Instantiate point from well known text format ``POINT           */
/*      (x,y)''.                                                        */
/************************************************************************/

OGRErr OGRPoint::importFromWkt(char **ppszInput)

{
    char szToken[OGR_WKT_TOKEN_MAX];
    const char *pszInput = *ppszInput;

    /* -------------------------------------------------------------------- */
    /*      Read and verify the ``POINT'' keyword token.                    */
    /* -------------------------------------------------------------------- */
    pszInput = OGRWktReadToken(pszInput, szToken);

    if (!EQUAL(szToken, "POINT"))
        return OGRERR_CORRUPT_DATA;

    /* -------------------------------------------------------------------- */
    /*      Check for EMPTY ... but treat like a point at 0,0.              */
    /* -------------------------------------------------------------------- */
    const char *pszPreScan;

    pszPreScan = OGRWktReadToken(pszInput, szToken);
    if (EQUAL(szToken, "EMPTY")) {
        *ppszInput = (char *)pszInput;
        empty();
        return OGRERR_NONE;
    }

    if (!EQUAL(szToken, "("))
        return OGRERR_CORRUPT_DATA;

    pszPreScan = OGRWktReadToken(pszPreScan, szToken);
    if (EQUAL(szToken, "EMPTY")) {
        pszInput = OGRWktReadToken(pszPreScan, szToken);

        if (!EQUAL(szToken, ")"))
            return OGRERR_CORRUPT_DATA;

        *ppszInput = (char *)pszInput;
        empty();
        return OGRERR_NONE;
    }

    /* -------------------------------------------------------------------- */
    /*      Read the point list which should consist of exactly one point.  */
    /* -------------------------------------------------------------------- */
    OGRRawPoint *poPoints = nullptr;
    double *padfZ = nullptr;
    int nMaxPoint = 0, nPoints = 0;

    pszInput = OGRWktReadPoints(pszInput, &poPoints, &padfZ, &nMaxPoint, &nPoints);
    if (pszInput == nullptr || nPoints != 1)
        return OGRERR_CORRUPT_DATA;

    x = poPoints[0].x;
    y = poPoints[0].y;

    CPLFree(poPoints);

    if (padfZ != nullptr) {
        z = padfZ[0];
        nCoordDimension = 3;
        CPLFree(padfZ);
    } else
        nCoordDimension = 2;

    *ppszInput = (char *)pszInput;

    return OGRERR_NONE;
}

/************************************************************************/
/*                            exportToWkt()                             */
/*                                                                      */
/*      Translate this structure into it's well known text format       */
/*      equivelent.                                                     */
/************************************************************************/

OGRErr OGRPoint::exportToWkt(char **ppszDstText) const

{
    char szTextEquiv[140];
    char szCoordinate[80];

    OGRMakeWktCoordinate(szCoordinate, x, y, z, nCoordDimension);
    sprintf(szTextEquiv, "POINT (%s)", szCoordinate);
    *ppszDstText = CPLStrdup(szTextEquiv);

    return OGRERR_NONE;
}

/************************************************************************/
/*                            getEnvelope()                             */
/************************************************************************/

void OGRPoint::getEnvelope(OGREnvelope *psEnvelope) const

{
    psEnvelope->MinX = psEnvelope->MaxX = getX();
    psEnvelope->MinY = psEnvelope->MaxY = getY();
}

/**
 * \fn double OGRPoint::getX() const;
 *
 * Fetch X coordinate.
 *
 * Relates to the SFCOM IPoint::get_X() method.
 *
 * @return the X coordinate of this point.
 */

/**
 * \fn double OGRPoint::getY() const;
 *
 * Fetch Y coordinate.
 *
 * Relates to the SFCOM IPoint::get_Y() method.
 *
 * @return the Y coordinate of this point.
 */

/**
 * \fn double OGRPoint::getZ() const;
 *
 * Fetch Z coordinate.
 *
 * Relates to the SFCOM IPoint::get_Z() method.
 *
 * @return the Z coordinate of this point, or zero if it is a 2D point.
 */

/**
 * \fn void OGRPoint::setX( double xIn );
 *
 * Assign point X coordinate.
 *
 * There is no corresponding SFCOM method.
 */

/**
 * \fn void OGRPoint::setY( double yIn );
 *
 * Assign point Y coordinate.
 *
 * There is no corresponding SFCOM method.
 */

/**
 * \fn void OGRPoint::setZ( double zIn );
 *
 * Assign point Z coordinate.  Setting a zero zIn value will make the point
 * 2D, and setting a non-zero value will make the point 3D (wkbPoint|wkbZ).
 *
 * There is no corresponding SFCOM method.
 */

/************************************************************************/
/*                               Equal()                                */
/************************************************************************/

OGRBoolean OGRPoint::Equals(OGRGeometry *poOther) const

{
    auto *poOPoint = (OGRPoint *)poOther;

    if (poOPoint == this)
        return TRUE;

    if (poOther->getGeometryType() != getGeometryType())
        return FALSE;

    // we should eventually test the SRS.

    if (poOPoint->getX() != getX() || poOPoint->getY() != getY() || poOPoint->getZ() != getZ())
        return FALSE;

    return TRUE;
}

/************************************************************************/
/*                             transform()                              */
/************************************************************************/

OGRErr OGRPoint::transform(OGRCoordinateTransformation *poCT)

{
#ifdef DISABLE_OGRGEOM_TRANSFORM
    return OGRERR_FAILURE;
#else
    if (poCT->Transform(1, &x, &y, &z)) {
        assignSpatialReference(poCT->GetTargetCS());
        return OGRERR_NONE;
    }
    return OGRERR_FAILURE;
#endif
}
