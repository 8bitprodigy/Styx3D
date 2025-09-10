/****************************************************************************************/
/*  SORTPAL.H                                                                           */
/*                                                                                      */
/*  Author:                                                                             */
/*  Description:                                                                        */
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
#ifndef SORTPAL_H
#define SORTPAL_H

#include "Utility.h"

#pragma pack(1)
typedef struct {
	int r,g,b;
} RGB;
#pragma pack()

extern jeBoolean sortPal(int size,uint8 *rgb_colors, int *permutation,int *usage,int flags);

#define SORTPAL_FAST  		0
#define SORTPAL_LAZY  		1
#define SORTPAL_OPTIMIZE	4

/** internal-used only : **/

typedef struct _palNode palNode;

typedef struct _palNode {
	RGB color;
	int index;
	int c1,c2,c3;
	palNode *next,*prev; // NULL if not yet decided
	jeBoolean visited;
	int d_next;
};

extern palNode * initPal(uint8 * rgb_colors,int size);
extern jeBoolean readOutPal(palNode *start_node,int size,uint8 *rgb_colors,int *permutation);

extern void greedySort(palNode *start_node,palNode *nodes,int size,int *usage);
extern void lazySort(palNode *start_node,palNode *nodes,int size,int *usage);
extern jeBoolean doOptimize(palNode *nodes,int size,int block,palNode **start_node_ptr,int *usage);

#if 1	// kills weights
#define weighted_node_distance(x,y,z) node_distance(x,y)
#else
extern int weighted_node_distance(palNode *x,palNode *y,int *usage);
#endif

extern int node_distance_sqr(palNode *x,palNode *y);
extern void reportTotLen(palNode *start_node,int *usage);
extern void restoreRGB(palNode *n);

#define cut_pal_node(node) do {	if ( (node)->next ) (node)->next->prev = (node)->prev;			\
								if ( (node)->prev ) (node)->prev->next = (node)->next; } while(0)

#define add_after(node,after) do {	\
	(node)->next = (after)->next;	(node)->prev = (after);	\
	if ( (node)->prev ) (node)->prev->next = (node);	\
	if ( (node)->next ) (node)->next->prev = (node);	\
} while(0)

/********************************/

// #define SQRT_MEASURE	// <- very slow
// #define SQR_MEASURE	// <- seems to help the lossy compression quality

#ifdef SQRT_MEASURE
#define node_distance(x,y) sqrt(node_distance_sqr(x,y))
#else
#ifdef SQR_MEASURE
#define node_distance(x,y) (uint32)min(square(node_distance_sqr(x,y)),(1<<24))
#else
#define node_distance(x,y) node_distance_sqr(x,y)
#endif //  SQR MEASURE
#endif // SQRT_MEASURE

#define set_d_next(node,usage) (node)->d_next = weighted_node_distance(node,(node)->next,usage)

#endif 	// H_FILE
