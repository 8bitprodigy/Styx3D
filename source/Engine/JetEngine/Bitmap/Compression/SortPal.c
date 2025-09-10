/****************************************************************************************/
/*  SORTPAL.C                                                                           */
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

/*****

 store d_next in each node, just for speed.
 computed by the first Sort()
 used by Optimize()

******/

#include "Utility.h"
#include "colorconv.h"
#include <math.h>
#include <limits.h>
#include <stdio.h>
#include <assert.h>
#include "SortPal.h"

#define START_W_MOST_ISOLATED

#define RGB_transform(rgb,y,u,v) 		conv_RGB_to_YUV(rgb.r,rgb.g,rgb.b,y,u,v)
#define undo_RGB_transform(y,u,v,rgb)	conv_YUV_to_RGB(y,u,v,&(rgb.r),&(rgb.g),&(rgb.b))
//#define DO_HSV

/******** protos ****************/

palNode * chooseStartPal(palNode *nodes,int size,int *usage);
palNode * initPal(uint8 * rgb_colors,int size);
jeBoolean readOutPal(palNode *start_node,int size,uint8 *rgb_colors,int *permutation);

void greedySort(palNode *start_node,palNode *nodes,int size,int *usage);
void lazySort(palNode *start_node,palNode *nodes,int size,int *usage);

jeBoolean doOptimize(palNode *nodes,int size,int block,palNode **start_node_ptr,int *usage);

int node_distance_sqr(palNode *x,palNode *y);
void reportTotLen(palNode *start_node,int *usage);

int weighted_node_distance(palNode *x,palNode *y,int *usage);

/********************************/

jeBoolean sortPal(int size,uint8 *rgb_colors, int *permutation,int *usage,int flags)
{
int i;
palNode *nodes,*start_node;

	if ( (nodes = initPal(rgb_colors,size)) == NULL )
		return JE_FALSE;
	
	start_node = chooseStartPal(nodes,size,usage);

	if ( flags & SORTPAL_LAZY )
	{
		lazySort(start_node,nodes,size,usage);
	}
	else
	{
		greedySort(start_node,nodes,size,usage);
	}

	reportTotLen(start_node,usage);

	/** we now have a greedy-sorted linked list. 
	*	
	*	now go through and see if any nodes can be
	*	moved to a new location to decrease the error
	*
	*	we try w/ singles, then pairs, then triplets, etc.
	*
	***/

	if ( flags & SORTPAL_OPTIMIZE )
	{

		for(i=0;i<(size/2);i++) {
			if ( doOptimize(nodes,size,i,&start_node,usage) ) {
				reportTotLen(start_node,usage);
				i=-1;
			}
		}
	}

	if ( ! readOutPal(start_node,size,rgb_colors,permutation) ) {
		BrandoError("bad linked list");
		destroy(nodes);
		return JE_FALSE;
	}

	reportTotLen(start_node,usage);

	destroy(nodes);

return JE_TRUE;
}


jeBoolean inList(palNode *test,palNode *head,palNode *tail)
{
palNode *cur;
	cur =head;
	while(cur != tail->next) {
		if ( cur == test ) return JE_TRUE;
		assert(cur->next->prev == cur);
		cur = cur->next;
	}
return JE_FALSE;
}

jeBoolean doOptimize(palNode *nodes,int size,int block,palNode **start_node_ptr,int *usage)
{
int i,j,d,nd;
palNode *head,*tail,*target,*start_node;
jeBoolean didFiddle;

	start_node = *start_node_ptr;
	didFiddle = JE_FALSE;

	for(i=0;i<size;i++) {
		head = nodes+i;
		tail = head; for(j=block;j-- && tail;) tail = tail->next;
		if ( head->prev ) {
			if ( tail && tail->next ) {
				for(j=0;j<size;j++) {	target = nodes+j;
					if ( ! inList(target,head,tail) && target->next && head != target->next ) {
						//d  =  weighted_node_distance(head,head->prev,usage) + weighted_node_distance(tail,tail->next,usage) + weighted_node_distance(target,target->next,usage);
						d = head->prev->d_next + tail->d_next + target->d_next;
						nd =	weighted_node_distance(head->prev,tail->next,usage) + 
								weighted_node_distance(head,target,usage) + 
								weighted_node_distance(tail,target->next,usage);
			
						if ( nd < d ) {
							// insert the block (head,tail) between target & target->next
							// first cut out cur
							tail->next->prev = head->prev;
							head->prev->next = tail->next;
							set_d_next(head->prev,usage);
							// put it in:
							tail->next = target->next;
							head->prev = target;
							head->prev->next = head;
							tail->next->prev = tail;
							set_d_next(tail,usage);
							set_d_next(head->prev,usage);
							didFiddle = JE_TRUE;
							break; // do it just for speed
						}
					}				
				}
			} else if ( tail ) {
				/** special case : we're at the end of the list : tail && !tail->next **/

				target = start_node;
				while(target && target->next && target->next != head) {
					d  =  head->prev->d_next + target->d_next;
					nd =  weighted_node_distance(head,target,usage) + weighted_node_distance(tail,target->next,usage);
		
					if ( nd < d ) {
						// insert the block (head,tail) between target & target->next
						// first cut out cur
						head->prev->next = NULL;	// sets a new end of the list
						head->prev->d_next = 0;
						// put it in:
						tail->next = target->next;
						head->prev = target;
						head->prev->next = head;
						tail->next->prev = tail;
						set_d_next(tail,usage);
						set_d_next(head->prev,usage);
						didFiddle = JE_TRUE;
						target = NULL;
					} else {
						target = target->next;
					}
				}
				
			}
		} else {
			/** special case where head is the start node , head->prev = NULL **/
			assert(tail && tail->next);

			target = tail->next;
			while(target && target->next) {
				d  =  tail->d_next + target->d_next;
				nd =  weighted_node_distance(head,target,usage) + weighted_node_distance(tail,target->next,usage);
		
				if ( nd < d ) {
					// insert the block (head,tail) between target & target->next
					// first cut out cur
					tail->next->prev = NULL;	// sets a new start_node
					start_node = *start_node_ptr = tail->next;
					// put it in:
					tail->next = target->next;
					head->prev = target;
					head->prev->next = head;
					tail->next->prev = tail;
					set_d_next(tail,usage);
					set_d_next(head->prev,usage);
					didFiddle = JE_TRUE;
					target = NULL;
				} else {
					target = target->next;
				}
			}
		}
	}

return didFiddle;
}

int nextBestD(palNode *cur,palNode **next,palNode *nodes,int size,int *usage)
{
int j,d,best_d;
palNode *vs;

	best_d = INT_MAX - 10;
	for(j=0;j<size;j++) {
		vs = nodes+j;
		if ( !(vs->visited) && !(vs->next) ) {
			d = weighted_node_distance(cur,vs,usage);
			if ( d < best_d ) {
				best_d = d;
				if ( next ) *next = vs;
			}
		}
	}
return best_d;
}

void greedySort(palNode *start_node,palNode *nodes,int size,int *usage)
{
palNode *cur;

	cur = start_node;
	while(cur) {
		cur->visited = JE_TRUE;

		cur->d_next = nextBestD(cur,&(cur->next),nodes,size,usage);

		if ( cur->next ) cur->next->prev = cur;
		cur = cur->next;
	}
}

void lazySort(palNode *start_node,palNode *nodes,int size,int *usage)
{
int j,best_d,d;
palNode *cur,*vs,*vs2;

	cur = start_node;
	while(cur) {
		cur->visited = JE_TRUE;
		best_d = INT_MAX;
		for(j=0;j<size;j++) { vs = nodes+j;
			if ( !(vs->visited) && !(vs->next) ) {
				d = weighted_node_distance(cur,vs,usage);
				
				vs->visited = JE_TRUE;
				d += nextBestD(vs,&vs2,nodes,size,usage);
				if ( vs2 ) {
					vs2->visited = JE_TRUE;
					d += nextBestD(vs2,NULL,nodes,size,usage);
					vs2->visited = JE_FALSE;
				}
				vs->visited = JE_FALSE;

				if ( d < best_d ) {
					best_d = d;
					cur->next = vs;
				}
			}
		}
		if ( cur->next ) {
			set_d_next(cur,usage);
			cur->next->prev = cur;
		}
		cur = cur->next;
	}
}

palNode * chooseStartPal(palNode *nodes,int size,int *usage)
{
int i,j,best_d,d;
palNode *start_node;

	/** find the two nodes with largest distance, and use one as the starter 
	*
	*	this is not right; it's probably better in general to find the
	*		centroid and start there
	*	what we want is to start with the one which will have the largest
	*		distance >after< optimal walking; that is, optimally walk a
	*		closed loop, then erase the single longest segment, gives you
	*		the optimal open loop.
	*
	*	"most_isolated" seems dandy
	*
	**/

	start_node = NULL;

#ifdef START_W_LARGEST_D
	best_d = 0;
	for(i=0;i<size;i++) {
		for(j=i+1;j<size;j++) {
			d = node_distance(nodes+i,nodes+j);
			if ( d > best_d ) {	
				best_d = d;
				start_node = nodes+i;
			}
		}
	}
#else 
#ifdef START_W_MOST_ISOLATED
	best_d = 0;
	for(i=0;i<size;i++) {
		d = 0;
		for(j=i+1;j<size;j++) {
			d += weighted_node_distance(nodes+i,nodes+j,usage);
		}
		if ( d > best_d ) {	
			best_d = d;
			start_node = nodes+i;
		}
	}
#else // start w/ centroid
	best_d = INT_MAX;
	for(i=0;i<size;i++) { cur = nodes+i;
		d = 0;
		for(j=0;j<size;j++) {
			d += weighted_node_distance(cur,nodes+j,usage);
		}
		if ( d < best_d ) {	
			best_d = d;
			start_node = cur;
		}
	}
#endif
#endif

	assert(start_node);	

return start_node;
}

palNode * initPal(uint8 * rgb_colors,int size)
{
int i;
palNode *nodes;

	if ( (nodes = (palNode *)newarray(palNode,size)) == NULL ) {
		BrandoError("node malloc failed");
		return NULL;
	}

	for(i=0;i<size;i++) {
		nodes[i].color.r = rgb_colors[i*3 + 0];
		nodes[i].color.g = rgb_colors[i*3 + 1];
		nodes[i].color.b = rgb_colors[i*3 + 2];
		nodes[i].index = i;
		RGB_transform( (nodes[i].color), 
			&(nodes[i].c1) , &(nodes[i].c2) , &(nodes[i].c3) );
	}

return nodes;
}

void restoreRGB(palNode *n)
{
	undo_RGB_transform( (n->c1) , (n->c2) , (n->c3)  , (n->color));
}

jeBoolean readOutPal(palNode *start_node,int size,uint8 *rgb_colors,int *permutation)
{
int i;
palNode *cur;
	i = 0;
	cur = start_node;
	while(cur && i < size) {
		permutation[ cur->index] = i;
		rgb_colors[i*3 + 0] = cur->color.r;
		rgb_colors[i*3 + 1] = cur->color.g;
		rgb_colors[i*3 + 2] = cur->color.b;
		i++;
		cur = cur->next;
	}
	if ( i != size )
		return JE_FALSE;
return JE_TRUE;
}

void reportTotLen(palNode *start_node,int *usage)
{
int d;
palNode *cur;

	d = 0;
	cur = start_node;
	while(cur && cur->next) {
		d += cur->d_next; 
		cur = cur->next;
	}
	printf("total color walk length = %d\n",d);

}

int node_distance_sqr(palNode *x,palNode *y)
{
int a,b,c;

	a = (x->c1 - y->c1);
	b = (x->c2 - y->c2);
	c = (x->c3 - y->c3);

#ifdef DO_HSV	// cyclic !
	a = abs(a);
	if ( a > 128 ) a = 256 - a;
#endif

return (a*a + b*b + c*c);
}

#ifndef weighted_node_distance
int weighted_node_distance(palNode *x,palNode *y,int *usage)
{
#if 0

double d;

	d = (double)node_distance(x,y) * (double)(usage[ x->index ] + usage[ y->index ]);
	d *= 0.0025;

return (int)(d + 0.5);

#else

return (node_distance(x,y) >> 3) * ((usage[ x->index ] + usage[ y->index ] + 96)>>6);

#endif
}
#endif


