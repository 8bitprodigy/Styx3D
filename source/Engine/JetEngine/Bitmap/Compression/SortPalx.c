/****************************************************************************************/
/*  SORTPALX.C                                                                          */
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

#include "Utility.h"
#include "colorconv.h"

#include <math.h>
#include <stdio.h>
#include <assert.h>
#include "SortPal.h"

#define EQUAL_DISTANCE 3

jeBoolean reducePal(int * ncolors_ptr,uint8 *palette,int *permutation,int *usage)
{
int i,ncolors,d;
palNode *cur,*vs,*start,*nodes;
jeBoolean didStuff;

	ncolors = *ncolors_ptr;

	if ( (nodes = initPal(palette,ncolors)) == NULL )
		return JE_FALSE;

	for(i=0;i<ncolors;i++) {
		nodes[i].index = i;
		nodes[i].prev = nodes+i-1;
		nodes[i].next = nodes+i+1;
	}
	start = nodes; start->prev = NULL;
	nodes[ncolors-1].next = NULL;

	while(start && usage[start->index] == 0) {
		start = start->next;
		ncolors--;
		start->prev = NULL;
	}

	for(cur=start->next;cur;cur = cur->next) {
		if ( usage[cur->index] == 0 ) {
			/** cut it out **/
			cut_pal_node(cur);
			ncolors --;
		}
	}

	/***
	*
	*	this cutter can be imperfect because we cut all colors
	*	surrounding a given first one; optimally we would choose
	*	the centroid of a glob as the one to be not cut
	*
	**/

	do {
		didStuff = JE_FALSE;
		i=0;
		for(cur=start;cur;cur = cur->next) {
			for(vs=cur->next;vs;vs = vs->next) {
				d = node_distance(cur,vs);
				if ( d < EQUAL_DISTANCE ) {
					// identical colors, kill 'vs'
					
					cut_pal_node(vs);
					ncolors --;
					permutation[ vs->index ] = i;
						// set to the *new* index of cur, != cur->index
				}
			}
			i++;
		}
	} while( didStuff);

	if ( ! readOutPal(start,ncolors,palette,permutation) ) {
		BrandoError("bad linked list");
		destroy(nodes);
		return JE_FALSE;
	}

	*ncolors_ptr = ncolors;

	destroy(nodes);

return JE_TRUE;
}

jeBoolean usePal(int *ncolors_ptr,int new_ncolors,uint8 * palette,int *permutation,int * usage)
{
int i,ncolors;
palNode *cur,*found,*start,*nodes,*freenodes,*foundnext;

	ncolors = *ncolors_ptr;

	if ( (nodes = initPal(palette,new_ncolors)) == NULL )
		return JE_FALSE;
	freenodes = nodes + ncolors;

	for(i=0;i<ncolors;i++) {
		cur = nodes+i;
		cur->index = i;
		cur->prev = nodes+i-1;
		cur->next = nodes+i+1;
		if ( i+1 < ncolors ) set_d_next(cur,usage);
	}
	start = nodes; start->prev = NULL;
	nodes[ncolors-1].next = NULL;

	while(ncolors < new_ncolors ) {

		/** find the largest hole and add a new node there 
		*	largest hole is measured >weighted by usage<
		**/

		found = start;
		for(cur=start;cur->next;cur = cur->next) {
			if ( cur->d_next > found->d_next ) {
				found = cur;
			}
		}

		foundnext = found->next;
		assert(foundnext);
		cur = freenodes++;
		add_after(cur,found);
		cur->c1 = (found->c1 + foundnext->c1)/2;
		cur->c2 = (found->c2 + foundnext->c2)/2;
		cur->c3 = (found->c3 + foundnext->c3)/2;
		cur->index = ncolors++;
		set_d_next(found,usage);
		set_d_next(cur,usage);
		restoreRGB(cur);
	}

	if ( ! readOutPal(start,ncolors,palette,permutation) ) {
		BrandoError("bad linked list");
		destroy(nodes);
		return JE_FALSE;
	}

	*ncolors_ptr = ncolors;

	destroy(nodes);

return JE_TRUE;
}

