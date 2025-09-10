/****************************************************************************************/
/*  SYMBOL.C                                                                            */
/*                                                                                      */
/*  Author: Eli Boling                                                                  */
/*  Description: Generic symbol table/property list implementation                      */
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
#include	<stdlib.h>
#include	<limits.h>
#include	<assert.h>
#include	<string.h>

#include	"vec3d.h"
#include	"getypes.h"
#include	"ram.h"

#include	"RefPool.h"
#include	"GCHeap.h"
#include	"Symbol.h"

//jeSymbol_Table *	TempST;

// Note:  You must not have more than 64k of hash buckets, as our hash value is 16bits
#define	NUMHASHBUCKETS	223
//#define	NUMHASHBUCKETS	5

typedef	struct	jeSymbol_Rec		jeSymbol_Rec;
typedef	struct	jeSymbol_ListRec	jeSymbol_ListRec;

typedef	struct	PropList
{
	jeSymbol_Rec *	Symbol;
	union
	{
		int					Integer;
		char *				String;
		float				Float;
		jeVec3d				Vec3d;
		JE_RGBA				Color;
		jeBoolean			Boolean;
		void *				Void;
		jeSymbol_ListRec *	List;
		jeSymbol_Rec *		Sym;		// Sym, not Symbol to reduce typo risks with Symbol, above
/*
		jeModel *		Model;
		jePortal *		Portal;
*/
	}	Value;
	struct PropList *	Next;
}	PropList;

typedef	struct	jeSymbol_Table
{
	jeSymbol_List **	Symbols;
	jeSymbol *			QualifierListProperty;
	int					RefCount;
	RefPool *			SymbolReferences;
	RefPool *			ListReferences;
	GCHeap *			SymbolHeap;
	GCHeap *			ListHeap;
	int					GCAbleOperationCount;
}	jeSymbol_Table;

#define	SYMREF_INCREMENT	100
#define	LISTREF_INCREMENT	20

#define	GCABLE_OP_COUNT_THRESHHOLD	20

#define	SYMBOL_REMOVED			0x01
#define	SYMBOL_WRITTEN			0x02
#define	SYMBOL_WRITING			0x04
#define	SYMBOL_BEINGDESTROYED	0x08
#define	SYMBOL_MARKED			0x10

typedef	struct			jeSymbol_Rec
{
	char *				Name;
	int					NameLength;
	unsigned short		HashValue;
	jeSymbol_Type		Type;
	int					EnumValue;
	PropList *			Properties;
	jeSymbol_Rec *		Qualifier;
//	int					RefCount;			// RefCount and SymbolId could be unioned
	unsigned int		SymbolId;			// to save space
	jeSymbol_Table *	SymbolTable;
	unsigned char		Flags;
}	jeSymbol_Rec;

typedef	struct	jeSymbol
{
	jeSymbol_Rec *		Symbol;
}	jeSymbol;

typedef	struct	SymListElt
{
	jeSymbol_Rec *		Symbol;
	struct SymListElt *	Next;
}	SymListElt;

typedef struct	jeSymbol_ListRec
{
	SymListElt *		Elts;
	int					CurrentIndex;
	SymListElt *		Current;
	jeSymbol_Table *	SymbolTable;
//	int				RefCount;
}	jeSymbol_ListRec;

typedef	struct	jeSymbol_List
{
	jeSymbol_ListRec *		List;
}	jeSymbol_List;

typedef	struct	Symbol_Array
{
	int				ElementCount;
	int				CurrentIndex;
	jeSymbol_Rec **	Symbols;
}	Symbol_Array;

#define	QUALLIST_PROPERTY	"*QualifierListProperty*"

static	void FinalizeSymbol(void *P);
static	void FinalizeList(void *P);

static	void jeSymbol_ListDestroyNoRef(jeSymbol_List **pSymList);

static	unsigned short	HashValues[256];
static	int				HashInitialized = 0;

#pragma message ("Name hashing doesn't support wide char")
static	void			HashInit(void)
{
	int	i;

	if	(HashInitialized)
		return;

	HashInitialized = 1;

	for	(i = 0; i < 256; i++)
		HashValues[i] = (unsigned short)rand();
}

static	unsigned short	Hash(const char *s, int Count)
{
	unsigned short	Value;

	HashInit();

	Value = 0;
	while	(Count--)
	{
		Value += HashValues[*s];
		s++;
	}

	return Value;
}

static	jeSymbol *	ReferenceSymbol(jeSymbol_Table *ST, jeSymbol_Rec *Symbol)
{
	jeSymbol *	HSymbol;
	
	assert(ST);
	assert(Symbol);

	HSymbol = (jeSymbol *)RefPool_RefCreate(ST->SymbolReferences);
	if	(HSymbol)
		HSymbol->Symbol = Symbol;

	return HSymbol;
}

static	jeSymbol_List *	ReferenceList(jeSymbol_Table *ST, jeSymbol_ListRec *List)
{
	jeSymbol_List *	HList;
	
	assert(ST);
	assert(List);

	HList = (jeSymbol_List *)RefPool_RefCreate(ST->ListReferences);
	if	(HList)
		HList->List = List;

	return HList;
}

static	Symbol_Array *Symbol_ArrayCreate(int ElementCount)
{
	Symbol_Array *	SymArray;

	SymArray = jeRam_Allocate(sizeof(*SymArray));
	if	(!SymArray)
		return SymArray;

	SymArray->Symbols = jeRam_Allocate(sizeof(*SymArray->Symbols) * ElementCount);
	if	(!SymArray->Symbols)
	{
		jeRam_Free(SymArray);
		return NULL;
	}
	memset(SymArray->Symbols, 0, sizeof(*SymArray->Symbols) * ElementCount);
	SymArray->ElementCount = ElementCount;
	SymArray->CurrentIndex = 0;

	return SymArray;
}

static	void	Symbol_ArrayDestroy(Symbol_Array **pSymArray)
{
	Symbol_Array *	SymArray;

	SymArray = *pSymArray;

	assert(pSymArray);
	assert(SymArray);

	jeRam_Free(SymArray->Symbols);
	jeRam_Free(SymArray);
	*pSymArray = NULL;
}

static	void	Symbol_ArrayAddSymbol(Symbol_Array *SymArray, jeSymbol_Rec *Symbol)
{
	assert(SymArray);
	assert(Symbol);
	assert(SymArray->ElementCount > SymArray->CurrentIndex);

	SymArray->Symbols[SymArray->CurrentIndex++] = Symbol;
}

static	jeSymbol_Rec *	Symbol_ArrayGetSymbol(Symbol_Array *SymArray, int Index)
{
	jeSymbol_Rec *	Symbol;

	assert(SymArray);
	assert(Index < SymArray->CurrentIndex);

	Symbol = SymArray->Symbols[Index];
	assert(Symbol);
	return Symbol;
}

static	jeBoolean	jeSymbol_IsValid(const jeSymbol_Rec *Sym)
{
	if	(!Sym)
		return JE_FALSE;

//	if	(Sym->RefCount == 0)
//		return JE_FALSE;

	if	(!Sym->Name)
		return JE_FALSE;

	return JE_TRUE;
}

jeBoolean	jeSymbol_TableIsValid(const jeSymbol_Table *ST)
{
	int	i;

	if	(!ST)
		return JE_FALSE;

	for	(i = 0; i < NUMHASHBUCKETS; i++)
	{
		SymListElt *	Elts;

		Elts = ST->Symbols[i]->List->Elts;
		while	(Elts)
		{
			if	(!jeSymbol_IsValid(Elts->Symbol))
				return JE_FALSE;
			Elts = Elts->Next;
		}
	}

	return JE_TRUE;
}

JETAPI	jeSymbol_Table * JETCC jeSymbol_TableCreate(void)
{
	jeSymbol_Table *	ST;
	int					i;

	ST = jeRam_Allocate(sizeof(*ST));
	if	(!ST)
		return ST;

	ST->Symbols = jeRam_Allocate(sizeof(*ST->Symbols) * NUMHASHBUCKETS);
	if	(!ST->Symbols)
	{
		jeRam_Free(ST);
		return NULL;
	}

	ST->RefCount = 1;
	ST->GCAbleOperationCount = 0;

	ST->SymbolReferences = RefPool_Create(SYMREF_INCREMENT);
	ST->ListReferences = RefPool_Create(LISTREF_INCREMENT);
	if	(!ST->SymbolReferences || !ST->ListReferences)
	{
		jeSymbol_TableDestroy(&ST);
		return NULL;
	}

	ST->SymbolHeap = GCHeap_Create(sizeof(jeSymbol_Rec), 100, FinalizeSymbol);
	ST->ListHeap = GCHeap_Create(sizeof(jeSymbol_ListRec), 100, FinalizeList);
	if	(!ST->SymbolHeap || !ST->ListHeap)
	{
		jeSymbol_TableDestroy(&ST);
		return NULL;
	}

	for	(i = 0; i < NUMHASHBUCKETS; i++)
	{
		ST->Symbols[i] = jeSymbol_ListCreate(ST);
		if	(!ST->Symbols[i])
		{
			jeSymbol_TableDestroy(&ST);
			return NULL;
		}
	}

	ST->QualifierListProperty = jeSymbol_Create(ST, NULL, QUALLIST_PROPERTY, JE_SYMBOL_TYPE_LIST);
	if	(!ST->QualifierListProperty)
	{
		jeSymbol_TableDestroy(&ST);
		return NULL;
	}

	return ST;
}

//JETAPI	const jeSymbol_List *JETCC jeSymbol_TableGetQualifiedSymbolList(
JETAPI	 jeSymbol_List *JETCC jeSymbol_TableGetQualifiedSymbolList(
	const jeSymbol_Table *	ST,
	const jeSymbol *		HQualifier)
{
	jeSymbol_List *	List;
	int				i;
	SymListElt *	Elts;

	assert(ST);
	List = jeSymbol_ListCreate((jeSymbol_Table *)ST);
	if	(!List)
		return List;
	
	for	(i = 0; i < NUMHASHBUCKETS; i++)
	{
		Elts = ST->Symbols[i]->List->Elts;
		while	(Elts)
		{
			if	((!HQualifier && !Elts->Symbol->Qualifier) || (Elts->Symbol->Qualifier == HQualifier->Symbol))
			{
				jeSymbol *	HSymbol;
				HSymbol = ReferenceSymbol((jeSymbol_Table *)ST, Elts->Symbol);
				if	(!HSymbol)
				{
					jeSymbol_ListDestroy(&List);
					return NULL;
				}
				
				if	(jeSymbol_ListAddSymbol(List, HSymbol) == JE_FALSE)
				{
					jeSymbol_Destroy(&HSymbol);
					jeSymbol_ListDestroy(&List);
					return NULL;
				}
				jeSymbol_Destroy(&HSymbol);
			}
			Elts = Elts->Next;
		}
	}
	return List;
}

#if 0
JETAPI	void JETCC jeSymbol_TableCreateRef(jeSymbol_Table *ST)
{
	assert(ST);
	assert(ST->RefCount > 0);
	ST->RefCount++;
}
#endif

#define	ST_SIGNATURE		0x30305453	/* ST00 */
#define	ST_SIGNATURE_END	0x31305453	/* ST01 */

static	jeBoolean WriteUInt(jeVFile *File, unsigned int Value)
{
	unsigned char	Buff[5];
	char *			pBuff;

	assert(File);

	pBuff = &Buff[0];
	while	(Value > 0x7f)
	{
		*pBuff++ = 0x80 | (Value & 0x7f);
		Value = Value >> 7;
	}
	assert(pBuff - Buff < 5);
	*pBuff++ = Value;
	return jeVFile_Write(File, Buff, pBuff - Buff);
}

static	jeBoolean ReadUInt(jeVFile *File, unsigned int *Value)
{
	unsigned char	C;
	int				Shift;

	assert(File);

	*Value = 0;
	Shift = 0;
	do
	{
		if	(jeVFile_Read(File, &C, 1) == JE_FALSE)
			return JE_FALSE;
		*Value = *Value | (((unsigned int)(C & ~0x80)) << Shift);
		Shift += 7;
	}	while	(C & 0x80);
	return JE_TRUE;
}

static	jeBoolean WriteString(jeVFile *File, const char *String)
{
	int	Length;

	assert(File);
	assert(String);

	Length = strlen(String);
	assert(Length < 0x8000000);
	if	(WriteUInt(File, Length) == JE_FALSE)
		return JE_FALSE;
	return jeVFile_Write(File, String, Length);
}

static	jeBoolean WriteFloat(jeVFile *File, jeFloat Float)
{
	return jeVFile_Write(File, &Float, sizeof(Float));
}

static	jeBoolean ReadFloat(jeVFile *File, jeFloat *Float)
{
	return jeVFile_Read(File, Float, sizeof(*Float));
}

static	char *	ReadString(jeVFile *File)
{
	int		Length;
	char *	String;

	assert(File);

	if	(ReadUInt(File, &Length) == JE_FALSE)
		return NULL;
	String = jeRam_Allocate(Length + 1);
	if	(!String)
		return NULL;
	if	(jeVFile_Read(File, String, Length) == JE_FALSE)
	{
		jeRam_Free(String);
		return NULL;
	}
	String[Length] = '\0';
	return String;
}

static	jeBoolean WriteSymbol(jeSymbol_Rec *Symbol, jeVFile *File, unsigned int *SymbolId)
{
	unsigned char	ReferenceWritten;

	assert(Symbol);
	assert(File);

	assert(!(Symbol->Flags & SYMBOL_WRITING));

	if	(Symbol->Flags & SYMBOL_WRITTEN)
	{
	
		ReferenceWritten = 0xff;
		if	(jeVFile_Write(File, &ReferenceWritten, sizeof(ReferenceWritten)) == JE_FALSE)
			return JE_FALSE;
		if	(WriteUInt(File, Symbol->SymbolId) == JE_FALSE)
			return JE_FALSE;
	
		return JE_TRUE;
	}
	else
	{
		unsigned char	QualifierPresent;

		assert(SymbolId);

		Symbol->Flags |= SYMBOL_WRITING;

		ReferenceWritten = 0;
		if	(jeVFile_Write(File, &ReferenceWritten, sizeof(ReferenceWritten)) == JE_FALSE)
			return JE_FALSE;

		if	(Symbol->Qualifier)
		{
			QualifierPresent = 0xff;
			if	(jeVFile_Write(File, &QualifierPresent, sizeof(QualifierPresent)) == JE_FALSE)
				return JE_FALSE;
			if	(WriteSymbol(Symbol->Qualifier, File, SymbolId) == JE_FALSE)
				return JE_FALSE;
		}
		else
		{
			QualifierPresent = 0;
			if	(jeVFile_Write(File, &QualifierPresent, sizeof(QualifierPresent)) == JE_FALSE)
				return JE_FALSE;
		}

		if	(WriteString(File, Symbol->Name) == JE_FALSE)
			return JE_FALSE;

		if	(WriteUInt(File, Symbol->Type) == JE_FALSE)
			return JE_FALSE;

		if	(WriteUInt(File, Symbol->EnumValue) == JE_FALSE)
			return JE_FALSE;

		Symbol->SymbolId = *SymbolId;
		*SymbolId = *SymbolId + 1;
		Symbol->Flags |= SYMBOL_WRITTEN;
		Symbol->Flags &= ~SYMBOL_WRITING;
	}

	return JE_TRUE;
}

static	jeSymbol_Rec *ReadSymbol(jeSymbol_Table *ST, jeVFile *File, Symbol_Array *SymArray)
{
	unsigned char	ReferenceWritten;
	jeSymbol_Rec *	Qualifier;
	jeSymbol_Type	Type;
	int				EnumValue;
	char *			Name;

	assert(ST);
	assert(File);
	assert(SymArray);

	if	(jeVFile_Read(File, &ReferenceWritten, sizeof(ReferenceWritten)) == JE_FALSE)
		return NULL;

	if	(ReferenceWritten == 0xff)
	{
		unsigned int	Index;
		jeSymbol_Rec *		Symbol;

		if	(ReadUInt(File, &Index) == JE_FALSE)
			return NULL;
		Symbol = Symbol_ArrayGetSymbol(SymArray, Index);
		return Symbol;
	}
	else
	{
		unsigned char	QualifierPresent;
		jeSymbol *		HSymbol;
//		jeSymbol *		HQualifier;
		jeSymbol		LocalQualifier;
		jeSymbol_Rec *	Symbol;

		assert(ReferenceWritten == 0);

		if	(jeVFile_Read(File, &QualifierPresent, sizeof(QualifierPresent)) == JE_FALSE)
			return NULL;
		if	(QualifierPresent == 0xff)
		{
			Qualifier = ReadSymbol(ST, File, SymArray);
			if	(!Qualifier)
				return NULL;
		}
		else
		{
			assert(QualifierPresent == 0);
			Qualifier = NULL;
		}
		Name = ReadString(File);
		if	(!Name)
			return NULL;

		if	(ReadUInt(File, &Type) == JE_FALSE)
		{
			jeRam_Free(Name);
			return NULL;
		}
		if	(ReadUInt(File, &EnumValue) == JE_FALSE)
		{
			jeRam_Free(Name);
			return NULL;
		}

		LocalQualifier.Symbol = Qualifier;
		Symbol = NULL;
		HSymbol = jeSymbol_Create(ST, Qualifier ? &LocalQualifier : NULL, Name, Type);
		if	(HSymbol)
		{
			Symbol = HSymbol->Symbol;
			Symbol_ArrayAddSymbol(SymArray, Symbol);
			jeSymbol_Destroy(&HSymbol);
		}

		jeRam_Free(Name);

		return Symbol;
	}
	assert(!"Shouldn't get here");
}

static	jeBoolean WriteSymbolList(jeVFile *File, const jeSymbol_ListRec *List)
{
	SymListElt *	Elts;
	int				Count;

	Elts = List->Elts;
	Count = 0;
	while	(Elts)
	{
		Count++;
		assert(Elts->Symbol->Flags & SYMBOL_WRITTEN);
		Elts = Elts->Next;
	}
	if	(WriteUInt(File, Count) == JE_FALSE)
		return JE_FALSE;

	Elts = List->Elts;
	while	(Elts)
	{
		if	(WriteUInt(File, Elts->Symbol->SymbolId) == JE_FALSE)
			return JE_FALSE;
		Elts = Elts->Next;
	}

	return JE_TRUE;
}

static	jeBoolean ReadSymbolList(jeSymbol_Table *ST, jeVFile *File, jeSymbol_ListRec **List, Symbol_Array *SymArray)
{
	int				Count;
	jeSymbol_List *	HList;

	HList = jeSymbol_ListCreate(ST);
	if	(!HList)
		return JE_FALSE;

	*List = HList->List;

	if	(ReadUInt(File, &Count) == JE_FALSE)
	{
		jeSymbol_ListDestroy(&HList);
		return JE_FALSE;
	}

	while	(Count--)
	{
		int				Index;
		jeSymbol_Rec *	Symbol;
		jeSymbol *		HSymbol;

		if	(ReadUInt(File, &Index) == JE_FALSE)
		{
			jeSymbol_ListDestroy(&HList);
			return JE_FALSE;
		}

		Symbol = Symbol_ArrayGetSymbol(SymArray, Index);
		HSymbol = ReferenceSymbol(ST, Symbol);
		if	(!HSymbol)
		{
			jeSymbol_ListDestroy(&HList);
			return JE_FALSE;
		}
		if	(jeSymbol_ListAddSymbol(HList, HSymbol) == JE_FALSE)
		{
			jeSymbol_Destroy(&HSymbol);
			jeSymbol_ListDestroy(&HList);
			return JE_FALSE;
		}
		jeSymbol_Destroy(&HSymbol);
	}
	return JE_TRUE;
}

static	jeBoolean WriteSymbolProperties(jeSymbol_Rec *Symbol, jeVFile *File)
{
	PropList *	PList;
	int			PropertyCount;

	assert(Symbol->Flags & SYMBOL_WRITTEN);

	if	(WriteUInt(File, Symbol->SymbolId) == JE_FALSE)
		return JE_FALSE;

	PList = Symbol->Properties;
	PropertyCount = 0;
	while	(PList)
	{
		PropertyCount++;
		PList = PList->Next;
	}

	if	(WriteUInt(File, PropertyCount) == JE_FALSE)
		return JE_FALSE;

	PList = Symbol->Properties;
	while	(PList)
	{
		unsigned char	UC;

		assert(PList->Symbol->Flags & SYMBOL_WRITTEN);
		if	(WriteUInt(File, PList->Symbol->SymbolId) == JE_FALSE)
			return JE_FALSE;
//		if	(WriteUInt(File, PList->Symbol->Type) == JE_FALSE)
//			return JE_FALSE;
		switch	(PList->Symbol->Type)
		{
		case	JE_SYMBOL_TYPE_INT:
			if	(WriteUInt(File, PList->Value.Integer) == JE_FALSE)
				return JE_FALSE;
			break;

		case	JE_SYMBOL_TYPE_STRING:
			if	(WriteString(File, PList->Value.String) == JE_FALSE)
				return JE_FALSE;
			break;

		case	JE_SYMBOL_TYPE_FLOAT:
			if	(WriteFloat(File, PList->Value.Float) == JE_FALSE)
				return JE_FALSE;
			break;

		case	JE_SYMBOL_TYPE_VEC3D:
			if	(WriteFloat(File, PList->Value.Vec3d.X) == JE_FALSE)
				return JE_FALSE;
			if	(WriteFloat(File, PList->Value.Vec3d.Y) == JE_FALSE)
				return JE_FALSE;
			if	(WriteFloat(File, PList->Value.Vec3d.Z) == JE_FALSE)
				return JE_FALSE;
			break;

		case	JE_SYMBOL_TYPE_COLOR:
			if	(WriteFloat(File, PList->Value.Color.r) == JE_FALSE)
				return JE_FALSE;
			if	(WriteFloat(File, PList->Value.Color.g) == JE_FALSE)
				return JE_FALSE;
			if	(WriteFloat(File, PList->Value.Color.b) == JE_FALSE)
				return JE_FALSE;
			break;

		case	JE_SYMBOL_TYPE_BOOLEAN:
			if	(PList->Value.Boolean == JE_TRUE)
				UC = 1;
			else
				UC = 0;
			if	(jeVFile_Write(File, &UC, 1) == JE_FALSE)
				return JE_FALSE;
			break;

		case	JE_SYMBOL_TYPE_ENUM:
			if	(WriteUInt(File, PList->Value.Sym->SymbolId) == JE_FALSE)
				return JE_FALSE;
			break;

		case	JE_SYMBOL_TYPE_PVOID:
#pragma message ("Need to implement property writers for PVOID?")
			assert(!"Not implemented");
			break;

		case	JE_SYMBOL_TYPE_LIST:
			if	(WriteSymbolList(File, PList->Value.List) == JE_FALSE)
				return JE_FALSE;
			break;

		case	JE_SYMBOL_TYPE_SYMBOL:
			if	(WriteUInt(File, PList->Value.Sym->SymbolId) == JE_FALSE)
				return JE_FALSE;
			break;

		case	JE_SYMBOL_TYPE_MODEL:
		case	JE_SYMBOL_TYPE_PORTAL:
			assert(!"Not implemented");

		case	JE_SYMBOL_TYPE_VOID:
			assert(!"Not a legal property type");
			break;
		}
		PList = PList->Next;
	}

	return JE_TRUE;
}

static	int	TypeSizes[] =
{
	sizeof(int),		//JE_SYMBOL_TYPE_INT
	sizeof(char *),		//JE_SYMBOL_TYPE_STRING
	sizeof(jeFloat),	//JE_SYMBOL_TYPE_FLOAT
	sizeof(jeVec3d),	//JE_SYMBOL_TYPE_VEC3D
	sizeof(jeRGBA),		//JE_SYMBOL_TYPE_COLOR
	sizeof(jeBoolean),	//JE_SYMBOL_TYPE_BOOLEAN
	sizeof(jeSymbol *),	//JE_SYMBOL_TYPE_ENUM
	sizeof(void *),		//JE_SYMBOL_TYPE_PVOID
	sizeof(jeSymbol_List *), //JE_SYMBOL_TYPE_LIST
	sizeof(jeSymbol *),	//JE_SYMBOL_TYPE_SYMBOL

	0, 					//	JE_SYMBOL_TYPE_MODEL,
	0,					//	JE_SYMBOL_TYPE_PORTAL,
};

static	jeBoolean SetProp(jeSymbol_Rec *Symbol, jeSymbol_Rec *Property, void *Data)
{
	jeSymbol *	HSymbol;
	jeSymbol *	HProperty;
	jeBoolean	Result;

#pragma message ("Really horrible way of setting a property internally.  Fix this.")
	HSymbol = ReferenceSymbol(Symbol->SymbolTable, Symbol);
	HProperty = ReferenceSymbol(Property->SymbolTable, Property);
	Result = jeSymbol_SetProperty(HSymbol, HProperty, Data, TypeSizes[Property->Type], Property->Type);
	jeSymbol_Destroy(&HSymbol);
	jeSymbol_Destroy(&HProperty);

	return Result;
}

static	jeBoolean ReadSymbolProperties(jeVFile *File, jeSymbol_Table *ST, Symbol_Array *SymArray)
{
	int				Index;
	jeSymbol_Rec *	Symbol;
	jeSymbol_Rec *	Property;
	int				Count;

	if	(ReadUInt(File, &Index) == JE_FALSE)
		return JE_FALSE;

	Symbol = Symbol_ArrayGetSymbol(SymArray, Index);
	if	(ReadUInt(File, &Count) == JE_FALSE)
		return JE_FALSE;

	while	(Count--)
	{
//		jeSymbol_Type	Type;
		PropList		PEntry;
		jeSymbol		LocalSymbol;
		jeSymbol_List	LocalList;

		if	(ReadUInt(File, &Index) == JE_FALSE)
			return JE_FALSE;
		Property = Symbol_ArrayGetSymbol(SymArray, Index);
		switch	(Property->Type)
		{
			unsigned char	UC;
			int				Index;

		case	JE_SYMBOL_TYPE_INT:
			if	(ReadUInt(File, &PEntry.Value.Integer) == JE_FALSE)
				return JE_FALSE;
			break;

		case	JE_SYMBOL_TYPE_FLOAT:
			if	(ReadFloat(File, &PEntry.Value.Float) == JE_FALSE)
				return JE_FALSE;
			break;

		case	JE_SYMBOL_TYPE_STRING:
			PEntry.Value.String = ReadString(File);
			if	(!PEntry.Value.String)
				return JE_FALSE;
			break;

		case	JE_SYMBOL_TYPE_VEC3D:
			if	(ReadFloat(File, &PEntry.Value.Vec3d.X) == JE_FALSE)
				return JE_FALSE;
			if	(ReadFloat(File, &PEntry.Value.Vec3d.Y) == JE_FALSE)
				return JE_FALSE;
			if	(ReadFloat(File, &PEntry.Value.Vec3d.Z) == JE_FALSE)
				return JE_FALSE;
			break;

		case	JE_SYMBOL_TYPE_COLOR:
			if	(ReadFloat(File, &PEntry.Value.Color.r) == JE_FALSE)
				return JE_FALSE;
			if	(ReadFloat(File, &PEntry.Value.Color.g) == JE_FALSE)
				return JE_FALSE;
			if	(ReadFloat(File, &PEntry.Value.Color.b) == JE_FALSE)
				return JE_FALSE;
			PEntry.Value.Color.a = 0.0f;
			break;

		case	JE_SYMBOL_TYPE_BOOLEAN:
			if	(jeVFile_Read(File, &UC, 1) == JE_FALSE)
				return JE_FALSE;
			if	(UC == 1)
				PEntry.Value.Boolean = JE_TRUE;
			else
			{
				assert(UC == 0);
				PEntry.Value.Boolean = JE_FALSE;
			}
			break;

		case	JE_SYMBOL_TYPE_ENUM:
		case	JE_SYMBOL_TYPE_SYMBOL:
			if	(ReadUInt(File, &Index) == JE_FALSE)
				return JE_FALSE;
			// Hacking a little bit here.
			LocalSymbol.Symbol = Symbol_ArrayGetSymbol(SymArray, Index);
			PEntry.Value.Sym = (jeSymbol_Rec *)&LocalSymbol;
			break;

		case	JE_SYMBOL_TYPE_PVOID:
			assert(!"Can't read this from a file");
			break;

		case	JE_SYMBOL_TYPE_LIST:
			if	(ReadSymbolList(ST, File, &LocalList.List, SymArray) == JE_FALSE)
				return JE_FALSE;
			PEntry.Value.List = (jeSymbol_ListRec *)&LocalList;
			break;

		case	JE_SYMBOL_TYPE_MODEL:
		case	JE_SYMBOL_TYPE_PORTAL:
			assert(!"Need to think about models and portals");
			break;

		case	JE_SYMBOL_TYPE_VOID:
			assert(!"Illegal property in a file");
			break;

		default:
			assert(!"Unknown property type");
		}

		if	(SetProp(Symbol, Property, &PEntry.Value) == JE_FALSE)
		{
			switch	(Property->Type)
			{
			case	JE_SYMBOL_TYPE_STRING:
				jeRam_Free(PEntry.Value.String);
				break;
			}

			return JE_FALSE;
		}
	}
	return JE_TRUE;
}

JETAPI	jeBoolean JETCC jeSymbol_TableWriteToFile(const jeSymbol_Table *ST, jeVFile *File)
{
	int				i;
	unsigned int	SymbolCount;
	SymListElt *	Elts;
	unsigned int	Signature;
	unsigned int	SymbolId;

	assert(ST);
	assert(File);
	
	/*
		If we unioned RefCount and SymbolId in the symbol structure, we could save
		runtime space in the symbol table by saving off all the refcounts in an array
		here, and assigning persistent ids in the RefCount field.
	*/
	SymbolCount = 0;
	for	(i = 0; i < NUMHASHBUCKETS; i++)
	{
		Elts = ST->Symbols[i]->List->Elts;
		while	(Elts)
		{
			SymbolCount++;
			Elts->Symbol->Flags &= ~SYMBOL_WRITTEN;
			Elts->Symbol->SymbolId = -1;
			Elts = Elts->Next;
		}
	}

	Signature = ST_SIGNATURE;
	if	(jeVFile_Write(File, &Signature, sizeof(Signature)) == JE_FALSE)
		return JE_FALSE;
	if	(jeVFile_Write(File, &SymbolCount, sizeof(SymbolCount)) == JE_FALSE)
		return JE_FALSE;

	SymbolId = 0;

	for	(i = 0; i < NUMHASHBUCKETS; i++)
	{
		Elts = ST->Symbols[i]->List->Elts;
		while	(Elts)
		{
			if	(WriteSymbol(Elts->Symbol, File, &SymbolId) == JE_FALSE)
				return JE_FALSE;
			Elts = Elts->Next;
		}
	}

	assert(SymbolId == SymbolCount);

	if	(jeVFile_Write(File, "STPROP", 6) == JE_FALSE)
		return JE_FALSE;

	// Now do all the properties.
	for	(i = 0; i < NUMHASHBUCKETS; i++)
	{
		Elts = ST->Symbols[i]->List->Elts;
		while	(Elts)
		{
			if	(WriteSymbolProperties(Elts->Symbol, File) == JE_FALSE)
				return JE_FALSE;
			Elts = Elts->Next;
		}
	}

	Signature = ST_SIGNATURE_END;
	if	(jeVFile_Write(File, &Signature, sizeof(Signature)) == JE_FALSE)
		return JE_FALSE;

	return JE_TRUE;
}

JETAPI	jeSymbol_Table *JETCC jeSymbol_TableCreateFromFile(jeVFile *File)
{
	jeSymbol_Table *	ST;
	int					SymbolCount;
	unsigned int		Signature;
//	jeSymbol **			SymArray;
	Symbol_Array *		SymArray;
//	int					SymbolId;
//	jeSymbol_Rec *		Symbol;
	char				PropSignature[6];
	int					i;

	assert(File);

	if	(jeVFile_Read(File, &Signature, sizeof(Signature)) == JE_FALSE)
		return NULL;
	if	(jeVFile_Read(File, &SymbolCount, sizeof(SymbolCount)) == JE_FALSE)
		return NULL;

	if	(Signature != ST_SIGNATURE)
		return NULL;

	ST = jeSymbol_TableCreate();
	if	(!ST)
		return ST;

	if	(SymbolCount > 0)
	{
//		SymbolId = 0;
//			SymArray = jeRam_Allocate(sizeof(*SymArray) * SymbolCount);
		SymArray = Symbol_ArrayCreate(SymbolCount);
		if	(!SymArray)
		{
			jeSymbol_TableDestroy(&ST);
			return NULL;
		}

		for	(i = 0; i < SymbolCount; i++)
		{
			if	(ReadSymbol(ST, File, SymArray) == NULL)
			{
				jeRam_Free(SymArray);
				jeSymbol_TableDestroy(&ST);
				return NULL;
			}
		}
//		Symbol_ArrayDestroy(&SymArray);
	}

	if	(jeVFile_Read(File, PropSignature, 6) == JE_FALSE)
	{
		if	(SymArray)
			Symbol_ArrayDestroy(&SymArray);
		jeSymbol_TableDestroy(&ST);
		return NULL;
	}
	if	(strncmp(PropSignature, "STPROP", 6))
	{
		if	(SymArray)
			Symbol_ArrayDestroy(&SymArray);
		jeSymbol_TableDestroy(&ST);
		return NULL;
	}

	for	(i = 0; i < SymbolCount; i++)
	{
		if	(ReadSymbolProperties(File, ST, SymArray) == JE_FALSE)
		{
			if	(SymArray)
				Symbol_ArrayDestroy(&SymArray);
			jeSymbol_TableDestroy(&ST);
			return NULL;
		}
	}

	if	(jeVFile_Read(File, &Signature, sizeof(Signature)) == JE_FALSE)
	{
		if	(SymArray)
			Symbol_ArrayDestroy(&SymArray);
		jeSymbol_TableDestroy(&ST);
		return NULL;
	}

	if	(Signature != ST_SIGNATURE_END)
	{
		if	(SymArray)
			Symbol_ArrayDestroy(&SymArray);
		jeSymbol_TableDestroy(&ST);
		return NULL;
	}

	if	(SymArray)
		Symbol_ArrayDestroy(&SymArray);

	return ST;
}

JETAPI	void JETCC jeSymbol_TableDestroy(jeSymbol_Table **pST)
{
	jeSymbol_Table *	ST;
	int					i;

	assert(pST);
	assert(*pST);

	ST = *pST;
//TempST = ST;
	assert(ST->RefCount > 0);

	if	(--ST->RefCount > 0)
		return;

	assert(ST->Symbols);

	if	(ST->QualifierListProperty)
		jeSymbol_Destroy(&ST->QualifierListProperty);

	for	(i = 0; i < NUMHASHBUCKETS; i++)
	{
		assert(ST->Symbols[i]);
		jeSymbol_ListDestroy(&ST->Symbols[i]);
	}

	if	(ST->SymbolReferences)
		RefPool_Destroy(&ST->SymbolReferences);

	if	(ST->ListReferences)
		RefPool_Destroy(&ST->ListReferences);

	if	(ST->SymbolHeap)
	{
		GCHeap_Sweep(ST->SymbolHeap);
		GCHeap_Destroy(&ST->SymbolHeap);
	}

	if	(ST->ListHeap)
	{
		GCHeap_Sweep(ST->ListHeap);
		GCHeap_Destroy(&ST->ListHeap);
	}

	jeRam_Free(ST->Symbols);
	jeRam_Free(ST);
//TempST = NULL;
	*pST = NULL;
}

JETAPI	jeSymbol *JETCC jeSymbol_TableFindSymbol(
	jeSymbol_Table *	ST,
	jeSymbol *			HQualifier,
	const char *		Name)
{
	unsigned short	HashValue;
	int				Length;
	SymListElt *	Elts;
	jeSymbol_Rec *	Sym;
	jeSymbol_Rec *	Qualifier;

	assert(ST);
	assert(Name);

	assert(jeSymbol_TableIsValid(ST) == JE_TRUE);

	Length = strlen(Name);

	HashValue = Hash(Name, Length);
	if	(HQualifier)
	{
		assert(HQualifier->Symbol);
		Qualifier = HQualifier->Symbol;
		HashValue += (unsigned short)Qualifier;
	}
	else
	{
		Qualifier = NULL;
	}

	Elts = ST->Symbols[HashValue % NUMHASHBUCKETS]->List->Elts;
	Sym = NULL;
	while	(Elts)
	{
		Sym = Elts->Symbol;
		if	(Length == Sym->NameLength)
		{
			// Is this the one?
			if	(Sym->Qualifier == Qualifier && !stricmp(Sym->Name, Name))
				break;
		}
		Elts = Elts->Next;
	}

	// If we found one, just return a ref to it.
	if	(Elts)
	{
		assert(Sym);
		return ReferenceSymbol(Sym->SymbolTable, Sym);
	}

	return NULL;
}

#if 1
typedef	jeBoolean	(*SymbolWalker)(jeSymbol_Rec *Symbol);
typedef	jeBoolean	(*ListWalker)(jeSymbol_ListRec *List, SymbolWalker Walker);

static	jeBoolean	WalkListWithoutReference(jeSymbol_ListRec *List, SymbolWalker Walker)
{
	SymListElt *	Elts;

	Elts = List->Elts;
	while	(Elts)
	{
		if	((Walker)(Elts->Symbol) == JE_FALSE)
			return JE_FALSE;
		Elts = Elts->Next;
	}
	return JE_TRUE;
}

static	jeBoolean	WalkListAndMark(jeSymbol_ListRec *List, SymbolWalker Walker)
{
	SymListElt *	Elts;

	assert(List->SymbolTable);
	assert(List->SymbolTable->ListHeap);

	GCHeap_MarkObject(List);

	Elts = List->Elts;
	while	(Elts)
	{
		if	((Walker)(Elts->Symbol) == JE_FALSE)
			return JE_FALSE;
		Elts = Elts->Next;
	}
	return JE_TRUE;
}

static	void	jeSymbol_Walk(jeSymbol_Rec *Symbol, SymbolWalker Walker, ListWalker LWalker)
{
	PropList *		PList;

	if	(Symbol->Qualifier)
	{
		if	((Walker)(Symbol->Qualifier) == JE_FALSE)
			return;
	}

	PList = Symbol->Properties;
	while	(PList)
	{
		if	((Walker)(PList->Symbol) == JE_FALSE)
			return;
		switch	(PList->Symbol->Type)
		{
		case	JE_SYMBOL_TYPE_ENUM:
		case	JE_SYMBOL_TYPE_SYMBOL:
			if	((Walker)(PList->Value.Sym) == JE_FALSE)
				return;
			break;

		case	JE_SYMBOL_TYPE_LIST:
			if	((LWalker)(PList->Value.List, Walker) == JE_FALSE)
				return;
			break;
		}
		PList = PList->Next;
	}
}
#endif

JETAPI	jeSymbol *JETCC jeSymbol_Create(
	jeSymbol_Table *	ST,
	jeSymbol *			HQualifier,
	const char *		Name,
	jeSymbol_Type		Type)
{
	unsigned short		HashValue;
	int					Length;
	SymListElt *		Elts;
	jeSymbol_ListRec *	SymList;
	jeSymbol_Rec *		Sym;
	jeSymbol_Rec *		Qualifier;
	jeSymbol *			HSymbol;
	jeSymbol_List		List;

	assert(ST);
	assert(Name);

	assert(jeSymbol_TableIsValid(ST) == JE_TRUE);

	if	(HQualifier)
		Qualifier = HQualifier->Symbol;
	else
		Qualifier = NULL;

	Length = strlen(Name);

	HashValue = Hash(Name, Length) + (unsigned short)Qualifier;
//	if	(Qualifier)
//		HashValue += Qualifier->HashValue;

	Elts = ST->Symbols[HashValue % NUMHASHBUCKETS]->List->Elts;
	Sym = NULL;
	while	(Elts)
	{
		Sym = Elts->Symbol;
		if	(Length == Sym->NameLength)
		{
			// Is this the one?
			if	(Sym->Qualifier == Qualifier && !stricmp(Sym->Name, Name))
				break;
		}
		Elts = Elts->Next;
	}

	// If we found one, just return a ref to it.
	if	(Elts)
	{
		assert(Sym);
//		jeSymbol_CreateRef(Sym);
		return ReferenceSymbol(Sym->SymbolTable, Sym);
//		return Sym;
	}

	// Didn't find it.  Create one.
//	Sym = jeRam_Allocate(sizeof(*Sym));
	Sym = GCHeap_AllocateFixed(ST->SymbolHeap);
	if	(!Sym)
		return NULL;
	Sym->Name = jeRam_Allocate(Length + 1);
	if	(!Sym->Name)
		return NULL;

	memcpy(Sym->Name, Name, Length + 1);
	Sym->NameLength = Length;
	Sym->Type = Type;
	Sym->HashValue = HashValue;
	Sym->EnumValue = 0;
	Sym->Properties = NULL;
	Sym->Qualifier = Qualifier;
	Sym->SymbolTable = ST;
//	Sym->RefCount = 1;
	Sym->Flags = 0;
//	if	(Qualifier)
//		jeSymbol_CreateRef(Qualifier);

	SymList = ST->Symbols[HashValue % NUMHASHBUCKETS]->List;

	HSymbol = (jeSymbol *)RefPool_RefCreate(ST->SymbolReferences);
	HSymbol->Symbol = Sym;

	List.List = SymList;
	if	(jeSymbol_ListAddSymbol(&List, HSymbol) == JE_FALSE)
	{
//		if	(Qualifier)
//			jeSymbol_Destroy(&Qualifier);
		jeRam_Free(Sym->Name);
//		jeRam_Free(Sym);
		return NULL;
	}

	// Should be referenced twice, once for the user, and once for the symbol list
//	assert(Sym->RefCount == 2);

	return HSymbol;
}

JETAPI	jeBoolean JETCC jeSymbol_Compare(const jeSymbol *S1, const jeSymbol *S2)
{
	if	(S1->Symbol == S2->Symbol)
		return JE_TRUE;
	else
		return JE_FALSE;
}

static	char *	DuplicateString(const char *S)
{
	int		Length;
	char *	Result;

	assert(S);

	Length = strlen(S) + 1;
	Result = jeRam_Allocate(Length);
	if	(Result)
		memcpy(Result, S, Length);
	return Result;
}

JETAPI	jeBoolean JETCC jeSymbol_Rename(jeSymbol *HSymbol, const char *NewName)
{
	jeSymbol_Rec *	Symbol;
	unsigned short	HashValue;
	int				Length;
	char *			NewNameCopy;

	assert(HSymbol);
	assert(HSymbol->Symbol);

	Symbol = HSymbol->Symbol;
	Length = strlen(NewName);
	HashValue = Hash(NewName, Length) + (unsigned short)Symbol->Qualifier;
//	if	(Symbol->Qualifier)
//		HashValue += Symbol->Qualifier->HashValue;
	NewNameCopy = DuplicateString(NewName);
	if	(!NewNameCopy)
		return JE_FALSE;

	if	(jeSymbol_ListAddSymbol(Symbol->SymbolTable->Symbols[HashValue % NUMHASHBUCKETS], HSymbol) == JE_FALSE)
	{
		jeRam_Free(NewNameCopy);
		return JE_FALSE;
	}
	jeRam_Free(Symbol->Name);
	Symbol->Name = NewNameCopy;
	Symbol->HashValue = HashValue;
	jeSymbol_ListRemoveSymbol(Symbol->SymbolTable->Symbols[Symbol->HashValue % NUMHASHBUCKETS], HSymbol);
	return JE_TRUE;
}

static	jeBoolean jeSymbol_MarkSymbol(jeSymbol_Rec *Sym)
{
	assert(Sym);

	if	(Sym->Flags & SYMBOL_MARKED)
		return JE_TRUE;

	Sym->Flags |= SYMBOL_MARKED;
	GCHeap_MarkObject(Sym);
	jeSymbol_Walk(Sym, jeSymbol_MarkSymbol, WalkListAndMark);
	return JE_TRUE;
}

static	jeBoolean jeSymbol_ClearMarks(jeSymbol_Rec *Sym)
{
	assert(Sym);

	if	(!(Sym->Flags & SYMBOL_MARKED))
		return JE_TRUE;

	Sym->Flags &= ~SYMBOL_MARKED;
	jeSymbol_Walk(Sym, jeSymbol_ClearMarks, WalkListWithoutReference);
	return JE_TRUE;
}

JETAPI	void JETCC jeSymbol_TableCollectGarbage(jeSymbol_Table *ST)
{
	void **	Ref;

	assert(ST);
	assert(ST->SymbolReferences);
	assert(ST->ListReferences);

//printf("\nGC\n");

	ST->GCAbleOperationCount = 0;

	Ref = RefPool_GetNextRef(ST->SymbolReferences, NULL);
	while	(Ref)
	{
		jeSymbol_MarkSymbol(*Ref);
		Ref = RefPool_GetNextRef(ST->SymbolReferences, Ref);
	}

	Ref = RefPool_GetNextRef(ST->ListReferences, NULL);
	while	(Ref)
	{
		WalkListAndMark(*Ref, jeSymbol_MarkSymbol);
		Ref = RefPool_GetNextRef(ST->ListReferences, Ref);
	}

	GCHeap_Sweep(ST->SymbolHeap);
	GCHeap_Sweep(ST->ListHeap);

	Ref = RefPool_GetNextRef(ST->SymbolReferences, NULL);
	while	(Ref)
	{
		jeSymbol_ClearMarks(*Ref);
		Ref = RefPool_GetNextRef(ST->SymbolReferences, Ref);
	}

	Ref = RefPool_GetNextRef(ST->ListReferences, NULL);
	while	(Ref)
	{
		WalkListWithoutReference(*Ref, jeSymbol_ClearMarks);
		Ref = RefPool_GetNextRef(ST->ListReferences, Ref);
	}
}

JETAPI	void JETCC jeSymbol_CreateRef(jeSymbol *HS, jeSymbol **Result)
{
	assert(HS);
	assert(HS->Symbol);
	*Result = ReferenceSymbol(HS->Symbol->SymbolTable, HS->Symbol);
}

static	void FinalizeSymbol(void *P)
{
	PropList *		PList;
	jeSymbol_Rec *	Symbol;

	assert(P);

	Symbol = P;

//	printf("Finalizing symbol %s\n", Symbol->Name);

	PList = Symbol->Properties;
	while	(PList)
	{
		PropList *	Temp;

		assert(PList->Symbol);

		if	(PList->Symbol->Type == JE_SYMBOL_TYPE_STRING)
		{
			assert(PList->Value.String);
			jeRam_Free(PList->Value.String);
		}
		Temp = PList;
		PList = PList->Next;
		jeRam_Free(Temp);
	}

	jeRam_Free(Symbol->Name);
}

JETAPI	void JETCC jeSymbol_Destroy(jeSymbol **pHS)
{
	assert(pHS);
	assert(*pHS);

	RefPool_RefDestroy((*pHS)->Symbol->SymbolTable->SymbolReferences, (void ***)pHS);
}

JETAPI	jeSymbol_Type JETCC jeSymbol_GetType(const jeSymbol *HSym)
{
	assert(HSym);
	assert(HSym->Symbol);
	return HSym->Symbol->Type;
}

JETAPI	jeSymbol *JETCC jeSymbol_GetQualifier(const jeSymbol *HSym)
{
	jeSymbol_Rec *	Sym;

	assert(HSym);
	assert(HSym->Symbol);

	Sym = HSym->Symbol;

	return ReferenceSymbol(Sym->SymbolTable, Sym->Qualifier);
}

static	PropList *	JETCC FindProperty(PropList *List, const jeSymbol_Rec *Sym)
{
//	assert(List);
	assert(Sym);

	while	(List && Sym != List->Symbol)
		List = List->Next;

	return List;
}

JETAPI	jeBoolean JETCC jeSymbol_SetProperty(
	jeSymbol *		HSym,
	jeSymbol *		HProperty,
	const void *	Data,
	int 			DataLength,
	jeSymbol_Type	Type)
{
	jeSymbol_Rec *	Sym;
	jeSymbol_Rec *	Property;
	PropList *	Prop;
	jeBoolean	IsNewProperty;

	assert(HSym);
	assert(HSym->Symbol);
	assert(HProperty);
	assert(HProperty->Symbol);
	assert(Data);
	assert(DataLength > 0);

	Sym = HSym->Symbol;
	Property = HProperty->Symbol;

	if	(Type != Property->Type)
		return JE_FALSE;

	Prop = FindProperty(Sym->Properties, Property);
	if	(!Prop)
	{
		Prop = jeRam_Allocate(sizeof(*Prop));
		if	(!Prop)
			return JE_FALSE;
		Prop->Symbol = Property;
		IsNewProperty = JE_TRUE;
	}
	else
	{
		IsNewProperty = JE_FALSE;
	}

	switch	(Type)
	{
	case	JE_SYMBOL_TYPE_INT:
		if	(DataLength != sizeof(Prop->Value.Integer))
			goto fail;
		assert(sizeof(Prop->Value.Integer) == sizeof(int));
		Prop->Value.Integer = *(int *)Data;
		break;

	case	JE_SYMBOL_TYPE_FLOAT:
		if	(DataLength != sizeof(Prop->Value.Float))
			goto fail;
		assert(sizeof(Prop->Value.Float) == sizeof(jeFloat));
		Prop->Value.Float = *(jeFloat *)Data;
		break;

	case	JE_SYMBOL_TYPE_VEC3D:
		if	(DataLength != sizeof(Prop->Value.Vec3d))
			goto fail;
		assert(sizeof(Prop->Value.Vec3d) == sizeof(jeVec3d));
		Prop->Value.Vec3d = *(jeVec3d *)Data;
		break;

	case	JE_SYMBOL_TYPE_COLOR:
		if	(DataLength != sizeof(Prop->Value.Color))
			goto fail;
		assert(sizeof(Prop->Value.Color) == sizeof(JE_RGBA));
		Prop->Value.Color = *(JE_RGBA *)Data;
		break;

	case	JE_SYMBOL_TYPE_BOOLEAN:
		if	(DataLength != sizeof(Prop->Value.Boolean))
			goto fail;
		assert(sizeof(Prop->Value.Boolean) == sizeof(jeBoolean));
		Prop->Value.Boolean = *(jeBoolean *)Data;
		break;

	case	JE_SYMBOL_TYPE_STRING:
		Prop->Value.String = DuplicateString(Data);
		if	(!Prop->Value.String)
			goto fail;
		break;

	case	JE_SYMBOL_TYPE_ENUM:
	case	JE_SYMBOL_TYPE_SYMBOL:
		if	(DataLength != sizeof(Prop->Value.Sym))
			goto fail;
		assert(sizeof(Prop->Value.Sym) == sizeof(jeSymbol *));
		Prop->Value.Sym = (*(jeSymbol **)Data)->Symbol;
		break;

	case	JE_SYMBOL_TYPE_LIST:
		if	(DataLength != sizeof(Prop->Value.List))
			goto fail;
		assert(sizeof(Prop->Value.List) == sizeof(jeSymbol_List *));
		Prop->Value.List = (*(jeSymbol_List **)Data)->List;

		if	(++Sym->SymbolTable->GCAbleOperationCount > GCABLE_OP_COUNT_THRESHHOLD)
			jeSymbol_TableCollectGarbage(Sym->SymbolTable);

		break;

	default:
#pragma message ("Need a few more properties implemented here")
		assert(!"Not implemented");
	}

	if	(IsNewProperty == JE_TRUE)
	{
		Prop->Next = Sym->Properties;
		Sym->Properties = Prop;
	}

	return JE_TRUE;

fail:
	if	(IsNewProperty == JE_TRUE)
		jeRam_Free(Prop);
	return JE_FALSE;
}

JETAPI	jeBoolean JETCC jeSymbol_GetProperty(
	jeSymbol *	HSym,
	jeSymbol *	HProperty,
	void *				Data,
	int 				DataLength,
	jeSymbol_Type 		Type)
{
	jeSymbol_Rec *	Sym;
	jeSymbol_Rec *	Property;
	PropList *		Prop;
//	jeBoolean		Result;

	assert(HSym);
	assert(HSym->Symbol);
	assert(HProperty);
	assert(HProperty->Symbol);
	assert(Data);
	assert(DataLength);

	Sym = HSym->Symbol;
	Property = HProperty->Symbol;

	Prop = FindProperty(Sym->Properties, Property);
	if	(!Prop)
		return JE_FALSE;

	if	(Property->Type != Type)
		return JE_FALSE;

	switch	(Type)
	{
	case	JE_SYMBOL_TYPE_INT:
		if	(DataLength != sizeof(Prop->Value.Integer))
			return JE_FALSE;
		assert(sizeof(Prop->Value.Integer) == sizeof(int));
		*(int *)Data = Prop->Value.Integer;
		break;

	case	JE_SYMBOL_TYPE_FLOAT:
		if	(DataLength != sizeof(Prop->Value.Float))
			return JE_FALSE;
		assert(sizeof(Prop->Value.Float) == sizeof(jeFloat));
		*(jeFloat *)Data = Prop->Value.Float;
		break;

	case	JE_SYMBOL_TYPE_VEC3D:
		if	(DataLength != sizeof(Prop->Value.Vec3d))
			return JE_FALSE;
		assert(sizeof(Prop->Value.Vec3d) == sizeof(jeVec3d));
		*(jeVec3d *)Data = Prop->Value.Vec3d;
		break;

	case	JE_SYMBOL_TYPE_COLOR:
		if	(DataLength != sizeof(Prop->Value.Color))
			return JE_FALSE;
		assert(sizeof(Prop->Value.Color) == sizeof(JE_RGBA));
		*(JE_RGBA *)Data = Prop->Value.Color;
		break;

	case	JE_SYMBOL_TYPE_BOOLEAN:
		if	(DataLength != sizeof(Prop->Value.Boolean))
			return JE_FALSE;
		assert(sizeof(Prop->Value.Boolean) == sizeof(jeBoolean));
		*(jeBoolean *)Data = Prop->Value.Boolean;
		break;

	case	JE_SYMBOL_TYPE_STRING:
		assert(sizeof(Prop->Value.String) == sizeof(char *));
		*(char **)Data = Prop->Value.String;
		break;

	case	JE_SYMBOL_TYPE_ENUM:
	case	JE_SYMBOL_TYPE_SYMBOL:
		if	(DataLength != sizeof(Prop->Value.Sym))
			return JE_FALSE;
		assert(sizeof(Prop->Value.Sym) == sizeof(jeSymbol *));
		*(jeSymbol **)Data = ReferenceSymbol(Prop->Value.Sym->SymbolTable, Prop->Value.Sym);
		if	(!*(jeSymbol **)Data)
			return JE_FALSE;
		break;

	case	JE_SYMBOL_TYPE_LIST:
		if	(DataLength != sizeof(Prop->Value.List))
			return JE_FALSE;
		assert(sizeof(Prop->Value.List) == sizeof(jeSymbol_List *));
		*(jeSymbol_List **)Data = ReferenceList(Prop->Value.List->SymbolTable, Prop->Value.List);
		if	(!*(jeSymbol_List **)Data)
			return JE_FALSE;
		break;

	default:
#pragma message ("Need a few more properties implemented here")
		assert(!"Not implemented");
	}

	return JE_TRUE;
}

JETAPI	jeBoolean JETCC jeSymbol_CopyProperty(
	jeSymbol *HDest,
	jeSymbol *HDestProp,
	jeSymbol *HSrc,
	jeSymbol *HSrcProp)
{
	jeSymbol_Rec *	Dest;
	jeSymbol_Rec *	DestProp;
	jeSymbol_Rec *	Src;
	jeSymbol_Rec *	SrcProp;
//	jeBoolean		Result;

	assert(HDest);
	assert(HDestProp);
	assert(HSrc);
	assert(HSrcProp);
	assert(HDest->Symbol);
	assert(HDestProp->Symbol);
	assert(HSrc->Symbol);
	assert(HSrcProp->Symbol);

	Dest = HDest->Symbol;
	DestProp = HDestProp->Symbol;
	Src = HSrc->Symbol;
	SrcProp = HSrcProp->Symbol;

	if	(DestProp->Type != SrcProp->Type)
		return JE_FALSE;

	switch	(DestProp->Type)
	{
		int				Integer;
		jeFloat			Float;
		jeVec3d			Vector;
		JE_RGBA			Color;
		jeSymbol *		Symbol;
		jeSymbol_List *	SymList;
		void *			Data;
		char *			String;
		jeBoolean		Result;

	case	JE_SYMBOL_TYPE_INT:
		if	(jeSymbol_GetProperty(HSrc, HSrcProp, &Integer, sizeof(Integer), JE_SYMBOL_TYPE_INT) == JE_FALSE)
			return JE_FALSE;
		return jeSymbol_SetProperty(HDest, HDestProp, &Integer, sizeof(Integer), JE_SYMBOL_TYPE_INT);

	case	JE_SYMBOL_TYPE_STRING:
		if	(jeSymbol_GetProperty(HSrc, HSrcProp, &String, sizeof(String), JE_SYMBOL_TYPE_STRING) == JE_FALSE)
			return JE_FALSE;
		String = DuplicateString(String);
		if	(!String)
			return JE_FALSE;
		Result = jeSymbol_SetProperty(HDest, HDestProp, String, sizeof(String), JE_SYMBOL_TYPE_STRING);
		if	(Result == JE_FALSE)
			jeRam_Free(String);
		return Result;

	case	JE_SYMBOL_TYPE_FLOAT:
		if	(jeSymbol_GetProperty(HSrc, HSrcProp, &Float, sizeof(Float), JE_SYMBOL_TYPE_FLOAT) == JE_FALSE)
			return JE_FALSE;
		return jeSymbol_SetProperty(HDest, HDestProp, &Float, sizeof(Float), JE_SYMBOL_TYPE_FLOAT);

	case	JE_SYMBOL_TYPE_COLOR:
		if	(jeSymbol_GetProperty(HSrc, HSrcProp, &Color, sizeof(Color), JE_SYMBOL_TYPE_COLOR) == JE_FALSE)
			return JE_FALSE;
		return jeSymbol_SetProperty(HDest, HDestProp, &Color, sizeof(Color), JE_SYMBOL_TYPE_COLOR);

	case	JE_SYMBOL_TYPE_VEC3D:
		if	(jeSymbol_GetProperty(HSrc, HSrcProp, &Vector, sizeof(Vector), JE_SYMBOL_TYPE_VEC3D) == JE_FALSE)
			return JE_FALSE;
		return jeSymbol_SetProperty(HDest, HDestProp, &Vector, sizeof(Vector), JE_SYMBOL_TYPE_VEC3D);

	case	JE_SYMBOL_TYPE_BOOLEAN:
		if	(jeSymbol_GetProperty(HSrc, HSrcProp, &Result, sizeof(Result), JE_SYMBOL_TYPE_BOOLEAN) == JE_FALSE)
			return JE_FALSE;
		return jeSymbol_SetProperty(HDest, HDestProp, &Result, sizeof(Result), JE_SYMBOL_TYPE_BOOLEAN);

	case	JE_SYMBOL_TYPE_ENUM:
		if	(jeSymbol_GetProperty(HSrc, HSrcProp, &Symbol, sizeof(Symbol), JE_SYMBOL_TYPE_ENUM) == JE_FALSE)
			return JE_FALSE;
		Result = jeSymbol_SetProperty(HDest, HDestProp, &Symbol, sizeof(Symbol), JE_SYMBOL_TYPE_ENUM);
		jeSymbol_Destroy(&Symbol);
		return Result;

	case	JE_SYMBOL_TYPE_PVOID:
#pragma message ("We don't copy void data, we just copy the pointer")
		if	(jeSymbol_GetProperty(HSrc, HSrcProp, &Data, sizeof(Data), JE_SYMBOL_TYPE_PVOID) == JE_FALSE)
			return JE_FALSE;
		return jeSymbol_SetProperty(HDest, HDestProp, &Data, sizeof(Data), JE_SYMBOL_TYPE_PVOID);

	case	JE_SYMBOL_TYPE_LIST:
		if	(jeSymbol_GetProperty(HSrc, HSrcProp, &SymList, sizeof(SymList), JE_SYMBOL_TYPE_LIST) == JE_FALSE)
			return JE_FALSE;
		Result = jeSymbol_SetProperty(HDest, HDestProp, &SymList, sizeof(SymList), JE_SYMBOL_TYPE_LIST);
		jeSymbol_ListDestroy(&SymList);
		return Result;

	case	JE_SYMBOL_TYPE_SYMBOL:
		if	(jeSymbol_GetProperty(HSrc, HSrcProp, &Symbol, sizeof(Symbol), JE_SYMBOL_TYPE_SYMBOL) == JE_FALSE)
			return JE_FALSE;
		Result = jeSymbol_SetProperty(HDest, HDestProp, &Symbol, sizeof(Symbol), JE_SYMBOL_TYPE_SYMBOL);
		jeSymbol_Destroy(&Symbol);
		return Result;

	case	JE_SYMBOL_TYPE_MODEL:
	case	JE_SYMBOL_TYPE_PORTAL:
#pragma message ("CopyProperty: Model and portal not implemented")
		assert(!"Not implemented");
		return JE_FALSE;

	case	JE_SYMBOL_TYPE_VOID:
		return JE_FALSE;

	default:
		assert(!"Bad symbol type");
		return JE_FALSE;
	}
	assert(!"Shouldn't get here");
	return JE_FALSE;
}

JETAPI	jeBoolean JETCC jeSymbol_GetEnumValue(const jeSymbol *HSym, int *Value)
{
	assert(HSym);
	assert(HSym->Symbol);
	assert(Value);

	if	(HSym->Symbol->Type != JE_SYMBOL_TYPE_ENUM)
		return JE_FALSE;
	*Value = HSym->Symbol->EnumValue;
	return JE_TRUE;
}

JETAPI	jeBoolean JETCC jeSymbol_SetEnumValue(jeSymbol *HSym, int Value)
{
	assert(HSym);
	assert(HSym->Symbol);

	if	(HSym->Symbol->Type != JE_SYMBOL_TYPE_ENUM)
		return JE_FALSE;
	HSym->Symbol->EnumValue = Value;
	return JE_TRUE;
}

JETAPI	const char *JETCC jeSymbol_GetName(const jeSymbol *HSym)
{
	assert(HSym);
	assert(HSym->Symbol);
	assert(HSym->Symbol->Name);

	return HSym->Symbol->Name;
}

JETAPI	jeBoolean	JETCC jeSymbol_GetFullName(const jeSymbol *HSym, char *Buff, int MaxLen)
{
	// Really slow implementation for deeply nested names
	jeSymbol_Rec *	Sym;

	assert(HSym);
	assert(HSym->Symbol);
	assert(Buff);

	Sym = HSym->Symbol;

	*Buff = '\0';
	if	(Sym->Qualifier)
	{
		jeSymbol *		HQualifier;
		jeBoolean		Result;

		HQualifier = ReferenceSymbol(Sym->SymbolTable, Sym->Qualifier);
		if	(!HQualifier)
			return JE_FALSE;
		Result = jeSymbol_GetFullName(HQualifier, Buff, MaxLen);
		jeSymbol_Destroy(&HQualifier);
		if	(Result == JE_FALSE)
			return JE_FALSE;
	}

	MaxLen -= strlen(Buff) + 2;
	if	(MaxLen < Sym->NameLength + 1)
		return JE_FALSE;

	strcat(Buff, "::");
	strcat(Buff, Sym->Name);

	return JE_TRUE;
}

JETAPI	jeSymbol_List *	JETCC jeSymbol_ListCreate(jeSymbol_Table *ST)
{
	jeSymbol_ListRec *	List;
	
	assert(ST);
	assert(ST->ListHeap);

	List = GCHeap_AllocateFixed(ST->ListHeap);
	if	(List)
	{
		jeSymbol_List *	HList;

		List->Elts = NULL;
		List->CurrentIndex = -1;
		List->Current = NULL;
		List->SymbolTable = ST;

		HList = (jeSymbol_List *)RefPool_RefCreate(ST->ListReferences);
		if	(HList)
			HList->List = List;

		return HList;
	}

	return NULL;
}

JETAPI	void JETCC jeSymbol_ListCreateRef(jeSymbol_List *HL, jeSymbol_List **Result)
{
	jeSymbol_List *	HList;

	assert(HL);
	assert(HL->List);

	HList = (jeSymbol_List *)RefPool_RefCreate(HL->List->SymbolTable->ListReferences);
	if	(HList)
		HList->List = HL->List;

	*Result = HList;
}

static	void FinalizeList(void *P)
{
	jeSymbol_ListRec *	List;
	SymListElt *		Elts;

	assert(P);
	List = P;

//	printf("Finalizing list %p\n", List);

	Elts = List->Elts;
	while	(Elts)
	{
		SymListElt *	Temp;

		Temp = Elts;
		Elts = Elts->Next;
		jeRam_Free(Temp);
	}
}

JETAPI	void JETCC jeSymbol_ListDestroy(jeSymbol_List **pSymList)
{
	assert(pSymList);
	assert(*pSymList);

	RefPool_RefDestroy((*pSymList)->List->SymbolTable->ListReferences, (void ***)pSymList);
}

JETAPI	jeSymbol *JETCC jeSymbol_ListGetSymbol(const jeSymbol_List *HL, int Index)
{
	SymListElt *		Elts;
	jeSymbol_ListRec *	L;

	assert(HL);
	assert(HL->List);

	L = HL->List;

	if	(L->CurrentIndex >= 0 && Index == L->CurrentIndex + 1)
	{
		if	(!L->Current)
			return NULL;

		L->Current = L->Current->Next;
		L->CurrentIndex++;

		if	(!L->Current)
			return NULL;
	}
	else
	{
		Elts = L->Elts;
	
		L->CurrentIndex = 0;
		while	(Elts && L->CurrentIndex != Index)
		{
			L->CurrentIndex++;
			Elts = Elts->Next;
		}
	
		if	(!Elts)
		{
			L->CurrentIndex = -1;
			return NULL;
		}
	
		L->Current = Elts;
	}

	return ReferenceSymbol(L->SymbolTable, L->Current->Symbol);
}

JETAPI	jeBoolean	JETCC jeSymbol_TableRemoveSymbol(
	jeSymbol_Table *	ST,
	jeSymbol *			HSymbol)
{
	jeSymbol_Rec *	Symbol;

	assert(ST);
	assert(HSymbol);
	assert(HSymbol->Symbol);

	Symbol = HSymbol->Symbol;

	if	(Symbol->Flags & SYMBOL_REMOVED)
		return JE_TRUE;

	Symbol->Flags |= SYMBOL_REMOVED;

	jeSymbol_ListRemoveSymbol(ST->Symbols[Symbol->HashValue % NUMHASHBUCKETS], HSymbol);

	return JE_TRUE;
}

JETAPI	jeBoolean JETCC jeSymbol_ListAddSymbol(jeSymbol_List *HL, jeSymbol *HS)
{
	SymListElt *		NewElt;
	jeSymbol_ListRec *	List;

	assert(HL);
	assert(HL->List);
	assert(HS);
	assert(HS->Symbol);

	List = HL->List;

	NewElt = jeRam_Allocate(sizeof(*NewElt));
	if	(!NewElt)
		return JE_FALSE;

	NewElt->Next = List->Elts;
	NewElt->Symbol = HS->Symbol;

	List->Elts = NewElt;
	List->CurrentIndex = -1;
	List->Current = NULL;

	return JE_TRUE;
}

JETAPI	void JETCC jeSymbol_ListRemoveSymbol(jeSymbol_List *HL, jeSymbol *HS)
{
	jeSymbol_ListRec *	L;
	SymListElt			Head;
	SymListElt *		Elts;
	SymListElt *		Temp;
	jeSymbol_Rec *		S;

	assert(HL);
	assert(HL->List);
	assert(HS);
	assert(HS->Symbol);

	L = HL->List;

	Head.Symbol = NULL;
	Head.Next = L->Elts;

	S = HS->Symbol;

	Elts = &Head;

	while	(Elts->Next->Symbol != S)
	{
		Elts = Elts->Next;
		assert(Elts->Next);
	}

	Temp = Elts->Next;
		   Elts->Next = Elts->Next->Next;
	if	(Elts == &Head)
		L->Elts = Elts->Next;

	if	(++L->SymbolTable->GCAbleOperationCount > GCABLE_OP_COUNT_THRESHHOLD)
		jeSymbol_TableCollectGarbage(L->SymbolTable);

	assert(Temp);
	assert(Temp->Symbol == S);
	jeRam_Free(Temp);
}

#if 1
#ifdef	_DEBUG
#if 0
static	void RefCountString(const jeSymbol *Sym, char *Buff)
{
	char *	p;

	if	(Sym->Symbol->Qualifier)
		RefCountString(Sym->Symbol->Qualifier, Buff);
	else
		*Buff = 0;

	p = Buff + strlen(Buff);
//	sprintf(p, "%d:", Sym->RefCount);
}
#endif

#if 1
jeBoolean jeSymbol_Dump(const jeSymbol *HSym, jeVFile *File)
{
	char 	Buff[256];
	jeSymbol_GetFullName(HSym, Buff, sizeof(Buff));
#if 0
{
	char	RFString[256];
	RefCountString(Sym, RFString);
	return jeVFile_Printf(File, "  %s (%s)\n", Buff, RFString);
}
#else
	return jeVFile_Printf(File, "  %s\n", Buff);
#endif
}
#else
static	int	indent;
jeBoolean jeSymbol_Dump1(jeSymbol_Rec *Sym)
{
	int	i;

	assert(Sym);

	for	(i = 0; i < indent; i++)
		printf(" ");
	printf("%s\n", Sym->Name);

	if	(Sym->Flags & SYMBOL_MARKED)
		return JE_TRUE;

	Sym->Flags |= SYMBOL_MARKED;
	indent += 2;
	jeSymbol_Walk(Sym, jeSymbol_Dump1, WalkListWithoutReference);
	indent -= 2;
	return JE_TRUE;
}

jeBoolean jeSymbol_Dump(const jeSymbol *HSym, jeVFile *File)
{
	indent = 0;
	printf("---\n");
//	printf("%s\n", jeSymbol_GetName(Sym));
//	jeSymbol_Walk((jeSymbol *)Sym, jeSymbol_Dump1, WalkListWithoutReference);
	jeSymbol_Dump1(HSym->Symbol);
//	jeSymbol_ClearMarks((jeSymbol **)&Sym);
	jeSymbol_ClearMarks(HSym->Symbol);
	return JE_TRUE;
}
#endif

static	int	__cdecl CmpSyms(const void *p1, const void *p2)
{
	jeSymbol_Rec *	S1;
	jeSymbol_Rec *	S2;
	jeSymbol		HS1;
	jeSymbol		HS2;
	char 		Buff1[1024];
	char 		Buff2[1024];

	S1 = *(jeSymbol_Rec **)p1;
	S2 = *(jeSymbol_Rec **)p2;

	HS1.Symbol = S1;
	HS2.Symbol = S2;
	jeSymbol_GetFullName(&HS1, Buff1, sizeof(Buff1));
	jeSymbol_GetFullName(&HS2, Buff2, sizeof(Buff2));
	return strcmp(Buff1, Buff2);
}

jeBoolean jeSymbol_TableDump(const jeSymbol_Table *ST, jeVFile *File, jeBoolean SortNames)
{
	int	i;
	int	MaxLength;
	int	MinLength;
	int	NumEmpty;
	int	TotalSyms;

	if	(File)
	{
		jeVFile_Printf(File, "Dump of Symbol Table\n");
		jeVFile_Printf(File, "--------------------\n");
	}
	else
	{
		printf("Dump of Symbol Table\n");
		printf("--------------------\n");
	}
	MaxLength = -1;
	MinLength = 100000;
	TotalSyms = 0;
	NumEmpty = 0;
	for	(i = 0; i < NUMHASHBUCKETS; i++)
	{
		SymListElt *	Elts;
		int				BucketCount;
		jeSymbol		LocalSym;

		BucketCount = 0;
	
		Elts = ST->Symbols[i]->List->Elts;
		if	(File)
			jeVFile_Printf(File, "Bucket %d:\n", i);
		else
			printf("Bucket %d:\n", i);
		if	(SortNames == JE_FALSE)
		{
			while	(Elts)
			{
				LocalSym.Symbol = Elts->Symbol;
				jeSymbol_Dump(&LocalSym, File);
				BucketCount++;
				Elts = Elts->Next;
			}
		}
		else
		{
			jeSymbol_Rec **	SortedSyms;
			int			j;

			while	(Elts)
			{
				BucketCount++;
				Elts = Elts->Next;
			}
			SortedSyms = jeRam_Allocate(sizeof(*SortedSyms) * BucketCount);
			if	(!SortedSyms)
				return JE_FALSE;
			Elts = ST->Symbols[i]->List->Elts;
			j = 0;
			while	(Elts)
			{
				SortedSyms[j++] = Elts->Symbol;
				Elts = Elts->Next;
			}
			qsort(SortedSyms, BucketCount, sizeof(*SortedSyms), CmpSyms);
			for	(j = 0; j < BucketCount; j++)
			{
				jeSymbol	LocalSym;
				LocalSym.Symbol = SortedSyms[j];
				jeSymbol_Dump(&LocalSym, File);
			}
			jeRam_Free(SortedSyms);
		}
		TotalSyms += BucketCount;
		if	(MinLength > BucketCount)
			MinLength = BucketCount;
		if	(MaxLength < BucketCount)
			MaxLength = BucketCount;
		if	(BucketCount == 0)
			NumEmpty++;
	}

	if	(File)
	{
		return jeVFile_Printf(File, "Max Length: %d\nMin Length: %d\nTotal Syms: %d\nAvg Syms: %4.2f\nNum Empty: %d", MaxLength, MinLength, TotalSyms, (float)TotalSyms / (float)NUMHASHBUCKETS, NumEmpty);
	}
	else
	{
		printf("Max Length: %d\nMin Length: %d\nTotal Syms: %d\nAvg Syms: %4.2f\nNum Empty: %d", MaxLength, MinLength, TotalSyms, (float)TotalSyms / (float)NUMHASHBUCKETS, NumEmpty);
		return JE_TRUE;
	}
}
#endif

#endif
