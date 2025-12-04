/******************************************************************************
 * shp2wkt.h
 *
 * v 1.0 2013/04/28
 *
 * Project:  Shapelib
 * Purpose:  Shapelib extending functions
 * Author:   cheungmine@gmail.com
 * 2013/04/28
 *
 * Copyright (c) 2013, cheungmine
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

/*
 * Well-known text (WKT) is a text markup language for representing vector
 * geometry objects on a map, spatial reference systems of spatial objects
 * and transformations between spatial reference systems. A binary equivalent,
 * known as well-known binary (WKB) is used to transfer and store the same
 * information on databases, such as PostGIS, Microsoft SQL Server and DB2.
 * The formats were originally defined by the Open Geospatial Consortium (OGC)
 * and described in their Simple Feature Access and Coordinate Transformation
 * Service specifications. The current standard definition is in the ISO/IEC
 * 13249-3:2011 standard, "Information technology -- Database languages --
 * SQL multimedia and application packages -- Part 3: Spatial" (SQL/MM).
 *
 * reference:
 *   http://en.wikipedia.org/wiki/Well-known_text
 *
 * POINT(6 10)
 * LINESTRING(3 4,10 50,20 25)
 * POLYGON((1 1,5 1,5 5,1 5,1 1),(2 2,2 3,3 3,3 2,2 2))
 * MULTIPOINT(3.5 5.6, 4.8 10.5)
 * MULTILINESTRING((3 4,10 50,20 25),(-5 -8,-10 -8,-15 -4))
 * MULTIPOLYGON(((1 1,5 1,5 5,1 5,1 1),(2 2,2 3,3 3,3 2,2 2)),((6 3,9 2,9 4,6 3)))
 * GEOMETRYCOLLECTION(POINT(4 6),LINESTRING(4 6,7 10))
 * POINT ZM (1 1 5 60)
 * POINT M (1 1 80)
 * POINT EMPTY
 * MULTIPOLYGON EMPTY
 */

#ifndef _SHP2WKT_H_INCLUDED
#define _SHP2WKT_H_INCLUDED

#ifdef _MSC_VER
 /* disable annoying warning: 4996 */
 #pragma warning (disable : 4996)
#endif

#include "shapefile_i.h"


int Point2WKT(const SHPObject *pObj, char *pbBuf, double offX, double offY, int dig);

int Arc2WKT(const SHPObject *pObj, char *pbBuf, double offX, double offY, int dig);

int Polygon2WKT(const SHPObject *pObj, char *pbBuf, double offX, double offY, int dig);

int MultiPoint2WKT(const SHPObject *pObj, char *pbBuf, double offX, double offY, int dig);

int PointZ2WKT(const SHPObject *pObj, char *pbBuf, double offX, double offY, double offZ, int dig, int digZ);

int ArcZ2WKT(const SHPObject *pObj, char *pbBuf, double offX, double offY, double offZ, int dig, int digZ);

int PolygonZ2WKT(const SHPObject *pObj, char *pbBuf, double offX, double offY, double offZ, int dig, int digZ);

int MultiPointZ2WKT(const SHPObject *pObj, char *pbBuf, double offX, double offY, double offZ, int dig, int digZ);

int PointM2WKT(const SHPObject *pObj, char *pbBuf, double offX, double offY, double offM, int dig, int digM);

int ArcM2WKT(const SHPObject *pObj, char *pbBuf, double offX, double offY, double offM, int dig, int digM);

int PolygonM2WKT(const SHPObject *pObj, char *pbBuf, double offX, double offY, double offM, int dig, int digM);

int MultiPointM2WKT(const SHPObject *pObj, char *pbBuf, double offX, double offY, double offM, int dig, int digM);

int exPoint2WKT(const SHPObjectEx *pObj, char *pbBuf, double offX, double offY, int dig);

int exPointZ2WKT(const SHPObjectEx *pObj, char *pbBuf, double offX, double offY, double offZ, int dig, int digZ);

int exPointM2WKT(const SHPObjectEx *pObj, char *pbBuf, double offX, double offY, double offM, int dig, int digM);

int exMultiPoint2WKT(const SHPObjectEx *pObj, char *pbBuf, double offX, double offY, int dig);

int exMultiPointZ2WKT(const SHPObjectEx *pObj, char *pbBuf, double offX, double offY, double offZ, int dig, int digZ);

int exMultiPointM2WKT(const SHPObjectEx *pObj, char *pbBuf, double offX, double offY, double offM, int dig, int digM);

int exArc2WKT(const SHPObjectEx *pObj, char *pbBuf, double offX, double offY, int dig);

int exArcZ2WKT(const SHPObjectEx *pObj, char *pbBuf, double offX, double offY, double offZ, int dig, int digZ);

int exArcM2WKT(const SHPObjectEx *pObj, char *pbBuf, double offX, double offY, double offM, int dig, int digM);

int exPolygon2WKT(const SHPObjectEx *pObj, char *pbBuf, double offX, double offY, int dig);

int exPolygonZ2WKT(const SHPObjectEx *pObj, char *pbBuf, double offX, double offY, double offZ, int dig, int digZ);

int exPolygonM2WKT(const SHPObjectEx *pObj, char *pbBuf, double offX, double offY, double offM, int dig, int digM);

int MultiPatch2WKT(const SHPObject *pObj, char *pbBuf, double offX, double offY, int dig);

int exMultiPatch2WKT(const SHPObjectEx *pObj, char *pbBuf, double offX, double offY, int dig);

#endif /* _SHP2WKT_H_INCLUDED */
