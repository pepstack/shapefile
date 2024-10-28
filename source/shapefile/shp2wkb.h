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


static ub1 wkb_bo_xdr = WKB_BYTEORDER_XDR;


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

static int Point2WKB (const SHPObject *pObj, void *pv, double offX, double offY)
{
    ub4 v4;
    double v8;
    int cb = 0;

    if (pv) {
        ValBufCopy(pv, cb, wkb_bo_xdr);

        v4 = BO_i32_htobe(WKB_Point2D);
        ValBufCopy(pv, cb, v4);

        v8 = BO_f64_htobe(*pObj->padfX + offX);
        ValBufCopy(pv, cb, v8);

        v8 = BO_f64_htobe(*pObj->padfY + offY);
        ValBufCopy(pv, cb, v8);
    } else {
        cb = WKBPointSize(1);
    }

    return cb;
}


#define WKBLineStringSize(np)   (WKBHeaderSize + sizeof(ub4) + (np)*2*sizeof(double))

#define WKBLineStringZSize(np)  (WKBHeaderSize + sizeof(ub4) + (np)*3*sizeof(double))

#define WKBLineStringMSize(np)  WKBLineStringZSize(np)


static int Arc2WKB (const SHPObject *pObj, void *pv, double offX, double offY)
{
    ub4 v4;
    double   v8;
    int cb = 0;

    if (pv) {
        int iPoint = 0;

        /* byte order flag */
        ValBufCopy(pv, cb, wkb_bo_xdr);

        /* wkb type */
        v4 = BO_i32_htobe(WKB_LineString2D);
        ValBufCopy(pv, cb, v4);

        /* num of points */
        v4 = BO_i32_htobe(pObj->nVertices);
        ValBufCopy(pv, cb, v4);

        for (; iPoint < pObj->nVertices; iPoint++) {
            v8 = BO_f64_htobe(pObj->padfX[iPoint] + offX);
            ValBufCopy(pv, cb, v8);

            v8 = BO_f64_htobe(pObj->padfY[iPoint] + offY);
            ValBufCopy(pv, cb, v8);
        }
    } else {
        cb = WKBLineStringSize(pObj->nVertices);
    }

    return cb;
}


static int Polygon2WKB (const SHPObject *pObj, void *pv, double offX, double offY)
{
    ub4 v4;
    double   v8;
    int cb = 0;

    if (pv) {
        int iPart = 0;

        /* byte order flag */
        ValBufCopy(pv, cb, wkb_bo_xdr);

        /* wkb type */
        v4 = BO_i32_htobe(WKB_Polygon2D);
        ValBufCopy(pv, cb, v4);

        /* num of rings */
        v4 = BO_i32_htobe(pObj->nParts);
        ValBufCopy(pv, cb, v4);

        for (; iPart < pObj->nParts; iPart++) {
            int p = pObj->panPartStart[iPart];
            int p1 = pObj->panPartStart[iPart+1];

            /* num of points */
            v4 = BO_i32_htobe(p1 - p);
            ValBufCopy(pv, cb, v4);

            for (; p < p1; p++) {
                v8 = BO_f64_htobe(pObj->padfX[p] + offX);
                ValBufCopy(pv, cb, v8);

                v8 = BO_f64_htobe(pObj->padfY[p] + offY);
                ValBufCopy(pv, cb, v8);
            }
        }
    } else {
        int iPart = 0;

        cb = WKBHeaderSize;

        /* num of rings */
        ValBufOffset(pv, cb, v4);

        for (; iPart < pObj->nParts; iPart++) {
            int np = pObj->panPartStart[iPart+1] - pObj->panPartStart[iPart];

            /* num of points */
            ValBufOffset(pv, cb, v4);

            ValBufOffsetN(pv, cb, v8, np);
            ValBufOffsetN(pv, cb, v8, np);
        }
    }

    return cb;
}


static int MultiPoint2WKB (const SHPObject *pObj, void *pv, double offX, double offY)
{
    ub4 v4;
    double v8;
    int cb = 0;

    if (pv) {
        int iPoint = 0;
        ValBufCopy(pv, cb, wkb_bo_xdr);

        v4 = BO_i32_htobe(WKB_MultiPoint2D);
        ValBufCopy(pv, cb, v4);

        v4 = BO_i32_htobe(pObj->nVertices);
        ValBufCopy(pv, cb, v4);

        for (; iPoint < pObj->nVertices; iPoint++) {
            ValBufCopy(pv, cb, wkb_bo_xdr);

            v4 = BO_i32_htobe(WKB_Point2D);
            ValBufCopy(pv, cb, v4);

            v8 = BO_f64_htobe(pObj->padfX[iPoint] + offX);
            ValBufCopy(pv, cb, v8);

            v8 = BO_f64_htobe(pObj->padfY[iPoint] + offY);
            ValBufCopy(pv, cb, v8);
        }
    } else {
        cb = WKBHeaderSize + sizeof(v4) + WKBPointSize(pObj->nVertices);
    }

    return cb;
}


static int PointZ2WKB (const SHPObject *pObj, void *pv, double offX, double offY, double offZ)
{
    ub4 v4;
    double v8;
    int cb = 0;

    if (pv) {
        ValBufCopy(pv, cb, wkb_bo_xdr);

        v4 = BO_i32_htobe(WKB_PointZ);
        ValBufCopy(pv, cb, v4);

        v8 = BO_f64_htobe(*pObj->padfX + offX);
        ValBufCopy(pv, cb, v8);

        v8 = BO_f64_htobe(*pObj->padfY + offY);
        ValBufCopy(pv, cb, v8);

        v8 = BO_f64_htobe(*pObj->padfZ + offZ);
        ValBufCopy(pv, cb, v8);
    } else {
        cb = WKBPointZSize(1);
    }

    return cb;
}


static int ArcZ2WKB (const SHPObject *pObj, void *pv, double offX, double offY, double offZ)
{
    ub4 v4;
    double   v8;
    int cb = 0;

    if (pv) {
        int iPoint = 0;

        /* byte order flag */
        ValBufCopy(pv, cb, wkb_bo_xdr);

        /* wkb type */
        v4 = BO_i32_htobe(WKB_LineStringZ);
        ValBufCopy(pv, cb, v4);

        /* num of points */
        v4 = BO_i32_htobe(pObj->nVertices);
        ValBufCopy(pv, cb, v4);

        for (; iPoint < pObj->nVertices; iPoint++) {
            v8 = BO_f64_htobe(pObj->padfX[iPoint] + offX);
            ValBufCopy(pv, cb, v8);

            v8 = BO_f64_htobe(pObj->padfY[iPoint] + offY);
            ValBufCopy(pv, cb, v8);

            v8 = BO_f64_htobe(pObj->padfZ[iPoint] + offZ);
            ValBufCopy(pv, cb, v8);
        }
    } else {
        cb = WKBLineStringZSize(pObj->nVertices);
    }

    return cb;
}


static int PolygonZ2WKB (const SHPObject *pObj, void *pv, double offX, double offY, double offZ)
{
    ub4 v4;
    double   v8;
    int cb = 0;

    if (pv) {
        int iPart = 0;

        /* byte order flag */
        ValBufCopy(pv, cb, wkb_bo_xdr);

        /* wkb type */
        v4 = BO_i32_htobe(WKB_PolygonZ);
        ValBufCopy(pv, cb, v4);

        /* num of rings */
        v4 = BO_i32_htobe(pObj->nParts);
        ValBufCopy(pv, cb, v4);

        for (; iPart < pObj->nParts; iPart++) {
            int p = pObj->panPartStart[iPart];
            int p1 = pObj->panPartStart[iPart+1];

            /* num of points */
            v4 = BO_i32_htobe(p1 - p);
            ValBufCopy(pv, cb, v4);

            for (; p < p1; p++) {
                v8 = BO_f64_htobe(pObj->padfX[p] + offX);
                ValBufCopy(pv, cb, v8);

                v8 = BO_f64_htobe(pObj->padfY[p] + offY);
                ValBufCopy(pv, cb, v8);

                v8 = BO_f64_htobe(pObj->padfZ[p] + offZ);
                ValBufCopy(pv, cb, v8);
            }
        }
    } else {
        int iPart = 0;

        /* byte order flag */
        ValBufOffset(pv, cb, wkb_bo_xdr);

        /* wkb type */
        ValBufOffset(pv, cb, v4);

        /* num of rings */
        ValBufOffset(pv, cb, v4);

        for (; iPart < pObj->nParts; iPart++) {
            int np = pObj->panPartStart[iPart+1] - pObj->panPartStart[iPart];

            /* num of points */
            ValBufOffset(pv, cb, v4);

            ValBufOffsetN(pv, cb, v8, np);
            ValBufOffsetN(pv, cb, v8, np);
            ValBufOffsetN(pv, cb, v8, np);
        }
    }

    return cb;
}


static int MultiPointZ2WKB (const SHPObject *pObj, void *pv, double offX, double offY, double offZ)
{
    ub4 v4;
    double v8;
    int cb = 0;

    if (pv) {
        int iPoint = 0;
        ValBufCopy(pv, cb, wkb_bo_xdr);

        v4 = BO_i32_htobe(WKB_MultiPointZ);
        ValBufCopy(pv, cb, v4);

        v4 = BO_i32_htobe(pObj->nVertices);
        ValBufCopy(pv, cb, v4);

        for (; iPoint < pObj->nVertices; iPoint++) {
            ValBufCopy(pv, cb, wkb_bo_xdr);

            v4 = BO_i32_htobe(WKB_PointZ);
            ValBufCopy(pv, cb, v4);

            v8 = BO_f64_htobe(pObj->padfX[iPoint] + offX);
            ValBufCopy(pv, cb, v8);

            v8 = BO_f64_htobe(pObj->padfY[iPoint] + offY);
            ValBufCopy(pv, cb, v8);

            v8 = BO_f64_htobe(pObj->padfZ[iPoint] + offZ);
            ValBufCopy(pv, cb, v8);
        }
    } else {
        cb = WKBHeaderSize + sizeof(v4) + WKBPointZSize(pObj->nVertices);
    }

    return cb;
}


static int PointM2WKB (const SHPObject *pObj, void *pv, double offX, double offY, double offM)
{
    ub4 v4;
    double v8;
    int cb = 0;

    if (pv) {
        ValBufCopy(pv, cb, wkb_bo_xdr);

        v4 = BO_i32_htobe(WKB_PointM);
        ValBufCopy(pv, cb, v4);

        v8 = BO_f64_htobe(*pObj->padfX + offX);
        ValBufCopy(pv, cb, v8);

        v8 = BO_f64_htobe(*pObj->padfY + offY);
        ValBufCopy(pv, cb, v8);

        v8 = BO_f64_htobe(*pObj->padfM + offM);
        ValBufCopy(pv, cb, v8);
    } else {
        cb = WKBPointMSize(1);
    }

    return cb;
}


static int ArcM2WKB (const SHPObject *pObj, void *pv, double offX, double offY, double offM)
{
    ub4 v4;
    double   v8;
    int cb = 0;

    if (pv) {
        int iPoint = 0;

        /* byte order flag */
        ValBufCopy(pv, cb, wkb_bo_xdr);

        /* wkb type */
        v4 = BO_i32_htobe(WKB_LineStringM);
        ValBufCopy(pv, cb, v4);

        /* num of points */
        v4 = BO_i32_htobe(pObj->nVertices);
        ValBufCopy(pv, cb, v4);

        for (; iPoint < pObj->nVertices; iPoint++) {
            v8 = BO_f64_htobe(pObj->padfX[iPoint] + offX);
            ValBufCopy(pv, cb, v8);

            v8 = BO_f64_htobe(pObj->padfY[iPoint] + offY);
            ValBufCopy(pv, cb, v8);

            v8 = BO_f64_htobe(pObj->padfM[iPoint] + offM);
            ValBufCopy(pv, cb, v8);
        }
    } else {
        cb = WKBLineStringMSize(pObj->nVertices);
    }

    return cb;
}


static int PolygonM2WKB (const SHPObject *pObj, void *pv, double offX, double offY, double offM)
{
    ub4 v4;
    double   v8;
    int cb = 0;

    if (pv) {
        int iPart = 0;

        /* byte order flag */
        ValBufCopy(pv, cb, wkb_bo_xdr);

        /* wkb type */
        v4 = BO_i32_htobe(WKB_PolygonM);
        ValBufCopy(pv, cb, v4);

        /* num of rings */
        v4 = BO_i32_htobe(pObj->nParts);
        ValBufCopy(pv, cb, v4);

        for (; iPart < pObj->nParts; iPart++) {
            int p = pObj->panPartStart[iPart];
            int p1 = pObj->panPartStart[iPart+1];

            /* num of points */
            v4 = BO_i32_htobe(p1 - p);
            ValBufCopy(pv, cb, v4);

            for (; p < p1; p++) {
                v8 = BO_f64_htobe(pObj->padfX[p] + offX);
                ValBufCopy(pv, cb, v8);

                v8 = BO_f64_htobe(pObj->padfY[p] + offY);
                ValBufCopy(pv, cb, v8);

                v8 = BO_f64_htobe(pObj->padfM[p] + offM);
                ValBufCopy(pv, cb, v8);
            }
        }
    } else {
        int iPart = 0;

        /* byte order flag */
        ValBufOffset(pv, cb, wkb_bo_xdr);

        /* wkb type */
        ValBufOffset(pv, cb, v4);

        /* num of rings */
        ValBufOffset(pv, cb, v4);

        for (; iPart < pObj->nParts; iPart++) {
            int np = pObj->panPartStart[iPart+1] - pObj->panPartStart[iPart];

            /* num of points */
            ValBufOffset(pv, cb, v4);

            ValBufOffsetN(pv, cb, v8, np);
            ValBufOffsetN(pv, cb, v8, np);
            ValBufOffsetN(pv, cb, v8, np);
        }
    }

    return cb;
}


static int MultiPointM2WKB (const SHPObject *pObj, void *pv, double offX, double offY, double offM)
{
    ub4 v4;
    double v8;
    int cb = 0;

    if (pv) {
        int iPoint = 0;
        ValBufCopy(pv, cb, wkb_bo_xdr);

        v4 = BO_i32_htobe(WKB_MultiPointM);
        ValBufCopy(pv, cb, v4);

        v4 = BO_i32_htobe(pObj->nVertices);
        ValBufCopy(pv, cb, v4);

        for (; iPoint < pObj->nVertices; iPoint++) {
            ValBufCopy(pv, cb, wkb_bo_xdr);

            v4 = BO_i32_htobe(WKB_PointM);
            ValBufCopy(pv, cb, v4);

            v8 = BO_f64_htobe(pObj->padfX[iPoint] + offX);
            ValBufCopy(pv, cb, v8);

            v8 = BO_f64_htobe(pObj->padfY[iPoint] + offY);
            ValBufCopy(pv, cb, v8);

            v8 = BO_f64_htobe(pObj->padfM[iPoint] + offM);
            ValBufCopy(pv, cb, v8);
        }
    } else {
        cb = WKBHeaderSize + sizeof(v4) + WKBPointMSize(pObj->nVertices);
    }

    return cb;
}


static int exPoint2WKB (const SHPObjectEx *pObj, void *pv, double offX, double offY)
{
    ub4 v4;
    double v8;
    int cb = 0;

    if (pv) {
        ValBufCopy(pv, cb, wkb_bo_xdr);

        v4 = BO_i32_htobe(WKB_Point2D);
        ValBufCopy(pv, cb, v4);

        v8 = BO_f64_htobe(pObj->pPoints[0].x + offX);
        ValBufCopy(pv, cb, v8);

        v8 = BO_f64_htobe(pObj->pPoints[0].y + offY);
        ValBufCopy(pv, cb, v8);
    } else {
        cb = WKBPointSize(1);
    }

    return cb;
}


static int exArc2WKB (const SHPObjectEx *pObj, void *pv, double offX, double offY)
{
    ub4 v4;
    double   v8;
    int cb = 0;

    if (pv) {
        int iPoint = 0;

        /* byte order flag */
        ValBufCopy(pv, cb, wkb_bo_xdr);

        /* wkb type */
        v4 = BO_i32_htobe(WKB_LineString2D);
        ValBufCopy(pv, cb, v4);

        /* num of points */
        v4 = BO_i32_htobe(pObj->nVertices);
        ValBufCopy(pv, cb, v4);

        for (; iPoint < pObj->nVertices; iPoint++) {
            v8 = BO_f64_htobe(pObj->pPoints[iPoint].x + offX);
            ValBufCopy(pv, cb, v8);

            v8 = BO_f64_htobe(pObj->pPoints[iPoint].y + offY);
            ValBufCopy(pv, cb, v8);
        }
    } else {
        cb = WKBLineStringSize(pObj->nVertices);
    }

    return cb;
}


static int exPolygon2WKB (const SHPObjectEx *pObj, void *pv, double offX, double offY)
{
    ub4 v4;
    double   v8;
    int cb = 0;

    if (pv) {
        int iPart = 0;

        /* byte order flag */
        ValBufCopy(pv, cb, wkb_bo_xdr);

        /* wkb type */
        v4 = BO_i32_htobe(WKB_Polygon2D);
        ValBufCopy(pv, cb, v4);

        /* num of rings */
        v4 = BO_i32_htobe(pObj->nParts);
        ValBufCopy(pv, cb, v4);

        for (; iPart < pObj->nParts; iPart++) {
            int p = pObj->panPartStart[iPart];
            int p1 = pObj->panPartStart[iPart+1];

            /* num of points */
            v4 = BO_i32_htobe(p1 - p);
            ValBufCopy(pv, cb, v4);

            for (; p < p1; p++) {
                v8 = BO_f64_htobe(pObj->pPoints[p].x + offX);
                ValBufCopy(pv, cb, v8);

                v8 = BO_f64_htobe(pObj->pPoints[p].y + offY);
                ValBufCopy(pv, cb, v8);
            }
        }
    } else {
        int iPart = 0;

        /* byte order flag */
        ValBufOffset(pv, cb, wkb_bo_xdr);

        /* wkb type */
        ValBufOffset(pv, cb, v4);

        /* num of rings */
        ValBufOffset(pv, cb, v4);

        for (; iPart < pObj->nParts; iPart++) {
            int np = pObj->panPartStart[iPart+1] - pObj->panPartStart[iPart];

            /* num of points */
            ValBufOffset(pv, cb, v4);

            ValBufOffsetN(pv, cb, v8, np);
            ValBufOffsetN(pv, cb, v8, np);
        }
    }

    return cb;
}


static int exMultiPoint2WKB (const SHPObjectEx *pObj, void *pv, double offX, double offY)
{
    ub4 v4;
    double v8;
    int cb = 0;

    if (pv) {
        int iPoint = 0;
        ValBufCopy(pv, cb, wkb_bo_xdr);

        v4 = BO_i32_htobe(WKB_MultiPoint2D);
        ValBufCopy(pv, cb, v4);

        v4 = BO_i32_htobe(pObj->nVertices);
        ValBufCopy(pv, cb, v4);

        for (; iPoint < pObj->nVertices; iPoint++) {
            ValBufCopy(pv, cb, wkb_bo_xdr);

            v4 = BO_i32_htobe(WKB_Point2D);
            ValBufCopy(pv, cb, v4);

            v8 = BO_f64_htobe(pObj->pPoints[iPoint].x + offX);
            ValBufCopy(pv, cb, v8);

            v8 = BO_f64_htobe(pObj->pPoints[iPoint].y + offY);
            ValBufCopy(pv, cb, v8);
        }
    } else {
        cb = WKBHeaderSize + sizeof(v4) + WKBPointSize(pObj->nVertices);
    }

    return cb;
}


static int exPointZ2WKB (const SHPObjectEx *pObj, void *pv, double offX, double offY, double offZ)
{
    ub4 v4;
    double v8;
    int cb = 0;

    if (pv) {
        ValBufCopy(pv, cb, wkb_bo_xdr);

        v4 = BO_i32_htobe(WKB_PointZ);
        ValBufCopy(pv, cb, v4);

        v8 = BO_f64_htobe(pObj->pPoints[0].x + offX);
        ValBufCopy(pv, cb, v8);

        v8 = BO_f64_htobe(pObj->pPoints[0].y + offY);
        ValBufCopy(pv, cb, v8);

        v8 = BO_f64_htobe(*pObj->padfZ + offZ);
        ValBufCopy(pv, cb, v8);
    } else {
        cb = WKBPointZSize(1);
    }

    return cb;
}


static int exArcZ2WKB (const SHPObjectEx *pObj, void *pv, double offX, double offY, double offZ)
{
    ub4 v4;
    double   v8;
    int cb = 0;

    if (pv) {
        int iPoint = 0;

        /* byte order flag */
        ValBufCopy(pv, cb, wkb_bo_xdr);

        /* wkb type */
        v4 = BO_i32_htobe(WKB_LineStringZ);
        ValBufCopy(pv, cb, v4);

        /* num of points */
        v4 = BO_i32_htobe(pObj->nVertices);
        ValBufCopy(pv, cb, v4);

        for (; iPoint < pObj->nVertices; iPoint++) {
            v8 = BO_f64_htobe(pObj->pPoints[iPoint].x + offX);
            ValBufCopy(pv, cb, v8);

            v8 = BO_f64_htobe(pObj->pPoints[iPoint].y + offY);
            ValBufCopy(pv, cb, v8);

            v8 = BO_f64_htobe(pObj->padfZ[iPoint] + offZ);
            ValBufCopy(pv, cb, v8);
        }
    } else {
        cb = WKBLineStringZSize(pObj->nVertices);
    }

    return cb;
}


static int exPolygonZ2WKB (const SHPObjectEx *pObj, void *pv, double offX, double offY, double offZ)
{
    ub4 v4;
    double   v8;
    int cb = 0;

    if (pv) {
        int iPart = 0;

        /* byte order flag */
        ValBufCopy(pv, cb, wkb_bo_xdr);

        /* wkb type */
        v4 = BO_i32_htobe(WKB_PolygonZ);
        ValBufCopy(pv, cb, v4);

        /* num of rings */
        v4 = BO_i32_htobe(pObj->nParts);
        ValBufCopy(pv, cb, v4);

        for (; iPart < pObj->nParts; iPart++) {
            int p = pObj->panPartStart[iPart];
            int p1 = pObj->panPartStart[iPart+1];

            /* num of points */
            v4 = BO_i32_htobe(p1 - p);
            ValBufCopy(pv, cb, v4);

            for (; p < p1; p++) {
                v8 = BO_f64_htobe(pObj->pPoints[p].x + offX);
                ValBufCopy(pv, cb, v8);

                v8 = BO_f64_htobe(pObj->pPoints[p].y + offY);
                ValBufCopy(pv, cb, v8);

                v8 = BO_f64_htobe(pObj->padfZ[p] + offZ);
                ValBufCopy(pv, cb, v8);
            }
        }
    } else {
        int iPart = 0;

        /* byte order flag */
        ValBufOffset(pv, cb, wkb_bo_xdr);

        /* wkb type */
        ValBufOffset(pv, cb, v4);

        /* num of rings */
        ValBufOffset(pv, cb, v4);

        for (; iPart < pObj->nParts; iPart++) {
            int np = pObj->panPartStart[iPart+1] - pObj->panPartStart[iPart];

            /* num of points */
            ValBufOffset(pv, cb, v4);

            ValBufOffsetN(pv, cb, v8, np);
            ValBufOffsetN(pv, cb, v8, np);
            ValBufOffsetN(pv, cb, v8, np);
        }
    }

    return cb;
}


static int exMultiPointZ2WKB (const SHPObjectEx *pObj, void *pv, double offX, double offY, double offZ)
{
    ub4 v4;
    double v8;
    int cb = 0;

    if (pv) {
        int iPoint = 0;
        ValBufCopy(pv, cb, wkb_bo_xdr);

        v4 = BO_i32_htobe(WKB_MultiPointZ);
        ValBufCopy(pv, cb, v4);

        v4 = BO_i32_htobe(pObj->nVertices);
        ValBufCopy(pv, cb, v4);

        for (; iPoint < pObj->nVertices; iPoint++) {
            ValBufCopy(pv, cb, wkb_bo_xdr);

            v4 = BO_i32_htobe(WKB_PointZ);
            ValBufCopy(pv, cb, v4);

            v8 = BO_f64_htobe(pObj->pPoints[iPoint].x + offX);
            ValBufCopy(pv, cb, v8);

            v8 = BO_f64_htobe(pObj->pPoints[iPoint].y + offY);
            ValBufCopy(pv, cb, v8);

            v8 = BO_f64_htobe(pObj->padfZ[iPoint] + offZ);
            ValBufCopy(pv, cb, v8);
        }
    } else {
        cb = WKBHeaderSize + sizeof(v4) + WKBPointZSize(pObj->nVertices);
    }

    return cb;
}


static int exPointM2WKB (const SHPObjectEx *pObj, void *pv, double offX, double offY, double offM)
{
    ub4 v4;
    double v8;
    int cb = 0;

    if (pv) {
        ValBufCopy(pv, cb, wkb_bo_xdr);

        v4 = BO_i32_htobe(WKB_PointM);
        ValBufCopy(pv, cb, v4);

        v8 = BO_f64_htobe(pObj->pPoints[0].x + offX);
        ValBufCopy(pv, cb, v8);

        v8 = BO_f64_htobe(pObj->pPoints[0].y + offY);
        ValBufCopy(pv, cb, v8);

        v8 = BO_f64_htobe(*pObj->padfM + offM);
        ValBufCopy(pv, cb, v8);
    } else {
        cb = WKBPointMSize(1);
    }

    return cb;
}


static int exArcM2WKB (const SHPObjectEx *pObj, void *pv, double offX, double offY, double offM)
{
    ub4 v4;
    double   v8;
    int cb = 0;

    if (pv) {
        int iPoint = 0;

        /* byte order flag */
        ValBufCopy(pv, cb, wkb_bo_xdr);

        /* wkb type */
        v4 = BO_i32_htobe(WKB_LineStringM);
        ValBufCopy(pv, cb, v4);

        /* num of points */
        v4 = BO_i32_htobe(pObj->nVertices);
        ValBufCopy(pv, cb, v4);

        for (; iPoint < pObj->nVertices; iPoint++) {
            v8 = BO_f64_htobe(pObj->pPoints[iPoint].x + offX);
            ValBufCopy(pv, cb, v8);

            v8 = BO_f64_htobe(pObj->pPoints[iPoint].y + offY);
            ValBufCopy(pv, cb, v8);

            v8 = BO_f64_htobe(pObj->padfM[iPoint] + offM);
            ValBufCopy(pv, cb, v8);
        }
    } else {
        cb = WKBLineStringMSize(pObj->nVertices);
    }

    return cb;
}


static int exPolygonM2WKB (const SHPObjectEx *pObj, void *pv, double offX, double offY, double offM)
{
    ub4 v4;
    double   v8;
    int cb = 0;

    if (pv) {
        int iPart = 0;

        /* byte order flag */
        ValBufCopy(pv, cb, wkb_bo_xdr);

        /* wkb type */
        v4 = BO_i32_htobe(WKB_PolygonM);
        ValBufCopy(pv, cb, v4);

        /* num of rings */
        v4 = BO_i32_htobe(pObj->nParts);
        ValBufCopy(pv, cb, v4);

        for (; iPart < pObj->nParts; iPart++) {
            int p = pObj->panPartStart[iPart];
            int p1 = pObj->panPartStart[iPart+1];

            /* num of points */
            v4 = BO_i32_htobe(p1 - p);
            ValBufCopy(pv, cb, v4);

            for (; p < p1; p++) {
                v8 = BO_f64_htobe(pObj->pPoints[p].x + offX);
                ValBufCopy(pv, cb, v8);

                v8 = BO_f64_htobe(pObj->pPoints[p].y + offY);
                ValBufCopy(pv, cb, v8);

                v8 = BO_f64_htobe(pObj->padfM[p] + offM);
                ValBufCopy(pv, cb, v8);
            }
        }
    } else {
        int iPart = 0;

        /* byte order flag */
        ValBufOffset(pv, cb, wkb_bo_xdr);

        /* wkb type */
        ValBufOffset(pv, cb, v4);

        /* num of rings */
        ValBufOffset(pv, cb, v4);

        for (; iPart < pObj->nParts; iPart++) {
            int np = pObj->panPartStart[iPart+1] - pObj->panPartStart[iPart];

            /* num of points */
            ValBufOffset(pv, cb, v4);

            ValBufOffsetN(pv, cb, v8, np);
            ValBufOffsetN(pv, cb, v8, np);
            ValBufOffsetN(pv, cb, v8, np);
        }
    }

    return cb;
}


static int exMultiPointM2WKB (const SHPObjectEx *pObj, void *pv, double offX, double offY, double offM)
{
    ub4 v4;
    double v8;
    int cb = 0;

    if (pv) {
        int iPoint = 0;
        ValBufCopy(pv, cb, wkb_bo_xdr);

        v4 = BO_i32_htobe(WKB_MultiPointM);
        ValBufCopy(pv, cb, v4);

        v4 = BO_i32_htobe(pObj->nVertices);
        ValBufCopy(pv, cb, v4);

        for (; iPoint < pObj->nVertices; iPoint++) {
            ValBufCopy(pv, cb, wkb_bo_xdr);

            v4 = BO_i32_htobe(WKB_PointM);
            ValBufCopy(pv, cb, v4);

            v8 = BO_f64_htobe(pObj->pPoints[iPoint].x + offX);
            ValBufCopy(pv, cb, v8);

            v8 = BO_f64_htobe(pObj->pPoints[iPoint].y + offY);
            ValBufCopy(pv, cb, v8);

            v8 = BO_f64_htobe(pObj->padfM[iPoint] + offM);
            ValBufCopy(pv, cb, v8);
        }
    } else {
        cb = WKBHeaderSize + sizeof(v4) + WKBPointMSize(pObj->nVertices);
    }

    return cb;
}


static int MultiPatch2WKB (const SHPObject *pObj, void *pv, double offX, double offY)
{
    /* TODO */
    return 0;
}


static int exMultiPatch2WKB (const SHPObjectEx *pObj, void *pv, double offX, double offY)
{
    /* TODO */
    return 0;
}

#endif /* _SHP2WKB_H_INCLUDED */
