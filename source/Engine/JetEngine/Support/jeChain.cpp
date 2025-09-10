/****************************************************************************************/
/*  JECHAIN.C                                                                           */
/*                                                                                      */
/*  Author:  John Pollard                                                               */
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
#include <assert.h>
#include <memory.h>

#include "jeChain.h"
#include "Ram.h"

//========================================================================================
//========================================================================================
#define ZeroMem(a) memset(a, 0, sizeof(*a))
#define ZeroMemArray(a, s) memset(a, 0, sizeof(*a)*s)

#define MAKEFOURCC(ch0, ch1, ch2, ch3) ((uint32)(uint8)(ch0) | ((uint32)(uint8)(ch1) << 8) |  ((uint32)(uint8)(ch2) << 16) | ((uint32)(uint8)(ch3) << 24 ))

#define JE_CHAIN_TAG			MAKEFOURCC('G', 'E', 'C', 'F')		// 'GE' 'C'hain 'F'ile
#define JE_CHAIN_VERSION		0x0000

// The jeChain keeps a list of jeChain_Links, current number of links, etc...
typedef struct jeChain
{
	int32					RefCount;

	uint32					NumLinks;
	jeChain_Link			*Links;

	const void				*LastLinkData;
	jeChain_Link			*LastLink;

	#ifdef _DEBUG
		struct jeChain		*Self;
	#endif
} jeChain;

// The jeChain_Link is used to keep linked list of items in the jeChain object
typedef struct jeChain_Link
{
	// I wanted to make this a const, but sonce we need to return it,
	//	I didn't want to cause any confusion by casting it to a non-const... sigh...
	void					*LinkData;				// LinkData is the user data the caller store on links (It CANNOT be NULL!)

	struct jeChain_Link		*Next;
	struct jeChain_Link		*Prev;

	#ifdef _DEBUG
		struct jeChain_Link	*Self;
	#endif
} jeChain_Link;

//========================================================================================
//	jeChain_Create
//========================================================================================
jeChain *jeChain_Create(void)
{
	jeChain		*Chain;

	Chain = (jeChain *)JE_RAM_ALLOCATE_STRUCT(jeChain);

	if (!Chain)
		return NULL;

	ZeroMem(Chain);

	Chain->RefCount = 1;

#ifdef _DEBUG
	Chain->Self = Chain;
#endif

	return Chain;
}

//========================================================================================
//	jeChain_CreateRef
//========================================================================================
jeBoolean jeChain_CreateRef(jeChain *Chain)
{
	assert(jeChain_IsValid(Chain) == JE_TRUE);

	Chain->RefCount++;

	return JE_TRUE;
}

//========================================================================================
//	jeChain_CreateFromFile
//========================================================================================
jeChain *jeChain_CreateFromFile(jeVFile *VFile, jeChain_ReadIOFunc *IOFunc, void *Context, jePtrMgr *PtrMgr)
{
	jeChain		*Chain = NULL;
	uint32		NumLinks, i;
	uint32		Tag;
	uint16		Version;

	if (PtrMgr)
	{
		if (!jePtrMgr_ReadPtr(PtrMgr, VFile, (void **)&Chain))
			return NULL;

		if (Chain)
		{				
			if (!jeChain_CreateRef(Chain))
				return NULL;

			return Chain;
		}
	}

	if (!jeVFile_Read(VFile, &Tag, sizeof(Tag)))
		return NULL;

	if (Tag != JE_CHAIN_TAG)
		return NULL;

	if (!jeVFile_Read(VFile, &Version, sizeof(Version)))
		return NULL;

	if (Version != JE_CHAIN_VERSION)
		return NULL;

	Chain = JE_RAM_ALLOCATE_STRUCT(jeChain);

	if (!Chain)
		return NULL;

	ZeroMem(Chain);
	Chain->RefCount = 1;

#ifdef _DEBUG
	Chain->Self = Chain;
#endif

	// Load the links
	if (!jeVFile_Read(VFile, &NumLinks, sizeof(NumLinks)))
		goto ExitWithError;

 	for (i=0; i< NumLinks; i++)
	{
		jeChain_Link		*Link;
		void				*LinkData;

		if (!IOFunc(VFile, &LinkData, Context, PtrMgr))
			goto ExitWithError;

		Link = jeChain_LinkCreate(LinkData);

		if (!Link)
			goto ExitWithError;

		if (!jeChain_AddLink(Chain, Link))
		{
			jeChain_LinkDestroy(&Link);
			goto ExitWithError;
		}
	}

	if (PtrMgr)
	{
		// Push the ptr on the stack
		if (!jePtrMgr_PushPtr(PtrMgr, Chain))
			goto ExitWithError;
	}

	return Chain;

	ExitWithError:
	{
		if (Chain)
			jeChain_Destroy(&Chain);

		return NULL;
	}
}


//========================================================================================
//	jeChain_WriteToFile
//========================================================================================
jeBoolean jeChain_WriteToFile(const jeChain *Chain, jeVFile *VFile, jeChain_IOFunc *IOFunc, void *Context, jePtrMgr *PtrMgr)
{
	uint32			Tag;
	uint16			Version;
	jeChain_Link	*Link;

	assert(jeChain_IsValid(Chain) == JE_TRUE);

	if (PtrMgr)	
	{
		uint32		Count;
			
		if (!jePtrMgr_WritePtr(PtrMgr, VFile, (void*)Chain, &Count))
			return JE_FALSE;

		if (Count)		// Ptr was on stack, so return 
			return JE_TRUE;
	}

	Tag = JE_CHAIN_TAG;
	Version = JE_CHAIN_VERSION;
	
	if (!jeVFile_Write(VFile, &Tag, sizeof(Tag)))
		return JE_FALSE;

	if (!jeVFile_Write(VFile, &Version, sizeof(Version)))
		return JE_FALSE;

	if (!jeVFile_Write(VFile, &Chain->NumLinks, sizeof(Chain->NumLinks)))
		return JE_FALSE;

	for (Link = Chain->Links; Link; Link = Link->Next)
	{
		if (!IOFunc(VFile, &Link->LinkData, Context, PtrMgr))
			return JE_FALSE;
	}

	if (PtrMgr)
	{
		// Push the ptr on the stack
		if (!jePtrMgr_PushPtr(PtrMgr, (void*)Chain))
			return JE_FALSE;
	}

	return JE_TRUE;
}

//========================================================================================
//	jeChain_Destroy
//========================================================================================
void jeChain_Destroy(jeChain **Chain)
{
	jeChain_Link		*Link, *Next;

	assert(Chain);
	assert(jeChain_IsValid(*Chain) == JE_TRUE);

	(*Chain)->RefCount --;

	if ((*Chain)->RefCount == 0)
	{
		// Free all the links
		for (Link = (*Chain)->Links; Link; Link = Next)
		{
			Next = Link->Next;

			assert(jeChain_LinkIsValid(Link) == JE_TRUE);

			if (Link->Next)
				Link->Next->Prev = Link->Prev;

			if (Link == (*Chain)->Links)
			{
				assert(Link->Prev == NULL);
				(*Chain)->Links = Link->Next;
			}
			else
			{
				assert(Link->Prev != NULL);
				Link->Prev->Next = Link->Next;
			}

			jeChain_LinkDestroy(&Link);
		}

		jeRam_Free(*Chain);
	}

	*Chain = NULL;
}

//========================================================================================
//	jeChain_IsValid
//========================================================================================
jeBoolean jeChain_IsValid(const jeChain *Chain)
{
	uint32			NumLinks;
	jeChain_Link	*Link, *LastLink;

	if (!Chain)
		return JE_FALSE;

#ifdef _DEBUG
	if (Chain->Self != Chain)
		return JE_FALSE;
#endif
	if (Chain->RefCount <= 0)
		return JE_FALSE;

	NumLinks = 0;

	LastLink = NULL;
	for (Link = Chain->Links; Link; Link = Link->Next)
	{
		if (!jeChain_LinkIsValid(Link))
			return JE_FALSE;
	
		assert(Link->Prev == LastLink);
		LastLink = Link;

		NumLinks++;
	}

	if (Chain->NumLinks != NumLinks)
		return JE_FALSE;

	return JE_TRUE;
}

//========================================================================================
//	jeChain_FindLink
//========================================================================================
jeChain_Link *jeChain_FindLink(const jeChain *Chain, void *LinkData)
{
	jeChain_Link		*Link;

	assert(jeChain_IsValid(Chain) == JE_TRUE);

	for (Link = Chain->Links; Link; Link = Link->Next)
	{
		if (Link->LinkData == LinkData)
			return Link;
	}

	return NULL;
}

//========================================================================================
//	jeChain_AddLink
//	Create a new link at the end of the current chain
//========================================================================================
jeBoolean jeChain_AddLink(jeChain *Chain, jeChain_Link *Link)
{
	jeChain_Link		*Tail;

	assert(jeChain_IsValid(Chain) == JE_TRUE);
	assert(jeChain_LinkIsValid(Link) == JE_TRUE);

	assert(Link->Next == NULL);			// This is a booboo if they have already added this link to another jeChain
	assert(Link->Prev == NULL);

	assert(Chain->NumLinks < 0xffffffff);
	Chain->NumLinks++;

	if (!Chain->Links)		// This case is easy
	{
		Chain->Links = Link;
		return JE_TRUE;
	}

	// Add to tail of chain
	for (Tail = Chain->Links; Tail->Next; Tail = Tail->Next);

	Tail->Next = Link;
	Link->Prev = Tail;

	return JE_TRUE;
}

//========================================================================================
//	jeChain_InsertLinkAfter
//	Create a new link and inserts after InsertAfter
//========================================================================================
jeBoolean jeChain_InsertLinkAfter(jeChain *Chain, jeChain_Link *InsertAfter, jeChain_Link *Link)
{
	jeChain_Link		*Current;

	assert(jeChain_IsValid(Chain) == JE_TRUE);
	assert(jeChain_LinkIsValid(Link) == JE_TRUE);

	assert(Link->Next == NULL);			// This is a booboo if they have already added this link to another jeChain
	assert(Link->Prev == NULL);
	assert(Chain->NumLinks < 0xffffffff);
	assert(Chain->Links);

	Chain->NumLinks++;

	// Find InsertAfter, and insert new link there
	for (Current = Chain->Links; Current; Current = Current->Next)
	{
		if (Current == InsertAfter)
			break;
	}

	assert(Current);

	if (!Current)
		return JE_FALSE;

	if (Current->Next)
		Current->Next->Prev = Link;

	Link->Next = Current->Next;
	Current->Next = Link;
	Link->Prev = Current;

	return JE_TRUE;
}

//========================================================================================
//	jeChain_InsertLinkBefore
//	Create a new link and inserts before InsertBefore
//========================================================================================
jeBoolean jeChain_InsertLinkBefore(jeChain *Chain, jeChain_Link *InsertBefore, jeChain_Link *Link)
{
	jeChain_Link		*Current;

	assert(jeChain_IsValid(Chain) == JE_TRUE);
	assert(jeChain_LinkIsValid(Link) == JE_TRUE);

	assert(Link->Next == NULL);			// This is a booboo if they have already added this link to another jeChain
	assert(Link->Prev == NULL);
	assert(Chain->NumLinks < 0xffffffff);
	assert(Chain->Links);

	Chain->NumLinks++;

	// Find InsertBefore, and insert new link there
	for (Current = Chain->Links; Current; Current = Current->Next)
	{
		if (Current == InsertBefore)
			break;
	}

	assert(Current);

	if (!Current)
		return JE_FALSE;

	if ( Current->Prev != NULL )
	{
		Current->Prev->Next = Link;
	}
	else
	{
		Chain->Links = Link;
	}

	Link->Next = Current;
	Link->Prev = Current->Prev;
	Current->Prev = Link;

	return JE_TRUE;
}

//========================================================================================
//	jeChain_AddLinkData
//	Create a new link at the end of the current chain, and sets Link->LinkData to LinkData
//========================================================================================
jeBoolean jeChain_AddLinkData(jeChain *Chain, void *LinkData)
{
	jeChain_Link		*Link;

	assert(jeChain_IsValid(Chain) == JE_TRUE);
	assert(LinkData);

	Link = jeChain_LinkCreate(LinkData);

	if (!Link)
		return JE_FALSE;

	return jeChain_AddLink(Chain, Link);
}

//========================================================================================
//	jeChain_InsertLinkData
//	Create a new link and inserts after InsertAfter, and sets Link->LinkData to LinkData
//========================================================================================
jeBoolean jeChain_InsertLinkData(jeChain *Chain, jeChain_Link *InsertAfter, void *LinkData)
{
	jeChain_Link		*Link;

	assert(jeChain_IsValid(Chain) == JE_TRUE);
	assert(LinkData);

	Link = jeChain_LinkCreate(LinkData);

	if (!Link)
		return JE_FALSE;

	return jeChain_InsertLinkBefore(Chain, InsertAfter, Link);
}

//========================================================================================
//	jeChain_RemoveLink
//========================================================================================
jeBoolean jeChain_RemoveLink(jeChain *Chain, jeChain_Link *Link)
{
	assert(jeChain_IsValid(Chain) == JE_TRUE);
	assert(jeChain_LinkIsValid(Link) == JE_TRUE);
	assert(Chain->NumLinks > 0);

	if (Link->Next)
		Link->Next->Prev = Link->Prev;

	if (Link == Chain->Links)
	{
		assert(Link->Prev == NULL);
		Chain->Links = Link->Next;
	}
	else
	{
		assert(Link->Prev != NULL);
		Link->Prev->Next = Link->Next;
	}

	// Assert code expects Next/Prev fields to be NULL when you add a link, so lets make it happy
	Link->Next = NULL;
	Link->Prev = NULL;

	Chain->NumLinks--;

	return JE_TRUE;
}

//========================================================================================
//	jeChain_RemoveLinkData
//========================================================================================
jeBoolean jeChain_RemoveLinkData(jeChain *Chain, void *LinkData)
{
	jeChain_Link		*Link;

	assert(jeChain_IsValid(Chain) == JE_TRUE);
	assert(LinkData);

	// Find the link
	Link = jeChain_FindLink(Chain, LinkData);

	if (!Link)
		return JE_FALSE;

	// Icestorm Begin
	// NOTE: Be sure, LastLink(Data) will be valid or NULL.
	if (Chain->LastLink==Link)
	{
		Chain->LastLink=Link->Next;
		if (Link->Next!=NULL) 
			Chain->LastLinkData=Link->Next->LinkData;
		else
			Chain->LastLinkData=NULL;
	}
	// Icestorm End

	// Remove it
	if (!jeChain_RemoveLink(Chain, Link))
		return JE_FALSE;

	// Destroy it
	jeChain_LinkDestroy(&Link);

	return JE_TRUE;
}

//========================================================================================
//	jeChain_GetLinkCount
//========================================================================================
uint32 jeChain_GetLinkCount(const jeChain *Chain)
{
	assert(jeChain_IsValid(Chain) == JE_TRUE);

	return Chain->NumLinks;
}

//========================================================================================
//	jeChain_GetFirstLink
//	This function is useful, when they want to iterate through the links themselves.
//========================================================================================
jeChain_Link *jeChain_GetFirstLink(const jeChain *Chain)
{
	assert(Chain);

	return Chain->Links;
}

//========================================================================================
//	jeChain_GetLinkByIndex
//========================================================================================
jeChain_Link *jeChain_GetLinkByIndex(const jeChain *Chain, uint32 Index)
{
	jeChain_Link		*Link;

	assert(jeChain_IsValid(Chain) == JE_TRUE);
	assert(jeChain_GetLinkCount(Chain) >= Index);

	for (Link = Chain->Links; Link; Link = Link->Next, Index--)
	{
		if (Index == 0)
			return Link;
	}

	assert(0);		// Invalid index if we get here
	return NULL;
}

//========================================================================================
//	jeChain_GetLinkDataByIndex
//========================================================================================
void *jeChain_GetLinkDataByIndex(const jeChain *Chain, uint32 Index)
{
	jeChain_Link		*Link;

	assert(jeChain_IsValid(Chain) == JE_TRUE);
	assert(jeChain_GetLinkCount(Chain) >= Index);

	for (Link = Chain->Links; Link; Link = Link->Next, Index--)
	{
		if (Index == 0)
			return Link->LinkData;
	}

	assert(0);		// Invalid index if we get here
	return NULL;
}

//========================================================================================
//	jeChain_GetNextLinkData
//	NULL returns the first in t e list.
//	Caches out the last item, so linear searches are faster
//========================================================================================
void *jeChain_GetNextLinkData(jeChain *Chain, void *Start)
{
	jeChain_Link	*Link;
	void			*LinkData;

	assert(Chain);

	if (!Start)								// This case is really easy
	{
		Link = Chain->Links;
	}
	else if (Chain->LastLinkData == Start)	// This case is easy
	{
		Link = Chain->LastLink;

		if (Link)
			Link = Link->Next;				// Get next link (NOTE that this next link CAN be NULL)
		else
			Link = Chain->Links;			// If link is NULL, wrap to first
	}
	else for (Link = Chain->Links; Link; Link = Link->Next)		// We will have to search now...
	{
		if (Link->LinkData == Start)
			break;
	}

	if (Link)
		LinkData = Link->LinkData;
	else
		LinkData = NULL;

	// Remember the last brush/link returned...
	Chain->LastLinkData = LinkData;
	Chain->LastLink = Link;

	return LinkData;
}

//========================================================================================
//	jeChain_LinkCreate
//========================================================================================
jeChain_Link *jeChain_LinkCreate(void *LinkData)
{
	jeChain_Link	*Link;

	assert(LinkData);

	Link = JE_RAM_ALLOCATE_STRUCT(jeChain_Link);

	if (!Link)
		return NULL;

	ZeroMem(Link);

#ifdef _DEBUG
	Link->Self = Link;
#endif

	Link->LinkData = LinkData;

	return Link;
}

//========================================================================================
//	jeChain_LinkDestroy
//========================================================================================
void jeChain_LinkDestroy(jeChain_Link **Link)
{
	assert(Link);
	assert(jeChain_LinkIsValid(*Link) == JE_TRUE);

	jeRam_Free(*Link);

	*Link = NULL;
}

//========================================================================================
//	jeChain_LinkIsValid
//========================================================================================
jeBoolean jeChain_LinkIsValid(const jeChain_Link *Link)
{
	if (!Link)
		return JE_FALSE;

#ifdef _DEBUG
	if (Link->Self != Link)
		return JE_FALSE;
#endif

	if (!Link->LinkData)
		return JE_FALSE;

	return JE_TRUE;
}

//========================================================================================
//	jeChain_LinkGetLinkData
//========================================================================================
void *jeChain_LinkGetLinkData(const jeChain_Link *Link)
{
	return Link->LinkData;
}

//========================================================================================
//	jeChain_LinkGetNext
//========================================================================================
jeChain_Link *jeChain_LinkGetNext(const jeChain_Link *Link)
{
	assert(Link);

	return Link->Next;
}

//========================================================================================
//	jeChain_LinkGetPrev
//========================================================================================
jeChain_Link *jeChain_LinkGetPrev(const jeChain_Link *Link)
{
	assert(Link);

	return Link->Prev;
}

//========================================================================================
//	jeChain_LinkDataGetIndex
//========================================================================================
uint32 jeChain_LinkDataGetIndex(const jeChain *Chain, void *LinkData)
{
	jeChain_Link		*Link;
	uint32				Index;

	assert(jeChain_IsValid(Chain) == JE_TRUE);

	for (Index = 0, Link = Chain->Links; Link; Link = Link->Next, Index++)
	{
		if (Link->LinkData == LinkData)
			return Index;
	}

	assert(0);		// Invalid linkdata if we get here
	return 0;
}

