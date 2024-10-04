/******************************************************************************
 * shptree.c
 *
 * v 1.6 2001/05/23 13:36:52 warmerda
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


typedef struct shape_tree_node
{
    /* region covered by this node */
    double      adfBoundsMin[4];
    double      adfBoundsMax[4];

    /* list of shapes stored at this node.  The papsShapeObj pointers
     or the whole list can be NULL */
    int         nShapeCount;
    int         *panShapeIds;
    SHPObject   **papsShapeObj;

    int         nSubNodes;
    struct shape_tree_node *apsSubNode[MAX_SUBNODE];
} SHPTreeNode;


typedef struct shape_tree_root
{
    SHPHandle   hSHP;

    int         nMaxDepth;
    int         nDimension;
    int         nTotalCount;

    SHPTreeNode *psRoot;
} SHPTree;


/* If the following is 0.5, nodes will be split in half.
 * If it is 0.6 then each subnode will contain 60% of the parent
 *  node, with 20% representing overlap.  This can be help to
 *  prevent small objects on a boundary from shifting too high up the tree
 */

#define SHP_SPLIT_RATIO  0.55

/**
 * Initialize a tree node
 */
static SHPTreeNode *SHPTreeNodeCreate (double *padfBoundsMin, double *padfBoundsMax)
{
    SHPTreeNode  *psTreeNode;

    psTreeNode = (SHPTreeNode *) malloc(sizeof(SHPTreeNode));

    psTreeNode->nShapeCount = 0;
    psTreeNode->panShapeIds = 0;
    psTreeNode->papsShapeObj = 0;

    psTreeNode->nSubNodes = 0;

    if (padfBoundsMin != 0) {
        memcpy(psTreeNode->adfBoundsMin, padfBoundsMin, sizeof(double) * 4);
    }

    if (padfBoundsMax != 0) {
        memcpy(psTreeNode->adfBoundsMax, padfBoundsMax, sizeof(double) * 4);
    }
    return psTreeNode;
}

/**
 * Create Tree
 */
SHPTreeHandle SHPCreateTree (SHPHandle hSHP, int nDimension, int nMaxDepth, double *padfBoundsMin, double *padfBoundsMax)

{
    SHPTree  *psTree;

    if (padfBoundsMin == 0 && hSHP == 0)
    return 0;

    /* Allocate the tree object */
    psTree = (SHPTree *) malloc(sizeof(SHPTree));

    psTree->hSHP = hSHP;
    psTree->nMaxDepth = nMaxDepth;
    psTree->nDimension = nDimension;

    /* If no max depth was defined, try to select a reasonable one that implies approximately 8 shapes per node */
    if (psTree->nMaxDepth == 0 && hSHP != 0) {
        int  nMaxNodeCount = 1;
        int  nShapeCount;
        SHPGetInfo(hSHP, &nShapeCount, 0, 0, 0);
        while (nMaxNodeCount*4 < nShapeCount) {
            psTree->nMaxDepth += 1;
            nMaxNodeCount = nMaxNodeCount * 2;
        }
    }

    /* Allocate the root node */
    psTree->psRoot = SHPTreeNodeCreate(padfBoundsMin, padfBoundsMax);

    /* Assign the bounds to the root node.  If none are passed in,
   *  use the bounds of the provided file otherwise the create
   *  function will have already set the bounds
   */
    if (padfBoundsMin == 0) {
        SHPGetInfo(hSHP, 0, 0,
        psTree->psRoot->adfBoundsMin, 
        psTree->psRoot->adfBoundsMax);
    }

    /* If we have a file, insert all it's shapes into the tree */
    if (hSHP != 0) {
        int  iShape, nShapeCount;
        SHPGetInfo(hSHP, &nShapeCount, 0, 0, 0);
        for (iShape = 0; iShape < nShapeCount; iShape++) {
            SHPObject  *psShape;
            psShape = SHPReadObject(hSHP, iShape);
            SHPTreeAddShapeId(psTree, psShape);
            SHPDestroyObject(psShape);
        }
    }
    return psTree;
}

/**
 * Destroy Tree Node
 */
static void SHPDestroyTreeNode (SHPTreeNode * psTreeNode)
{
    int i;

    for (i = 0; i < psTreeNode->nSubNodes; i++) {
        if (psTreeNode->apsSubNode[i] != 0) {
            SHPDestroyTreeNode(psTreeNode->apsSubNode[i]);
        }
    }

    if (psTreeNode->panShapeIds != 0) {
        free(psTreeNode->panShapeIds);
    }

    if (psTreeNode->papsShapeObj != 0) {
        for (i = 0; i < psTreeNode->nShapeCount; i++) {
            if (psTreeNode->papsShapeObj[i] != 0) {
                SHPDestroyObject(psTreeNode->papsShapeObj[i]);
            }
        }

        free(psTreeNode->papsShapeObj);
    }

    free(psTreeNode);
}

/**
 * Destroy Tree
 */
void SHPDestroyTree (SHPTreeHandle psTree)
{
    SHPDestroyTreeNode(psTree->psRoot);
    free(psTree);
}

/**
 * Do the given boxes overlap at all?
 */
int SHPCheckBoundsOverlap (double *padfBox1Min, double *padfBox1Max, double *padfBox2Min, double *padfBox2Max, int nDimension)
{
    int iDim;
    for (iDim = 0; iDim < nDimension; iDim++) {
        if (padfBox2Max[iDim] < padfBox1Min[iDim]) {
            return SHAPEFILE_FALSE;
        }
        if (padfBox1Max[iDim] < padfBox2Min[iDim]) {
            return SHAPEFILE_FALSE;
        }
    }
    return SHAPEFILE_TRUE;
}

/**
 * Does the given shape fit within the indicated extents?
 */
static int SHPCheckObjectContained (
    SHPObject * psObject,
    int nDimension,
    double *padfBoundsMin,
    double *padfBoundsMax)
{
    if (psObject->dfXMin < padfBoundsMin[0] || psObject->dfXMax > padfBoundsMax[0]) {
        return SHAPEFILE_FALSE;
    }
    if (psObject->dfYMin < padfBoundsMin[1] || psObject->dfYMax > padfBoundsMax[1]) {
        return SHAPEFILE_FALSE;
    }
    if (nDimension == 2) {
        return SHAPEFILE_TRUE;
    }
    if (psObject->dfZMin < padfBoundsMin[2] || psObject->dfZMax < padfBoundsMax[2]) {
        return SHAPEFILE_FALSE;
    }
    if (nDimension == 3) {
        return SHAPEFILE_TRUE;
    }
    if (psObject->dfMMin < padfBoundsMin[3] || psObject->dfMMax < padfBoundsMax[3]) {
        return SHAPEFILE_FALSE;
    }

    return SHAPEFILE_TRUE;
}

/**
 * Split a region into two subregion evenly, cutting along the longest dimension
 */
void SHPTreeSplitBounds (
    double *padfBoundsMinIn,
    double *padfBoundsMaxIn,
    double *padfBoundsMin1,
    double *padfBoundsMax1,
    double *padfBoundsMin2,
    double *padfBoundsMax2)
{
    /* The output bounds will be very similar to the input bounds, so just copy over to start */
    memcpy(padfBoundsMin1, padfBoundsMinIn, sizeof(double) * 4);
    memcpy(padfBoundsMax1, padfBoundsMaxIn, sizeof(double) * 4);
    memcpy(padfBoundsMin2, padfBoundsMinIn, sizeof(double) * 4);
    memcpy(padfBoundsMax2, padfBoundsMaxIn, sizeof(double) * 4);
    
    /* Split in X direction */
    if ((padfBoundsMaxIn[0] - padfBoundsMinIn[0]) > (padfBoundsMaxIn[1] - padfBoundsMinIn[1])) {
        double  dfRange = padfBoundsMaxIn[0] - padfBoundsMinIn[0];
        padfBoundsMax1[0] = padfBoundsMinIn[0] + dfRange * SHP_SPLIT_RATIO;
        padfBoundsMin2[0] = padfBoundsMaxIn[0] - dfRange * SHP_SPLIT_RATIO;
    } else {
        /* Otherwise split in Y direction */
        double  dfRange = padfBoundsMaxIn[1] - padfBoundsMinIn[1];
        padfBoundsMax1[1] = padfBoundsMinIn[1] + dfRange * SHP_SPLIT_RATIO;
        padfBoundsMin2[1] = padfBoundsMaxIn[1] - dfRange * SHP_SPLIT_RATIO;
    }
}

/**
 * Tree Node Add ShapeId
 */
static int SHPTreeNodeAddShapeId (
    SHPTreeNode *psTreeNode,
    SHPObject *psObject,
    int nMaxDepth,
    int nDimension)
{
    int    i;
  
    /* If there are subnodes, then consider wiether this object will fit in them */
    if (nMaxDepth > 1 && psTreeNode->nSubNodes > 0) {
        for (i = 0; i < psTreeNode->nSubNodes; i++) {
            if (SHPCheckObjectContained(psObject, nDimension, 
                psTreeNode->apsSubNode[i]->adfBoundsMin,
                psTreeNode->apsSubNode[i]->adfBoundsMax)) {
                return SHPTreeNodeAddShapeId(psTreeNode->apsSubNode[i], psObject, nMaxDepth-1, nDimension);
            }
        }
    }
#if MAX_SUBNODE == 4
    else if (nMaxDepth > 1 && psTreeNode->nSubNodes == 0) {
        /* Otherwise, consider creating four subnodes if could fit into them,
        * and adding to the appropriate subnode */
        double  adfBoundsMinH1[4], adfBoundsMaxH1[4];
        double  adfBoundsMinH2[4], adfBoundsMaxH2[4];
        double  adfBoundsMin1[4], adfBoundsMax1[4];
        double  adfBoundsMin2[4], adfBoundsMax2[4];
        double  adfBoundsMin3[4], adfBoundsMax3[4];
        double  adfBoundsMin4[4], adfBoundsMax4[4];

        SHPTreeSplitBounds(psTreeNode->adfBoundsMin, psTreeNode->adfBoundsMax,
            adfBoundsMinH1, adfBoundsMaxH1, adfBoundsMinH2, adfBoundsMaxH2);

        SHPTreeSplitBounds(adfBoundsMinH1, adfBoundsMaxH1,
            adfBoundsMin1, adfBoundsMax1, adfBoundsMin2, adfBoundsMax2);

        SHPTreeSplitBounds(adfBoundsMinH2, adfBoundsMaxH2,
            adfBoundsMin3, adfBoundsMax3, adfBoundsMin4, adfBoundsMax4);

        if (SHPCheckObjectContained(psObject, nDimension, adfBoundsMin1, adfBoundsMax1) ||
            SHPCheckObjectContained(psObject, nDimension, adfBoundsMin2, adfBoundsMax2) ||
            SHPCheckObjectContained(psObject, nDimension, adfBoundsMin3, adfBoundsMax3) ||
            SHPCheckObjectContained(psObject, nDimension, adfBoundsMin4, adfBoundsMax4)) {
            psTreeNode->nSubNodes = 4;
            psTreeNode->apsSubNode[0] = SHPTreeNodeCreate(adfBoundsMin1, adfBoundsMax1);
            psTreeNode->apsSubNode[1] = SHPTreeNodeCreate(adfBoundsMin2, adfBoundsMax2);
            psTreeNode->apsSubNode[2] = SHPTreeNodeCreate(adfBoundsMin3, adfBoundsMax3);
            psTreeNode->apsSubNode[3] = SHPTreeNodeCreate(adfBoundsMin4, adfBoundsMax4);

            /* recurse back on this node now that it has subnodes */
            return SHPTreeNodeAddShapeId(psTreeNode, psObject, nMaxDepth, nDimension);
        }
    }
#endif /* MAX_SUBNODE == 4 */

#if MAX_SUBNODE == 2
    else if (nMaxDepth > 1 && psTreeNode->nSubNodes == 0) {
        /* Otherwise, consider creating two subnodes if could fit into them,
     *  and adding to the appropriate subnode */
        double  adfBoundsMin1[4], adfBoundsMax1[4];
        double  adfBoundsMin2[4], adfBoundsMax2[4];

        SHPTreeSplitBounds(psTreeNode->adfBoundsMin, psTreeNode->adfBoundsMax,
            adfBoundsMin1, adfBoundsMax1, adfBoundsMin2, adfBoundsMax2);

        if (SHPCheckObjectContained(psObject, nDimension, adfBoundsMin1, adfBoundsMax1)) {
            psTreeNode->nSubNodes = 2;
            psTreeNode->apsSubNode[0] = SHPTreeNodeCreate(adfBoundsMin1, adfBoundsMax1);
            psTreeNode->apsSubNode[1] = SHPTreeNodeCreate(adfBoundsMin2, adfBoundsMax2);

            return SHPTreeNodeAddShapeId(psTreeNode->apsSubNode[0], psObject, nMaxDepth - 1, nDimension);
        } else if (SHPCheckObjectContained(psObject, nDimension, adfBoundsMin2, adfBoundsMax2)) {
            psTreeNode->nSubNodes = 2;
            psTreeNode->apsSubNode[0] = SHPTreeNodeCreate(adfBoundsMin1, adfBoundsMax1);
            psTreeNode->apsSubNode[1] = SHPTreeNodeCreate(adfBoundsMin2, adfBoundsMax2);
            return(SHPTreeNodeAddShapeId(psTreeNode->apsSubNode[1], psObject, nMaxDepth - 1, nDimension));
        }
    }
#endif /* MAX_SUBNODE == 2 */

    /* If none of that worked, just add it to this nodes list */
    psTreeNode->nShapeCount++;

    psTreeNode->panShapeIds = SfRealloc(psTreeNode->panShapeIds, sizeof(int) * psTreeNode->nShapeCount);
    psTreeNode->panShapeIds[psTreeNode->nShapeCount-1] = psObject->nShapeId;

    if (psTreeNode->papsShapeObj != 0) {
        psTreeNode->papsShapeObj = SfRealloc(psTreeNode->papsShapeObj, sizeof(void *) * psTreeNode->nShapeCount);
        psTreeNode->papsShapeObj[psTreeNode->nShapeCount-1] = 0;
    }

    return SHAPEFILE_TRUE;
}

/**
 * Add a shape to the tree, but don't keep a pointer to the object data, just keep the shapeid
 */
int SHPTreeAddShapeId(SHPTreeHandle psTree, SHPObject * psObject)

{
    return(SHPTreeNodeAddShapeId(psTree->psRoot, psObject, psTree->nMaxDepth, psTree->nDimension));
}

/**
 * Work function implementing SHPTreeFindLikelyShapes() on a tree node by tree node basis
 */
void SHPTreeCollectShapeIds (
    SHPTree *hTree,
    SHPTreeNode *psTreeNode,
    double * padfBoundsMin,
    double *padfBoundsMax,
    int *pnShapeCount,
    int *pnMaxShapes,
    int **ppanShapeList)
{
    int i;
    
    /* Does this node overlap the area of interest at all?  If not,
   *   return without adding to the list at all */
    if (!SHPCheckBoundsOverlap(psTreeNode->adfBoundsMin, psTreeNode->adfBoundsMax,
        padfBoundsMin, padfBoundsMax, hTree->nDimension)) {
        return;
    }

    /* Grow the list to hold the shapes on this node */
    if (*pnShapeCount + psTreeNode->nShapeCount > *pnMaxShapes) {
        *pnMaxShapes = (*pnShapeCount + psTreeNode->nShapeCount) * 2 + 20;
        *ppanShapeList = (int*) SfRealloc(*ppanShapeList, sizeof(int) * (*pnMaxShapes));
    }

    /* Add the local nodes shapeids to the list */
    for (i = 0; i < psTreeNode->nShapeCount; i++) {
        (*ppanShapeList)[(*pnShapeCount)++] = psTreeNode->panShapeIds[i];
    }
    
    /* Recurse to subnodes if they exist */
    for (i = 0; i < psTreeNode->nSubNodes; i++) {
        if (psTreeNode->apsSubNode[i] != 0) {
            SHPTreeCollectShapeIds(hTree, psTreeNode->apsSubNode[i],
                padfBoundsMin, padfBoundsMax, pnShapeCount, pnMaxShapes, ppanShapeList);
        }
    }
}

/**
 * Find all shapes within tree nodes for which the tree node
 *  bounding box overlaps the search box.  The return value is
 *  an array of shapeids terminated by a -1.  The shapeids will
 *  be in order, as hopefully this will result in faster (more sequential)
 *  reading from the file. 
 */
int* SHPTreeFindLikelyShapes(SHPTreeHandle hTree, double *padfBoundsMin, double *padfBoundsMax, int *pnShapeCount)
{
    int  *panShapeList=0, nMaxShapes = 0;
    int  i, j, tempId;

    /* Perform the search by recursive descent */
    *pnShapeCount = 0;

    SHPTreeCollectShapeIds(hTree, hTree->psRoot, padfBoundsMin, padfBoundsMax,
        pnShapeCount, &nMaxShapes, &panShapeList);

    /*??TODO: For now I just use a bubble sort to order the shapeids,
   * but this should really be a quicksort */
    for (i = 0; i < *pnShapeCount-1; i++) {
        for (j = 0; j < (*pnShapeCount) - i - 1; j++) {
            if (panShapeList[j] > panShapeList[j+1]) {
                tempId = panShapeList[j];
                panShapeList[j] = panShapeList[j+1];
                panShapeList[j+1] = tempId;
            }
        }
    }
    return panShapeList;
}

/**
 * This is the recurve version of SHPTreeTrimExtraNodes() that walks the tree cleaning it up
 */
static int SHPTreeNodeTrim(SHPTreeNode * psTreeNode)
{
    int    i;

    /* Trim subtrees, and free subnodes that come back empty */
    for (i = 0; i < psTreeNode->nSubNodes; i++) {
        if (SHPTreeNodeTrim(psTreeNode->apsSubNode[i])) {
            SHPDestroyTreeNode(psTreeNode->apsSubNode[i]);
            psTreeNode->apsSubNode[i] = psTreeNode->apsSubNode[psTreeNode->nSubNodes-1];
            psTreeNode->nSubNodes--;

            /* process the new occupant of this subnode entry */
            i--;
        }
    }

    /* We should be trimmed if we have no subnodes, and no shapes */
    return(psTreeNode->nSubNodes == 0 && psTreeNode->nShapeCount == 0);
}

/**
 * Trim empty nodes from the tree.  Note that we never trim an empty root node
 */
void SHPTreeTrimExtraNodes (SHPTreeHandle hTree)
{
    SHPTreeNodeTrim(hTree->psRoot);
}
