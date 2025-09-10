/****************************************************************************************/
/*  STRBLOCK.C																			*/
/*                                                                                      */
/*  Author: Mike Sandige	                                                            */
/*  Description: String block implementation.											*/
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

//   a list of strings implemented as a single block of memory for fast
//   loading.  The 'Data' Field is interpreted as an array of integer 
//   offsets relative to the beginning of the data field.  After the int list
//   is the packed string data.  Since no additional allocations are needed 
//   this object can be file loaded as one block. 


#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "StrBlock.h"
#include "Ram.h"
#include "Errorlog.h"

#define STRBLOCK_MAX_STRINGLEN 255


typedef struct jeStrBlock
{
	int Count;
	jeStrBlock *SanityCheck;
	union 
		{
			int IntArray[1];		// char offset into CharArray for string[n]
			char CharArray[1];
		} Data;
		
} jeStrBlock;


JETAPI int JETCC jeStrBlock_GetChecksum(const jeStrBlock *SB)
{
	int Count;
	int Len;
	int i,j;
	const char *Str;
	int Checksum=0;
	assert( SB != NULL );

	Count = jeStrBlock_GetCount(SB);
	for (i=0; i<Count; i++)
		{
			Str = jeStrBlock_GetString(SB,i);
			assert(Str!=NULL);
			Len = strlen(Str);
			for (j=0; j<Len; j++)
				 {
					Checksum += (int)Str[j];
				}
			Checksum = Checksum*3;
		}
	return Checksum;
}

JETAPI jeStrBlock *JETCC jeStrBlock_Create(void)
{
	jeStrBlock *SB;
	
	SB = JE_RAM_ALLOCATE_STRUCT_CLEAR(jeStrBlock);

	if ( SB == NULL )
		{
			jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE, "jeStrBlock_Create.");
			return NULL;
		}
	SB->Count=0;
	SB->SanityCheck = SB;
	return SB;
}


JETAPI void JETCC jeStrBlock_Destroy(jeStrBlock **SB)
{
	assert( (*SB)->SanityCheck == (*SB) );
	assert(  SB != NULL );
	assert( *SB != NULL );	
	jeRam_Free( *SB );
	*SB = NULL;
}


static int JETCC jeStrBlock_BlockSize(const jeStrBlock *B)
{
	int Offset;
	const char *LastStr;
	assert( B != NULL );
	assert( B->SanityCheck == B );

	if ( B->Count == 0 )
		return 0;
	Offset = B->Data.IntArray[B->Count-1];
	LastStr = &(B->Data.CharArray[Offset]);

	return strlen(LastStr) + 1 + Offset;
}


JETAPI void JETCC jeStrBlock_Delete(jeStrBlock **ppSB,int Nth)
{
	int BlockSize;
	int StringLen;
	int CloseSize;
	const char *String;
	assert(  ppSB  != NULL );
	assert( *ppSB  != NULL );
	assert( Nth >=0 );
	assert( Nth < (*ppSB)->Count );
	assert( (*ppSB)->SanityCheck == (*ppSB) );

	String = jeStrBlock_GetString(*ppSB,Nth);
	assert( String != NULL );
	StringLen = strlen(String) + 1;
		
	BlockSize = jeStrBlock_BlockSize(*ppSB);

	{
		jeStrBlock *B = *ppSB;
		char *ToBeReplaced;
		char *Replacement=NULL;
		int i;
		ToBeReplaced = &((*ppSB)->Data.CharArray[(*ppSB)->Data.IntArray[Nth]]);
		if (Nth< (*ppSB)->Count-1)
			Replacement  = &((*ppSB)->Data.CharArray[(*ppSB)->Data.IntArray[Nth+1]]);
		for (i=Nth+1,CloseSize = 0; i<(*ppSB)->Count ; i++)
			{
				CloseSize += strlen(&((*ppSB)->Data.CharArray[(*ppSB)->Data.IntArray[i]])) +1;
				B->Data.IntArray[i] -= StringLen;
			}
		for (i=0; i<(*ppSB)->Count ; i++)
			{
				B->Data.IntArray[i] -= sizeof(int);
			}
		// crunch out Nth string
		if (Nth< (*ppSB)->Count-1)
			memmove(ToBeReplaced,Replacement,CloseSize);
		// crunch out Nth index
		memmove(&(B->Data.IntArray[Nth]),
				&(B->Data.IntArray[Nth+1]),
				BlockSize - ( sizeof(int) *  (Nth+1) ) );

	}
	
	{
		jeStrBlock * NewjeStrBlock;

		NewjeStrBlock = (jeStrBlock *)jeRam_Realloc( *ppSB, 
			BlockSize				// size of data block
			+ sizeof(jeStrBlock)		// size of strblock structure
			- StringLen				// size of dying string
			- sizeof(int) );		// size of new index to string
		if ( NewjeStrBlock != NULL )
			{
				*ppSB = NewjeStrBlock;
				(*ppSB)->SanityCheck = NewjeStrBlock;
			}
	}

	(*ppSB)->Count--;
}



JETAPI jeBoolean JETCC jeStrBlock_FindString(const jeStrBlock* pSB, const char* String, int* pIndex)
{
	int i;
	int Count;
	const char *Str;

	assert(pSB != NULL);
	assert(String != NULL);
	assert(pIndex != NULL);
	assert( pSB->SanityCheck == pSB );

	Count = jeStrBlock_GetCount(pSB);
	for (i=0; i<Count; i++)
	{
		Str = jeStrBlock_GetString(pSB,i);
		if(strcmp(String, Str) == 0)
		{
			*pIndex = i;
			return JE_TRUE;
		}
	}
	return JE_FALSE;
}


JETAPI jeBoolean JETCC jeStrBlock_Append(jeStrBlock **ppSB,const char *String)
{
	int BlockSize;
	assert(  ppSB  != NULL );
	assert( *ppSB  != NULL );
	assert( String != NULL );
	assert( (*ppSB)->SanityCheck == (*ppSB) );

	if (strlen(String)>=STRBLOCK_MAX_STRINGLEN)
		{
			jeErrorLog_Add(JE_ERR_BAD_PARAMETER, "jeStrBlock_Append: string too long.");
			return JE_FALSE;
		}

	BlockSize = jeStrBlock_BlockSize(*ppSB);

	{
		jeStrBlock * NewjeStrBlock;

		NewjeStrBlock = (jeStrBlock*)jeRam_Realloc( *ppSB, 
			BlockSize				// size of data block
			+ sizeof(jeStrBlock)		// size of strblock structure
			+ strlen(String) + 1		// size of new string
			+ sizeof(int) );		// size of new index to string
		if ( NewjeStrBlock == NULL )
			{
				jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE, "jeStrBlock_Append: failed to allocate space for new string.");
				return JE_FALSE;
			}
		*ppSB = NewjeStrBlock;
		(*ppSB)->SanityCheck = NewjeStrBlock;
	}

	{
		jeStrBlock *B = *ppSB;
		int i;
		for (i=0; i<B->Count; i++)
			{
				B->Data.IntArray[i] += sizeof(int);
			}
		if (B->Count > 0)
			{
				memmove(&(B->Data.IntArray[B->Count+1]),
						&(B->Data.IntArray[B->Count]),
						BlockSize - sizeof(int) * B->Count);
			}
		B->Data.IntArray[B->Count] = BlockSize + sizeof(int);
		strcpy(&(B->Data.CharArray[B->Data.IntArray[B->Count]]),String);
	}
	(*ppSB)->Count++;
	return JE_TRUE;
}

JETAPI const char *JETCC jeStrBlock_GetString(const jeStrBlock *SB, int Index)
{
	assert( SB != NULL );
	assert( Index >= 0 );
	assert( Index < SB->Count );
	assert( SB->SanityCheck == SB );
	return &(SB->Data.CharArray[SB->Data.IntArray[Index]]);
}

JETAPI int JETCC jeStrBlock_GetCount(const jeStrBlock *SB)
{
	assert( SB != NULL);
	assert( SB->SanityCheck == SB );
	return SB->Count;
}


#define STRBLOCK_BIN_FILE_TYPE 0x424B4253	// 'SBKB'


typedef struct
{
	int Count;
	uint32 Size;
} jeStrBlock_FileHeader;

JETAPI jeStrBlock* JETCC jeStrBlock_CreateFromFile(jeVFile* pFile)
{
	int32 u;
	jeStrBlock *SB;
	jeStrBlock_FileHeader Header;

	assert( pFile != NULL );

	if(jeVFile_Read(pFile, &u, sizeof(u)) == JE_FALSE)
	{
		jeErrorLog_Add( JE_ERR_FILEIO_READ , "jeStrBlock_CreateFromFile: Failed to read header.");
		return NULL;
	}

	if (u!=STRBLOCK_BIN_FILE_TYPE)
		{
			jeErrorLog_Add( JE_ERR_FILEIO_FORMAT , "jeStrBlock_CreateFromFile: Bad or wrong header.");
			return NULL;
		}

	if (jeVFile_Read(pFile, &Header,sizeof(jeStrBlock_FileHeader)) == JE_FALSE)
		{
			jeErrorLog_Add( JE_ERR_FILEIO_READ , "jeStrBlock_CreateFromFile: Failed to read header block.");
			return NULL;
		}
	
	SB = (jeStrBlock *)jeRam_AllocateClear( sizeof(jeStrBlock) + Header.Size );
	if( SB == NULL )
		{
			jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE, "jeStrBlock_CreateFromFile.");
			return NULL;	
		}
	SB->SanityCheck = SB;
	SB->Count = Header.Count; 

	if (jeVFile_Read(pFile, &(SB->Data),Header.Size) == JE_FALSE)
		{
			jeErrorLog_Add( JE_ERR_FILEIO_READ , "jeStrBlock_CreateFromFile.");
			return NULL;
		}
	return SB;
}
			

JETAPI jeBoolean JETCC jeStrBlock_WriteToFile(const jeStrBlock *SB,jeVFile *pFile)
{
	uint32 u;
	jeStrBlock_FileHeader Header;

	assert( SB != NULL );
	assert( pFile != NULL );
	assert( SB->SanityCheck == SB );

	// Write the format flag
	u = STRBLOCK_BIN_FILE_TYPE;
	if(jeVFile_Write(pFile, &u, sizeof(u)) == JE_FALSE)
		{
			jeErrorLog_Add( JE_ERR_FILEIO_WRITE , "jeStrBlock_WriteToFile: Failed to write header.");
			return JE_FALSE;
		}

	Header.Size = jeStrBlock_BlockSize(SB);
	Header.Count = SB->Count;

	if(jeVFile_Write(pFile, &Header, sizeof(jeStrBlock_FileHeader)) == JE_FALSE)
		{
			jeErrorLog_Add( JE_ERR_FILEIO_WRITE , "jeStrBlock_WriteToFile: Failed to write header block.");
			return JE_FALSE;
		}
	
	if (jeVFile_Write(pFile, &(SB->Data),Header.Size) == JE_FALSE)
		{
			jeErrorLog_Add( JE_ERR_FILEIO_WRITE , "jeStrBlock_WriteToFile: Failed to write string data.");
			return JE_FALSE;
		}
		
	return JE_TRUE;
}
