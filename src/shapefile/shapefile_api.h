/**
 * @file shapefile_api.h
 * @brief shapefile public api.
 *
 * @author cheungmine@qq.com
 * @copyright © 2024-2025 anomura.cc, All Rights Reserved.
 * @version 0.0.2
 * @date 2025-12-05 02:23:33
 * @note
 * @since 2024-10-10 20:24:42
 */

/******************************************************************************
 * v 1.4 2005/03/24 14:26:44
 *
 * Project:  Shapelib
 * Purpose:  Primary include file for Shapelib.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ** Last modified: cheungmine
 *
 * Copyright (c) 1999, Frank Warmerdam
 *
 * This software is available under the following "MIT Style" license,
 * or at the option of the licensee under the LGPL (see LICENSE.LGPL).  This
 * option is discussed in more detail in shapelib.html.
 *
 * --
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
 *****************************************************************************/
#ifndef SHAPEFILE_API_H_PUBLIC
#define SHAPEFILE_API_H_PUBLIC

#ifdef    __cplusplus
extern "C" {
#endif

#include "shapefile_def.h"


SHAPEFILE_API const char * shapefile_lib_version(const char **_libname);

/* -------------------------------------------------------------------- */
/*      SHP API Prototypes                                              */
/* -------------------------------------------------------------------- */
SHAPEFILE_API SHPHandle SHPOpen (const char *pszShapeFile, const char *pszAccess);

SHAPEFILE_API SHPHandle SHPCreate (const char *pszShapeFile, int nShapeType);

SHAPEFILE_API void SHPGetInfo (SHPHandle hSHP, int *pnEntities, int *pnShapeType, double *padfMinBound, double *padfMaxBound);

SHAPEFILE_API int SHPGetType (SHPHandle hSHP, int *bHasZ, int *bHasM);

SHAPEFILE_API SHPObject* SHPReadObject (SHPHandle hSHP, int iShape);

SHAPEFILE_API int SHPReadObjectEx (SHPHandle psSHP, int iShape, SHPObjectEx *psShape);

SHAPEFILE_API int SHPReadObjectBounds (SHPHandle hSHP, int iShape, SHPBounds *Bounds, double *pointEpsilon);

SHAPEFILE_API int SHPReadObjectEnvelope (SHPHandle hSHP, int iShape, SHPEnvelope *rect, double *pointEpsilon);

SHAPEFILE_API int SHPWriteObject (SHPHandle hSHP, int iShape, SHPObject *psObject);

SHAPEFILE_API void SHPDestroyObject (SHPObject * psObject);

SHAPEFILE_API SHPObjectEx* SHPCreateObjectEx (SHPObjectEx ** ppsObject);

SHAPEFILE_API void SHPDestroyObjectEx (SHPObjectEx *psObject);

SHAPEFILE_API void SHPComputeExtents (SHPObject *psObject);

SHAPEFILE_API SHPObject* SHPCreateObject (int nSHPType, int nShapeId,
    int nParts, const int *panPartStart, const int *panPartType,
    int nVertices, const double *padfX, const double *padfY, const double *padfZ, const double *padfM);

SHAPEFILE_API SHPObject* SHPCreateObject2 (int nSHPType, int nShapeId,
    int nParts, const int *panPartStart, const int *panPartType,
    int nVertices, const SHPPointType *padXY, const double *padfZ, const double *padfM);

SHAPEFILE_API SHPObject* SHPCreateSimpleObject (int nSHPType,
    int nVertices, const double *padfX, const double *padfY, const double *padfZ);

SHAPEFILE_API int SHPRewindObject (SHPObject *psObject);

SHAPEFILE_API int SHPRewindObjectEx (SHPObjectEx *psObjectEx);

SHAPEFILE_API void SHPClose (SHPHandle hSHP);

SHAPEFILE_API void SHPWriteHeader (SHPHandle hSHP);

SHAPEFILE_API const char* SHPTypeName (int nSHPType);

SHAPEFILE_API const char* SHPPartTypeName (int nPartType);

SHAPEFILE_API double SHPLengthOfXYs (double *padfX, double *padfY, int start, int end);

SHAPEFILE_API double SHPLengthOfPoints (SHPPointType *pPoints, int start, int end);

SHAPEFILE_API double SHPAreaOfXYs (double *padfX, double *padfY, int start, int end, int *CCW);

SHAPEFILE_API double SHPAreaOfPoints (SHPPointType *pPoints, int start, int end, int *CCW);

SHAPEFILE_API double SHPObjectExGetLength (const SHPObjectEx *psObject);

/**
 * SHPObjectExGetArea
 *   get area of polygon object
 * Returns:
 *   > 0: CCW, Counter Clock Wise, RHR = Right Hand Rule
 *   < 0: CW, Clock Wise, LHR = Left Hand Rule
 */
SHAPEFILE_API double SHPObjectExGetArea (const SHPObjectEx *psObject);

SHAPEFILE_API double SHPObjectGetLength (const SHPObject *psObject);

/**
 * SHPObjectGetArea
 *   get area of polygon object
 * Returns:
 *   > 0: CCW, Counter Clock Wise, RHR = Right Hand Rule
 *   < 0: CW, Clock Wise, LHR = Left Hand Rule
 */
SHAPEFILE_API double SHPObjectGetArea (const SHPObject *psObject);

SHAPEFILE_API void SHPObjectReversePoints (SHPObject *psObject);

SHAPEFILE_API void SHPObjectExReversePoints (SHPObjectEx *psObject);

/**
 * SHPObjectValidatePolygon
*   make points of polygon as given isCCW flag
 * Parameters:
 *   isCCW - 1 for CCW; 0 for CW
 * Returns:
 *   = 1: done
 *   = 0: no operation
 *   = -1: error type
 */
SHAPEFILE_API int SHPObjectValidatePolygon (SHPObject *psObject, int isCCW);

/**
 * SHPObjectExValidatePolygon
 *   make points of polygon as given isCCW flag
 * Parameters:
 *   isCCW - 1 for CCW; 0 for CW
 * Returns:
 *   = 1: make points is done
 *   = 0: no operation
 *   = -1: error type
 */
SHAPEFILE_API int SHPObjectExValidatePolygon (SHPObjectEx *psObject, int isCCW);

SHAPEFILE_API int SHPObject2WKB (const SHPObject *psObject, void *wkbBuffer,
    double offsetX, double offsetY, double offsetZ, double offsetM);

SHAPEFILE_API int SHPObject2WKT (const SHPObject *psObject, char *wktBuffer,
    double offsetX, double offsetY, double offsetZ, double offsetM,
    int nDecimalsXY, int nDecimalsZ, int nDecimalsM);

SHAPEFILE_API int SHPObjectEx2WKB (const SHPObjectEx *psObject, void *wkbBuffer,
    double offsetX, double offsetY, double offsetZ, double offsetM);

SHAPEFILE_API int SHPObjectEx2WKT (const SHPObjectEx *psObject, char *wktBuffer,
    double offsetX, double offsetY, double offsetZ, double offsetM,
    int nDecimalsXY, int nDecimalsZ, int nDecimalsM);


/*************************************************************************
 *                             SHPTree Index API
 ************************************************************************/

SHAPEFILE_API SHPTreeHandle SHPCreateTree (SHPHandle hSHP, int nDimension, int nMaxDepth, double *padfBoundsMin, double *padfBoundsMax);

SHAPEFILE_API void  SHPDestroyTree (SHPTreeHandle hTree);

SHAPEFILE_API int SHPTreeAddShapeId (SHPTreeHandle hTree, SHPObject *psObject);

SHAPEFILE_API void  SHPTreeTrimExtraNodes (SHPTreeHandle hTree);

SHAPEFILE_API int*  SHPTreeFindLikelyShapes (SHPTreeHandle hTree, double *padfBoundsMin, double *padfBoundsMax, int *pnShapeCount);

SHAPEFILE_API int SHPCheckBoundsOverlap (double *padfBox1Min, double *padfBox1Max, double *padfBox2Min, double *padfBox2Max, int nDimension);


/*************************************************************************
 *                             SHAPES MBR Tree API
 ************************************************************************/
SHAPEFILE_API void SHPMBRTreeReset (SHPHandle hSHP, int bClose);

SHAPEFILE_API SHPMBRTree SHPGetMBRTree (SHPHandle hSHP);

SHAPEFILE_API int SHPMBRTreeAddShape(SHPMBRTree rtree, const SHPEnvelope *shapeEnv, void *shapeData, int treeLevel);

SHAPEFILE_API int SHPMBRTreeSearch (SHPMBRTree rtree, const SHPEnvelope *searchEnv, int(* onSearchShape)(void * shapeData,  void *userParam), void *userParam);


/*************************************************************************
 *                             DBF API
 ************************************************************************/

SHAPEFILE_API DBFHandle DBFOpen (const char * pszDBFFile, const char * pszAccess);

SHAPEFILE_API DBFHandle DBFCreate (const char * pszDBFFile);

SHAPEFILE_API int DBFGetFieldCount (DBFHandle psDBF);

SHAPEFILE_API int DBFGetRecordCount (DBFHandle psDBF);

SHAPEFILE_API int DBFAddField (DBFHandle hDBF, const char *pszFieldName, DBFFieldType eType, int nWidth, int nDecimals);

SHAPEFILE_API DBFFieldType DBFGetFieldInfo (DBFHandle psDBF, int iField, char *pszFieldName, int *pnWidth, int *pnDecimals);

SHAPEFILE_API void DBFGetFieldInfo2 (DBFHandle psDBF, int iField, DBFFieldInfo *pFieldInfo);

SHAPEFILE_API int DBFGetFieldIndex (DBFHandle psDBF, const char *pszFieldName);

SHAPEFILE_API int DBFReadIntegerAttribute (DBFHandle hDBF, int iShape, int iField);

SHAPEFILE_API double DBFReadDoubleAttribute (DBFHandle hDBF, int iShape, int iField);

SHAPEFILE_API const char* DBFReadStringAttribute (DBFHandle hDBF, int iShape, int iField);

SHAPEFILE_API int DBFReadCopyStringAttribute (DBFHandle hDBF, int iShape, int iField, char *buffer);

SHAPEFILE_API const char* DBFReadLogicalAttribute (DBFHandle hDBF, int iShape, int iField);

SHAPEFILE_API int DBFIsAttributeNULL (DBFHandle hDBF, int iShape, int iField );

SHAPEFILE_API int DBFWriteIntegerAttribute (DBFHandle hDBF, int iShape, int iField, int nFieldValue);

SHAPEFILE_API int DBFWriteDoubleAttribute (DBFHandle hDBF, int iShape, int iField, double dFieldValue);

SHAPEFILE_API int DBFWriteStringAttribute (DBFHandle hDBF, int iShape, int iField, const char * pszFieldValue);

SHAPEFILE_API int DBFWriteLogicalAttribute (DBFHandle hDBF, int iShape, int iField, const char lFieldValue);

SHAPEFILE_API int DBFWriteAttributeDirectly (DBFHandle psDBF, int hEntity, int iField, void * pValue);

SHAPEFILE_API const char* DBFReadTuple (DBFHandle psDBF, int hEntity);

SHAPEFILE_API int DBFWriteTuple (DBFHandle psDBF, int hEntity, void *pRawTuple);

SHAPEFILE_API DBFHandle DBFCloneEmpty (DBFHandle psDBF, const char * pszFilename);

SHAPEFILE_API void  DBFClose (DBFHandle hDBF);

SHAPEFILE_API void  DBFUpdateHeader (DBFHandle hDBF);

SHAPEFILE_API char DBFGetNativeFieldType (DBFHandle hDBF, int iField);

#ifdef    __cplusplus
}
#endif

#endif /* SHAPEFILE_API_H_PUBLIC */