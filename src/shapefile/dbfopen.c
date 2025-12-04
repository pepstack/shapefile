/******************************************************************************
 * dbfopen.h
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


static void StringToUpper (char *str, int len)
{
    char *p = str;
    while (len-- > 0 && *p) {
       *p = toupper(*p);
        p++;
    }
}

/**
 * This is called to write out the file header, and field descriptions
 *  before writing any actual data records.
 * This also computes all the DBFDataSet field offset/size/decimals
 *  and so forth values.
 */
static void DBFWriteHeader (DBFHandle psDBF)
{
    unsigned char abyHeader[XBASE_FLDHDR_SZ];
    int i;

    if (!psDBF->bNoHeader) {
        return;
    }

    psDBF->bNoHeader = SHAPEFILE_FALSE;

    /* Initialize the file header information */
    for (i = 0; i < XBASE_FLDHDR_SZ; i++) {
        abyHeader[i] = 0;
    }

    abyHeader[0] = 0x03;

    /* write out a dummy date */
    abyHeader[1] = 95;                  /* YY */
    abyHeader[2] = 7;                   /* MM */
    abyHeader[3] = 26;                  /* DD */

    /* record count preset at zero */
    abyHeader[8] = (unsigned char) (psDBF->nHeaderLength % 256);
    abyHeader[9] = (unsigned char) (psDBF->nHeaderLength / 256);

    abyHeader[10] = (unsigned char) (psDBF->nRecordLength % 256);
    abyHeader[11] = (unsigned char) (psDBF->nRecordLength / 256);

    /* Write the initial 32 byte file header, and all the field descriptions */
    fseek(psDBF->fp, 0, 0);
    fwrite(abyHeader, XBASE_FLDHDR_SZ, 1, psDBF->fp);
    fwrite(psDBF->pszHeader, XBASE_FLDHDR_SZ, psDBF->nFields, psDBF->fp);

    /* Write out the newline character if there is room for it */
    if (psDBF->nHeaderLength > 32*psDBF->nFields + 32) {
        char    cNewline;
        cNewline = 0x0d;
        fwrite(&cNewline, 1, 1, psDBF->fp);
    }
}


/**
 * Write out the current record if there is one.
 */
static void DBFFlushRecord (DBFHandle psDBF)
{
    int nRecordOffset;
    if (psDBF->bCurrentRecordModified && psDBF->nCurrentRecord > -1) {
        psDBF->bCurrentRecordModified = SHAPEFILE_FALSE;

        nRecordOffset = psDBF->nRecordLength * psDBF->nCurrentRecord + psDBF->nHeaderLength;

        fseek(psDBF->fp, nRecordOffset, 0);
        fwrite(psDBF->pszCurrentRecord, psDBF->nRecordLength, 1, psDBF->fp);
    }
}


void DBFUpdateHeader (DBFHandle psDBF)
{
    unsigned char abyFileHeader[32];

    if (psDBF->bNoHeader) {
        DBFWriteHeader (psDBF);
    }

    DBFFlushRecord (psDBF);

    fseek(psDBF->fp, 0, 0);
    fread(abyFileHeader, 32, 1, psDBF->fp);

    abyFileHeader[4] = (unsigned char) (psDBF->nRecords % 256);
    abyFileHeader[5] = (unsigned char) ((psDBF->nRecords/256) % 256);
    abyFileHeader[6] = (unsigned char) ((psDBF->nRecords/ (256*256)) % 256);
    abyFileHeader[7] = (unsigned char) ((psDBF->nRecords/ (256*256*256)) % 256);

    fseek(psDBF->fp, 0, 0);
    fwrite(abyFileHeader, 32, 1, psDBF->fp);

    fflush(psDBF->fp);
}


/**
 * Open a .dbf file.
 */
DBFHandle DBFOpen (const char * pszFilename, const char * pszAccess)
{
    DBFHandle           psDBF;
    unsigned char       *pabyBuf;
    int                 nFields, nHeadLen, nRecLen, iField, i;
    char                *pszBasename, *pszFullname;

    /* We only allow the access strings "rb" and "r+" */
    if (strcmp(pszAccess,"r") != 0 &&
        strcmp(pszAccess,"r+") != 0 &&
        strcmp(pszAccess,"rb") != 0 && strcmp(pszAccess,"rb+") != 0 &&
        strcmp(pszAccess,"r+b") != 0) {
            return 0;
    }

    if (strcmp(pszAccess,"r") == 0) {
        pszAccess = "rb";
    }

    if (strcmp(pszAccess,"r+") == 0) {
        pszAccess = "rb+";
    }

    /* Compute the base (layer) name.
   * If there is any extension on the passed in filename we will strip it off
   */
    pszBasename = (char *) malloc(strlen (pszFilename)+5);
    strcpy(pszBasename, pszFilename);

    for (i = (int) strlen (pszBasename)-1;
       i > 0 &&
       pszBasename[i] != '.' &&
       pszBasename[i] != '/' &&
       pszBasename[i] != '\\';
       i--) {
        /* do nothing */
    }

    if (pszBasename[i] == '.') {
        pszBasename[i] = '\0';
    }

    pszFullname = (char *) malloc(strlen (pszBasename) + 5);
    sprintf(pszFullname, "%s.dbf", pszBasename);

    psDBF = (DBFHandle) calloc(1, sizeof(DBFInfo));
    psDBF->fp = fopen(pszFullname, pszAccess);

    if (psDBF->fp == 0) {
        sprintf(pszFullname, "%s.DBF", pszBasename);
        psDBF->fp = fopen(pszFullname, pszAccess);
    }

    free(pszBasename);
    free(pszFullname);

    if (!psDBF->fp) {
        free(psDBF);
        return 0;
    }

    psDBF->bNoHeader = SHAPEFILE_FALSE;
    psDBF->nCurrentRecord = -1;
    psDBF->bCurrentRecordModified = SHAPEFILE_FALSE;

    /* Read Table Header info */
    pabyBuf = (unsigned char *) malloc(500);
    if (fread(pabyBuf, 32, 1, psDBF->fp) != 1) {
            fclose(psDBF->fp);
            free(pabyBuf);
            free(psDBF);
            return 0;
    }

    psDBF->nRecords = pabyBuf[4] + pabyBuf[5]*256 + pabyBuf[6]*256*256 + pabyBuf[7]*256*256*256;
    psDBF->nHeaderLength = nHeadLen = pabyBuf[8] + pabyBuf[9]*256;
    psDBF->nRecordLength = nRecLen = pabyBuf[10] + pabyBuf[11]*256;
    psDBF->nFields = nFields = (nHeadLen - 32) / 32;
    psDBF->pszCurrentRecord = (char *) malloc(nRecLen);

    /* Read in Field Definitions */
    pabyBuf = (unsigned char *) SfRealloc (pabyBuf,nHeadLen);
    psDBF->pszHeader = (char *) pabyBuf;

    fseek(psDBF->fp, 32, 0);
    if (fread(pabyBuf, nHeadLen-32, 1, psDBF->fp) != 1) {
        fclose(psDBF->fp);
        free(pabyBuf);
        free(psDBF);
        return 0;
    }

    psDBF->panFieldOffset = (int *) malloc(sizeof(int) * nFields);
    psDBF->panFieldSize = (int *) malloc(sizeof(int) * nFields);
    psDBF->panFieldDecimals = (int *) malloc(sizeof(int) * nFields);
    psDBF->pachFieldType = (char *) malloc(sizeof(char) * nFields);

    for (iField = 0; iField < nFields; iField++) {
        unsigned char *pabyFInfo;
        pabyFInfo = pabyBuf+iField*32;

        if (pabyFInfo[11] == 'N' || pabyFInfo[11] == 'F') {
                psDBF->panFieldSize[iField] = pabyFInfo[16];
                psDBF->panFieldDecimals[iField] = pabyFInfo[17];
        } else {
                psDBF->panFieldSize[iField] = pabyFInfo[16] + pabyFInfo[17]*256;
                psDBF->panFieldDecimals[iField] = 0;
        }

        psDBF->pachFieldType[iField] = (char) pabyFInfo[11];
        if (iField == 0) {
            psDBF->panFieldOffset[iField] = 1;
        } else {
            psDBF->panFieldOffset[iField] = psDBF->panFieldOffset[iField-1] + psDBF->panFieldSize[iField-1];
        }
    }

    return (psDBF);
}


void DBFClose (DBFHandle psDBF)
{
    if (psDBF==0) {
        return;
    }

    if (psDBF->bNoHeader) {
        DBFWriteHeader (psDBF);
    }

    DBFFlushRecord (psDBF);

    /* Update last access date, and number of records if we have write access */
    if (psDBF->bUpdated) {
        DBFUpdateHeader (psDBF);
    }

    fclose(psDBF->fp);

    if (psDBF->panFieldOffset != 0) {
        free(psDBF->panFieldOffset);
        free(psDBF->panFieldSize);
        free(psDBF->panFieldDecimals);
        free(psDBF->pachFieldType);
    }

    free(psDBF->pszHeader);
    free(psDBF->pszCurrentRecord);
    free(psDBF);
}


/**
 * Create a new .dbf file
 */
DBFHandle DBFCreate (const char * pszFilename)
{
    DBFHandle   psDBF;
    FILE        *fp;
    char        *pszFullname, *pszBasename;
    int         i;

    /* Compute the base (layer) name.  If there is any extension
   *  on the passed in filename we will strip it off
   */
    pszBasename = (char *) malloc(strlen (pszFilename)+5);
    strcpy(pszBasename, pszFilename);

    for (i = (int) strlen (pszBasename)-1;
        i > 0 &&
        pszBasename[i] != '.' &&
        pszBasename[i] != '/' &&
        pszBasename[i] != '\\';
        i--) {
        /* do nothing */
    }

    if (pszBasename[i] == '.') {
        pszBasename[i] = '\0';
    }

    pszFullname = (char *) malloc(strlen (pszBasename) + 5);
    sprintf(pszFullname, "%s.dbf", pszBasename);
    free(pszBasename);

    /* Create the file */
    fp = fopen(pszFullname, "wb");
    if (fp == 0) {
        return (0);
    }

    fputc (0, fp);
    fclose(fp);

    fp = fopen(pszFullname, "rb+");

    if (fp == 0) {
        return (0);
    }

    free(pszFullname);

    /* Create the info structure */
    psDBF = (DBFHandle) calloc(1, sizeof(DBFInfo));

    psDBF->fp = fp;
    psDBF->nRecordLength = 1;
    psDBF->nHeaderLength = 33;

    psDBF->nCurrentRecord = -1;
    psDBF->bCurrentRecordModified = SHAPEFILE_FALSE;
    psDBF->bNoHeader = SHAPEFILE_TRUE;

    return (psDBF);
}


/**
 * Add a field to a newly created .dbf file before any records are written.
 */
int DBFAddField (DBFHandle psDBF, const char * pszFieldName, DBFFieldType eType, int nWidth, int nDecimals)
{
    char        *pszFInfo;
    int         i;

    /* Do some checking to ensure we can add records to this file */
    if (psDBF->nRecords > 0) {
        return (-1);
    }

    if (!psDBF->bNoHeader) {
        return (-1);
    }

    if (eType != FTDouble && nDecimals != 0) {
        return (-1);
    }

    if (eType == FTDateS) {
        nWidth = 10;
        nDecimals = 0;
    }

    if (nWidth < 1) {
        return -1;
    }

    /* SfRealloc all the arrays larger to hold the additional field information */
    psDBF->nFields++;
    psDBF->panFieldOffset = (int *) SfRealloc (psDBF->panFieldOffset, sizeof(int) * psDBF->nFields);
    psDBF->panFieldSize = (int *) SfRealloc (psDBF->panFieldSize, sizeof(int) * psDBF->nFields);
    psDBF->panFieldDecimals = (int *) SfRealloc (psDBF->panFieldDecimals, sizeof(int) * psDBF->nFields);
    psDBF->pachFieldType = (char *) SfRealloc (psDBF->pachFieldType, sizeof(char) * psDBF->nFields);

    /* Assign the new field information fields */
    psDBF->panFieldOffset[psDBF->nFields-1] = psDBF->nRecordLength;
    psDBF->nRecordLength += nWidth;
    psDBF->panFieldSize[psDBF->nFields-1] = nWidth;
    psDBF->panFieldDecimals[psDBF->nFields-1] = nDecimals;

    if (eType == FTLogical) {
        psDBF->pachFieldType[psDBF->nFields-1] = 'L';
    } else if (eType == FTString) {
        psDBF->pachFieldType[psDBF->nFields-1] = 'C';
    } else if (eType == FTDateS) {
        psDBF->pachFieldType[psDBF->nFields-1] = 'S';
    } else {
        psDBF->pachFieldType[psDBF->nFields-1] = 'N';
    }

    /* Extend the required header information */
    psDBF->nHeaderLength += 32;
    psDBF->bUpdated = SHAPEFILE_FALSE;

    psDBF->pszHeader = (char *) SfRealloc (psDBF->pszHeader,psDBF->nFields*32);

    pszFInfo = psDBF->pszHeader + 32 * (psDBF->nFields-1);

    for (i = 0; i < 32; i++) {
        pszFInfo[i] = '\0';
    }

    strncpy(pszFInfo, pszFieldName, strnlen(pszFieldName, 10));

    pszFInfo[11] = psDBF->pachFieldType[psDBF->nFields-1];

    if (eType == FTString || eType == FTDateS) {
        pszFInfo[16] = (unsigned char) (nWidth % 256);
        pszFInfo[17] = (unsigned char) (nWidth / 256);
    } else {
        pszFInfo[16] = (unsigned char) nWidth;
        pszFInfo[17] = (unsigned char) nDecimals;
    }

    /* Make the current record buffer appropriately larger */
    psDBF->pszCurrentRecord =
        (char *) SfRealloc (psDBF->pszCurrentRecord, psDBF->nRecordLength);

    return (psDBF->nFields-1);
}


/**
 * Read one of the attribute fields of a record
 */
static void *DBFReadAttribute (DBFHandle psDBF, int hEntity, int iField, char chReqType)
{
    int           nRecordOffset;
    unsigned char *pabyRec;
    void          *pReturnField = 0;

    if (hEntity < 0 || hEntity >= psDBF->nRecords) {
        return (0);
    }

    if (iField < 0 || iField >= psDBF->nFields) {
        return (0);
    }

    if (psDBF->nCurrentRecord != hEntity) {
        DBFFlushRecord (psDBF);
        nRecordOffset = psDBF->nRecordLength * hEntity + psDBF->nHeaderLength;

        if (fseek(psDBF->fp, nRecordOffset, 0) != 0) {
            /* fseek failed on DBF file */
            return 0;
        }

        if (fread(psDBF->pszCurrentRecord, psDBF->nRecordLength, 1, psDBF->fp) != 1) {
            /* fread failed on DBF file */
            return 0;
        }

        psDBF->nCurrentRecord = hEntity;
    }

    pabyRec = (unsigned char *) psDBF->pszCurrentRecord;

    strncpy(psDBF->szStringField, ((const char *) pabyRec) + psDBF->panFieldOffset[iField],
        psDBF->panFieldSize[iField]);
    psDBF->szStringField[ psDBF->panFieldSize[iField] ] = '\0';

    pReturnField = psDBF->szStringField;

    if (chReqType == 'N') {
        psDBF->dDoubleField = atof (psDBF->szStringField);
        pReturnField = &psDBF->dDoubleField;
    }
#ifdef TRIM_DBF_WHITESPACE
    /* Should we trim white space off the string attribute value? */
    else {
        char *pchSrc, *pchDst;

        pchDst = pchSrc = psDBF->szStringField;
        while (*pchSrc == ' ') {
            pchSrc++;
        }

        while (*pchSrc != '\0') {
            *(pchDst++) = * (pchSrc++);
        }

        *pchDst = '\0';
        while (pchDst != psDBF->szStringField && * (--pchDst) == ' ') {
            *pchDst = '\0';
        }
    }
#endif

    return (pReturnField);
}


/**
 * Read an integer attribute
 */
int DBFReadIntegerAttribute (DBFHandle psDBF, int iRecord, int iField)
{
    double *pdValue = (double*) DBFReadAttribute (psDBF, iRecord, iField, 'N');

    if (pdValue == 0) {
        return 0;
    } else {
        return ((int) *pdValue);
    }
}


/**
 * Read a double attribute
 */
double DBFReadDoubleAttribute (DBFHandle psDBF, int iRecord, int iField)
{
    double *pdValue = (double*) DBFReadAttribute (psDBF, iRecord, iField, 'N');
    if (pdValue == 0) {
        return 0.0;
    } else {
        return (*pdValue);
    }
}


/**
 * Read a string attribute
 */
const char * DBFReadStringAttribute (DBFHandle psDBF, int iRecord, int iField)
{
    return (const char *) DBFReadAttribute (psDBF, iRecord, iField, 'C');
}


/**
 * Read and copy a string attribute
 */
int DBFReadCopyStringAttribute (DBFHandle psDBF, int iRecord, int iField, char *buffer)
{
    const char *psz = (const char *) DBFReadAttribute (psDBF, iRecord, iField, 'C');
    if (psz) {
        int cb = (int) strlen(psz) + 1;
        if (buffer) {
            memcpy(buffer, psz, cb);
        }
        return cb;
    } else {
        return -1;
    }
}


/**
 * Read a logical attribute
 */
const char * DBFReadLogicalAttribute (DBFHandle psDBF, int iRecord, int iField)
{
    return (const char *) DBFReadAttribute (psDBF, iRecord, iField, 'L');
}


/**
 * Return SHAPEFILE_TRUE if value for field is 0
 *  Contributed by Jim Matthews.
 */
int DBFIsAttributeNULL (DBFHandle psDBF, int iRecord, int iField)
{
    int i;
    const char  *pszValue = DBFReadStringAttribute (psDBF, iRecord, iField);
    if (pszValue == 0) {
        return SHAPEFILE_TRUE;
    }

    switch (psDBF->pachFieldType[iField]) {
    case 'N':
    case 'F':
        /* We accept all asterisks or all blanks as 0 though according to the spec
     * I think it should be all asterisks
     */
        if (pszValue[0] == '*') {
            return SHAPEFILE_TRUE;
        }

        for (i = 0; pszValue[i] != '\0'; i++) {
            if (pszValue[i] != ' ') {
                return SHAPEFILE_FALSE;
            }
        }
        return SHAPEFILE_TRUE;

    case 'D':
        /* 0 date fields have value "00000000" */
        return strncmp (pszValue,"00000000",8) == 0;

    case 'L':
        /* 0 boolean fields have value "?" */
        return pszValue[0] == '?';

    default:
        /* empty string fields are considered 0 */
        return strlen (pszValue) == 0;
    }
}


/**
 * Return the number of fields in this table
 */
int DBFGetFieldCount (DBFHandle psDBF)
{
    return (psDBF->nFields);
}


/**
 * Return the number of records in this table
 */
int DBFGetRecordCount (DBFHandle psDBF)
{
    return (psDBF->nRecords);
}


/**
 * Return any requested information about the field
 */
DBFFieldType DBFGetFieldInfo (DBFHandle psDBF, int iField, char *pszFieldName,
    int *pnWidth, int *pnDecimals)
{
    int i;

    if (iField < 0 || iField >= psDBF->nFields) {
        return (FTInvalid);
    }

    if (pnWidth != 0) {
        *pnWidth = psDBF->panFieldSize[iField];
    }

    if (pnDecimals != 0) {
        *pnDecimals = psDBF->panFieldDecimals[iField];
    }

    if (pszFieldName != 0) {
        strncpy(pszFieldName, (char *) psDBF->pszHeader+iField*32, 11);
        pszFieldName[11] = '\0';
        for (i = 10; i > 0 && pszFieldName[i] == ' '; i--) {
        pszFieldName[i] = '\0';
        }
    }

    if (psDBF->pachFieldType[iField] == 'L') {
        return (FTLogical);
    } else if (psDBF->pachFieldType[iField] == 'S') {
        return (FTDateS);
    } else if (psDBF->pachFieldType[iField] == 'N' ||
        psDBF->pachFieldType[iField] == 'F' ||
        psDBF->pachFieldType[iField] == 'D') {
        if (psDBF->panFieldDecimals[iField] > 0) {
            return (FTDouble);
        } else {
            return (FTInteger);
        }
    } else {
        return (FTString);
    }
}


void DBFGetFieldInfo2 (DBFHandle psDBF,  int iField, DBFFieldInfo *pFieldInfo)
{
    int  i;
    assert (iField >= 0 && iField < psDBF->nFields && pFieldInfo);

    pFieldInfo->iField    = iField;
    pFieldInfo->eType     = FTInvalid;
    pFieldInfo->nWidth    = psDBF->panFieldSize[iField];
    pFieldInfo->nDecimals = psDBF->panFieldDecimals[iField];
    pFieldInfo->isNotNull  = 0;  /* can be null */

    strncpy(pFieldInfo->szFieldName, (const char*) psDBF->pszHeader+iField*32, MAX_DBF_FIELD_NAME_LEN);
    pFieldInfo->szFieldName[MAX_DBF_FIELD_NAME_LEN] = '\0';

    for (i = MAX_DBF_FIELD_NAME_LEN-1; i > 0 && pFieldInfo->szFieldName[i] == ' '; i--) {
        pFieldInfo->szFieldName[i] = '\0';
    }

    if (psDBF->pachFieldType[iField] == 'L') {
        pFieldInfo->eType = FTLogical;
    } else if (psDBF->pachFieldType[iField] == 'S') {
        pFieldInfo->eType = FTDateS;
    } else if (psDBF->pachFieldType[iField] == 'N' ||
        psDBF->pachFieldType[iField] == 'F' ||
        psDBF->pachFieldType[iField] == 'D') {
        if (psDBF->panFieldDecimals[iField] > 0) {
            pFieldInfo->eType = FTDouble;
        } else {
            pFieldInfo->eType = FTInteger;
        }
    } else {
        pFieldInfo->eType = FTString;
    }
}


/**
 * Write an attribute record to the file
 */
static int DBFWriteAttribute (DBFHandle psDBF, int hEntity, int iField, void * pValue)
{
    int nRecordOffset, i, j, nWidth, SFieldLen, nRetResult = SHAPEFILE_TRUE;
    unsigned char  *pabyRec;
    char      szSField[400], szFormat[20];

    /* Is this a valid record?  */
    if (hEntity < 0 || hEntity > psDBF->nRecords) {
        return (SHAPEFILE_FALSE);
    }

    if (psDBF->bNoHeader) {
        DBFWriteHeader (psDBF);
    }

    /* Is this a brand new record? */
    if (hEntity == psDBF->nRecords) {
        DBFFlushRecord (psDBF);
        psDBF->nRecords++;
        for (i = 0; i < psDBF->nRecordLength; i++) {
            psDBF->pszCurrentRecord[i] = ' ';
        }
        psDBF->nCurrentRecord = hEntity;
    }

    /* Is this an existing record, but different than the last one  we accessed? */
    if (psDBF->nCurrentRecord != hEntity) {
        DBFFlushRecord (psDBF);
        nRecordOffset = psDBF->nRecordLength * hEntity + psDBF->nHeaderLength;
        fseek(psDBF->fp, nRecordOffset, 0);
        fread(psDBF->pszCurrentRecord, psDBF->nRecordLength, 1, psDBF->fp);
        psDBF->nCurrentRecord = hEntity;
    }

    pabyRec = (unsigned char *) psDBF->pszCurrentRecord;

    psDBF->bCurrentRecordModified = SHAPEFILE_TRUE;
    psDBF->bUpdated = SHAPEFILE_TRUE;

    /* Translate 0 value to valid DBF file representation.
   *  Contributed by Jim Matthews.
   */
    if (!pValue) {
        switch (psDBF->pachFieldType[iField]) {
        case 'N':
        case 'F':
            /* 0 numeric fields have value "****************" */
            memset((char *) (pabyRec+psDBF->panFieldOffset[iField]), '*', psDBF->panFieldSize[iField]);
            break;
        case 'D':
            /* 0 date fields have value "00000000" */
            memset((char *) (pabyRec+psDBF->panFieldOffset[iField]), '0', psDBF->panFieldSize[iField]);
            break;
        case 'L':
            /* 0 boolean fields have value "?" */
            memset((char *) (pabyRec+psDBF->panFieldOffset[iField]), '?', psDBF->panFieldSize[iField]);
            break;
        default:
            /* empty string fields are considered 0 */
            memset((char *) (pabyRec+psDBF->panFieldOffset[iField]), '\0', psDBF->panFieldSize[iField]);
            break;
        }
        return SHAPEFILE_TRUE;
    }

    /* Assign all the record fields */
    switch (psDBF->pachFieldType[iField]) {
    case 'D':
    case 'N':
    case 'F':
        if (psDBF->panFieldDecimals[iField] == 0) {
            nWidth = psDBF->panFieldSize[iField];

            if ((int) sizeof(szSField)-2 < nWidth) {
                nWidth = sizeof(szSField)-2;
            }

            sprintf(szFormat, "%%%dd", nWidth);
            sprintf(szSField, szFormat, (int) * ((double *) pValue));

            SFieldLen = (int)strnlen(szSField, sizeof(szSField));
            if (SFieldLen > psDBF->panFieldSize[iField]) {
                szSField[psDBF->panFieldSize[iField]] = '\0';
                nRetResult = SHAPEFILE_FALSE;
            }
            memcpy((char *) (pabyRec+psDBF->panFieldOffset[iField]), szSField, SFieldLen);
        } else {
            nWidth = psDBF->panFieldSize[iField];

            if ((int) sizeof(szSField) - 2 < nWidth) {
                nWidth = sizeof(szSField) - 2;
            }

            sprintf(szFormat, "%%%d.%df", nWidth, psDBF->panFieldDecimals[iField]);
            sprintf(szSField, szFormat, * ((double *) pValue));

            SFieldLen = (int)strnlen(szSField, sizeof(szSField));
            if (SFieldLen > psDBF->panFieldSize[iField]) {
                szSField[psDBF->panFieldSize[iField]] = '\0';
                nRetResult = SHAPEFILE_FALSE;
            }
            memcpy((char *)(pabyRec + psDBF->panFieldOffset[iField]), szSField, SFieldLen);
        }
        break;
    case 'L':
        if (psDBF->panFieldSize[iField] >= 1  && (*(char*)pValue == 'F' || * (char*)pValue == 'T')) {
            *(pabyRec+psDBF->panFieldOffset[iField]) = * (char*)pValue;
        }
        break;
    default:
        if ((int) strlen ((char *) pValue) > psDBF->panFieldSize[iField]) {
            j = psDBF->panFieldSize[iField];
            nRetResult = SHAPEFILE_FALSE;
        } else {
            memset(pabyRec+psDBF->panFieldOffset[iField], ' ',
            psDBF->panFieldSize[iField]);
            j = (int) strlen ((char *) pValue);
        }

        strncpy((char *) (pabyRec+psDBF->panFieldOffset[iField]), (char *) pValue, j);
        break;
    }
    return (nRetResult);
}


/**
 * Write an attribute record to the file, but without any
 *  reformatting based on type.  The provided buffer is written
 *  as is to the field position in the record.
 */
int DBFWriteAttributeDirectly (DBFHandle psDBF, int hEntity, int iField, void * pValue)
{
    int  nRecordOffset, i, j;
    unsigned char *pabyRec;

    /* Is this a valid record? */
    if (hEntity < 0 || hEntity > psDBF->nRecords) {
        return (SHAPEFILE_FALSE);
    }

    if (psDBF->bNoHeader) {
        DBFWriteHeader (psDBF);
    }

    /* Is this a brand new record? */
    if (hEntity == psDBF->nRecords) {
        DBFFlushRecord (psDBF);
        psDBF->nRecords++;
        for (i = 0; i < psDBF->nRecordLength; i++) {
            psDBF->pszCurrentRecord[i] = ' ';
        }
        psDBF->nCurrentRecord = hEntity;
    }

    /* Is this an existing record, but different than the last one we accessed? */
    if (psDBF->nCurrentRecord != hEntity) {
        DBFFlushRecord (psDBF);
        nRecordOffset = psDBF->nRecordLength * hEntity + psDBF->nHeaderLength;
        fseek(psDBF->fp, nRecordOffset, 0);
        fread(psDBF->pszCurrentRecord, psDBF->nRecordLength, 1, psDBF->fp);
        psDBF->nCurrentRecord = hEntity;
    }

    pabyRec = (unsigned char *) psDBF->pszCurrentRecord;

    /* Assign all the record fields */
    if ((int)strlen ((char *) pValue) > psDBF->panFieldSize[iField]) {
        j = psDBF->panFieldSize[iField];
    } else {
        memset(pabyRec+psDBF->panFieldOffset[iField], ' ', psDBF->panFieldSize[iField]);
        j = (int) strlen ((char *) pValue);
    }

    strncpy((char *) (pabyRec+psDBF->panFieldOffset[iField]), (char *) pValue, j);
    psDBF->bCurrentRecordModified = SHAPEFILE_TRUE;
    psDBF->bUpdated = SHAPEFILE_TRUE;
    return (SHAPEFILE_TRUE);
}


/**
 * Write a double attribute
 */
int DBFWriteDoubleAttribute (DBFHandle psDBF, int iRecord, int iField, double dValue)
{
    return DBFWriteAttribute(psDBF, iRecord, iField, (void *) &dValue);
}


int DBFWriteIntegerAttribute (DBFHandle psDBF, int iRecord, int iField, int nValue)
{
    double  dValue = nValue;
    return DBFWriteAttribute(psDBF, iRecord, iField, (void *) &dValue);
}


int DBFWriteStringAttribute (DBFHandle psDBF, int iRecord, int iField, const char *pszValue)
{
    return DBFWriteAttribute(psDBF, iRecord, iField, (void*)pszValue);
}


int DBFWrite0Attribute (DBFHandle psDBF, int iRecord, int iField)
{
    return DBFWriteAttribute(psDBF, iRecord, iField, 0);
}


int DBFWriteLogicalAttribute (DBFHandle psDBF, int iRecord, int iField, const char lValue)
{
    return DBFWriteAttribute(psDBF, iRecord, iField, (void *) (&lValue));
}


/**
 * Write an attribute record to the file
 */
int DBFWriteTuple (DBFHandle psDBF, int hEntity, void * pRawTuple)
{
    int         nRecordOffset, i;
    unsigned char *pabyRec;

    /* Is this a valid record? */
    if (hEntity < 0 || hEntity > psDBF->nRecords) {
        return (SHAPEFILE_FALSE);
    }

    if (psDBF->bNoHeader) {
        DBFWriteHeader (psDBF);
    }

    /* Is this a brand new record? */
    if (hEntity == psDBF->nRecords) {
        DBFFlushRecord (psDBF);
        psDBF->nRecords++;
        for (i = 0; i < psDBF->nRecordLength; i++) {
            psDBF->pszCurrentRecord[i] = ' ';
        }
        psDBF->nCurrentRecord = hEntity;
    }

    /* Is this an existing record, but different than the last one we accessed? */
    if (psDBF->nCurrentRecord != hEntity) {
        DBFFlushRecord (psDBF);
        nRecordOffset = psDBF->nRecordLength * hEntity + psDBF->nHeaderLength;
        fseek(psDBF->fp, nRecordOffset, 0);
        fread(psDBF->pszCurrentRecord, psDBF->nRecordLength, 1, psDBF->fp);
        psDBF->nCurrentRecord = hEntity;
    }

    pabyRec = (unsigned char *) psDBF->pszCurrentRecord;
    memcpy (pabyRec, pRawTuple,  psDBF->nRecordLength);
    psDBF->bCurrentRecordModified = SHAPEFILE_TRUE;
    psDBF->bUpdated = SHAPEFILE_TRUE;
    return (SHAPEFILE_TRUE);
}


/**
 * Read one of the attribute fields of a record
 */
const char * DBFReadTuple (DBFHandle psDBF, int hEntity)
{
    int  nRecordOffset;
    unsigned char  *pabyRec;

    /* Have we read the record? */
    if (hEntity < 0 || hEntity >= psDBF->nRecords) {
        return (0);
    }

    if (psDBF->nCurrentRecord != hEntity) {
        DBFFlushRecord (psDBF);
        nRecordOffset = psDBF->nRecordLength * hEntity + psDBF->nHeaderLength;
        fseek(psDBF->fp, nRecordOffset, 0);
        fread(psDBF->pszCurrentRecord, psDBF->nRecordLength, 1, psDBF->fp);
        psDBF->nCurrentRecord = hEntity;
    }

    pabyRec = (unsigned char *) psDBF->pszCurrentRecord;

    if (psDBF->nTupleLen < psDBF->nRecordLength) {
        psDBF->nTupleLen = psDBF->nRecordLength;
        psDBF->pReturnTuple = (char *) SfRealloc (psDBF->pReturnTuple, psDBF->nRecordLength);
    }
    memcpy (psDBF->pReturnTuple, pabyRec, psDBF->nRecordLength);
    return (psDBF->pReturnTuple);
}


/**
 * Read one of the attribute fields of a record.
 */
DBFHandle DBFCloneEmpty (DBFHandle psDBF, const char * pszFilename)
{
    DBFHandle   newDBF;

    newDBF = DBFCreate(pszFilename);
    if (!newDBF) {
        return 0;
    }

    newDBF->nFields = psDBF->nFields;
    newDBF->nRecordLength = psDBF->nRecordLength;
    newDBF->nHeaderLength = psDBF->nHeaderLength;

    newDBF->pszHeader = (char *) malloc(newDBF->nHeaderLength);
    memcpy(newDBF->pszHeader, psDBF->pszHeader, newDBF->nHeaderLength);

    newDBF->panFieldOffset = (int *) malloc(sizeof(int) * psDBF->nFields);
    memcpy(newDBF->panFieldOffset, psDBF->panFieldOffset, sizeof(int) * psDBF->nFields);
    newDBF->panFieldSize = (int *) malloc(sizeof(int) * psDBF->nFields);
    memcpy(newDBF->panFieldSize, psDBF->panFieldSize, sizeof(int) * psDBF->nFields);
    newDBF->panFieldDecimals = (int *) malloc(sizeof(int) * psDBF->nFields);
    memcpy(newDBF->panFieldDecimals, psDBF->panFieldDecimals, sizeof(int) * psDBF->nFields);
    newDBF->pachFieldType = (char *) malloc(sizeof(int) * psDBF->nFields);
    memcpy(newDBF->pachFieldType, psDBF->pachFieldType, sizeof(int) * psDBF->nFields);

    newDBF->bNoHeader = SHAPEFILE_TRUE;
    newDBF->bUpdated = SHAPEFILE_TRUE;

    DBFWriteHeader(newDBF);
    DBFClose(newDBF);

    newDBF = DBFOpen(pszFilename, "rb+");

    return (newDBF);
}


/**
 * Return the DBase field type for the specified field.
 * Value can be one of:
 *  'C' (String), 'D' (Date), 'F' (Float),
 *  'N' (Numeric, with or without decimal),
 *  'L' (Logical),
 *  'M' (Memo: 10 digits .DBT block ptr)
 */
char DBFGetNativeFieldType (DBFHandle psDBF, int iField)
{
    if (iField >=0 && iField < psDBF->nFields) {
        return psDBF->pachFieldType[iField];
    }
    return  ' ';
}


/**
 * Get the index number for a field in a .dbf file.
 *   Contributed by Jim Matthews.
 */
int DBFGetFieldIndex (DBFHandle psDBF, const char *pszFieldName)
{
    char name[12], name1[12], name2[12];
    int  i;

    strncpy(name1, pszFieldName, 11);
    name1[11] = '\0';
    StringToUpper(name1, 11);

    for (i = 0; i < DBFGetFieldCount (psDBF); i++) {
        DBFGetFieldInfo(psDBF, i, name, 0, 0);

        memcpy(name2, name, 11);

        name2[11] = '\0';
        StringToUpper(name2, 11);

        if (! strncmp(name1, name2, 10)) {
            return (i);
        }
    }
    return (-1);
}
