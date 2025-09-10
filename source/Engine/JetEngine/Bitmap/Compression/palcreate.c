/****************************************************************************************/
/*  PalCreate                                                                           */
/*                                                                                      */
/*  Author: Charles Bloom                                                               */
/*  Description:  Palette Creation code                                                 */
/*                                                                                      */
/*  The contents of this file are subject to the Jet3D Public License                   */
/*  Version 1.02 (the "License"); you may not use this file except in                   */
/*  compliance with the License. You may obtain a copy of the License at                */
/*  http://www.jet3d.com                                                                */
/*                                                                                      */
/*  Software distributed under the License is distributed on an "AS IS"                 */
/*  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See                */
/*  the License for the specific language governing rights and limitations              */
/*  under the License.                                                                  */
/*                                                                                      */
/*  The Original Code is Jet3D, released December 12, 1999.                             */
/*  Copyright (C) 1996-1999 Eclipse Entertainment, L.L.C. All Rights Reserved           */
/*                                                                                      */
/****************************************************************************************/
#define DO_YUV_DEFINE (JE_TRUE)

/**********

createPalGood goes in about 0.8 seconds
with about 0.5 of those in the "CreatePalOctTree" function

// <> could use Optimize

---------------

createPaletteFast :
	trivial kludge:
		gather colors in an octree
		sorts colors on popularity
		adds them to the palette, trying to avoiding adding extremely similar colors
		has some speed-ups (like croppping low-count leaves)

createPaletteGood :
	this is the "optimal" octree color quantizer. (see below, note on non-optimality)
	it is *VERY FAST* !
	gather all colors in an octree
	prune isolated strands so all nodes have > 1 kids
	the primary action is a "collapse" move:
		a leaf is cut, so that its color will be mapped to a sibling
		if it has no siblings, the color gets mapped to its parent
	each leaf keeps track of the "cost" (= increase in MSE) of cutting it
	each node has color which is the weighted average of its children
	the "cost" of a node, is the cost of all its children, plus the cost to
		move its new centroid.  This is exact, it's kind of subtle. see later
	we keep removing the node which has the lowest cost to cut
		(we use a radix sort to sort on cutCost ; this gives us the speed win)

my fast (incremental) way to compute the JE_TRUE node cost :
	JE_TRUE_cost = Sum[kids] kid_count * (kid_color - new_color)^2
	my_cost = Sum[kids] kid_count * (kid_color - node_color)^2
					+ node_count * (node_color - new_color)^2

	JE_TRUE_cost = Sum[kids] kid_count * (kid_color - new_color)^2
			  = Sum[kids] kid_count * ((kid_color - node_color) + (node_color - new_color))^2
			  = Sum[kids] kid_count * ((kid_color - node_color)^2 + (node_color - new_color)^2
										+ 2 * (kid_color - node_color) * (node_color - new_color))
			= approx_cost + 2 *  (node_color - new_color) * { Sum[kids] kid_count * (kid_color - node_color) }
					  
	the correction here is exactly zero! why :
		Sum[kids] kid_count * (kid_color - node_color) = (Sum[kids] kid_count * kid_color) - node_count * node_color = 0 !
	since that's the definition of node_color !

why this isn't exactly optimal:
	because octree children without the same parent are never grouped.
	the classic example	is in the binary tree, values 128 and 127 should have a cost of 1 to be
		merged together, but 128 will be merged with all values > 128 first.
	that is, the square boundaries of the tree are unnatural cuts in color space.
	this means that the "cutCost" is not accurate; there could be an actual lower MSE cost
		than our cutCost.
	furthermore, cutCost should be relative to all other leaves, not to their parent nodes,
		so that when I cut one leaf it changes the cutCosts of all other leaves.

***********/
/*}{*************************************************/

#include "palcreate.h"
#include "Tsc.h"
#include "paloptimize.h"
#include "Ram.h"
#include "YUV.h"
#include "MemPool.h"
#include "Image.h"
#include <stdlib.h>
#include <assert.h>

/*******/

#define allocate(ptr)	ptr = jeRam_AllocateClear(sizeof(*ptr))
#define clear(ptr)		memset(ptr,0,sizeof(*ptr))

/*}{*************************************************/

jeBitmap_Palette * createPaletteGood(const jeBitmap_Info * Info,const void * Bits);
jeBitmap_Palette * createPaletteFast(const jeBitmap_Info * Info,const void * Bits);

paletteCreater myPaletteCreater = createPaletteGood;

void setCreatePaletteFunc(paletteCreater func)
{
	assert( func == createPaletteGood || func == createPaletteFast );
	myPaletteCreater = func;
}

jeBitmap_Palette * createPalette(const jeBitmap_Info * Info,const void * Bits)
{
	assert(Info && Bits);
	switch(Info->Format)
	{
		case JE_PIXELFORMAT_8BIT_PAL :
			return Info->Palette;
		case JE_PIXELFORMAT_8BIT_GRAY :
		{
		jeBitmap_Palette * Pal;
		uint8 GrayPal[256];
		int i;
			Pal = jeBitmap_Palette_Create(JE_PIXELFORMAT_8BIT_GRAY,256);
			if ( ! Pal ) return NULL;
			for(i=0;i<256;i++) GrayPal[i] = i;
			jeBitmap_Palette_SetData(Pal,GrayPal,JE_PIXELFORMAT_8BIT_GRAY,256);
		return Pal;
		}
		default:
			return myPaletteCreater(Info,Bits);
	}
}

jeBitmap_Palette * createPaletteFromBitmap(const jeBitmap * Bitmap,jeBoolean Optimize)
{
jeBitmap * Lock;
jeBitmap_Info Info;
const void * Bits;
jeBitmap_Palette * Pal;

	if ( ! jeBitmap_GetInfo(Bitmap,&Info,NULL) )
		return NULL;

	if ( ! jeBitmap_LockForRead((jeBitmap *)Bitmap,&Lock,0,0,JE_PIXELFORMAT_24BIT_RGB,JE_FALSE,0) )
		return NULL;
	
	if ( ! jeBitmap_GetInfo(Lock,&Info,NULL) )
		return NULL;

	Bits = (const void *) jeBitmap_GetBits(Lock);

	Pal = createPalette(&Info,Bits);

	if ( Pal && Optimize )
	{
	uint8 paldata[768];

		if ( ! jeBitmap_Palette_GetData(Pal,paldata,JE_PIXELFORMAT_24BIT_RGB,256) )
			assert(0);
		
		paletteOptimize(&Info,Bits,paldata,256,0);
		
		if ( ! jeBitmap_Palette_SetData(Pal,paldata,JE_PIXELFORMAT_24BIT_RGB,256) )
			assert(0);
	}
		
	jeBitmap_UnLock(Lock);

return Pal;
}

/*}{*************************************************/

typedef struct octNode octNode;
struct octNode 
{
	octNode * kids[8];
	octNode * parent;
	int count,nKids;
	int R,G,B;

	// for the pruner:
	uint32 cutCost;	// this could overflow in the upper root nodes
	octNode *prev,*next; // sorted linked list of leaves
};

#define square(x)	((x)*(x))

#define RGBbits(R,G,B,bits) (((((R)>>(bits))&1)<<2) + ((((G)>>(bits))&1)<<1) + (((B)>>((bits)))&1))

#define RADIX_SIZE	1024

int createOctTreeFromImage(octNode * root,const image *im);
int createOctTree(octNode * root,const jeBitmap_Info * Info,const void * Bits,jeBoolean doYUV);
jeBitmap_Palette * createPaletteGoodSub(const jeBitmap_Info * Info,const void * Bits,const image *im);
static void addOctNode(octNode *root,int R,int G,int B,int *nLeavesPtr);
static void gatherLeaves(octNode *node,octNode *** leavesPtrPtr,int minCount);
static void gatherLeavesCutting(octNode *node,octNode *** leavesPtrPtr);
static int leafCompareCount(const void *a,const void *b);
static int leafCompareCost(const void *a,const void *b);
int findClosest(int R,int G,int B,uint8 *palette,int palEntries,int *foundPalPtr);
void computeOctRGBs(octNode *node);
void computeCutCosts(octNode *node);
void readLeavesToPal(octNode **leaves,int gotLeaves,uint8 *palette,int palEntries);
void insertRadix(octNode * radix,octNode *leaf);

/*}{*************************************************/

static MemPool * octNodePool = NULL;
static int PoolRefs = 0;

void PalCreate_Start(void)
{
	if ( PoolRefs == 0 )
	{
	int num;
		// we do addOctNode, one for each unique color
		// make the poolhunks 64k
		num = (1<<16) / sizeof(octNode);
		octNodePool = MemPool_Create(sizeof(octNode),num,num);
		assert(octNodePool);
	}
	PoolRefs ++;
}

void PalCreate_Stop(void)
{
	PoolRefs --;
	if ( PoolRefs == 0 )
	{
		MemPool_Destroy(&octNodePool);
	}
}

/*}{*************************************************/

jeBitmap_Palette * createPaletteFast(const jeBitmap_Info * Info,const void * Bits)
{
octNode * root;
int nLeaves,minCount,gotLeaves;
octNode ** leaves,**leavesPtr;
uint8 palette[768];
int palEntries = 256;
jeBitmap_Palette * Pal;

	pushTSC();

	// read the whole image into an octree
	//	this is the only pass over the input plane

	MemPool_Reset(octNodePool);
	root = (octNode *)MemPool_GetHunk(octNodePool);
	assert(root);
	nLeaves = createOctTree(root,Info,Bits,JE_FALSE);

	leaves = (octNode **)jeRam_AllocateClear(sizeof(octNode *)*nLeaves);
	assert(leaves);
	
	// gather leaves into a linear array
	//	for speed we ignore leaves with a count lower than [x]

	gotLeaves = 0;
	for( minCount = 3; minCount >= 0 && gotLeaves < palEntries ; minCount-- )
	{
		leavesPtr = leaves;
		gatherLeaves(root,&leavesPtr,minCount);
		gotLeaves = ((uint32)leavesPtr - (uint32)leaves)/sizeof(octNode *);
	}

	// sort the leaves by count

	qsort(leaves,gotLeaves,sizeof(octNode *),leafCompareCount);

	// read the sorted leaves in by count; we try to only read in leaves
	//	that are farther than 'minDistance' from nodes already in the palette

	readLeavesToPal(leaves,gotLeaves,palette,palEntries);

	destroy(leaves);

	showPopTSC("createPalFast");

	Pal = jeBitmap_Palette_Create(JE_PIXELFORMAT_24BIT_RGB,palEntries);
	if ( ! Pal )
		return NULL;
	if ( ! jeBitmap_Palette_SetData(Pal,palette,JE_PIXELFORMAT_24BIT_RGB,palEntries) )
		assert(0);
return Pal;
}

/*}{*************************************************/

jeBitmap_Palette * createPaletteGood(const jeBitmap_Info * Info,const void * Bits)
{
	return createPaletteGoodSub(Info,Bits,NULL);
}
jeBitmap_Palette * createPaletteFromImage(const image *im)
{
	return createPaletteGoodSub(NULL,NULL,im);
}

jeBitmap_Palette * createPaletteGoodSub(const jeBitmap_Info * Info,const void * Bits,const image *im)
{
octNode * root;
int nLeaves,i,gotLeaves,radixN;
octNode ** leaves,**leavesPtr;
octNode *leaf,*node;
octNode *radix;
uint8 palette[768],*palPtr;
int palEntries = 256;
jeBitmap_Palette * Pal;
jeBoolean DoYUV;

	pushTSC();

	if ( im )
		DoYUV = JE_FALSE;
	else
		DoYUV = DO_YUV_DEFINE;

	// <> hack !
//	if ( Info->Format == JE_PIXELFORMAT_24BIT_RGB )

//	else
//		DoYUV = JE_FALSE;

	// read the whole image into an octree
	//	this is the only pass over the input plane

	MemPool_Reset(octNodePool);
	root = (octNode *)MemPool_GetHunk(octNodePool);
	assert(root);

	if ( im )
		nLeaves = createOctTreeFromImage(root,im);
	else
		nLeaves = createOctTree(root,Info,Bits,DoYUV);

	leaves = (octNode **)jeRam_AllocateClear(sizeof(octNode *)*nLeaves);
	assert(leaves);

	computeOctRGBs(root);
	root->parent = root;
	computeCutCosts(root);
	root->parent = NULL;
	
	// gather leaves into a linear array
	//	for speed we ignore leaves with a count lower than [x]

	leavesPtr = leaves;
	gatherLeavesCutting(root,&leavesPtr);
	gotLeaves = ((uint32)leavesPtr - (uint32)leaves)/sizeof(octNode *);

	// if gotLeaves < palEntries, just exit asap
	if ( gotLeaves < palEntries )
	{
		readLeavesToPal(leaves,gotLeaves,palette,palEntries);
		goto done;
	}

	// sort the leaves by cutCost
	// radix sort instead of qsort

	radix = (octNode *)jeRam_AllocateClear(sizeof(octNode)*RADIX_SIZE);
	assert(radix);

	for(i=0;i<RADIX_SIZE;i++)
		radix[i].next = radix[i].prev = &radix[i];

	for(i=0;i<gotLeaves;i++)
		insertRadix(radix,leaves[i]);

	// keep cutting the tail
	radixN = 0;
	while(gotLeaves > palEntries)
	{
		while( (leaf = radix[radixN].next) == &(radix[radixN]) )
		{
			radixN++;
			assert( radixN < RADIX_SIZE );
		}
		// cut it
		leaf->prev->next = leaf->next;
		leaf->next->prev = leaf->prev;

		node = leaf->parent;
		assert(node);
		node->nKids --;

		// might turn its parent into a leaf;
		// if so, add it to the list
			// nKids no longer corresponds to the actual number of active kids

		if ( node->nKids == 0 )
			insertRadix(radix,node);
		else
			gotLeaves--;
	}

	// read the sorted leaves in by count; we try to only read in leaves
	//	that are farther than 'minDistance' from nodes already in the palette

	palPtr = palette;
	radixN = RADIX_SIZE-1;
	leaf = radix[radixN].prev;	
	for(i=0;i<palEntries && radixN>0;i++)
	{
		*palPtr++ = leaf->R;
		*palPtr++ = leaf->G;
		*palPtr++ = leaf->B;
		leaf = leaf->prev;
		while ( leaf == &(radix[radixN]) )
		{
			radixN --;
			if ( ! radixN )
				break;
			leaf = radix[radixN].prev;
		}
	}

	destroy(radix);

done:

	destroy(leaves);

	showPopTSC("createPalGood");

	if ( DoYUV )
	{
		YUVb_to_RGBb_line(palette,palette,palEntries);
	}

	Pal = jeBitmap_Palette_Create(JE_PIXELFORMAT_24BIT_RGB,palEntries);
	if ( ! Pal )
		return NULL;

	if ( ! jeBitmap_Palette_SetData(Pal,palette,JE_PIXELFORMAT_24BIT_RGB,palEntries) )
		assert(0);

return Pal;
}

/*}{*************************************************/

void insertRadix(octNode * radix,octNode *leaf)
{
octNode *insertAt;

	if ( leaf->cutCost >= RADIX_SIZE ) 
	{
		octNode * head;
		insertAt = head = & radix[RADIX_SIZE-1];
		while(insertAt->cutCost < leaf->cutCost && insertAt->next != head )
			insertAt = insertAt->next;
	}
	else
		insertAt = & radix[leaf->cutCost];

	leaf->next = insertAt->next;
	leaf->next->prev = leaf;
	insertAt->next = leaf;
	insertAt->next->prev = insertAt;
}

int findClosest(int R,int G,int B,uint8 *palette,int palEntries,int *foundPalPtr)
{
int i,d,bestD,bestP;
	bestD = 99999999; bestP = -1;
	for(i=palEntries;i--;)
	{
		d = square(R - palette[0]) + square(G - palette[1]) + square(B - palette[2]);
		palette += 3;
		if ( d < bestD ) 
		{
			bestD = d;
			bestP = i;
		}
	}
	if ( foundPalPtr ) *foundPalPtr = bestP;
return bestD;
}

static void addOctNode(octNode *root,int R,int G,int B,int *nLeavesPtr)
{
int idx;
int bits;
octNode *node;

	node = root;
	for(bits=7;bits>=0;bits--) 
	{
		idx = RGBbits(R,G,B,bits);
		if ( ! node->kids[idx] ) 
		{
			node->nKids ++;
			node->kids[idx] = (octNode *)MemPool_GetHunk(octNodePool);
			node->kids[idx]->parent = node;
		}
		node->count ++;
		node = node->kids[idx];
	}
	if ( node->count == 0 ) (*nLeavesPtr)++;
	node->count ++;
	node->R = R;
	node->G = G;
	node->B = B;
}

static void gatherLeaves(octNode *node,octNode *** leavesPtrPtr,int minCount)
{
	if ( node->count <= minCount ) return;
	if ( node->nKids == 0 ) 
	{
		*(*leavesPtrPtr)++ = node;	
	}
	else
	{
		int i;
		for(i=0;i<8;i++)
		{
			if ( node->kids[i] ) gatherLeaves(node->kids[i],leavesPtrPtr,minCount);
		}
	}
}

static void gatherLeavesCutting(octNode *node,octNode *** leavesPtrPtr)
{
	if ( node->nKids > 0 ) 
	{
		int i;
		for(i=0;i<8;i++)
		{
			if ( node->kids[i] )
			{
				if ( node->kids[i]->count <= 1 || node->kids[i]->cutCost <= 1 )
				{
					//freeOctNodes(node->kids[i]);
					node->kids[i] = NULL;
					node->nKids--;
				}
				else
				{
					gatherLeavesCutting(node->kids[i],leavesPtrPtr);
					
					if ( node->kids[i]->nKids == 1 )
					{
						octNode *kid;
						int j;
						kid = node->kids[i];
						for(j=0;j<8;j++)
							if ( kid->kids[j] ) 
								node->kids[i] = kid->kids[j];
						assert( node->kids[i] != kid );
						node->kids[i]->cutCost = kid->cutCost;
						//destroy(kid);
						node->kids[i]->parent = node;
					}
				}
			}
		}
	}

	if ( node->nKids == 0 ) 
	{
		*(*leavesPtrPtr)++ = node;	
	}
}

static int leafCompareCount(const void *a,const void *b)
{
octNode *na,*nb;
	na = *((octNode **)a);
	nb = *((octNode **)b);
return (nb->count) - (na->count);
}
static int leafCompareCost(const void *a,const void *b)
{
octNode *na,*nb;
	na = *((octNode **)a);
	nb = *((octNode **)b);
return (nb->cutCost) - (na->cutCost);
}

void computeCutCosts(octNode *node)
{
	assert(node->parent);
	node->cutCost = square(node->R - node->parent->R)
					+ square(node->G - node->parent->G)
					+ square(node->B - node->parent->B);
	node->cutCost *= node->count;
	
	if ( node->nKids > 0 )
	{
	int i;
		for(i=0;i<8;i++)
			if ( node->kids[i] )
			{
				computeCutCosts(node->kids[i]);
				node->cutCost += node->kids[i]->cutCost;
			}
	}
}

void computeOctRGBs(octNode *node)
{
	if ( node->nKids > 0 )
	{
	int R,G,B;
	int i;
	octNode *kid;
		R = G = B = 0;
		for(i=0;i<8;i++)
			if ( node->kids[i] )
				computeOctRGBs(node->kids[i]);
		for(i=0;i<8;i++)
		{
			if ( kid = node->kids[i] )
			{
				R += kid->count * kid->R;
				G += kid->count * kid->G;
				B += kid->count * kid->B;
			}
		}
		node->R = R / (node->count);
		node->G = G / (node->count);
		node->B = B / (node->count);
	}
}

void readLeavesToPal(octNode **leaves,int gotLeaves,uint8 *palette,int palEntries)
{
octNode **leavesPtr;
uint8 *palPtr;
int R,G,B;
int i,palGot;
int distance,minDistance;

	palPtr = palette; palGot = 0;
	for(minDistance=256;minDistance>=0 && palGot < palEntries;minDistance>>=1)
	{
		leavesPtr = leaves;
		for(i=0;i<gotLeaves;i++)
		{
			R = (*leavesPtr)->R;
			G = (*leavesPtr)->G;
			B = (*leavesPtr)->B;
			leavesPtr++;
			distance = findClosest(R,G,B,palette,palGot,NULL);
			if ( distance >= minDistance )
			{
				*palPtr++ = R;
				*palPtr++ = G;
				*palPtr++ = B;
				palGot ++;
				if ( palGot == palEntries )
					break;
			}
		}
	}
}

/*}{*************************************************/

int createOctTree(octNode * root,const jeBitmap_Info * Info,const void * Bits,jeBoolean doYUV)
{
int nLeaves;
int w,h,xtra,bpp,x,y;
jePixelFormat Format;
const jePixelFormat_Operations * ops;
int R,G,B,A;
jePixelFormat_Decomposer Decompose;

	assert(Bits);

	nLeaves = 0;

	Format = Info->Format;
	w = Info->Width;
	h = Info->Height;
	xtra = Info->Stride - Info->Width;
	bpp = jePixelFormat_BytesPerPel(Format);

	ops = jePixelFormat_GetOperations(Format);
	assert(ops);
	Decompose = ops->DecomposePixel;
	assert(Decompose);

//	pushTSC();

	if ( doYUV )
	{
		switch(bpp)
		{
			default:
			case 0:
				return JE_FALSE;
			case 1:
			{
			const uint8 *ptr;
				ptr = (uint8*)Bits;
				for(y=h;y--;)
				{
					for(x=w;x--;)
					{
						Decompose(*ptr++,&R,&G,&B,&A);
						RGBi_to_YUVi(R,G,B,&R,&G,&B);
						addOctNode(root,R,G,B,&nLeaves);
					}
					ptr += xtra;
				}
				break;
			}
			case 2:
			{
			const uint16 *ptr;
			uint32 R,G,B,A;
				ptr = (uint16 *)Bits;
				for(y=h;y--;)
				{
					for(x=w;x--;)
					{
						Decompose(*ptr++,(int *)&R,(int *)&G,(int *)&B,(int *)&A);
						RGBi_to_YUVi(R,G,B,(int *)&R,(int *)&G,(int *)&B);
						addOctNode(root,R,G,B,&nLeaves);
					}
					ptr += xtra;
				}
				break;
			}

			case 3:
			{
			const uint8 *ptr;
			uint32 R,G,B,A,Pixel;

				ptr = (uint8*)Bits;
				xtra *= 3;

				switch(Format)
				{
				case JE_PIXELFORMAT_24BIT_RGB :
					for(y=h;y--;)
					{
						for(x=w;x--;)
						{
							RGBb_to_YUVi(ptr,(int *)&R,(int *)&G,(int *)&B);
							ptr += 3;
							addOctNode(root,R,G,B,&nLeaves);
						}
						ptr += xtra;
					}
					break;
				case JE_PIXELFORMAT_24BIT_BGR :
					for(y=h;y--;)
					{
						for(x=w;x--;)
						{
							B = *ptr++;
							G = *ptr++;
							R = *ptr++;
							RGBi_to_YUVi(R,G,B,(int *)&R,(int *)&G,(int *)&B);
							addOctNode(root,R,G,B,&nLeaves);
						}
						ptr += xtra;
					}
					break;
				default:
					// can't get here now
					for(y=h;y--;)
					{
						for(x=w;x--;)
						{
							Pixel = (ptr[0]<<16) + (ptr[1]<<8) + ptr[2]; ptr += 3;
							Decompose(Pixel,(int *)&R,(int *)&G,(int *)&B,(int *)&A);
							RGBi_to_YUVi(R,G,B,(int *)&R,(int *)&G,(int *)&B);
							addOctNode(root,R,G,B,&nLeaves);
						}
						ptr += xtra;
					}
					break;
				}
				break;
			}

			case 4:
			{
			const uint32 *ptr;
			uint32 R,G,B,A;
				ptr = (uint32 *)Bits;
				for(y=h;y--;)
				{
					for(x=w;x--;)
					{
						Decompose(*ptr++,(int *)&R,(int *)&G,(int *)&B,(int *)&A);
						RGBi_to_YUVi(R,G,B,(int *)&R,(int *)&G,(int *)&B);
						addOctNode(root,R,G,B,&nLeaves);
					}
					ptr += xtra;
				}
				break;
			}
		}
	}
	else
	{
		switch(bpp)
		{
			default:
			case 0:
				return JE_FALSE;
			case 1:
			{
			const uint8 *ptr;
				ptr = (uint8*)Bits;
				for(y=h;y--;)
				{
					for(x=w;x--;)
					{
						Decompose(*ptr++,&R,&G,&B,&A);
						addOctNode(root,R,G,B,&nLeaves);
					}
					ptr += xtra;
				}
				break;
			}
			case 2:
			{
			const uint16 *ptr;
			uint32 R,G,B,A;
				ptr = (uint16 *)Bits;
				for(y=h;y--;)
				{
					for(x=w;x--;)
					{
						Decompose(*ptr++,(int *)&R,(int *)&G,(int *)&B,(int *)&A);
						addOctNode(root,R,G,B,&nLeaves);
					}
					ptr += xtra;
				}
				break;
			}

			case 3:
			{
			const uint8 *ptr;
			uint32 R,G,B,A,Pixel;

				ptr = (uint8*)Bits;
				xtra *= 3;

				switch(Format)
				{
				case JE_PIXELFORMAT_24BIT_RGB :
					for(y=h;y--;)
					{
						for(x=w;x--;)
						{
							R = *ptr++;
							G = *ptr++;
							B = *ptr++;
							addOctNode(root,R,G,B,&nLeaves);
						}
						ptr += xtra;
					}
					break;
				case JE_PIXELFORMAT_24BIT_BGR :
					for(y=h;y--;)
					{
						for(x=w;x--;)
						{
							B = *ptr++;
							G = *ptr++;
							R = *ptr++;
							addOctNode(root,R,G,B,&nLeaves);
						}
						ptr += xtra;
					}
					break;
				default:
					// can't get here now
					for(y=h;y--;)
					{
						for(x=w;x--;)
						{
							Pixel = (ptr[0]<<16) + (ptr[1]<<8) + ptr[2]; ptr += 3;
							Decompose(Pixel,(int *)&R,(int *)&G,(int *)&B,(int *)&A);
							addOctNode(root,R,G,B,&nLeaves);
						}
						ptr += xtra;
					}
					break;
				}
			}

			case 4:
			{
			const uint32 *ptr;
			uint32 R,G,B,A;
				ptr = (uint32 *)Bits;
				for(y=h;y--;)
				{
					for(x=w;x--;)
					{
						Decompose(*ptr++,(int *)&R,(int *)&G,(int *)&B,(int *)&A);
						addOctNode(root,R,G,B,&nLeaves);
					}
					ptr += xtra;
				}
				break;
			}
		}
	}

//	showPopTSC("create Pal OctTree");

return nLeaves;
}

int createOctTreeFromImage(octNode * root,const image *im)
{
int nLeaves;
int s;
int *Yp,*Up,*Vp;

	nLeaves = 0;

	assert(im->planes >= 3);

	Yp = im->data[0][0];
	Up = im->data[1][0];
	Vp = im->data[2][0];

	for(s=im->plane_size;s--;)
	{
		addOctNode(root,*Yp++,*Up++,*Vp++,&nLeaves);
	}

return nLeaves;
}

/*}{*************************************************/
