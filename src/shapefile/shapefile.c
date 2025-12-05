/******************************************************************************
 * shapefile.c
 *
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
#include "shapefile_i.h"

// Shapefile使用大端字节序。小端主机需要字节交换

/**
 * A realloc cover function that will access a 0 pointer as a valid input
 */
void * SfRealloc (void * pMem, int nNewSize)
{
    void *p;
    if (pMem == 0) {
        p = malloc(nNewSize);
        if (!p) {
            exit(EXIT_OUTMEMORY);
        }
        return p;
    } else {
        p = realloc (pMem, nNewSize);
        if (!p) {
            exit(EXIT_OUTMEMORY);
        }
        return p;
    }
}

static void SwapDouble(double *a, double *b)
{
    double t = *a;
    *a = *b;
    *b = t;
}

static void SwapPointType(SHPPointType *p, SHPPointType *q)
{
    double t = p->x;
    p->x = q->x;
    q->x = t;

    t = p->y;
    p->y = q->y;
    q->y = t;
}

static int SHPTypeHasParts(int nSHPType)
{
    return (nSHPType == SHPT_POLYGON ||
        nSHPType == SHPT_ARC ||
        nSHPType == SHPT_POLYGONZ ||
        nSHPType == SHPT_POLYGONM ||
        nSHPType == SHPT_ARCZ ||
        nSHPType == SHPT_ARCM ||
        nSHPType == SHPT_MULTIPATCH);
}

const char * shapefile_lib_version(const char **_libname)
{
    if (_libname) {
        *_libname = LIBNAME;
    }
    return LIBVERSION;
}


/**
 * Write out a header for the .shp and .shx files as well as the
 *   contents of the index (.shx) file
 */
void SHPWriteHeader(SHPHandle psSHP)
{
    ub1     abyHeader[100];
    int      i;
    int      i32;
    double   dValue;
    int      *panSHX;

    /* Prepare header block for .shp file */
    for (i = 0; i < 100; i++) {
        abyHeader[i] = 0;
    }

    abyHeader[2] = 0x27;                                /* magic cookie */
    abyHeader[3] = 0x0a;

    i32 = psSHP->nFileSize/2;                           /* file size: big-endian */
    ByteCopy(&i32, abyHeader+24, 4);
    BO_htobe32_buf(abyHeader+24);

    i32 = 1000;                                         /* version: little-endian */
    ByteCopy(&i32, abyHeader+28, 4);
    BO_htole32_buf(abyHeader+28);

    i32 = psSHP->nShapeType;                            /* shape type: little-endian */
    ByteCopy(&i32, abyHeader+32, 4);
    BO_htole32_buf(abyHeader+32);

    dValue = psSHP->adBoundsMin[0];                     /* set bounds: little-endian */
    ByteCopy(&dValue, abyHeader+36, 8);
    BO_htole64_buf(abyHeader+36);

    dValue = psSHP->adBoundsMin[1];
    ByteCopy(&dValue, abyHeader+44, 8);
    BO_htole64_buf(abyHeader+44);

    dValue = psSHP->adBoundsMax[0];
    ByteCopy(&dValue, abyHeader+52, 8);
    BO_htole64_buf(abyHeader+52);

    dValue = psSHP->adBoundsMax[1];
    ByteCopy(&dValue, abyHeader+60, 8);
    BO_htole64_buf(abyHeader+60);

    dValue = psSHP->adBoundsMin[2];                     /* z */
    ByteCopy(&dValue, abyHeader+68, 8);
    BO_htole64_buf(abyHeader+68);

    dValue = psSHP->adBoundsMax[2];
    ByteCopy(&dValue, abyHeader+76, 8);
    BO_htole64_buf(abyHeader+76);

    dValue = psSHP->adBoundsMin[3];                     /* m */
    ByteCopy(&dValue, abyHeader+84, 8);
    BO_htole64_buf(abyHeader+84);

    dValue = psSHP->adBoundsMax[3];
    ByteCopy(&dValue, abyHeader+92, 8);
    BO_htole64_buf(abyHeader+92);

    /* Write .shp file header */
    if (fseek(psSHP->fpSHP, 0, 0) != 0 || fwrite(abyHeader, 100, 1, psSHP->fpSHP) != 1) {
        return;
    }

    /* Prepare, and write .shx file header */
    i32 = (psSHP->nRecords * 2 * sizeof(int) + 100)/2;   /* file size: big-endian */
    ByteCopy(&i32, abyHeader+24, 4);
    BO_htobe32_buf(abyHeader+24);

    if (fseek(psSHP->fpSHX, 0, 0) != 0 || fwrite(abyHeader, 100, 1, psSHP->fpSHX) != 1) {
        return;
    }

    /* Write out the .shx contents */
    panSHX = (int *) malloc(sizeof(int) * 2 * psSHP->nRecords);

    for (i = 0; i < psSHP->nRecords; i++) {
        panSHX[i*2  ] = psSHP->panRecOffset[i]/2;
        panSHX[i*2+1] = psSHP->panRecSize[i]/2;

        BO_htobe32_buf(panSHX+i*2);
        BO_htobe32_buf(panSHX+i*2+1);
    }

    if ((int)fwrite(panSHX, sizeof(int)*2, psSHP->nRecords, psSHP->fpSHX) != psSHP->nRecords) {
        perror("Failure writing .shx contents.\n");
    }

    SafeFree(panSHX);

    /* Flush to disk */
    fflush(psSHP->fpSHP);
    fflush(psSHP->fpSHX);
}


/**
 * SHPHasZM()
 */
static void SHPHasZM(int nSHPType, int *bHasZ, int *bHasM)
{
    if (nSHPType == SHPT_ARCM ||
        nSHPType == SHPT_POINTM ||
        nSHPType == SHPT_POLYGONM ||
        nSHPType == SHPT_MULTIPOINTM) {
        if (bHasM) {
            *bHasM = SHAPEFILE_TRUE;
        }
        if (bHasZ) {
            *bHasZ = SHAPEFILE_FALSE;
        }
    } else if (nSHPType == SHPT_ARCZ ||
        nSHPType == SHPT_POINTZ ||
        nSHPType == SHPT_POLYGONZ ||
        nSHPType == SHPT_MULTIPOINTZ ||
        nSHPType == SHPT_MULTIPATCH) {
        if (bHasM) {
            *bHasM = SHAPEFILE_TRUE;
        }
        if (bHasZ) {
            *bHasZ = SHAPEFILE_TRUE;
        }
    } else {
        if (bHasM) {
            *bHasM = SHAPEFILE_FALSE;
        }
        if (bHasZ) {
            *bHasZ = SHAPEFILE_FALSE;
        }
    }
}


/**
 * Open the .shp and .shx files based on the basename of the files or either file name
 */
SHPHandle SHPOpen(const char * pszLayer, const char * pszAccess)
{
    char       *pszFullname, *pszBasename;
    SHPHandle  psSHP;

    ub1       *pabyBuf;
    int        i, nOffset, nLength;
    double     dValue;

    /* Ensure the access string is one of the legal ones.
     *  We ensure the result string indicates binary to avoid common problems on Windows
     */
    if (strcmp(pszAccess,"rb+") == 0 || strcmp(pszAccess,"r+b") == 0 || strcmp(pszAccess,"r+") == 0) {
        pszAccess = "r+b";
    } else {
        pszAccess = "rb";
    }

    /* Initialize the info structure */
    psSHP = (SHPHandle) calloc(sizeof(SHPInfo), 1);
    psSHP->bUpdated = SHAPEFILE_FALSE;

    /* Compute the base (layer) name.  If there is any extension
   *   on the passed in filename we will strip it off */
    pszBasename = (char *) malloc(strlen(pszLayer)+5);
    strcpy(pszBasename, pszLayer);
    for (i = (int) strlen(pszBasename)-1; i > 0 &&
        pszBasename[i] != '.' && pszBasename[i] != '/' &&
        pszBasename[i] != '\\'; i--) {
        /* do nothing */
    }

    if (pszBasename[i] == '.') {
        pszBasename[i] = '\0';
    }

    /* Open the .shp and .shx files.  Note that files pulled from
   *  a PC to Unix with upper case filenames won't work! */
    pszFullname = (char *) malloc(strlen(pszBasename) + 5);
    sprintf(pszFullname, "%s.shp", pszBasename);
    psSHP->fpSHP = fopen(pszFullname, pszAccess);

    if (psSHP->fpSHP == 0) {
        sprintf(pszFullname, "%s.SHP", pszBasename);
        psSHP->fpSHP = fopen(pszFullname, pszAccess);
    }

    if (psSHP->fpSHP == 0) {
        SafeFree(psSHP);
        SafeFree(pszBasename);
        SafeFree(pszFullname);
        return(0);
    }

    sprintf(pszFullname, "%s.shx", pszBasename);
    psSHP->fpSHX = fopen(pszFullname, pszAccess);
    if (psSHP->fpSHX == 0) {
        sprintf(pszFullname, "%s.SHX", pszBasename);
        psSHP->fpSHX = fopen(pszFullname, pszAccess);
    }

    if (psSHP->fpSHX == 0) {
        fclose(psSHP->fpSHP);
        SafeFree(psSHP);
        SafeFree(pszBasename);
        SafeFree(pszFullname);
        return(0);
    }

    SafeFree(pszFullname);
    SafeFree(pszBasename);

    /* Read the file size from the SHP file */
    pabyBuf = (ub1 *) malloc(100);
    fread(pabyBuf, 100, 1, psSHP->fpSHP);

    // 读取 SHP 文件大小（正确版本）
    int32_t i32;
    memcpy(&i32, pabyBuf+24, 4);
    BO_betoh32_buf(&i32);  // 添加字节序转换
    psSHP->nFileSize = i32 * 2;

    // Read SHX file Header info
    if (fread(pabyBuf, 100, 1, psSHP->fpSHX) != 1 ||
        pabyBuf[0] != 0 ||
        pabyBuf[1] != 0 ||
        pabyBuf[2] != 0x27 ||
        (pabyBuf[3] != 0x0a && pabyBuf[3] != 0x0d)) {
        fclose(psSHP->fpSHP);
        fclose(psSHP->fpSHX);
        SafeFree(psSHP);
        return(0);
    }

    // 修复：读取SHX文件头的记录数（使用BO_betoh32_buf）
    memcpy(&i32, pabyBuf+24, 4);
    BO_betoh32_buf(&i32);
    psSHP->nRecords = (i32 * 2 - 100) / 8;

    memcpy(&psSHP->nShapeType, pabyBuf+32, 4);
    BO_letoh32_buf(&psSHP->nShapeType);

    if (psSHP->nRecords < 0 || psSHP->nRecords > SHAPEFILE_RECORDS_MAX) {
        fclose(psSHP->fpSHP);
        fclose(psSHP->fpSHX);
        SafeFree(psSHP);
        return(0);
    }

    /* Read the bounds */
    BO_letoh64_buf(pabyBuf+36);
    memcpy(&dValue, pabyBuf+36, 8);
    psSHP->adBoundsMin[0] = dValue;

    BO_letoh64_buf(pabyBuf+44);
    memcpy(&dValue, pabyBuf+44, 8);
    psSHP->adBoundsMin[1] = dValue;

    BO_letoh64_buf(pabyBuf+52);
    memcpy(&dValue, pabyBuf+52, 8);
    psSHP->adBoundsMax[0] = dValue;

    BO_letoh64_buf(pabyBuf+60);
    memcpy(&dValue, pabyBuf+60, 8);
    psSHP->adBoundsMax[1] = dValue;

    BO_letoh64_buf(pabyBuf+68);         /* z */
    memcpy(&dValue, pabyBuf+68, 8);
    psSHP->adBoundsMin[2] = dValue;

    BO_letoh64_buf(pabyBuf+76);
    memcpy(&dValue, pabyBuf+76, 8);
    psSHP->adBoundsMax[2] = dValue;

    BO_letoh64_buf(pabyBuf+84);         /* z */
    memcpy(&dValue, pabyBuf+84, 8);
    psSHP->adBoundsMin[3] = dValue;

    BO_letoh64_buf(pabyBuf+92);
    memcpy(&dValue, pabyBuf+92, 8);
    psSHP->adBoundsMax[3] = dValue;

    SafeFree(pabyBuf);

    /* Read the .shx file to get the offsets to each record in the .shp file */
    psSHP->nMaxRecords = psSHP->nRecords;
    psSHP->panRecOffset = (uint32_t *) malloc(sizeof(uint32_t) * MAX_V2(1, psSHP->nMaxRecords));
    psSHP->panRecSize = (uint32_t *) malloc(sizeof(uint32_t) * MAX_V2(1, psSHP->nMaxRecords));
    pabyBuf = (ub1 *) malloc(8 * MAX_V2(1, psSHP->nRecords));

    if ((uint32_t) fread(pabyBuf, 8, psSHP->nRecords, psSHP->fpSHX) != psSHP->nRecords) {
        /* SHX is short or unreadable for some reason. */
        fclose(psSHP->fpSHP);
        fclose(psSHP->fpSHX);
        SafeFree(psSHP->panRecOffset);
        SafeFree(psSHP->panRecSize);
        SafeFree(psSHP);
        return (0);
    }

    for (i = 0; i < psSHP->nRecords; i++) {
        memcpy(&nOffset, pabyBuf + i * 8, 4);
        BO_betoh32_buf(&nOffset);  // be

        memcpy(&nLength, pabyBuf + i * 8 + 4, 4);
        BO_betoh32_buf(&nLength);  // be

        psSHP->panRecOffset[i] = nOffset * 2;
        psSHP->panRecSize[i] = nLength * 2;
    }

    SafeFree(pabyBuf);
    return(psSHP);
}


/**
 * Close the .shp and .shx files
 */
void SHPClose(SHPHandle psSHP)
{
    SHPMBRTreeReset(psSHP, 1);

    /* Update the header if we have modified anything */
    if (psSHP->bUpdated) {
        SHPWriteHeader(psSHP);
    }

    /* Free all resources, and close files */
    SafeFree(psSHP->panRecOffset);
    SafeFree(psSHP->panRecSize);
    fclose(psSHP->fpSHX);
    fclose(psSHP->fpSHP);

    if (psSHP->pabyRec != 0) {
        SafeFree(psSHP->pabyRec);
    }
    SafeFree(psSHP);
}


/**
 * Fetch general information about the shape file
 */
void SHPGetInfo(SHPHandle psSHP,
    int *pnEntities,
    int *pnShapeType,
    double *padfMinBound,
    double *padfMaxBound) {
    int i;

    if (psSHP == 0) {
        return;
    }

    if (pnEntities != 0) {
        *pnEntities = psSHP->nRecords;
    }

    if (pnShapeType != 0) {
        *pnShapeType = psSHP->nShapeType;
    }

    for (i = 0; i < 4; i++) {
        if (padfMinBound != 0) {
            padfMinBound[i] = psSHP->adBoundsMin[i];
        }
        if (padfMaxBound != 0) {
            padfMaxBound[i] = psSHP->adBoundsMax[i];
        }
    }
}


/**
 * SHPGetType()
 */
int SHPGetType(SHPHandle hSHP, int *bHasZ, int *bHasM)
{
    if (bHasZ || bHasM) {
        SHPHasZM(hSHP->nShapeType, bHasZ, bHasM);
    }

    switch(hSHP->nShapeType) {
    case SHPT_POLYGON:
    case SHPT_POLYGONZ:
    case SHPT_POLYGONM:
        return SHAPE_TYPE_POLYGON;

    case SHPT_ARC:
    case SHPT_ARCZ:
    case SHPT_ARCM:
        return SHAPE_TYPE_LINE;

    case SHPT_POINT:
    case SHPT_POINTZ:
    case SHPT_POINTM:
    case SHPT_MULTIPOINT:
    case SHPT_MULTIPOINTZ:
    case SHPT_MULTIPOINTM:
        return SHAPE_TYPE_POINT;

    case SHPT_NULL:
        return SHAPE_TYPE_NIL;
    }

    return -1;
}


/**
 * Create a new shape file and return a handle to the open shape file with read/write access
 */
SHPHandle SHPCreate(const char * pszLayer, int nShapeType)
{
    char        *pszBasename = NULL;
    char        *pszFullname = NULL;
    int         i;
    FILE        *fpSHP = NULL;
    FILE        *fpSHX = NULL;
    ub1         abyHeader[100];
    int32_t     i32;
    double      dValue;

    /* Compute the base (layer) name. */
    pszBasename = (char *) malloc(strlen(pszLayer) + 5);
    if (pszBasename == NULL) {
        return NULL; // 检查 malloc
    }
    strcpy(pszBasename, pszLayer);
    for (i = (int) strlen(pszBasename) - 1;
        i > 0 && pszBasename[i] != '.' && pszBasename[i] != '/' && pszBasename[i] != '\\';
        i--) {
        /* do nothing */
    }
    if (pszBasename[i] == '.') {
        pszBasename[i] = '\0';
    }

    /* Open the two files so we can write their headers */
    pszFullname = (char *) malloc(strlen(pszBasename) + 5);
    if (pszFullname == NULL) { // 检查 malloc
        SafeFree(pszBasename);
        return NULL;
    }

    sprintf(pszFullname, "%s.shp", pszBasename);

    fpSHP = fopen(pszFullname, "wb");
    if (fpSHP == 0) {
        SafeFree(pszBasename);
        SafeFree(pszFullname);
        return NULL;
    }

    sprintf(pszFullname, "%s.shx", pszBasename);
    fpSHX = fopen(pszFullname, "wb");
    if (fpSHX == 0) {
        fclose(fpSHP);
        SafeFree(pszBasename);
        SafeFree(pszFullname);
        return NULL;
    }

    SafeFree(pszFullname);
    SafeFree(pszBasename);
    pszFullname = NULL;
    pszBasename = NULL;

    /* Prepare header block for .shp file */
    memset(abyHeader, 0, sizeof(abyHeader));
    abyHeader[2] = 0x27;                                /* magic cookie */
    abyHeader[3] = 0x0a;

    i32 = 50;                                           /* file size: be */
    ByteCopy(&i32, abyHeader + 24, 4);
    BO_htobe32_buf(abyHeader + 24);

    i32 = 1000;                                         /* version: le */
    ByteCopy(&i32, abyHeader + 28, 4);
    BO_htole32_buf(abyHeader + 28);

    i32 = nShapeType;                                   /* shape type: le */
    ByteCopy(&i32, abyHeader + 32, 4);
    BO_htole32_buf(abyHeader + 32);

    /* Set bounds (X, Y, Z, M) to 0.0, with correct little-endian byte order */
    dValue = 0.0;
    ByteCopy(&dValue, abyHeader + 36, 8); // Xmin
    BO_htole64_buf(abyHeader + 36);

    ByteCopy(&dValue, abyHeader + 44, 8); // Ymin
    BO_htole64_buf(abyHeader + 44);

    ByteCopy(&dValue, abyHeader + 52, 8); // Xmax
    BO_htole64_buf(abyHeader + 52);

    ByteCopy(&dValue, abyHeader + 60, 8); // Ymax
    BO_htole64_buf(abyHeader + 60);

    ByteCopy(&dValue, abyHeader + 68, 8); // Zmin
    BO_htole64_buf(abyHeader + 68);

    ByteCopy(&dValue, abyHeader + 76, 8); // Zmax
    BO_htole64_buf(abyHeader + 76);

    ByteCopy(&dValue, abyHeader + 84, 8); // Mmin
    BO_htole64_buf(abyHeader + 84);

    ByteCopy(&dValue, abyHeader + 92, 8); // Mmax
    BO_htole64_buf(abyHeader + 92);

    /* Write .shp file header */
    if (fwrite(abyHeader, 100, 1, fpSHP) != 1) {
        fclose(fpSHP);
        fclose(fpSHX);
        return NULL;
    }

    /* Prepare, and write .shx file header */
    i32 = 50;                            /* file size: be */
    ByteCopy(&i32, abyHeader + 24, 4);
    BO_htobe32_buf(abyHeader + 24);

    if (fwrite(abyHeader, 100, 1, fpSHX) != 1) {
        fclose(fpSHP);
        fclose(fpSHX);
        return NULL;
    }

    /* Close the files, and then open them as regular existing files */
    fclose(fpSHP);
    fclose(fpSHX);

    return SHPOpen(pszLayer, "r+b");
}

/**
 * Compute a bounds rectangle for a shape, and set it into the
 *  indicated location in the record
 */
static void _SHPSetBounds(ub1 * pabyRec, SHPObject * psShape)
{
    ByteCopy(&(psShape->dfXMin), pabyRec +  0, 8);
    ByteCopy(&(psShape->dfYMin), pabyRec +  8, 8);
    ByteCopy(&(psShape->dfXMax), pabyRec + 16, 8);
    ByteCopy(&(psShape->dfYMax), pabyRec + 24, 8);

    // 记录内容（包括边界框）必须是 小端序
    BO_htole64_buf(pabyRec + 0);
    BO_htole64_buf(pabyRec + 8);
    BO_htole64_buf(pabyRec + 16);
    BO_htole64_buf(pabyRec + 24);
}

/**
 * Recompute the extents of a shape.
 * Automatically done by SHPCreateObject().
 */
void SHPComputeExtents(SHPObject * psObject)
{
    /* A shape with no vertices has no meaningful extents. */
    if (psObject->nVertices < 1) {
        return;
    }

    /* Initialize extents with the first vertex */
    psObject->dfXMin = psObject->dfXMax = psObject->padfX[0];
    psObject->dfYMin = psObject->dfYMax = psObject->padfY[0];

    /* Initialize Z and M extents only if the respective arrays exist */
    if (psObject->padfZ) {
        psObject->dfZMin = psObject->dfZMax = psObject->padfZ[0];
    }
    if (psObject->padfM) {
        psObject->dfMMin = psObject->dfMMax = psObject->padfM[0];
    }

    /* Loop from the second vertex to find the true min/max */
    for (int i = 1; i < psObject->nVertices; i++) {
        psObject->dfXMin = MIN_V2(psObject->dfXMin, psObject->padfX[i]);
        psObject->dfYMin = MIN_V2(psObject->dfYMin, psObject->padfY[i]);
        psObject->dfXMax = MAX_V2(psObject->dfXMax, psObject->padfX[i]);
        psObject->dfYMax = MAX_V2(psObject->dfYMax, psObject->padfY[i]);

        /* Update Z extents only if Z values are present */
        if (psObject->padfZ) {
            psObject->dfZMin = MIN_V2(psObject->dfZMin, psObject->padfZ[i]);
            psObject->dfZMax = MAX_V2(psObject->dfZMax, psObject->padfZ[i]);
        }

        /* Update M extents only if M values are present */
        if (psObject->padfM) {
            psObject->dfMMin = MIN_V2(psObject->dfMMin, psObject->padfM[i]);
            psObject->dfMMax = MAX_V2(psObject->dfMMax, psObject->padfM[i]);
        }
    }
}

/**
 * Create a shape object.  It should be freed with SHPDestroyObject()
 */
SHPObject *SHPCreateObject (
    int nSHPType,
    int nShapeId,
    int nParts,
    const int *panPartStart,
    const int *panPartType,
    int nVertices,
    const double *padfX,
    const double *padfY,
    const double *padfZ,
    const double *padfM)
{
    SHPObject *psObject = NULL;
    int bHasZ, bHasM;

    if (nVertices < 0 || nParts < 0) {
        /* Invalid Values */
        return NULL;
    }
    if (nVertices > 0 && (!padfX || !padfY)) {
        /* X and Y are required for any vertex */
        return NULL;
    }

    psObject = (SHPObject *) calloc(1, sizeof(SHPObject));
    if (psObject == NULL) {
        return NULL; /* Out of memory */
    }

    psObject->nSHPType = nSHPType;
    psObject->nShapeId = nShapeId;

    /* Establish whether this shape type has M, and Z values */
    SHPHasZM(nSHPType, &bHasZ, &bHasM);

    /* Capture parts.  Note that part type is optional, and defaults to ring */
    if (nParts > 0 && SHPTypeHasParts(nSHPType)) {
        if (panPartStart == NULL) {
            /* nParts > 0 requires a valid panPartStart array */
            SafeFree(psObject);
            return NULL;
        }

        psObject->nParts = nParts;

        /* Allocate part arrays */
        psObject->panPartStart = (int32_t *) calloc(psObject->nParts + 1, sizeof(int32_t));
        if (psObject->panPartStart == NULL) {
            SafeFree(psObject);
            return NULL; /* Out of memory */
        }

        psObject->panPartType = (int32_t *) malloc(sizeof(int32_t) * psObject->nParts);
        if (psObject->panPartType == NULL) {
            SafeFree(psObject->panPartStart);
            SafeFree(psObject);
            return NULL; /* Out of memory */
        }

        /* Copy part information */
        for (int i = 0; i < nParts; i++) {
            psObject->panPartStart[i] = panPartStart[i];
            if (panPartType != NULL) {
                psObject->panPartType[i] = panPartType[i];
            } else {
                psObject->panPartType[i] = SHPP_RING;
            }
        }

        /* Set the sentinel value for the last part's end vertex */
        psObject->panPartStart[psObject->nParts] = nVertices;

        /* Strict validation: first part must start at vertex 0 */
        if (psObject->panPartStart[0] != 0) {
            /* This is an error in the caller's code. Do not silently fix it. */
            SafeFree(psObject->panPartType);
            SafeFree(psObject->panPartStart);
            SafeFree(psObject);
            return NULL; /* Please fix your code! */
        }
    } else {
        /* No parts for this shape type or nParts is 0 */
        psObject->nParts = 0;
        psObject->panPartStart = NULL;
        psObject->panPartType = NULL;
    }

    /* Capture vertices.  Note that Z and M are optional, but X and Y are not */
    if (nVertices > 0) {
        psObject->padfX = (double *) malloc(nVertices * sizeof(double));
        if (psObject->padfX == NULL) {
            SafeFree(psObject->panPartType);
            SafeFree(psObject->panPartStart);
            SafeFree(psObject);
            return NULL; /* Out of memory */
        }
        psObject->padfY = (double *) malloc(nVertices * sizeof(double));
        if (psObject->padfY == NULL) {
            SafeFree(psObject->padfX);
            SafeFree(psObject->panPartType);
            SafeFree(psObject->panPartStart);
            SafeFree(psObject);
            return NULL; /* Out of memory */
        }

        if (padfZ && bHasZ) {
            psObject->padfZ = (double *) malloc(nVertices * sizeof(double));
            if (psObject->padfZ == NULL) {
                SafeFree(psObject->padfX);
                SafeFree(psObject->padfY);
                SafeFree(psObject->panPartType);
                SafeFree(psObject->panPartStart);
                SafeFree(psObject);
                return NULL; /* Out of memory */
            }
        } else {
            psObject->padfZ = (double *) calloc(nVertices, sizeof(double));
        }

        if (padfM  && bHasM) {
            psObject->padfM = (double *) malloc(nVertices * sizeof(double));
            if (psObject->padfM == NULL) {
                SafeFree(psObject->padfX);
                SafeFree(psObject->padfY);
                SafeFree(psObject->padfZ);
                SafeFree(psObject->panPartType);
                SafeFree(psObject->panPartStart);
                SafeFree(psObject);
                return NULL; /* Out of memory */
            }
        } else {
            psObject->padfM = (double *) calloc(nVertices, sizeof(double));
        }

        /* Copy vertex data */
        memcpy(psObject->padfX, padfX, sizeof(double) * nVertices);
        memcpy(psObject->padfY, padfY, sizeof(double) * nVertices);

        if (psObject->padfZ && bHasZ) {
            memcpy(psObject->padfZ, padfZ, sizeof(double) * nVertices);
        }
        if (psObject->padfM && bHasM) {
            memcpy(psObject->padfM, padfM, sizeof(double) * nVertices);
        }
    }

    /* Compute the extents */
    psObject->nVertices = nVertices;
    SHPComputeExtents(psObject);
    return(psObject);
}

/**
 * Create a shape object.  It should be freed with SHPDestroyObject()
 */
SHPObject * SHPCreateObject2 (
    int nSHPType,
    int nShapeId,
    int nParts,
    const int *panPartStart,
    const int *panPartType,
    int nVertices,
    const SHPPointType *padXY,
    const double *padfZ,
    const double * padfM)
{
    int  bHasZ, bHasM;
    SHPObject *psObject = (SHPObject *) calloc(1, sizeof(SHPObject));
    if (psObject == NULL) {
        return NULL; /* Out of memory */
    }

    psObject->nSHPType = nSHPType;
    psObject->nShapeId = nShapeId;

    /* Establish whether this shape type has M, and Z values */
    SHPHasZM (nSHPType, &bHasZ, &bHasM);

    /* Capture parts.  Note that part type is optional, and defaults to ring */
    if (SHPTypeHasParts(nSHPType)) {
        psObject->nParts = MAX_V2(1, nParts);

        psObject->panPartStart = (int32_t *)  malloc(sizeof(int32_t) * (psObject->nParts + 1));
        psObject->panPartType = (int32_t *)  malloc(sizeof(int32_t) * psObject->nParts);
        psObject->panPartStart[0] = 0;
        psObject->panPartStart[psObject->nParts] = nVertices;
        psObject->panPartType[0] = SHPP_RING;

        for (int i = 0; i < nParts; i++) {
            psObject->panPartStart[i] = panPartStart[i];
            if (panPartType != 0) {
                psObject->panPartType[i] = panPartType[i];
            } else {
                psObject->panPartType[i] = SHPP_RING;
            }
        }

        if (psObject->panPartStart[0] != 0) {
            /* Please fix your code! */
            psObject->panPartStart[0] = 0;
        }
    }

    /* Capture vertices.  Note that Z and M are optional, but X and Y are not */
    if (nVertices > 0) {
        psObject->padfX = (double *) calloc(sizeof(double),nVertices);
        psObject->padfY = (double *) calloc(sizeof(double),nVertices);
        psObject->padfZ = (double *) calloc(sizeof(double),nVertices);
        psObject->padfM = (double *) calloc(sizeof(double),nVertices);

        if (!psObject->padfX || !psObject->padfY || psObject->padfZ || psObject->padfM) {
            SafeFree(psObject->padfX);
            SafeFree(psObject->padfY);
            SafeFree(psObject->padfZ);
            SafeFree(psObject->padfM);
            SafeFree(psObject);
            return NULL; /* Out of memory */
        }

        for (int i = 0; i < nVertices; i++) {
            psObject->padfX[i] = padXY[i].x;
            psObject->padfY[i] = padXY[i].y;

            if (padfZ && bHasZ) {
                psObject->padfZ[i] = padfZ[i];
            }
            if (padfM && bHasM) {
                psObject->padfM[i] = padfM[i];
            }
        }
    }

    /* Compute the extents */
    psObject->nVertices = nVertices;
    SHPComputeExtents(psObject);
    return (psObject);
}

/**
 * Create a nil shape object.  Destroy with SHPDestroyObjectEx().
 *  cheungmine@gmail.com
 */
SHPObjectEx * SHPCreateObjectEx (SHPObjectEx ** ppsObject)
{
    SHPObjectEx* psObject = (SHPObjectEx *) calloc(1, sizeof(SHPObjectEx));
    *ppsObject = psObject;
    return psObject;
}

/**
 * Create a simple (common) shape object
 * Destroy with SHPDestroyObject()
 */
SHPObject * SHPCreateSimpleObject (
    int nSHPType,
    int nVertices,
    const double *padfX,
    const double *padfY,
    const double *padfZ)
{
    return SHPCreateObject(nSHPType, -1, 0, 0, 0, nVertices, padfX, padfY, padfZ, 0);
}

/**
 * Write out the vertices of a new structure
 *  Note that it is only possible to write vertices at the end of the file
 */
int SHPWriteObject(SHPHandle psSHP, int nShapeId, SHPObject * psObject)
{
    ub1   *pabyRec;
    int    i32;
    int    nRecordOffset, i, nRecordSize = 0;

    psSHP->bUpdated = SHAPEFILE_TRUE;

    /* Ensure that shape object matches the type of the file it is being written to */
    SHAPEFILE_ASSERT(psObject->nSHPType == psSHP->nShapeType || psObject->nSHPType == SHPT_NULL);

    /* Either blow an assertion, or if they are disabled, set the shapeid to -1 for appends */
    SHAPEFILE_ASSERT(nShapeId == -1 || (nShapeId >= 0 && nShapeId < psSHP->nRecords));

    if (nShapeId != -1 && nShapeId >= psSHP->nRecords) {
        nShapeId = -1;
    }

    /* Add the new entity to the in memory index */
    if (nShapeId == -1 && psSHP->nRecords+1 > psSHP->nMaxRecords) {
        psSHP->nMaxRecords =(uint32_t) (psSHP->nMaxRecords * 1.3 + 100);
        psSHP->panRecOffset = (uint32_t *) SfRealloc(psSHP->panRecOffset, sizeof(uint32_t) * psSHP->nMaxRecords);
        psSHP->panRecSize = (uint32_t *) SfRealloc(psSHP->panRecSize, sizeof(uint32_t) * psSHP->nMaxRecords);
    }

    /* Initialize record */
    pabyRec = (ub1 *) malloc(psObject->nVertices * 4 * sizeof(double) + psObject->nParts * 8 + 128);

    /* Extract vertices for a Polygon or Arc */
    if (psObject->nSHPType == SHPT_POLYGON ||
        psObject->nSHPType == SHPT_POLYGONZ ||
        psObject->nSHPType == SHPT_POLYGONM ||
        psObject->nSHPType == SHPT_ARC ||
        psObject->nSHPType == SHPT_ARCZ ||
        psObject->nSHPType == SHPT_ARCM ||
        psObject->nSHPType == SHPT_MULTIPATCH) {
        int32_t  i, nPoints, nParts;
        nPoints = psObject->nVertices;
        nParts = psObject->nParts;

        _SHPSetBounds(pabyRec + 12, psObject);

        BO_htole32_buf(&nPoints);
        BO_htole32_buf(&nParts);

        ByteCopy(&nPoints, pabyRec + 40 + 8, 4);
        ByteCopy(&nParts, pabyRec + 36 + 8, 4);

        nRecordSize = 52;

        /* Write part start positions */
        ByteCopy(psObject->panPartStart, pabyRec + 44 + 8, 4 * psObject->nParts);

        for (int i = 0; i < psObject->nParts; i++) {
            BO_htole32_buf(pabyRec + 44 + 8 + 4*i);
            nRecordSize += 4;
        }

        /* Write part types */
        ByteCopy(psObject->panPartType, pabyRec + 44 + 8 + 4 * psObject->nParts, 4 * psObject->nParts);

        /* Write multipatch part types if needed */
        if (psObject->nSHPType == SHPT_MULTIPATCH) {
            memcpy(pabyRec + nRecordSize, psObject->panPartType, 4*psObject->nParts);

            for (i = 0; i < psObject->nParts; i++) {
                BO_htole32_buf(pabyRec + nRecordSize);
                nRecordSize += 4;
            }
        }

        /* Write the (x,y) vertex values */
        for (i = 0; i < psObject->nVertices; i++) {
            ByteCopy(psObject->padfX + i, pabyRec + nRecordSize, 8);
            ByteCopy(psObject->padfY + i, pabyRec + nRecordSize + 8, 8);
            BO_htole64_buf(pabyRec + nRecordSize);
            BO_htole64_buf(pabyRec + nRecordSize + 8);
            nRecordSize += 2 * 8;
        }

        /* Write the Z coordinates (if any) */
        if (psObject->nSHPType == SHPT_POLYGONZ ||
            psObject->nSHPType == SHPT_ARCZ ||
            psObject->nSHPType == SHPT_MULTIPATCH) {
            ByteCopy(&(psObject->dfZMin), pabyRec + nRecordSize, 8);
            BO_htole64_buf(pabyRec + nRecordSize);

            nRecordSize += 8;

            ByteCopy(&(psObject->dfZMax), pabyRec + nRecordSize, 8);
            BO_htole64_buf(pabyRec + nRecordSize);

            nRecordSize += 8;

            for (i = 0; i < psObject->nVertices; i++) {
                ByteCopy(psObject->padfZ + i, pabyRec + nRecordSize, 8);
                BO_htole64_buf(pabyRec + nRecordSize);
                nRecordSize += 8;
            }
        }

        /* Write the M values, if any */
        if (psObject->nSHPType == SHPT_POLYGONM || psObject->nSHPType == SHPT_ARCM
#ifndef DISABLE_MULTIPATCH_MEASURE
            || psObject->nSHPType == SHPT_MULTIPATCH
#endif
            || psObject->nSHPType == SHPT_POLYGONZ || psObject->nSHPType == SHPT_ARCZ) {

            ByteCopy(&(psObject->dfMMin), pabyRec + nRecordSize, 8);
            BO_htole64_buf(pabyRec + nRecordSize);

            nRecordSize += 8;

            ByteCopy(&(psObject->dfMMax), pabyRec + nRecordSize, 8);
            BO_htole64_buf(pabyRec + nRecordSize);

            nRecordSize += 8;

            for (i = 0; i < psObject->nVertices; i++) {
                ByteCopy(psObject->padfM + i, pabyRec + nRecordSize, 8);
                BO_htole64_buf(pabyRec + nRecordSize);
                nRecordSize += 8;
            }
        }
    } else if (psObject->nSHPType == SHPT_MULTIPOINT ||
        psObject->nSHPType == SHPT_MULTIPOINTZ ||
        psObject->nSHPType == SHPT_MULTIPOINTM) {
        /* Extract vertices for a MultiPoint */
        int i, nPoints;

        nPoints = psObject->nVertices;

        _SHPSetBounds(pabyRec + 12, psObject);

        BO_htole32_buf(&nPoints);
        ByteCopy(&nPoints, pabyRec + 44, 4);

        for (i = 0; i < psObject->nVertices; i++) {
            ByteCopy(psObject->padfX + i, pabyRec + 48 + i*16, 8);
            ByteCopy(psObject->padfY + i, pabyRec + 48 + i*16 + 8, 8);
            BO_htole64_buf(pabyRec + 48 + i*16);
            BO_htole64_buf(pabyRec + 48 + i*16 + 8);
        }

        nRecordSize = 48 + 16 * psObject->nVertices;

        if (psObject->nSHPType == SHPT_MULTIPOINTZ) {

            ByteCopy(&(psObject->dfZMin), pabyRec + nRecordSize, 8);
            BO_htole64_buf(pabyRec + nRecordSize);
            nRecordSize += 8;

            ByteCopy(&(psObject->dfZMax), pabyRec + nRecordSize, 8);
            BO_htole64_buf(pabyRec + nRecordSize);
            nRecordSize += 8;

            for (i = 0; i < psObject->nVertices; i++) {
                ByteCopy(psObject->padfZ + i, pabyRec + nRecordSize, 8);
                BO_htole64_buf(pabyRec + nRecordSize);
                nRecordSize += 8;
            }
        }

        if (psObject->nSHPType == SHPT_MULTIPOINTZ || psObject->nSHPType == SHPT_MULTIPOINTM) {
            ByteCopy(&(psObject->dfMMin), pabyRec + nRecordSize, 8);
            BO_htole64_buf(pabyRec + nRecordSize);
            nRecordSize += 8;

            ByteCopy(&(psObject->dfMMax), pabyRec + nRecordSize, 8);
            BO_htole64_buf(pabyRec + nRecordSize);
            nRecordSize += 8;

            for (i = 0; i < psObject->nVertices; i++) {
                ByteCopy(psObject->padfM + i, pabyRec + nRecordSize, 8);
                BO_htole64_buf(pabyRec + nRecordSize);
                nRecordSize += 8;
            }
        }
    } else if (psObject->nSHPType == SHPT_POINT ||
        psObject->nSHPType == SHPT_POINTZ ||
        psObject->nSHPType == SHPT_POINTM) {
        /* Write point */
        ByteCopy(psObject->padfX, pabyRec + 12, 8);
        ByteCopy(psObject->padfY, pabyRec + 20, 8);

        BO_htole64_buf(pabyRec + 12);
        BO_htole64_buf(pabyRec + 20);

        nRecordSize = 28;

        if (psObject->nSHPType == SHPT_POINTZ) {
            ByteCopy(psObject->padfZ, pabyRec + nRecordSize, 8);
            BO_htole64_buf(pabyRec + nRecordSize);
            nRecordSize += 8;
        }

        if (psObject->nSHPType == SHPT_POINTZ || psObject->nSHPType == SHPT_POINTM) {
            ByteCopy(psObject->padfM, pabyRec + nRecordSize, 8);
            BO_htole64_buf(pabyRec + nRecordSize);
            nRecordSize += 8;
        }
    } else if (psObject->nSHPType == SHPT_NULL) {
        /* Not much to do for null geometries */
        nRecordSize = 12;
    } else {
        /* unknown type */
        SHAPEFILE_ASSERT(SHAPEFILE_FALSE);
    }

    /* Establish where we are going to put this record. If we are
    *  rewriting and existing record, and it will fit, then put it
    *  back where the original came from.  Otherwise write at the end
    */
    if (nShapeId == -1 || psSHP->panRecSize[nShapeId] < nRecordSize-8) {
        if (nShapeId == -1) {
            nShapeId = psSHP->nRecords++;
        }
        psSHP->panRecOffset[nShapeId] = nRecordOffset = psSHP->nFileSize;
        psSHP->panRecSize[nShapeId] = nRecordSize-8;
        psSHP->nFileSize += nRecordSize;
    } else {
        nRecordOffset = psSHP->panRecOffset[nShapeId];
    }

    /* Set the shape type, record number, and record size */
    i32 = nShapeId+1;                                   /* record # */
    BO_htole32_buf(&i32);
    ByteCopy(&i32, pabyRec, 4);

    i32 = (nRecordSize-8)/2;                            /* record size */
    BO_htole32_buf(&i32);
    ByteCopy(&i32, pabyRec + 4, 4);

    i32 = psObject->nSHPType;                           /* shape type */
    BO_htole32_buf(&i32);
    ByteCopy(&i32, pabyRec + 8, 4);

    /* Write out record */
    if (fseek(psSHP->fpSHP, nRecordOffset, 0) != 0 || fwrite(pabyRec, nRecordSize, 1, psSHP->fpSHP) < 1) {
        SafeFree(pabyRec);
        return -1;
    }

    SafeFree(pabyRec);

    /* Expand file wide bounds based on this shape */
    if (psSHP->adBoundsMin[0] == 0.0 && psSHP->adBoundsMax[0] == 0.0 &&
        psSHP->adBoundsMin[1] == 0.0 && psSHP->adBoundsMax[1] == 0.0) {
        if (psObject->nSHPType == SHPT_NULL || psObject->nVertices == 0) {
            psSHP->adBoundsMin[0] = psSHP->adBoundsMax[0] = 0.0;
            psSHP->adBoundsMin[1] = psSHP->adBoundsMax[1] = 0.0;
            psSHP->adBoundsMin[2] = psSHP->adBoundsMax[2] = 0.0;
            psSHP->adBoundsMin[3] = psSHP->adBoundsMax[3] = 0.0;
        } else {
            psSHP->adBoundsMin[0] = psSHP->adBoundsMax[0] = psObject->padfX[0];
            psSHP->adBoundsMin[1] = psSHP->adBoundsMax[1] = psObject->padfY[0];
            psSHP->adBoundsMin[2] = psSHP->adBoundsMax[2] = psObject->padfZ[0];
            psSHP->adBoundsMin[3] = psSHP->adBoundsMax[3] = psObject->padfM[0];
        }
    }

    for (i = 0; i < psObject->nVertices; i++) {
        psSHP->adBoundsMin[0] = MIN_V2(psSHP->adBoundsMin[0],psObject->padfX[i]);
        psSHP->adBoundsMin[1] = MIN_V2(psSHP->adBoundsMin[1],psObject->padfY[i]);
        psSHP->adBoundsMin[2] = MIN_V2(psSHP->adBoundsMin[2],psObject->padfZ[i]);
        psSHP->adBoundsMin[3] = MIN_V2(psSHP->adBoundsMin[3],psObject->padfM[i]);
        psSHP->adBoundsMax[0] = MAX_V2(psSHP->adBoundsMax[0],psObject->padfX[i]);
        psSHP->adBoundsMax[1] = MAX_V2(psSHP->adBoundsMax[1],psObject->padfY[i]);
        psSHP->adBoundsMax[2] = MAX_V2(psSHP->adBoundsMax[2],psObject->padfZ[i]);
        psSHP->adBoundsMax[3] = MAX_V2(psSHP->adBoundsMax[3],psObject->padfM[i]);
    }

    return(nShapeId );
}

/**
 * Read the vertices, parts, and other non-attribute information for one shape
 */
SHPObject * SHPReadObject(SHPHandle psSHP, int hEntity)
{
    SHPObject *psShape;
    /* Validate the record/entity number */
    if (hEntity < 0 || hEntity >= psSHP->nRecords) {
        return NULL;;
    }
    /* Ensure our record buffer is large enough */
    if (psSHP->panRecSize[hEntity] + 8 > psSHP->nBufSize) {
        psSHP->nBufSize = psSHP->panRecSize[hEntity] + 8;
        psSHP->pabyRec = (ub1 *) SfRealloc(psSHP->pabyRec, psSHP->nBufSize);
        if (!psSHP->pabyRec) {
            return NULL;
        }
    }
    /* Read the record */
    if (fseek(psSHP->fpSHP, psSHP->panRecOffset[hEntity], 0) != 0 ||
        fread(psSHP->pabyRec, psSHP->panRecSize[hEntity] + 8, 1, psSHP->fpSHP) != 1) {
        return 0;
    }
    /* Allocate and minimally initialize the object */
    psShape = (SHPObject *) calloc(1,sizeof(SHPObject));
    if (!psShape) {
        return 0;
    }

    psShape->nShapeId = hEntity;
    memcpy(&psShape->nSHPType, psSHP->pabyRec + 8, 4);
    BO_letoh32_buf(&(psShape->nSHPType));

    /* Extract vertices for a Polygon or Arc */
    if (SHPTypeHasParts(psShape->nSHPType)) {
        int nPoints, nParts, i, nOffset;

        /* Extract part/point count, and build vertex and part arrays to proper size */
        memcpy(&nPoints, psSHP->pabyRec + 40 + 8, 4);
        memcpy(&nParts, psSHP->pabyRec + 36 + 8, 4);

        BO_letoh32_buf(&nPoints);
        BO_letoh32_buf(&nParts);

        if (nPoints==0) {
            free(psShape);
            return 0;
        }

        /* Get the X/Y bounds */
        memcpy(&(psShape->dfXMin), psSHP->pabyRec + 8 +  4, 8);
        memcpy(&(psShape->dfYMin), psSHP->pabyRec + 8 + 12, 8);
        memcpy(&(psShape->dfXMax), psSHP->pabyRec + 8 + 20, 8);
        memcpy(&(psShape->dfYMax), psSHP->pabyRec + 8 + 28, 8);

        BO_letoh64_buf(&(psShape->dfXMin));
        BO_letoh64_buf(&(psShape->dfYMin));
        BO_letoh64_buf(&(psShape->dfXMax));
        BO_letoh64_buf(&(psShape->dfYMax));

        psShape->nVertices = nPoints;
        psShape->padfX = (double *) calloc(nPoints, sizeof(double));
        psShape->padfY = (double *) calloc(nPoints, sizeof(double));
        psShape->padfZ = (double *) calloc(nPoints, sizeof(double));
        psShape->padfM = (double *) calloc(nPoints, sizeof(double));

        psShape->nParts = nParts;
        psShape->panPartStart = (int *) calloc(nParts+1, sizeof(int));
        psShape->panPartType = (int *) calloc(nParts, sizeof(int));

        for (i = 0; i < nParts; i++) {
            psShape->panPartType[i] = SHPP_RING;
        }

        /* Copy out the part array from the record */
        memcpy(psShape->panPartStart, psSHP->pabyRec + 44 + 8, 4 * nParts);

        for (i = 0; i < nParts; i++) {
            BO_letoh32_buf(psShape->panPartStart+i);
        }

        nOffset = 44 + 8 + 4*nParts;
        psShape->panPartStart[psShape->nParts] = psShape->nVertices;

        /* If this is a multipatch, we will also have parts types */
        if (psShape->nSHPType == SHPT_MULTIPATCH) {
            memcpy(psShape->panPartType, psSHP->pabyRec + nOffset, 4*nParts);

            for (i = 0; i < nParts; i++) {
                BO_letoh32_buf(psShape->panPartType+i);
            }

            nOffset += 4*nParts;
        }

        /* Copy out the vertices from the record */
        for (i = 0; i < nPoints; i++) {
            memcpy(psShape->padfX + i, psSHP->pabyRec + nOffset + i * 16, 8);
            memcpy(psShape->padfY + i, psSHP->pabyRec + nOffset + i * 16 + 8, 8);

            BO_letoh64_buf(psShape->padfX + i);
            BO_letoh64_buf(psShape->padfY + i);
        }

        nOffset += 16*nPoints;

        /* If we have a Z coordinate, collect that now */
        if (psShape->nSHPType == SHPT_POLYGONZ || psShape->nSHPType == SHPT_ARCZ || psShape->nSHPType == SHPT_MULTIPATCH) {
            memcpy(&(psShape->dfZMin), psSHP->pabyRec + nOffset, 8);
            memcpy(&(psShape->dfZMax), psSHP->pabyRec + nOffset + 8, 8);

            BO_letoh64_buf(&(psShape->dfZMin));
            BO_letoh64_buf(&(psShape->dfZMax));

            for (i = 0; i < nPoints; i++) {
                memcpy(psShape->padfZ + i, psSHP->pabyRec + nOffset + 16 + i*8, 8);
                BO_letoh64_buf(psShape->padfZ + i);
            }

            nOffset += 16 + 8*nPoints;
        }

        /* If we have a M measure value, then read it now.
        * We assume that the measure can be present for any shape if the size is
        *  big enough, but really it will only occur for the Z shapes (options), and the M shapes */
        if (psSHP->panRecSize[hEntity]+8 >= nOffset + 16 + 8*nPoints) {
            memcpy(&(psShape->dfMMin), psSHP->pabyRec + nOffset, 8);
            memcpy(&(psShape->dfMMax), psSHP->pabyRec + nOffset + 8, 8);

            BO_letoh64_buf(&(psShape->dfMMin));
            BO_letoh64_buf(&(psShape->dfMMax));

            for (i = 0; i < nPoints; i++) {
                memcpy(psShape->padfM + i, psSHP->pabyRec + nOffset + 16 + i*8, 8);
                BO_letoh64_buf(psShape->padfM + i);
            }
        }
    } else if (psShape->nSHPType == SHPT_MULTIPOINT ||
        psShape->nSHPType == SHPT_MULTIPOINTM ||
        psShape->nSHPType == SHPT_MULTIPOINTZ) {
        /* Extract vertices for a MultiPoint */
        int nPoints, i, nOffset;

        memcpy(&nPoints, psSHP->pabyRec + 44, 4);

        BO_letoh32_buf(&nPoints);

        if (nPoints==0) {
            free(psShape);
            return 0;
        }

        psShape->nVertices = nPoints;
        psShape->padfX = (double *) calloc(nPoints,sizeof(double));
        psShape->padfY = (double *) calloc(nPoints,sizeof(double));
        psShape->padfZ = (double *) calloc(nPoints,sizeof(double));
        psShape->padfM = (double *) calloc(nPoints,sizeof(double));

        for (i = 0; i < nPoints; i++) {
            memcpy(psShape->padfX+i, psSHP->pabyRec + 48 + 16 * i, 8);
            memcpy(psShape->padfY+i, psSHP->pabyRec + 48 + 16 * i + 8, 8);

            BO_letoh64_buf(psShape->padfX + i);
            BO_letoh64_buf(psShape->padfY + i);
        }

        nOffset = 48 + 16*nPoints;

        /* Get the X/Y bounds */
        memcpy(&(psShape->dfXMin), psSHP->pabyRec + 8 +  4, 8);
        memcpy(&(psShape->dfYMin), psSHP->pabyRec + 8 + 12, 8);
        memcpy(&(psShape->dfXMax), psSHP->pabyRec + 8 + 20, 8);
        memcpy(&(psShape->dfYMax), psSHP->pabyRec + 8 + 28, 8);

        BO_letoh64_buf(&(psShape->dfXMin));
        BO_letoh64_buf(&(psShape->dfYMin));
        BO_letoh64_buf(&(psShape->dfXMax));
        BO_letoh64_buf(&(psShape->dfYMax));

        /* If we have a Z coordinate, collect that now */
        if (psShape->nSHPType == SHPT_MULTIPOINTZ) {
            memcpy(&(psShape->dfZMin), psSHP->pabyRec + nOffset, 8);
            memcpy(&(psShape->dfZMax), psSHP->pabyRec + nOffset + 8, 8);

            BO_letoh64_buf(&(psShape->dfZMin));
            BO_letoh64_buf(&(psShape->dfZMax));

            for (i = 0; i < nPoints; i++) {
                memcpy(psShape->padfZ + i, psSHP->pabyRec + nOffset + 16 + i*8, 8);
                BO_letoh64_buf(psShape->padfZ + i);
            }

            nOffset += 16 + 8*nPoints;
        }

        /* If we have a M measure value, then read it now.  We assume
        *  that the measure can be present for any shape if the size is
        *  big enough, but really it will only occur for the Z shapes
        * (options), and the M shapes
        */
        if (psSHP->panRecSize[hEntity]+8 >= nOffset + 16 + 8*nPoints) {
            memcpy(&(psShape->dfMMin), psSHP->pabyRec + nOffset, 8);
            memcpy(&(psShape->dfMMax), psSHP->pabyRec + nOffset + 8, 8);

            BO_letoh64_buf(&(psShape->dfMMin));
            BO_letoh64_buf(&(psShape->dfMMax));

            for (i = 0; i < nPoints; i++) {
                memcpy(psShape->padfM + i, psSHP->pabyRec + nOffset + 16 + i*8, 8);
                BO_letoh64_buf(psShape->padfM + i);
            }
        }
    } else if (psShape->nSHPType == SHPT_POINT ||
        psShape->nSHPType == SHPT_POINTM ||
        psShape->nSHPType == SHPT_POINTZ) {
        /* Extract vertices for a point */
        int nOffset;

        psShape->nVertices = 1;
        psShape->padfX = (double *) calloc(1,sizeof(double));
        psShape->padfY = (double *) calloc(1,sizeof(double));
        psShape->padfZ = (double *) calloc(1,sizeof(double));
        psShape->padfM = (double *) calloc(1,sizeof(double));

        memcpy(psShape->padfX, psSHP->pabyRec + 12, 8);
        memcpy(psShape->padfY, psSHP->pabyRec + 20, 8);

        BO_letoh64_buf(psShape->padfX);
        BO_letoh64_buf(psShape->padfY);

        nOffset = 20 + 8;

        /* If we have a Z coordinate, collect that now */
        if (psShape->nSHPType == SHPT_POINTZ) {
            memcpy(psShape->padfZ, psSHP->pabyRec + nOffset, 8);
            BO_letoh64_buf(psShape->padfZ);
            nOffset += 8;
        }

        /* If we have a M measure value, then read it now.  We assume
     *  that the measure can be present for any shape if the size is
     *  big enough, but really it will only occur for the Z shapes
     * (options), and the M shapes.
     */
        if (psSHP->panRecSize[hEntity]+8 >= nOffset + 8) {
            memcpy(psShape->padfM, psSHP->pabyRec + nOffset, 8);
            BO_letoh64_buf(psShape->padfM);
        }

        /* Since no extents are supplied in the record, apply them from the single vertex */
        psShape->dfXMin = psShape->dfXMax = psShape->padfX[0];
        psShape->dfYMin = psShape->dfYMax = psShape->padfY[0];
        psShape->dfZMin = psShape->dfZMax = psShape->padfZ[0];
        psShape->dfMMin = psShape->dfMMax = psShape->padfM[0];
    } else {
        free(psShape);
        return 0;
    }

    return(psShape);
}

/**
 * return SHPType
 */
int SHPReadObjectBounds(SHPHandle psSHP, int hEntity, SHPBounds *Bounds, double *pointEpsilon)
{
    int nSHPType;

    if (psSHP->panRecSize[hEntity]+8 > psSHP->nBufSize) {
        psSHP->nBufSize = psSHP->panRecSize[hEntity]+8;
        psSHP->pabyRec = (ub1 *) SfRealloc(psSHP->pabyRec,psSHP->nBufSize);
    }

    if (fseek(psSHP->fpSHP, psSHP->panRecOffset[hEntity], 0) != 0 ||
        fread(psSHP->pabyRec, psSHP->panRecSize[hEntity]+8, 1, psSHP->fpSHP) != 1) {
        return (SHPT_NULL);
    }

    memcpy(&nSHPType, psSHP->pabyRec + 8, 4);
    BO_letoh32_buf(&(nSHPType));

    if (nSHPType == SHPT_POLYGON ||
        nSHPType == SHPT_ARC ||
        nSHPType == SHPT_POLYGONZ ||
        nSHPType == SHPT_POLYGONM ||
        nSHPType == SHPT_ARCZ ||
        nSHPType == SHPT_ARCM ||
        nSHPType == SHPT_MULTIPATCH) {
        int nPoints, nParts, nOffset;

        memcpy(&nPoints, psSHP->pabyRec + 40 + 8, 4);
        memcpy(&nParts, psSHP->pabyRec + 36 + 8, 4);

        BO_letoh32_buf(&nPoints);
        BO_letoh32_buf(&nParts);

        if (nPoints <= 0) {
            return (SHPT_NULL);
        }

        memcpy(&(Bounds->XMin), psSHP->pabyRec + 8 +  4, 8);
        memcpy(&(Bounds->YMin), psSHP->pabyRec + 8 + 12, 8);
        memcpy(&(Bounds->XMax), psSHP->pabyRec + 8 + 20, 8);
        memcpy(&(Bounds->YMax), psSHP->pabyRec + 8 + 28, 8);

        BO_letoh64_buf(&(Bounds->XMin));
        BO_letoh64_buf(&(Bounds->YMin));
        BO_letoh64_buf(&(Bounds->XMax));
        BO_letoh64_buf(&(Bounds->YMax));

        nOffset = 44 + 8 + 4*nParts;

        if (nSHPType == SHPT_MULTIPATCH) {
            nOffset += 4*nParts;
        }

        nOffset += 16*nPoints;

        if (nSHPType == SHPT_POLYGONZ ||
            nSHPType == SHPT_ARCZ ||
            nSHPType == SHPT_MULTIPATCH) {
            memcpy(&(Bounds->ZMin), psSHP->pabyRec + nOffset, 8);
            memcpy(&(Bounds->ZMax), psSHP->pabyRec + nOffset + 8, 8);

            BO_letoh64_buf(&(Bounds->ZMin));
            BO_letoh64_buf(&(Bounds->ZMax));

            nOffset += 16 + 8*nPoints;
        }

        if (psSHP->panRecSize[hEntity]+8 >= nOffset + 16 + 8*nPoints) {
            memcpy(&(Bounds->MMin), psSHP->pabyRec + nOffset, 8);
            memcpy(&(Bounds->MMax), psSHP->pabyRec + nOffset + 8, 8);

            BO_letoh64_buf(&(Bounds->MMin));
            BO_letoh64_buf(&(Bounds->MMax));
        }
    } else if (nSHPType == SHPT_MULTIPOINT ||
        nSHPType == SHPT_MULTIPOINTM ||
        nSHPType == SHPT_MULTIPOINTZ) {
        int nPoints, nOffset;
        memcpy(&nPoints, psSHP->pabyRec + 44, 4);

        BO_letoh32_buf(&nPoints);

        if (nPoints <= 0) {
            return (SHPT_NULL);
        }

        nOffset = 48 + 16*nPoints;

        memcpy(&(Bounds->XMin), psSHP->pabyRec + 8 +  4, 8);
        memcpy(&(Bounds->YMin), psSHP->pabyRec + 8 + 12, 8);
        memcpy(&(Bounds->XMax), psSHP->pabyRec + 8 + 20, 8);
        memcpy(&(Bounds->YMax), psSHP->pabyRec + 8 + 28, 8);

        BO_letoh64_buf(&(Bounds->XMin));
        BO_letoh64_buf(&(Bounds->YMin));
        BO_letoh64_buf(&(Bounds->XMax));
        BO_letoh64_buf(&(Bounds->YMax));

        if (nSHPType == SHPT_MULTIPOINTZ) {
            memcpy(&(Bounds->ZMin), psSHP->pabyRec + nOffset, 8);
            memcpy(&(Bounds->ZMax), psSHP->pabyRec + nOffset + 8, 8);

            BO_letoh64_buf(&(Bounds->ZMin));
            BO_letoh64_buf(&(Bounds->ZMax));

            nOffset += 16 + 8*nPoints;
        }

        if (psSHP->panRecSize[hEntity]+8 >= nOffset + 16 + 8*nPoints) {
            memcpy(&(Bounds->MMin), psSHP->pabyRec + nOffset, 8);
            memcpy(&(Bounds->MMax), psSHP->pabyRec + nOffset + 8, 8);

            BO_letoh64_buf(&(Bounds->MMin));
            BO_letoh64_buf(&(Bounds->MMax));
        }
    } else if (nSHPType == SHPT_POINT ||
        nSHPType == SHPT_POINTM ||
        nSHPType == SHPT_POINTZ) {
        int nOffset;

        memcpy(&Bounds->XMin, psSHP->pabyRec + 12, 8);
        memcpy(&Bounds->YMin, psSHP->pabyRec + 20, 8);

        BO_letoh64_buf(&Bounds->XMin);
        BO_letoh64_buf(&Bounds->YMin);

        nOffset = 20 + 8;

        if (nSHPType == SHPT_POINTZ) {
            memcpy(&Bounds->ZMin, psSHP->pabyRec + nOffset, 8);
            BO_letoh64_buf(&Bounds->ZMin);
            nOffset += 8;
        }

        if (psSHP->panRecSize[hEntity]+8 >= nOffset + 8) {
            memcpy(&Bounds->MMin, psSHP->pabyRec + nOffset, 8);
            BO_letoh64_buf(&Bounds->MMin);
        }

        if (pointEpsilon) {
            double Epsilon = *pointEpsilon;
            Bounds->XMax = Bounds->XMin + Epsilon;
            Bounds->YMax = Bounds->YMin + Epsilon;
            Bounds->ZMax = Bounds->ZMin + Epsilon;
            Bounds->MMax = Bounds->MMin + Epsilon;
        } else {
            Bounds->XMax = Bounds->XMin;
            Bounds->YMax = Bounds->YMin;
            Bounds->ZMax = Bounds->ZMin;
            Bounds->MMax = Bounds->MMin;
        }
    } else {
        return(SHPT_NULL);
    }

    return(nSHPType);
}


SHAPEFILE_API int SHPReadObjectEnvelope(SHPHandle psSHP, int hEntity, SHPEnvelope *env, double *pointEpsilon)
{
    int nSHPType;

    if (psSHP->panRecSize[hEntity]+8 > psSHP->nBufSize) {
        psSHP->nBufSize = psSHP->panRecSize[hEntity]+8;
        psSHP->pabyRec = (ub1 *) SfRealloc(psSHP->pabyRec,psSHP->nBufSize);
    }

    if (fseek(psSHP->fpSHP, psSHP->panRecOffset[hEntity], 0) != 0 ||
        fread(psSHP->pabyRec, psSHP->panRecSize[hEntity]+8, 1, psSHP->fpSHP) != 1) {
        return (SHPT_NULL);
    }

    memcpy(&nSHPType, psSHP->pabyRec + 8, 4);
    BO_letoh32_buf(&(nSHPType));

    if (nSHPType == SHPT_POLYGON ||
        nSHPType == SHPT_ARC ||
        nSHPType == SHPT_POLYGONZ ||
        nSHPType == SHPT_POLYGONM ||
        nSHPType == SHPT_ARCZ ||
        nSHPType == SHPT_ARCM ||
        nSHPType == SHPT_MULTIPATCH) {
        int nPoints, nParts;

        memcpy(&nPoints, psSHP->pabyRec + 40 + 8, 4);
        memcpy(&nParts, psSHP->pabyRec + 36 + 8, 4);

        BO_letoh32_buf(&nPoints);
        BO_letoh32_buf(&nParts);

        if (nPoints <= 0) {
            return (SHPT_NULL);
        }

        memcpy(&(env->XMin), psSHP->pabyRec + 8 +  4, 8);
        memcpy(&(env->YMin), psSHP->pabyRec + 8 + 12, 8);
        memcpy(&(env->XMax), psSHP->pabyRec + 8 + 20, 8);
        memcpy(&(env->YMax), psSHP->pabyRec + 8 + 28, 8);

        BO_letoh64_buf(&(env->XMin));
        BO_letoh64_buf(&(env->YMin));
        BO_letoh64_buf(&(env->XMax));
        BO_letoh64_buf(&(env->YMax));
    } else if (nSHPType == SHPT_MULTIPOINT ||
        nSHPType == SHPT_MULTIPOINTM ||
        nSHPType == SHPT_MULTIPOINTZ) {
        int nPoints;
        memcpy(&nPoints, psSHP->pabyRec + 44, 4);

        BO_letoh32_buf(&nPoints);

        if (nPoints <= 0) {
            return (SHPT_NULL);
        }

        memcpy(&(env->XMin), psSHP->pabyRec + 8 +  4, 8);
        memcpy(&(env->YMin), psSHP->pabyRec + 8 + 12, 8);
        memcpy(&(env->XMax), psSHP->pabyRec + 8 + 20, 8);
        memcpy(&(env->YMax), psSHP->pabyRec + 8 + 28, 8);

        BO_letoh64_buf(&(env->XMin));
        BO_letoh64_buf(&(env->YMin));
        BO_letoh64_buf(&(env->XMax));
        BO_letoh64_buf(&(env->YMax));
    } else if (nSHPType == SHPT_POINT ||
        nSHPType == SHPT_POINTM ||
        nSHPType == SHPT_POINTZ) {
        memcpy(&env->XMin, psSHP->pabyRec + 12, 8);
        memcpy(&env->YMin, psSHP->pabyRec + 20, 8);

        BO_letoh64_buf(&env->XMin);
        BO_letoh64_buf(&env->YMin);

        if (pointEpsilon) {
            double Epsilon = *pointEpsilon;
            env->XMax = env->XMin + Epsilon;
            env->YMax = env->YMin + Epsilon;
        } else {
            env->XMax = env->XMin;
            env->YMax = env->YMin;
        }
    } else {
        return(SHPT_NULL);
    }

    return(nSHPType);
}


/**
 * Read the vertices, parts, and other non-attribute information for one shape
 *   cheungmine 2008-12
 */
int SHPReadObjectEx(SHPHandle psSHP, int hEntity, SHPObjectEx *psShape)
{
    /* Validate the record/entity number */
    if (hEntity < 0 || hEntity >= psSHP->nRecords) {
        return (SHAPEFILE_FALSE);
    }

    /* Ensure our record buffer is large enough */
    if (psSHP->panRecSize[hEntity] + 8 > psSHP->nBufSize) {
        psSHP->nBufSize = psSHP->panRecSize[hEntity] + 8;
        psSHP->pabyRec = (ub1 *) SfRealloc(psSHP->pabyRec, psSHP->nBufSize);
        if (!psSHP->pabyRec) {
            return (SHAPEFILE_FALSE);
        }
    }

    /* Read the record */
    if (fseek(psSHP->fpSHP, psSHP->panRecOffset[hEntity], 0) != 0 ||
        fread(psSHP->pabyRec, psSHP->panRecSize[hEntity]+8, 1, psSHP->fpSHP) != 1) {
        return(SHAPEFILE_FALSE);
    }

    /* Allocate and minimally initialize the object */
    psShape->nShapeId = hEntity;
    memcpy(&psShape->nSHPType, psSHP->pabyRec + 8, 4);
    BO_letoh32_buf(&(psShape->nSHPType));

    /* Extract vertices for a Polygon or Arc */
    if (psShape->nSHPType == SHPT_POLYGON ||
        psShape->nSHPType == SHPT_ARC ||
        psShape->nSHPType == SHPT_POLYGONZ ||
        psShape->nSHPType == SHPT_POLYGONM ||
        psShape->nSHPType == SHPT_ARCZ ||
        psShape->nSHPType == SHPT_ARCM ||
        psShape->nSHPType == SHPT_MULTIPATCH) {
        int nPoints, nParts, i, nOffset;

        /* Extract part/point count, and build vertex and part arrays to proper size */
        memcpy(&nPoints, psSHP->pabyRec + 40 + 8, 4);
        memcpy(&nParts, psSHP->pabyRec + 36 + 8, 4);

        BO_letoh32_buf(&nPoints);
        BO_letoh32_buf(&nParts);

        if (! nPoints) {
            return SHAPEFILE_FALSE;
        }

        /* Get the X/Y bounds */
        memcpy(&(psShape->dfXMin), psSHP->pabyRec + 8 +  4, 8);
        memcpy(&(psShape->dfYMin), psSHP->pabyRec + 8 + 12, 8);
        memcpy(&(psShape->dfXMax), psSHP->pabyRec + 8 + 20, 8);
        memcpy(&(psShape->dfYMax), psSHP->pabyRec + 8 + 28, 8);

        BO_letoh64_buf(&(psShape->dfXMin));
        BO_letoh64_buf(&(psShape->dfYMin));
        BO_letoh64_buf(&(psShape->dfXMax));
        BO_letoh64_buf(&(psShape->dfYMax));

        psShape->nVertices = nPoints;

        if (psShape->nPointsSize < nPoints) {
            psShape->nPointsSize = (nPoints/MEM_BLKSIZE+1)*MEM_BLKSIZE;
            psShape->pPoints = (SHPPointType *) realloc(psShape->pPoints, psShape->nPointsSize*sizeof(SHPPointType));
            psShape->padfZ = (double *) realloc(psShape->padfZ, psShape->nPointsSize*sizeof(double));
            psShape->padfM = (double *) realloc(psShape->padfM, psShape->nPointsSize*sizeof(double));
        }

        psShape->nParts = nParts;

        if (psShape->nPartsSize <= nParts) {
            psShape->nPartsSize = ((nParts+16)/16)*16;
            psShape->panPartStart = (int *) realloc(psShape->panPartStart, psShape->nPartsSize*sizeof(int));
            psShape->panPartType = (int *) realloc(psShape->panPartType, psShape->nPartsSize*sizeof(int));
        }

        for (i = 0; i < nParts; i++) {
            psShape->panPartType[i] = SHPP_RING;
        }

        /* Copy out the part array from the record */
        memcpy(psShape->panPartStart, psSHP->pabyRec + 44 + 8, 4 * nParts);

        for (i = 0; i < nParts; i++) {
            BO_letoh32_buf(psShape->panPartStart+i);
        }

        nOffset = 44 + 8 + 4*nParts;

        psShape->panPartStart[psShape->nParts] = psShape->nVertices;

        /* If this is a multipatch, we will also have parts types */
        if (psShape->nSHPType == SHPT_MULTIPATCH) {
            memcpy(psShape->panPartType, psSHP->pabyRec + nOffset, 4*nParts);

            for (i = 0; i < nParts; i++) {
                BO_letoh32_buf(psShape->panPartType+i);
            }

            nOffset += 4*nParts;
        }

        /* Copy out the vertices from the record */
        for (i = 0; i < nPoints; i++) {
            memcpy(&psShape->pPoints[i].x, psSHP->pabyRec + nOffset + i * 16, 8);
            memcpy(&psShape->pPoints[i].y, psSHP->pabyRec + nOffset + i * 16 + 8, 8);
        }

        for (i = 0; i < nPoints; i++) {
            BO_letoh64_buf(&psShape->pPoints[i].x);
            BO_letoh64_buf(&psShape->pPoints[i].y);
        }

        nOffset += 16*nPoints;

        /* If we have a Z coordinate, collect that now */
        if (psShape->nSHPType == SHPT_POLYGONZ || psShape->nSHPType == SHPT_ARCZ || psShape->nSHPType == SHPT_MULTIPATCH) {
            memcpy(&(psShape->dfZMin), psSHP->pabyRec + nOffset, 8);
            memcpy(&(psShape->dfZMax), psSHP->pabyRec + nOffset + 8, 8);

            BO_letoh64_buf(&(psShape->dfZMin));
            BO_letoh64_buf(&(psShape->dfZMax));

            for (i = 0; i < nPoints; i++) {
                memcpy(psShape->padfZ + i, psSHP->pabyRec + nOffset + 16 + i*8, 8);
            }
            for (i = 0; i < nPoints; i++) {
                BO_letoh64_buf(psShape->padfZ + i);
            }
            nOffset += 16 + 8*nPoints;
        }

        /* If we have a M measure value, then read it now.  We assume
         *  that the measure can be present for any shape if the size is
         *  big enough, but really it will only occur for the Z shapes
         *  (options), and the M shapes.
         */
        if (psSHP->panRecSize[hEntity]+8 >= nOffset + 16 + 8*nPoints) {
            memcpy(&(psShape->dfMMin), psSHP->pabyRec + nOffset, 8);
            memcpy(&(psShape->dfMMax), psSHP->pabyRec + nOffset + 8, 8);

            BO_letoh64_buf(&(psShape->dfMMin));
            BO_letoh64_buf(&(psShape->dfMMax));

            for (i = 0; i < nPoints; i++) {
                memcpy(psShape->padfM + i, psSHP->pabyRec + nOffset + 16 + i*8, 8);
                BO_letoh64_buf(psShape->padfM + i);
            }
        }
    } else if (psShape->nSHPType == SHPT_MULTIPOINT ||
        psShape->nSHPType == SHPT_MULTIPOINTM ||
        psShape->nSHPType == SHPT_MULTIPOINTZ) {
        /* Extract vertices for a MultiPoint */
        int nPoints, i, nOffset;

        memcpy(&nPoints, psSHP->pabyRec + 44, 4);
        BO_letoh32_buf(&nPoints);
        if (! nPoints) {
            return SHAPEFILE_FALSE;
        }
        psShape->nVertices = nPoints;
        if (psShape->nPointsSize < nPoints) {
            psShape->nPointsSize = (nPoints/MEM_BLKSIZE+1)*MEM_BLKSIZE;
            psShape->pPoints = (SHPPointType *) realloc(psShape->pPoints, psShape->nPointsSize*sizeof(SHPPointType));
            psShape->padfZ = (double *) realloc(psShape->padfZ, psShape->nPointsSize*sizeof(double));
            psShape->padfM = (double *) realloc(psShape->padfM, psShape->nPointsSize*sizeof(double));
        }
        for (i = 0; i < nPoints; i++) {
            memcpy(&psShape->pPoints[i].x, psSHP->pabyRec + 48 + 16 * i, 8);
            memcpy(&psShape->pPoints[i].y, psSHP->pabyRec + 48 + 16 * i + 8, 8);

            BO_letoh64_buf(&psShape->pPoints[i].x);
            BO_letoh64_buf(&psShape->pPoints[i].y);
        }
        nOffset = 48 + 16*nPoints;

        /* Get the X/Y bounds */
        memcpy(&(psShape->dfXMin), psSHP->pabyRec + 8 +  4, 8);
        memcpy(&(psShape->dfYMin), psSHP->pabyRec + 8 + 12, 8);
        memcpy(&(psShape->dfXMax), psSHP->pabyRec + 8 + 20, 8);
        memcpy(&(psShape->dfYMax), psSHP->pabyRec + 8 + 28, 8);

        BO_letoh64_buf(&(psShape->dfXMin));
        BO_letoh64_buf(&(psShape->dfYMin));
        BO_letoh64_buf(&(psShape->dfXMax));
        BO_letoh64_buf(&(psShape->dfYMax));

        /* If we have a Z coordinate, collect that now */
        if (psShape->nSHPType == SHPT_MULTIPOINTZ) {
            memcpy(&(psShape->dfZMin), psSHP->pabyRec + nOffset, 8);
            memcpy(&(psShape->dfZMax), psSHP->pabyRec + nOffset + 8, 8);

            BO_letoh64_buf(&(psShape->dfZMin));
            BO_letoh64_buf(&(psShape->dfZMax));

            for (i = 0; i < nPoints; i++) {
                memcpy(psShape->padfZ + i, psSHP->pabyRec + nOffset + 16 + i*8, 8);
                BO_letoh64_buf(psShape->padfZ + i);
            }

            nOffset += 16 + 8*nPoints;
        }

        /* If we have a M measure value, then read it now.  We assume
         *  that the measure can be present for any shape if the size is
         *  big enough, but really it will only occur for the Z shapes
         * (options), and the M shapes
         */
        if (psSHP->panRecSize[hEntity]+8 >= nOffset + 16 + 8*nPoints) {
            memcpy(&(psShape->dfMMin), psSHP->pabyRec + nOffset, 8);
            memcpy(&(psShape->dfMMax), psSHP->pabyRec + nOffset + 8, 8);

            BO_letoh64_buf(&(psShape->dfMMin));
            BO_letoh64_buf(&(psShape->dfMMax));

            for (i = 0; i < nPoints; i++) {
                memcpy(psShape->padfM + i, psSHP->pabyRec + nOffset + 16 + i*8, 8);
                BO_letoh64_buf(psShape->padfM + i);
            }
        }
    } else if (psShape->nSHPType == SHPT_POINT ||
        psShape->nSHPType == SHPT_POINTM ||
        psShape->nSHPType == SHPT_POINTZ) {
        /* Extract vertices for a point */
        int nOffset;
        psShape->nVertices = 1;
        if (psShape->nPointsSize < 1) {
            psShape->nPointsSize = 8;
            psShape->pPoints = (SHPPointType *) realloc(psShape->pPoints, psShape->nPointsSize*sizeof(SHPPointType));
            psShape->padfZ = (double *) realloc(psShape->padfZ, psShape->nPointsSize*sizeof(double));
            psShape->padfM = (double *) realloc(psShape->padfM, psShape->nPointsSize*sizeof(double));
        }
        memcpy(&psShape->pPoints[0].x, psSHP->pabyRec + 12, 8);
        memcpy(&psShape->pPoints[0].y, psSHP->pabyRec + 20, 8);

        BO_letoh64_buf(&psShape->pPoints[0].x);
        BO_letoh64_buf(&psShape->pPoints[0].y);

        nOffset = 20 + 8;

        /* If we have a Z coordinate, collect that now */
        if (psShape->nSHPType == SHPT_POINTZ) {
            memcpy(psShape->padfZ, psSHP->pabyRec + nOffset, 8);
            BO_letoh64_buf(psShape->padfZ);
            nOffset += 8;
        }

        /* If we have a M measure value, then read it now.  We assume
         *  that the measure can be present for any shape if the size is
         *  big enough, but really it will only occur for the Z shapes
         *  (options), and the M shapes
         */
        if (psSHP->panRecSize[hEntity]+8 >= nOffset + 8) {
            memcpy(psShape->padfM, psSHP->pabyRec + nOffset, 8);
            BO_letoh64_buf(psShape->padfM);
        }

        /* Since no extents are supplied in the record, apply them from the single vertex */
        psShape->dfXMin = psShape->dfXMax = psShape->pPoints[0].x;
        psShape->dfYMin = psShape->dfYMax = psShape->pPoints[0].y;
        psShape->dfZMin = psShape->dfZMax = psShape->padfZ[0];
        psShape->dfMMin = psShape->dfMMax = psShape->padfM[0];
    } else {
        return (SHAPEFILE_FALSE);
    }
    return (SHAPEFILE_TRUE);
}

/**
 * SHPTypeName
 */
const char * SHPTypeName(int nSHPType)
{
    switch(nSHPType) {
    case SHPT_NULL:
        return "NullShape";
    case SHPT_POINT:
        return "Point";
    case SHPT_ARC:
        return "Arc";
    case SHPT_POLYGON:
        return "Polygon";
    case SHPT_MULTIPOINT:
        return "MultiPoint";
    case SHPT_POINTZ:
        return "PointZ";
    case SHPT_ARCZ:
        return "ArcZ";
    case SHPT_POLYGONZ:
        return "PolygonZ";
    case SHPT_MULTIPOINTZ:
        return "MultiPointZ";
    case SHPT_POINTM:
        return "PointM";
    case SHPT_ARCM:
        return "ArcM";
    case SHPT_POLYGONM:
        return "PolygonM";
    case SHPT_MULTIPOINTM:
        return "MultiPointM";
    case SHPT_MULTIPATCH:
        return "MultiPatch";
    default:
        return "UnknownShapeType";
    }
}

/**
 * SHPPartTypeName
 */
const char * SHPPartTypeName(int nPartType)
{
    switch(nPartType) {
    case SHPP_TRISTRIP:
        return "TriangleStrip";
    case SHPP_TRIFAN:
        return "TriangleFan";
    case SHPP_OUTERRING:
        return "OuterRing";
    case SHPP_INNERRING:
        return "InnerRing";
    case SHPP_FIRSTRING:
        return "FirstRing";
    case SHPP_RING:
        return "Ring";
    default:
        return "UnknownPartType";
    }
}

void SHPDestroyObject(SHPObject * psShape)
{
    if (psShape) {
        SafeFree(psShape->padfX);
        SafeFree(psShape->padfY);
        SafeFree(psShape->padfZ);
        SafeFree(psShape->padfM);
        SafeFree(psShape->panPartStart);
        SafeFree(psShape->panPartType);
        SafeFree(psShape);
    }
}

void  SHPDestroyObjectEx(SHPObjectEx * psShape)
{
    if (psShape) {
        SafeFree(psShape->pPoints);
        SafeFree(psShape->padfZ);
        SafeFree(psShape->padfM);
        SafeFree(psShape->panPartStart);
        SafeFree(psShape->panPartType);
        SafeFree(psShape);
    }
}

/**
 * Reset the winding of polygon objects to adhere to the specification
 */
int SHPRewindObject(SHPObject *psObject)
{
    int  iOpRing, bAltered = 0;

    /* Do nothing if this is not a polygon object */
    if (psObject->nSHPType != SHPT_POLYGON &&
        psObject->nSHPType != SHPT_POLYGONZ &&
        psObject->nSHPType != SHPT_POLYGONM) {
        return 0;
    }

    if (psObject->nVertices == 0 || psObject->nParts == 0) {
        return 0;
    }

    /* Process each of the rings */
    for (iOpRing = 0; iOpRing < psObject->nParts; iOpRing++) {
        int      bInner, iVert, nVertCount, nVertStart, iCheckRing;
        double   dfSum, dfTestX, dfTestY;

        /* Determine if this ring is an inner ring or an outer ring
        *  relative to all the other rings.  For now we assume the
        *  first ring is outer and all others are inner, but eventually
        *  we need to fix this to handle multiple island polygons and
        *  unordered sets of rings
        */
        dfTestX = psObject->padfX[psObject->panPartStart[iOpRing]];
        dfTestY = psObject->padfY[psObject->panPartStart[iOpRing]];

        bInner = SHAPEFILE_FALSE;
        for (iCheckRing = 0; iCheckRing < psObject->nParts; iCheckRing++) {
            int iEdge;

            if (iCheckRing == iOpRing) {
                continue;
            }

            nVertStart = psObject->panPartStart[iCheckRing];

            nVertCount = psObject->panPartStart[iCheckRing+1] - psObject->panPartStart[iCheckRing];

            for (iEdge = 0; iEdge < nVertCount; iEdge++) {
                int iNext;

                if (iEdge < nVertCount-1) {
                    iNext = iEdge+1;
                } else {
                    iNext = 0;
                }

                if ((psObject->padfY[iEdge+nVertStart] < dfTestY && psObject->padfY[iNext+nVertStart] >= dfTestY) ||
                    (psObject->padfY[iNext+nVertStart] < dfTestY && psObject->padfY[iEdge+nVertStart] >= dfTestY)) {
                    if (psObject->padfX[iEdge+nVertStart] +
                        (dfTestY - psObject->padfY[iEdge+nVertStart]) / (psObject->padfY[iNext+nVertStart] -
                        psObject->padfY[iEdge+nVertStart]) *
                        (psObject->padfX[iNext+nVertStart] - psObject->padfX[iEdge+nVertStart]) < dfTestX) {
                        bInner = !bInner;
                    }
                }
            }
        }

        /* Determine the current order of this ring so we will know if it has to be reversed */
        nVertStart = psObject->panPartStart[iOpRing];

        nVertCount = psObject->panPartStart[iOpRing+1] - psObject->panPartStart[iOpRing];

        dfSum = 0.0;
        for (iVert = nVertStart; iVert < nVertStart+nVertCount-1; iVert++) {
            dfSum += psObject->padfX[iVert] * psObject->padfY[iVert+1] - psObject->padfY[iVert] * psObject->padfX[iVert+1];
        }

        dfSum += psObject->padfX[iVert] * psObject->padfY[nVertStart] - psObject->padfY[iVert] * psObject->padfX[nVertStart];

        /* Reverse if necessary */
        if ((dfSum < 0.0 && bInner) || (dfSum > 0.0 && !bInner)) {
            int   i;
            bAltered++;

            for (i = 0; i < nVertCount/2; i++) {
                double dfSaved;

                /* Swap X */
                dfSaved = psObject->padfX[nVertStart+i];
                psObject->padfX[nVertStart+i] =
                psObject->padfX[nVertStart+nVertCount-i-1];
                psObject->padfX[nVertStart+nVertCount-i-1] = dfSaved;

                /* Swap Y */
                dfSaved = psObject->padfY[nVertStart+i];
                psObject->padfY[nVertStart+i] =
                psObject->padfY[nVertStart+nVertCount-i-1];
                psObject->padfY[nVertStart+nVertCount-i-1] = dfSaved;

                /* Swap Z */
                if (psObject->padfZ) {
                    dfSaved = psObject->padfZ[nVertStart+i];
                    psObject->padfZ[nVertStart+i] =
                    psObject->padfZ[nVertStart+nVertCount-i-1];
                    psObject->padfZ[nVertStart+nVertCount-i-1] = dfSaved;
                }

                /* Swap M */
                if (psObject->padfM) {
                    dfSaved = psObject->padfM[nVertStart+i];
                    psObject->padfM[nVertStart+i] =
                    psObject->padfM[nVertStart+nVertCount-i-1];
                    psObject->padfM[nVertStart+nVertCount-i-1] = dfSaved;
                }
            }
        }
    }
    return bAltered;
}

int SHPRewindObjectEx(SHPObjectEx *psObject)
{
 int  iOpRing, bAltered = 0;

    /* Do nothing if this is not a polygon object */
    if (psObject->nSHPType != SHPT_POLYGON &&
        psObject->nSHPType != SHPT_POLYGONZ &&
        psObject->nSHPType != SHPT_POLYGONM) {
        return 0;
    }

    if (psObject->nVertices == 0 || psObject->nParts == 0) {
        return 0;
    }

    /* Process each of the rings */
    for (iOpRing = 0; iOpRing < psObject->nParts; iOpRing++) {
        int      bInner, iVert, nVertCount, nVertStart, iCheckRing;
        double   dfSum, dfTestX, dfTestY;

        /* Determine if this ring is an inner ring or an outer ring
        *  relative to all the other rings.  For now we assume the
        *  first ring is outer and all others are inner, but eventually
        *  we need to fix this to handle multiple island polygons and
        *  unordered sets of rings
        */
        dfTestX = psObject->pPoints[psObject->panPartStart[iOpRing]].x;
        dfTestY = psObject->pPoints[psObject->panPartStart[iOpRing]].y;

        bInner = SHAPEFILE_FALSE;
        for (iCheckRing = 0; iCheckRing < psObject->nParts; iCheckRing++) {
            int iEdge;
            if (iCheckRing == iOpRing) {
                continue;
            }
            nVertStart = psObject->panPartStart[iCheckRing];
            nVertCount = psObject->panPartStart[iCheckRing+1] - psObject->panPartStart[iCheckRing];
            for (iEdge = 0; iEdge < nVertCount; iEdge++) {
                int iNext;
                if (iEdge < nVertCount-1) {
                    iNext = iEdge+1;
                } else {
                    iNext = 0;
                }
                if ((psObject->pPoints[iEdge+nVertStart].y < dfTestY && psObject->pPoints[iNext+nVertStart].y >= dfTestY) ||
                    (psObject->pPoints[iNext+nVertStart].y < dfTestY && psObject->pPoints[iEdge+nVertStart].y >= dfTestY)) {
                    if (psObject->pPoints[iEdge+nVertStart].x + (dfTestY - psObject->pPoints[iEdge+nVertStart].y) /
                        (psObject->pPoints[iNext+nVertStart].y - psObject->pPoints[iEdge+nVertStart].y) *
                        (psObject->pPoints[iNext+nVertStart].x - psObject->pPoints[iEdge+nVertStart].x) < dfTestX) {
                        bInner = !bInner;
                    }
                }
            }
        }

        /* Determine the current order of this ring so we will know if it has to be reversed */
        nVertStart = psObject->panPartStart[iOpRing];
        nVertCount = psObject->panPartStart[iOpRing+1] - psObject->panPartStart[iOpRing];
        dfSum = 0.0;
        for (iVert = nVertStart; iVert < nVertStart+nVertCount-1; iVert++) {
            dfSum += psObject->pPoints[iVert].x * psObject->pPoints[iVert+1].y - psObject->pPoints[iVert].y * psObject->pPoints[iVert+1].x;
        }
        dfSum += psObject->pPoints[iVert].x * psObject->pPoints[nVertStart].y - psObject->pPoints[iVert].y * psObject->pPoints[nVertStart].x;

        /* Reverse if necessary */
        if ((dfSum < 0.0 && bInner) || (dfSum > 0.0 && !bInner)) {
            int   i;
            bAltered++;
            for (i = 0; i < nVertCount/2; i++) {
                double dfSaved;

                /* Swap X */
                dfSaved = psObject->pPoints[nVertStart+i].x;
                psObject->pPoints[nVertStart+i].x = psObject->pPoints[nVertStart+nVertCount-i-1].x;
                psObject->pPoints[nVertStart+nVertCount-i-1].x = dfSaved;

                /* Swap Y */
                dfSaved = psObject->pPoints[nVertStart+i].y;
                psObject->pPoints[nVertStart+i].y = psObject->pPoints[nVertStart+nVertCount-i-1].y;
                psObject->pPoints[nVertStart+nVertCount-i-1].y = dfSaved;

                /* Swap Z */
                if (psObject->padfZ) {
                    dfSaved = psObject->padfZ[nVertStart+i];
                    psObject->padfZ[nVertStart+i] = psObject->padfZ[nVertStart+nVertCount-i-1];
                    psObject->padfZ[nVertStart+nVertCount-i-1] = dfSaved;
                }

                /* Swap M */
                if (psObject->padfM) {
                    dfSaved = psObject->padfM[nVertStart+i];
                    psObject->padfM[nVertStart+i] = psObject->padfM[nVertStart+nVertCount-i-1];
                    psObject->padfM[nVertStart+nVertCount-i-1] = dfSaved;
                }
            }
        }
    }
    return bAltered;
}

double SHPLengthOfXYs(double *padfX, double *padfY, int start, int end)
{
    int i;
    double dx, dy;
    double sum = 0;
    int np = end - start - 1;
    for (i = 0; i < np; i++) {
        dx = padfX[start + i + 1] - padfX[start + i];
        dy = padfY[start + i + 1] - padfY[start + i];
        sum += sqrt(dx*dx + dy*dy);
    }
    return sum;
}

double SHPLengthOfPoints(SHPPointType *pPoints, int start, int end)
{
    int i;
    double dx, dy;
    double sum = 0;
    SHPPointType *pt = pPoints + start;
    int np = end - start - 1;
    for (i = 0; i < np; i++) {
        dx = pt[i+1].x - pt[i].x;
        dy = pt[i+1].y - pt[i].y;
        sum += sqrt(dx*dx + dy*dy);
    }
    return sum;
}

double SHPAreaOfXYs(double *padfX, double *padfY, int start, int end, int *CCW)
{
    int i;
    double sum = 0;
    double *pX = padfX + start;
    double *pY = padfY + start;
    int np = end - start - 1;
    for (i = 1; i < np; i++) {
        sum += (pX[i] - pX[0]) * (pY[i+1] - pY[i-1]);
    }
    if (CCW) {
        *CCW = SGNOF(sum);
    }
    return sum/2;
}

double SHPAreaOfPoints(SHPPointType *pPoints, int start, int end, int *CCW)
{
    int i;
    double sum = 0;
    SHPPointType *pt = pPoints + start;
    int np = end - start - 1;
    for (i = 1; i < np; i++) {
        sum += (pt[i].x - pt[0].x) * (pt[i+1].y - pt[i-1].y);
    }
    if (CCW) {
        *CCW = SGNOF(sum);
    }
    return sum/2;
}

double SHPObjectExGetLength(const SHPObjectEx *psObject)
{
    int iPart;
    double len = 0;
    switch (psObject->nSHPType) {
    /* polygon */
    case SHPT_POLYGON:
    case SHPT_POLYGONZ:
    case SHPT_POLYGONM:
    /* patches */
    case SHPT_MULTIPATCH:
    /* line */
    case SHPT_ARC:
    case SHPT_ARCZ:
    case SHPT_ARCM:
        for (iPart = 0; iPart < psObject->nParts; iPart++) {
            len += SHPLengthOfPoints(psObject->pPoints, psObject->panPartStart[iPart], psObject->panPartStart[iPart+1]);
        }
        break;
    /* point */
    case SHPT_NULL:
    case SHPT_POINT:
    case SHPT_POINTZ:
    case SHPT_MULTIPOINT:
    case SHPT_MULTIPOINTZ:
    case SHPT_POINTM:
    case SHPT_MULTIPOINTM:
        return 0;
    }
    return len;
}

double SHPObjectExGetArea(const SHPObjectEx *psObject)
{
    int iPart;
    double area = 0;
    switch (psObject->nSHPType) {
    /* polygon */
    case SHPT_POLYGON:
    case SHPT_POLYGONZ:
    case SHPT_POLYGONM:
    case SHPT_MULTIPATCH:
        for (iPart = 0; iPart < psObject->nParts; iPart++) {
            area += SHPAreaOfPoints(psObject->pPoints, psObject->panPartStart[iPart], psObject->panPartStart[iPart+1], 0);
        }
        break;
    /* line */
    case SHPT_ARC:
    case SHPT_ARCZ:
    case SHPT_ARCM:
        return 0;
    /* point */
    case SHPT_NULL:
    case SHPT_POINT:
    case SHPT_POINTZ:
    case SHPT_MULTIPOINT:
    case SHPT_MULTIPOINTZ:
    case SHPT_POINTM:
    case SHPT_MULTIPOINTM:
        return 0;
    }
    return area;
}

double SHPObjectGetLength(const SHPObject *psObject)
{
    int iPart;
    double len = 0;
    switch (psObject->nSHPType) {
    /* polygon */
    case SHPT_POLYGON:
    case SHPT_POLYGONZ:
    case SHPT_POLYGONM:
    /* patches */
    case SHPT_MULTIPATCH:
    /* line */
    case SHPT_ARC:
    case SHPT_ARCZ:
    case SHPT_ARCM:
        for (iPart = 0; iPart < psObject->nParts; iPart++) {
            len += SHPLengthOfXYs(psObject->padfX, psObject->padfY, psObject->panPartStart[iPart], psObject->panPartStart[iPart+1]);
        }
        break;
    /* point */
    case SHPT_NULL:
    case SHPT_POINT:
    case SHPT_POINTZ:
    case SHPT_MULTIPOINT:
    case SHPT_MULTIPOINTZ:
    case SHPT_POINTM:
    case SHPT_MULTIPOINTM:
        return 0;
    }
    return len;
}

double SHPObjectGetArea(const SHPObject *psObject)
{
    int iPart;
    double area = 0;
    switch (psObject->nSHPType) {
    /* polygon */
    case SHPT_POLYGON:
    case SHPT_POLYGONZ:
    case SHPT_POLYGONM:
    case SHPT_MULTIPATCH:
        for (iPart = 0; iPart < psObject->nParts; iPart++) {
            area += SHPAreaOfXYs(psObject->padfX, psObject->padfY, psObject->panPartStart[iPart], psObject->panPartStart[iPart+1], 0);
        }
        break;

    /* line */
    case SHPT_ARC:
    case SHPT_ARCZ:
    case SHPT_ARCM:

    /* point */
    case SHPT_NULL:
    case SHPT_POINT:
    case SHPT_POINTZ:
    case SHPT_MULTIPOINT:
    case SHPT_MULTIPOINTZ:
    case SHPT_POINTM:
    case SHPT_MULTIPOINTM:
        return 0;
    }
    return area;
}

void SHPObjectReversePoints(SHPObject *psObject)
{
    if (psObject->nVertices > 1) {
        double *x0, *x1, *y0, *y1, *z0, *z1, *m0, *m1;
        int HasZ, HasM, start, end, p;
        z0 = z1 = m0 = m1 = 0;
        SHPHasZM(psObject->nSHPType, &HasZ, &HasM);
        for (p = 0; p < psObject->nParts; p++) {
            start = psObject->panPartStart[p];
            end = psObject->panPartStart[p+1] - 1;
            if (end - start > 2) {
                x0 =  psObject->padfX + start;
                x1 =  psObject->padfX + end;
                y0 =  psObject->padfY + start;
                y1 =  psObject->padfY + end;
                if (HasZ) {
                    z0 =  psObject->padfZ + start;
                    z1 =  psObject->padfZ + end;
                }
                if (HasM) {
                    m0 =  psObject->padfM + start;
                    m1 =  psObject->padfM + end;
                }
                while (x1 > x0) {
                    SwapDouble(x0++, x1--);
                    SwapDouble(y0++, y1--);
                    if (HasZ) {
                        SwapDouble(z0++, z1--);
                    }
                    if (HasM) {
                        SwapDouble(m0++, m1--);
                    }
                }
            }
        }
    }
}

void SHPObjectExReversePoints(SHPObjectEx *psObject)
{
    if (psObject->nVertices > 1) {
        SHPPointType *p0, *p1;
        double *z0, *z1, *m0, *m1;
        int HasZ, HasM, start, end, p;
        z0 = z1 = m0 = m1 = 0;
        SHPHasZM(psObject->nSHPType, &HasZ, &HasM);
        for (p = 0; p < psObject->nParts; p++) {
            start = psObject->panPartStart[p];
            end = psObject->panPartStart[p+1] - 1;
            if (end - start > 2) {
                p0 =  psObject->pPoints + start;
                p1 =  psObject->pPoints + end;
                if (HasZ) {
                    z0 =  psObject->padfZ + start;
                    z1 =  psObject->padfZ + end;
                }
                if (HasM) {
                    m0 =  psObject->padfM + start;
                    m1 =  psObject->padfM + end;
                }
                while (p1 > p0) {
                    SwapPointType(p0++, p1--);
                    if (HasZ) {
                        SwapDouble(z0++, z1--);
                    }
                    if (HasM) {
                        SwapDouble(m0++, m1--);
                    }
                }
            }
        }
    }
}

int SHPObjectValidatePolygon(SHPObject *psObject, int isCCW)
{
    int ret;
    double flag;
    /* Do nothing if this is not a polygon object */
    if (psObject->nSHPType != SHPT_POLYGON && psObject->nSHPType != SHPT_POLYGONZ && psObject->nSHPType != SHPT_POLYGONM) {
        /* invalid type */
        return (-1);
    }
    ret = SHPRewindObject(psObject);
    flag = SHPObjectGetArea(psObject);
    if (flag > 0) {
        if (isCCW == 1) {
            /* required CCW is fit */
            return ret;
        } else {
            /* required CW not fit */
            SHPObjectReversePoints(psObject);
            return 1;
        }
    } else {
        if (isCCW == 1) {
            /* required CCW not fit */
            SHPObjectReversePoints(psObject);
            return 1;
        } else {
            /* required CW is fit */
            return ret;
        }
    }
}

int SHPObjectExValidatePolygon(SHPObjectEx *psObject, int isCCW)
{
    int ret;
    double flag;
    /* Do nothing if this is not a polygon object */
    if (psObject->nSHPType != SHPT_POLYGON && psObject->nSHPType != SHPT_POLYGONZ && psObject->nSHPType != SHPT_POLYGONM) {
        /* invalid type */
        return (-1);
    }
    ret = SHPRewindObjectEx(psObject);
    flag = SHPObjectExGetArea(psObject);
    if (flag > 0) {
        if (isCCW == 1) {
            /* required CCW is fit */
            return ret;
        } else {
            /* required CW not fit */
            SHPObjectExReversePoints(psObject);
            return 1;
        }
    } else {
        if (isCCW == 1) {
            /* required CCW not fit */
            //SHPObjectExReversePoints(psObject);
            return 1;
        } else {
            /* required CW is fit */
            return ret;
        }
    }
}

int SHPObject2WKB(const SHPObject *psObject, void *wkbBuffer, double offsetX, double offsetY, double offsetZ, double offsetM)
{
    switch (psObject->nSHPType) {
    case SHPT_NULL:
        return -1;
    case SHPT_POINT:
        return Point2WKB(psObject, wkbBuffer, offsetX, offsetY);
    case SHPT_ARC:
        return Arc2WKB(psObject, wkbBuffer, offsetX, offsetY);
    case SHPT_POLYGON:
        return Polygon2WKB(psObject, wkbBuffer, offsetX, offsetY);
    case SHPT_MULTIPOINT:
        return MultiPoint2WKB(psObject, wkbBuffer, offsetX, offsetY);
    case SHPT_POINTZ:
        return PointZ2WKB(psObject, wkbBuffer, offsetX, offsetY, offsetZ);
    case SHPT_ARCZ:
        return ArcZ2WKB(psObject, wkbBuffer, offsetX, offsetY, offsetZ);
    case SHPT_POLYGONZ:
        return PolygonZ2WKB(psObject, wkbBuffer, offsetX, offsetY, offsetZ);
    case SHPT_MULTIPOINTZ:
        return MultiPointZ2WKB(psObject, wkbBuffer, offsetX, offsetY, offsetZ);
    case SHPT_POINTM:
        return PointM2WKB(psObject, wkbBuffer, offsetX, offsetY, offsetM);
    case SHPT_ARCM:
        return ArcM2WKB(psObject, wkbBuffer, offsetX, offsetY, offsetM);
    case SHPT_POLYGONM:
        return PolygonM2WKB(psObject, wkbBuffer, offsetX, offsetY, offsetM);
    case SHPT_MULTIPOINTM:
        return MultiPointM2WKB(psObject, wkbBuffer, offsetX, offsetY, offsetM);
    case SHPT_MULTIPATCH:
        return MultiPatch2WKB(psObject, wkbBuffer, offsetX, offsetY);
    }
    return 0;
}

int SHPObject2WKT(const SHPObject *psObject, char *wktBuffer, double offsetX, double offsetY, double offsetZ, double offsetM, int nDecimalsXY, int nDecimalsZ, int nDecimalsM)
{
    switch (psObject->nSHPType) {
    case SHPT_NULL:
        return -1;
    case SHPT_POINT:
        return Point2WKT(psObject, wktBuffer, offsetX, offsetY, nDecimalsXY);
    case SHPT_ARC:
        return Arc2WKT(psObject, wktBuffer, offsetX, offsetY, nDecimalsXY);
    case SHPT_POLYGON:
        return Polygon2WKT(psObject, wktBuffer, offsetX, offsetY, nDecimalsXY);
    case SHPT_MULTIPOINT:
        return MultiPoint2WKT(psObject, wktBuffer, offsetX, offsetY, nDecimalsXY);
    case SHPT_POINTZ:
        return PointZ2WKT(psObject, wktBuffer, offsetX, offsetY, offsetZ, nDecimalsXY, nDecimalsZ);
    case SHPT_ARCZ:
        return ArcZ2WKT(psObject, wktBuffer, offsetX, offsetY, offsetZ, nDecimalsXY, nDecimalsZ);
    case SHPT_POLYGONZ:
        return PolygonZ2WKT(psObject, wktBuffer, offsetX, offsetY, offsetZ, nDecimalsXY, nDecimalsZ);
    case SHPT_MULTIPOINTZ:
        return MultiPointZ2WKT(psObject, wktBuffer, offsetX, offsetY, offsetZ, nDecimalsXY, nDecimalsZ);
    case SHPT_POINTM:
        return PointM2WKT(psObject, wktBuffer, offsetX, offsetY, offsetM, nDecimalsXY, nDecimalsM);
    case SHPT_ARCM:
        return ArcM2WKT(psObject, wktBuffer, offsetX, offsetY, offsetM, nDecimalsXY, nDecimalsM);
    case SHPT_POLYGONM:
        return PolygonM2WKT(psObject, wktBuffer, offsetX, offsetY, offsetM, nDecimalsXY, nDecimalsM);
    case SHPT_MULTIPOINTM:
        return MultiPointM2WKT(psObject, wktBuffer, offsetX, offsetY, offsetM, nDecimalsXY, nDecimalsM);
    case SHPT_MULTIPATCH:
        return MultiPatch2WKT(psObject, wktBuffer, offsetX, offsetY, nDecimalsXY);
    }
    return 0;
}

int SHPObjectEx2WKB(const SHPObjectEx *psObject, void *wkbBuffer, double offsetX, double offsetY, double offsetZ, double offsetM)
{
    switch (psObject->nSHPType) {
    case SHPT_NULL:
        return -1;
    case SHPT_POINT:
        return exPoint2WKB(psObject, wkbBuffer, offsetX, offsetY);
    case SHPT_ARC:
        return exArc2WKB(psObject, wkbBuffer, offsetX, offsetY);
    case SHPT_POLYGON:
        return exPolygon2WKB(psObject, wkbBuffer, offsetX, offsetY);
    case SHPT_MULTIPOINT:
        return exMultiPoint2WKB(psObject, wkbBuffer, offsetX, offsetY);
    case SHPT_POINTZ:
        return exPointZ2WKB(psObject, wkbBuffer, offsetX, offsetY, offsetZ);
    case SHPT_ARCZ:
        return exArcZ2WKB(psObject, wkbBuffer, offsetX, offsetY, offsetZ);
    case SHPT_POLYGONZ:
        return exPolygonZ2WKB(psObject, wkbBuffer, offsetX, offsetY, offsetZ);
    case SHPT_MULTIPOINTZ:
        return exMultiPointZ2WKB(psObject, wkbBuffer, offsetX, offsetY, offsetZ);
    case SHPT_POINTM:
        return exPointM2WKB(psObject, wkbBuffer, offsetX, offsetY, offsetM);
    case SHPT_ARCM:
        return exArcM2WKB(psObject, wkbBuffer, offsetX, offsetY, offsetM);
    case SHPT_POLYGONM:
        return exPolygonM2WKB(psObject, wkbBuffer, offsetX, offsetY, offsetM);
    case SHPT_MULTIPOINTM:
        return exMultiPointM2WKB(psObject, wkbBuffer, offsetX, offsetY, offsetM);
    case SHPT_MULTIPATCH:
        return exMultiPatch2WKB(psObject, wkbBuffer, offsetX, offsetY);
    }
    return 0;
}

int SHPObjectEx2WKT(const SHPObjectEx *psObject, char *wktBuffer, double offsetX, double offsetY, double offsetZ, double offsetM, int nDecimalsXY, int nDecimalsZ, int nDecimalsM)
{
    switch (psObject->nSHPType) {
    case SHPT_NULL:
        return -1;
    case SHPT_POINT:
        return exPoint2WKT(psObject, wktBuffer, offsetX, offsetY, nDecimalsXY);
    case SHPT_ARC:
        return exArc2WKT(psObject, wktBuffer, offsetX, offsetY, nDecimalsXY);
    case SHPT_POLYGON:
        return exPolygon2WKT(psObject, wktBuffer, offsetX, offsetY, nDecimalsXY);
    case SHPT_MULTIPOINT:
        return exMultiPoint2WKT(psObject, wktBuffer, offsetX, offsetY, nDecimalsXY);
    case SHPT_POINTZ:
        return exPointZ2WKT(psObject, wktBuffer, offsetX, offsetY, offsetZ, nDecimalsXY, nDecimalsZ);
    case SHPT_ARCZ:
        return exArcZ2WKT(psObject, wktBuffer, offsetX, offsetY, offsetZ, nDecimalsXY, nDecimalsZ);
    case SHPT_POLYGONZ:
        return exPolygonZ2WKT(psObject, wktBuffer, offsetX, offsetY, offsetZ, nDecimalsXY, nDecimalsZ);
    case SHPT_MULTIPOINTZ:
        return exMultiPointZ2WKT(psObject, wktBuffer, offsetX, offsetY, offsetZ, nDecimalsXY, nDecimalsZ);
    case SHPT_POINTM:
        return exPointM2WKT(psObject, wktBuffer, offsetX, offsetY, offsetM, nDecimalsXY, nDecimalsM);
    case SHPT_ARCM:
        return exArcM2WKT(psObject, wktBuffer, offsetX, offsetY, offsetM, nDecimalsXY, nDecimalsM);
    case SHPT_POLYGONM:
        return exPolygonM2WKT(psObject, wktBuffer, offsetX, offsetY, offsetM, nDecimalsXY, nDecimalsM);
    case SHPT_MULTIPOINTM:
        return exMultiPointM2WKT(psObject, wktBuffer, offsetX, offsetY, offsetM, nDecimalsXY, nDecimalsM);
    case SHPT_MULTIPATCH:
        return exMultiPatch2WKT(psObject, wktBuffer, offsetX, offsetY, nDecimalsXY);
    }
    return 0;
}

/*************************************************************************
 *                             SHAPES MBR Tree API
 ************************************************************************/
void SHPMBRTreeReset(SHPHandle hSHP, int bClose)
{
    RTREE_ROOT rtRoot = hSHP->MBRTree.rtRoot;
    if (rtRoot) {
        hSHP->MBRTree.rtRoot = NULL;
        RTreeDestroy(rtRoot);
    }
    if (! bClose) {
        hSHP->MBRTree.rtRoot = RTreeCreate(NULL);
    }
}

SHPMBRTree SHPGetMBRTree(SHPHandle hSHP)
{
    return &(hSHP->MBRTree);
}

int SHPMBRTreeAddShape(SHPMBRTree rtree, const SHPEnvelope *shapeEnv, void *shapeData, int treeLevel)
{
    return RTreeInsertMbr(rtree->rtRoot, (RTREE_MBR *) shapeEnv, shapeData, treeLevel);
}

int SHPMBRTreeSearch(SHPMBRTree rtree, const SHPEnvelope *searchEnv, int(* onSearchShape)(void * shapeData,  void *userParam), void *userParam)
{
    return RTreeSearchMbr(rtree->rtRoot, (const RTREE_MBR *)searchEnv, onSearchShape, &userParam);
}
