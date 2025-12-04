/******************************************************************************
 * shp2wkb.h
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
 */

#ifndef _SHP2WKB_H_INCLUDED
#define _SHP2WKB_H_INCLUDED

#ifdef _MSC_VER
 /* disable annoying warning: 4996 */
#   pragma warning (disable : 4996)
#endif

#include "shapefile_i.h"


#define WriteWkbBoXdr(dstaddr, offcb) do { \
        *((ub1*)(dstaddr) + offcb) = WKB_BYTEORDER_XDR; \
        offcb++; \
    } while(0)


#define ValBufCopy(dstaddr, offcb, src) do { \
        memcpy((ub1*)(dstaddr) + offcb, &(src), sizeof(src)); \
        offcb += sizeof(src); \
    } while(0)


#define ValBufOffset(dstaddr, offcb, src)  offcb += sizeof(src)

#define ValBufOffsetN(dstaddr, offcb, src, num)  offcb += (sizeof(src)*(num))

#define WKBHeaderSize    (sizeof(ub1) + sizeof(ub4))

/*************************** Well-known Binary (WKB) *************************/

#define WKBPointSize(num)     (num)*(WKBHeaderSize + 2*sizeof(double))
#define WKBPointZSize(num)    (num)*(WKBHeaderSize + 3*sizeof(double))
#define WKBPointMSize(num)    WKBPointZSize(num)
#define WKBPointZMSize(num)   (num)*(WKBHeaderSize + 4*sizeof(double))

int Point2WKB(const SHPObject *pObj, void *pv, double offX, double offY);

#define WKBLineStringSize(np)   (WKBHeaderSize + sizeof(ub4) + (np)*2*sizeof(double))

#define WKBLineStringZSize(np)  (WKBHeaderSize + sizeof(ub4) + (np)*3*sizeof(double))

#define WKBLineStringMSize(np)  WKBLineStringZSize(np)


int Arc2WKB(const SHPObject *pObj, void *pv, double offX, double offY);

int Polygon2WKB(const SHPObject *pObj, void *pv, double offX, double offY);

int MultiPoint2WKB(const SHPObject *pObj, void *pv, double offX, double offY);

int PointZ2WKB(const SHPObject *pObj, void *pv, double offX, double offY, double offZ);

int ArcZ2WKB(const SHPObject *pObj, void *pv, double offX, double offY, double offZ);

int PolygonZ2WKB(const SHPObject *pObj, void *pv, double offX, double offY, double offZ);

int MultiPointZ2WKB(const SHPObject *pObj, void *pv, double offX, double offY, double offZ);

int PointM2WKB(const SHPObject *pObj, void *pv, double offX, double offY, double offM);

int ArcM2WKB(const SHPObject *pObj, void *pv, double offX, double offY, double offM);

int PolygonM2WKB(const SHPObject *pObj, void *pv, double offX, double offY, double offM);

int MultiPointM2WKB(const SHPObject *pObj, void *pv, double offX, double offY, double offM);

int exPoint2WKB(const SHPObjectEx *pObj, void *pv, double offX, double offY);

int exArc2WKB(const SHPObjectEx *pObj, void *pv, double offX, double offY);

int exPolygon2WKB(const SHPObjectEx *pObj, void *pv, double offX, double offY);

int exMultiPoint2WKB(const SHPObjectEx *pObj, void *pv, double offX, double offY);

int exPointZ2WKB(const SHPObjectEx *pObj, void *pv, double offX, double offY, double offZ);

int exArcZ2WKB(const SHPObjectEx *pObj, void *pv, double offX, double offY, double offZ);

int exPolygonZ2WKB(const SHPObjectEx *pObj, void *pv, double offX, double offY, double offZ);

int exMultiPointZ2WKB(const SHPObjectEx *pObj, void *pv, double offX, double offY, double offZ);

int exPointM2WKB(const SHPObjectEx *pObj, void *pv, double offX, double offY, double offM);

int exArcM2WKB(const SHPObjectEx *pObj, void *pv, double offX, double offY, double offM);

int exPolygonM2WKB(const SHPObjectEx *pObj, void *pv, double offX, double offY, double offM);

int exMultiPointM2WKB(const SHPObjectEx *pObj, void *pv, double offX, double offY, double offM);

int MultiPatch2WKB(const SHPObject *pObj, void *pv, double offX, double offY);

int exMultiPatch2WKB(const SHPObjectEx *pObj, void *pv, double offX, double offY);

#endif /* _SHP2WKB_H_INCLUDED */
