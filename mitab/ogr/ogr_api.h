/******************************************************************************
 * $Id: ogr_api.h 10646 2007-01-18 02:38:10Z warmerdam $
 *
 * Project:  OpenGIS Simple Features Reference Implementation
 * Purpose:  C API for OGR Geometry, Feature, Layers, DataSource and drivers.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 2002, Frank Warmerdam
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

#ifndef _OGR_API_H_INCLUDED
#define _OGR_API_H_INCLUDED

/**
 * \file ogr_api.h
 *
 * C API and defines for OGRFeature, OGRGeometry, and OGRDataSource
 * related classes.
 *
 * See also: ogr_geometry.h, ogr_feature.h, ogrsf_frmts.h
 */

#include "ogr_core.h"

CPL_C_START

/* -------------------------------------------------------------------- */
/*      Geometry related functions (ogr_geometry.h)                     */
/* -------------------------------------------------------------------- */
typedef void *OGRGeometryH;

#ifndef _DEFINED_OGRSpatialReferenceH
#define _DEFINED_OGRSpatialReferenceH

typedef void *OGRSpatialReferenceH;
typedef void *OGRCoordinateTransformationH;

#endif

struct _CPLXMLNode;

/* From base OGRGeometry class */

OGRErr CPL_DLL OGR_G_CreateFromWkb(unsigned char *, OGRSpatialReferenceH, OGRGeometryH *, int);
OGRErr CPL_DLL OGR_G_CreateFromWkt(char **, OGRSpatialReferenceH, OGRGeometryH *);
void CPL_DLL OGR_G_DestroyGeometry(OGRGeometryH);
OGRGeometryH CPL_DLL OGR_G_CreateGeometry(OGRwkbGeometryType);

int CPL_DLL OGR_G_GetDimension(OGRGeometryH);
int CPL_DLL OGR_G_GetCoordinateDimension(OGRGeometryH);
void CPL_DLL OGR_G_SetCoordinateDimension(OGRGeometryH, int);
OGRGeometryH CPL_DLL OGR_G_Clone(OGRGeometryH);
void CPL_DLL OGR_G_GetEnvelope(OGRGeometryH, OGREnvelope *);
OGRErr CPL_DLL OGR_G_ImportFromWkb(OGRGeometryH, unsigned char *, int);
OGRErr CPL_DLL OGR_G_ExportToWkb(OGRGeometryH, OGRwkbByteOrder, unsigned char *);
int CPL_DLL OGR_G_WkbSize(OGRGeometryH hGeom);
OGRErr CPL_DLL OGR_G_ImportFromWkt(OGRGeometryH, char **);
OGRErr CPL_DLL OGR_G_ExportToWkt(OGRGeometryH, char **);
OGRwkbGeometryType CPL_DLL OGR_G_GetGeometryType(OGRGeometryH);
const char CPL_DLL *OGR_G_GetGeometryName(OGRGeometryH);
void CPL_DLL OGR_G_DumpReadable(OGRGeometryH, FILE *, const char *);
void CPL_DLL OGR_G_FlattenTo2D(OGRGeometryH);
void CPL_DLL OGR_G_CloseRings(OGRGeometryH);

OGRGeometryH CPL_DLL OGR_G_CreateFromGML(const char *);
char CPL_DLL *OGR_G_ExportToGML(OGRGeometryH);

#if defined(_CPL_MINIXML_H_INCLUDED)
OGRGeometryH CPL_DLL OGR_G_CreateFromGMLTree(const CPLXMLNode *);
CPLXMLNode CPL_DLL *OGR_G_ExportToGMLTree(OGRGeometryH);
CPLXMLNode CPL_DLL *OGR_G_ExportEnvelopeToGMLTree(OGRGeometryH);
#endif

void CPL_DLL OGR_G_AssignSpatialReference(OGRGeometryH, OGRSpatialReferenceH);
OGRSpatialReferenceH CPL_DLL OGR_G_GetSpatialReference(OGRGeometryH);
OGRErr CPL_DLL OGR_G_Transform(OGRGeometryH, OGRCoordinateTransformationH);
// OGRErr CPL_DLL OGR_G_TransformTo(OGRGeometryH, OGRSpatialReferenceH);

int CPL_DLL OGR_G_Intersects(OGRGeometryH, OGRGeometryH);
int CPL_DLL OGR_G_Equals(OGRGeometryH, OGRGeometryH);
int CPL_DLL OGR_G_Disjoint(OGRGeometryH, OGRGeometryH);
int CPL_DLL OGR_G_Touches(OGRGeometryH, OGRGeometryH);
int CPL_DLL OGR_G_Crosses(OGRGeometryH, OGRGeometryH);
int CPL_DLL OGR_G_Within(OGRGeometryH, OGRGeometryH);
int CPL_DLL OGR_G_Contains(OGRGeometryH, OGRGeometryH);
int CPL_DLL OGR_G_Overlaps(OGRGeometryH, OGRGeometryH);

OGRGeometryH CPL_DLL OGR_G_GetBoundary(OGRGeometryH);
OGRGeometryH CPL_DLL OGR_G_ConvexHull(OGRGeometryH);
OGRGeometryH CPL_DLL OGR_G_Buffer(OGRGeometryH, double, int);
OGRGeometryH CPL_DLL OGR_G_Intersection(OGRGeometryH, OGRGeometryH);
OGRGeometryH CPL_DLL OGR_G_Union(OGRGeometryH, OGRGeometryH);
OGRGeometryH CPL_DLL OGR_G_Difference(OGRGeometryH, OGRGeometryH);
OGRGeometryH CPL_DLL OGR_G_SymmetricDifference(OGRGeometryH, OGRGeometryH);
double CPL_DLL OGR_G_Distance(OGRGeometryH, OGRGeometryH);

double CPL_DLL OGR_G_GetArea(OGRGeometryH);
int CPL_DLL OGR_G_Centroid(OGRGeometryH, OGRGeometryH);

void CPL_DLL OGR_G_Empty(OGRGeometryH);

/* backward compatibility */
int CPL_DLL OGR_G_Intersect(OGRGeometryH, OGRGeometryH);
int CPL_DLL OGR_G_Equal(OGRGeometryH, OGRGeometryH);

/* Methods for getting/setting vertices in points, line strings and rings */
int CPL_DLL OGR_G_GetPointCount(OGRGeometryH);
double CPL_DLL OGR_G_GetX(OGRGeometryH, int);
double CPL_DLL OGR_G_GetY(OGRGeometryH, int);
double CPL_DLL OGR_G_GetZ(OGRGeometryH, int);
void CPL_DLL OGR_G_GetPoint(OGRGeometryH, int iPoint, double *, double *, double *);
void CPL_DLL OGR_G_SetPoint(OGRGeometryH, int iPoint, double, double, double);
void CPL_DLL OGR_G_SetPoint_2D(OGRGeometryH, int iPoint, double, double);
void CPL_DLL OGR_G_AddPoint(OGRGeometryH, double, double, double);
void CPL_DLL OGR_G_AddPoint_2D(OGRGeometryH, double, double);

/* Methods for getting/setting rings and members collections */

int CPL_DLL OGR_G_GetGeometryCount(OGRGeometryH);
OGRGeometryH CPL_DLL OGR_G_GetGeometryRef(OGRGeometryH, int);
OGRErr CPL_DLL OGR_G_AddGeometry(OGRGeometryH, OGRGeometryH);
OGRErr CPL_DLL OGR_G_AddGeometryDirectly(OGRGeometryH, OGRGeometryH);
OGRErr CPL_DLL OGR_G_RemoveGeometry(OGRGeometryH, int, int);

OGRGeometryH CPL_DLL OGRBuildPolygonFromEdges(OGRGeometryH hLinesAsCollection, int bBestEffort,
                                              int bAutoClose, double dfTolerance, OGRErr *peErr);

OGRErr CPL_DLL OGRSetGenerate_DB2_V72_BYTE_ORDER(int bGenerate_DB2_V72_BYTE_ORDER);

int CPL_DLL OGRGetGenerate_DB2_V72_BYTE_ORDER(void);

/* -------------------------------------------------------------------- */
/*      Feature related (ogr_feature.h)                                 */
/* -------------------------------------------------------------------- */

typedef void *OGRFieldDefnH;
typedef void *OGRFeatureDefnH;
typedef void *OGRFeatureH;

/* OGRFieldDefn */

OGRFieldDefnH CPL_DLL OGR_Fld_Create(const char *, OGRFieldType);
void CPL_DLL OGR_Fld_Destroy(OGRFieldDefnH);

void CPL_DLL OGR_Fld_SetName(OGRFieldDefnH, const char *);
const char CPL_DLL *OGR_Fld_GetNameRef(OGRFieldDefnH);
OGRFieldType CPL_DLL OGR_Fld_GetType(OGRFieldDefnH);
void CPL_DLL OGR_Fld_SetType(OGRFieldDefnH, OGRFieldType);
OGRJustification CPL_DLL OGR_Fld_GetJustify(OGRFieldDefnH);
void CPL_DLL OGR_Fld_SetJustify(OGRFieldDefnH, OGRJustification);
int CPL_DLL OGR_Fld_GetWidth(OGRFieldDefnH);
void CPL_DLL OGR_Fld_SetWidth(OGRFieldDefnH, int);
int CPL_DLL OGR_Fld_GetPrecision(OGRFieldDefnH);
void CPL_DLL OGR_Fld_SetPrecision(OGRFieldDefnH, int);
void CPL_DLL OGR_Fld_Set(OGRFieldDefnH, const char *, OGRFieldType, int, int, OGRJustification);

const char CPL_DLL *OGR_GetFieldTypeName(OGRFieldType);

/* OGRFeatureDefn */

OGRFeatureDefnH CPL_DLL OGR_FD_Create(const char *);
void CPL_DLL OGR_FD_Destroy(OGRFeatureDefnH);
void CPL_DLL OGR_FD_Release(OGRFeatureDefnH);
const char CPL_DLL *OGR_FD_GetName(OGRFeatureDefnH);
int CPL_DLL OGR_FD_GetFieldCount(OGRFeatureDefnH);
OGRFieldDefnH CPL_DLL OGR_FD_GetFieldDefn(OGRFeatureDefnH, int);
int CPL_DLL OGR_FD_GetFieldIndex(OGRFeatureDefnH, const char *);
void CPL_DLL OGR_FD_AddFieldDefn(OGRFeatureDefnH, OGRFieldDefnH);
OGRwkbGeometryType CPL_DLL OGR_FD_GetGeomType(OGRFeatureDefnH);
void CPL_DLL OGR_FD_SetGeomType(OGRFeatureDefnH, OGRwkbGeometryType);
int CPL_DLL OGR_FD_Reference(OGRFeatureDefnH);
int CPL_DLL OGR_FD_Dereference(OGRFeatureDefnH);
int CPL_DLL OGR_FD_GetReferenceCount(OGRFeatureDefnH);

/* OGRFeature */

OGRFeatureH CPL_DLL OGR_F_Create(OGRFeatureDefnH);
void CPL_DLL OGR_F_Destroy(OGRFeatureH);
OGRFeatureDefnH CPL_DLL OGR_F_GetDefnRef(OGRFeatureH);

OGRErr CPL_DLL OGR_F_SetGeometryDirectly(OGRFeatureH, OGRGeometryH);
OGRErr CPL_DLL OGR_F_SetGeometry(OGRFeatureH, OGRGeometryH);
OGRGeometryH CPL_DLL OGR_F_GetGeometryRef(OGRFeatureH);
OGRFeatureH CPL_DLL OGR_F_Clone(OGRFeatureH);
int CPL_DLL OGR_F_Equal(OGRFeatureH, OGRFeatureH);

int CPL_DLL OGR_F_GetFieldCount(OGRFeatureH);
OGRFieldDefnH CPL_DLL OGR_F_GetFieldDefnRef(OGRFeatureH, int);
int CPL_DLL OGR_F_GetFieldIndex(OGRFeatureH, const char *);

int CPL_DLL OGR_F_IsFieldSet(OGRFeatureH, int);
void CPL_DLL OGR_F_UnsetField(OGRFeatureH, int);
OGRField CPL_DLL *OGR_F_GetRawFieldRef(OGRFeatureH, int);

int CPL_DLL OGR_F_GetFieldAsInteger(OGRFeatureH, int);
double CPL_DLL OGR_F_GetFieldAsDouble(OGRFeatureH, int);
const char CPL_DLL *OGR_F_GetFieldAsString(OGRFeatureH, int);
const int CPL_DLL *OGR_F_GetFieldAsIntegerList(OGRFeatureH, int, int *);
const double CPL_DLL *OGR_F_GetFieldAsDoubleList(OGRFeatureH, int, int *);
char CPL_DLL **OGR_F_GetFieldAsStringList(OGRFeatureH, int);
GByte CPL_DLL *OGR_F_GetFieldAsBinary(OGRFeatureH, int, int *);
int CPL_DLL OGR_F_GetFieldAsDateTime(OGRFeatureH, int, int *, int *, int *, int *, int *, int *,
                                     int *);

void CPL_DLL OGR_F_SetFieldInteger(OGRFeatureH, int, int);
void CPL_DLL OGR_F_SetFieldDouble(OGRFeatureH, int, double);
void CPL_DLL OGR_F_SetFieldString(OGRFeatureH, int, const char *);
void CPL_DLL OGR_F_SetFieldIntegerList(OGRFeatureH, int, int, int *);
void CPL_DLL OGR_F_SetFieldDoubleList(OGRFeatureH, int, int, double *);
void CPL_DLL OGR_F_SetFieldStringList(OGRFeatureH, int, char **);
void CPL_DLL OGR_F_SetFieldRaw(OGRFeatureH, int, OGRField *);
void CPL_DLL OGR_F_SetFieldBinary(OGRFeatureH, int, int, GByte *);
void CPL_DLL OGR_F_SetFieldDateTime(OGRFeatureH, int, int, int, int, int, int, int, int);

long CPL_DLL OGR_F_GetFID(OGRFeatureH);
OGRErr CPL_DLL OGR_F_SetFID(OGRFeatureH, long);
void CPL_DLL OGR_F_DumpReadable(OGRFeatureH, FILE *);
OGRErr CPL_DLL OGR_F_SetFrom(OGRFeatureH, OGRFeatureH, int);

const char CPL_DLL *OGR_F_GetStyleString(OGRFeatureH);
void CPL_DLL OGR_F_SetStyleString(OGRFeatureH, const char *);
void CPL_DLL OGR_F_SetStyleStringDirectly(OGRFeatureH, char *);

/* -------------------------------------------------------------------- */
/*      ogrsf_frmts.h                                                   */
/* -------------------------------------------------------------------- */

typedef void *OGRLayerH;
typedef void *OGRDataSourceH;
typedef void *OGRSFDriverH;

/* OGRLayer */

OGRGeometryH CPL_DLL OGR_L_GetSpatialFilter(OGRLayerH);
void CPL_DLL OGR_L_SetSpatialFilter(OGRLayerH, OGRGeometryH);
void CPL_DLL OGR_L_SetSpatialFilterRect(OGRLayerH, double, double, double, double);
OGRErr CPL_DLL OGR_L_SetAttributeFilter(OGRLayerH, const char *);
void CPL_DLL OGR_L_ResetReading(OGRLayerH);
OGRFeatureH CPL_DLL OGR_L_GetNextFeature(OGRLayerH);
OGRErr CPL_DLL OGR_L_SetNextByIndex(OGRLayerH, long);
OGRFeatureH CPL_DLL OGR_L_GetFeature(OGRLayerH, long);
OGRErr CPL_DLL OGR_L_SetFeature(OGRLayerH, OGRFeatureH);
OGRErr CPL_DLL OGR_L_CreateFeature(OGRLayerH, OGRFeatureH);
OGRErr CPL_DLL OGR_L_DeleteFeature(OGRLayerH, long);
OGRFeatureDefnH CPL_DLL OGR_L_GetLayerDefn(OGRLayerH);
OGRSpatialReferenceH CPL_DLL OGR_L_GetSpatialRef(OGRLayerH);
int CPL_DLL OGR_L_GetFeatureCount(OGRLayerH, int);
OGRErr CPL_DLL OGR_L_GetExtent(OGRLayerH, OGREnvelope *, int);
int CPL_DLL OGR_L_TestCapability(OGRLayerH, const char *);
OGRErr CPL_DLL OGR_L_CreateField(OGRLayerH, OGRFieldDefnH, int);
OGRErr CPL_DLL OGR_L_StartTransaction(OGRLayerH);
OGRErr CPL_DLL OGR_L_CommitTransaction(OGRLayerH);
OGRErr CPL_DLL OGR_L_RollbackTransaction(OGRLayerH);
int CPL_DLL OGR_L_Reference(OGRLayerH);
int CPL_DLL OGR_L_Dereference(OGRLayerH);
int CPL_DLL OGR_L_GetRefCount(OGRLayerH);
OGRErr CPL_DLL OGR_L_SyncToDisk(OGRLayerH);
GIntBig CPL_DLL OGR_L_GetFeaturesRead(OGRLayerH);
const char CPL_DLL *OGR_L_GetFIDColumn(OGRLayerH);
const char CPL_DLL *OGR_L_GetGeometryColumn(OGRLayerH);
/* OGRDataSource */

void CPL_DLL OGR_DS_Destroy(OGRDataSourceH);
const char CPL_DLL *OGR_DS_GetName(OGRDataSourceH);
int CPL_DLL OGR_DS_GetLayerCount(OGRDataSourceH);
OGRLayerH CPL_DLL OGR_DS_GetLayer(OGRDataSourceH, int);
OGRLayerH CPL_DLL OGR_DS_GetLayerByName(OGRDataSourceH, const char *);
OGRErr CPL_DLL OGR_DS_DeleteLayer(OGRDataSourceH, int);
OGRSFDriverH CPL_DLL OGR_DS_GetDriver(OGRDataSourceH);
OGRLayerH CPL_DLL OGR_DS_CreateLayer(OGRDataSourceH, const char *, OGRSpatialReferenceH,
                                     OGRwkbGeometryType, char **);
OGRLayerH CPL_DLL OGR_DS_CopyLayer(OGRDataSourceH, OGRLayerH, const char *, char **);
int CPL_DLL OGR_DS_TestCapability(OGRDataSourceH, const char *);
OGRLayerH CPL_DLL OGR_DS_ExecuteSQL(OGRDataSourceH, const char *, OGRGeometryH, const char *);
void CPL_DLL OGR_DS_ReleaseResultSet(OGRDataSourceH, OGRLayerH);
int CPL_DLL OGR_DS_Reference(OGRDataSourceH);
int CPL_DLL OGR_DS_Dereference(OGRDataSourceH);
int CPL_DLL OGR_DS_GetRefCount(OGRDataSourceH);
int CPL_DLL OGR_DS_GetSummaryRefCount(OGRDataSourceH);
OGRErr CPL_DLL OGR_DS_SyncToDisk(OGRDataSourceH);

/* OGRSFDriver */

const char CPL_DLL *OGR_Dr_GetName(OGRSFDriverH);
OGRDataSourceH CPL_DLL OGR_Dr_Open(OGRSFDriverH, const char *, int);
int CPL_DLL OGR_Dr_TestCapability(OGRSFDriverH, const char *);
OGRDataSourceH CPL_DLL OGR_Dr_CreateDataSource(OGRSFDriverH, const char *, char **);
OGRDataSourceH CPL_DLL OGR_Dr_CopyDataSource(OGRSFDriverH, OGRDataSourceH, const char *, char **);
OGRErr CPL_DLL OGR_Dr_DeleteDataSource(OGRSFDriverH, const char *);

/* OGRSFDriverRegistrar */

OGRDataSourceH CPL_DLL OGROpen(const char *, int, OGRSFDriverH *);
OGRDataSourceH CPL_DLL OGROpenShared(const char *, int, OGRSFDriverH *);
OGRErr CPL_DLL OGRReleaseDataSource(OGRDataSourceH);
void CPL_DLL OGRRegisterDriver(OGRSFDriverH);
int CPL_DLL OGRGetDriverCount(void);
OGRSFDriverH CPL_DLL OGRGetDriver(int);
OGRSFDriverH CPL_DLL OGRGetDriverByName(const char *);
int CPL_DLL OGRGetOpenDSCount(void);
OGRDataSourceH CPL_DLL OGRGetOpenDS(int iDS);

/* note: this is also declared in ogrsf_frmts.h */
void CPL_DLL OGRRegisterAll(void);
void CPL_DLL OGRCleanupAll(void);

CPL_C_END

#endif /* ndef _OGR_API_H_INCLUDED */
