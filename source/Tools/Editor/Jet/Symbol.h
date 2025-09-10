/****************************************************************************************/
/*  SYMBOL.H                                                                            */
/*                                                                                      */
/*  Author: Eli Boling                                                                  */
/*  Description: Generic symbol table/property list interface                           */
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
#ifndef	SYMBOL_H
#define	SYMBOL_H

#include	"basetype.h"
#include	"vfile.h"

#ifdef	__cplusplus
extern "C" {
#endif

//--------------------------------
// Types
//--------------------------------

typedef	enum
{
	JE_SYMBOL_TYPE_INT,				// Basic integer type
	JE_SYMBOL_TYPE_STRING,			// Basic char *
	JE_SYMBOL_TYPE_FLOAT,			// Basic float type
	JE_SYMBOL_TYPE_VEC3D,			// jeVec3d by value
	JE_SYMBOL_TYPE_COLOR,			// JE_RGBA (RGB only) by value
	JE_SYMBOL_TYPE_BOOLEAN,			// jeBoolean
	JE_SYMBOL_TYPE_ENUM,			// An enumeration
	JE_SYMBOL_TYPE_PVOID,			// pointer to void (application side only)
	JE_SYMBOL_TYPE_LIST,			// List of symbols
	JE_SYMBOL_TYPE_SYMBOL,			// Another symbol
//	JE_SYMBOL_TYPE_OBJECT,			// Special tag to distinguish object instances (hack?)

	JE_SYMBOL_TYPE_MODEL,			// Pointer to a jeModel
	JE_SYMBOL_TYPE_PORTAL,			// Pointer to a jePortal

	JE_SYMBOL_TYPE_VOID				// No type at all

}	jeSymbol_Type;

typedef	struct	jeSymbol		jeSymbol;

typedef	struct	jeSymbol_Table	jeSymbol_Table;

typedef struct	jeSymbol_List	jeSymbol_List;

//--------------------------------
// APIs
//--------------------------------

JETAPI	jeSymbol_Table * JETCC jeSymbol_TableCreate(void);
	// Creates a symbol table

//JETAPI	const jeSymbol_List *JETCC jeSymbol_TableGetQualifiedSymbolList(const jeSymbol_Table *ST, const jeSymbol *Qualifier);
JETAPI	jeSymbol_List *JETCC jeSymbol_TableGetQualifiedSymbolList(const jeSymbol_Table *ST, const jeSymbol *Qualifier);

JETAPI	void JETCC jeSymbol_TableCreateRef(jeSymbol_Table *ST);
	// Refs a symbol table.

JETAPI	jeSymbol_Table *JETCC jeSymbol_TableCreateFromFile(jeVFile *File);
	// Reloads a symbol table from a file.

JETAPI	jeBoolean JETCC jeSymbol_TableWriteToFile(const jeSymbol_Table *ST, jeVFile *File);
	// Writes a symbol table to a file

JETAPI	void JETCC jeSymbol_TableDestroy(jeSymbol_Table **ST);
	// Destroys the symbol table

JETAPI	jeSymbol *JETCC jeSymbol_TableFindSymbol(jeSymbol_Table *ST, jeSymbol *Qualifier, const char *Name);
	// Finds the given symbol in the symbol table.  The resulting symbol, if found, is NOT
	// referenced.

JETAPI	jeSymbol *JETCC jeSymbol_Create(jeSymbol_Table *ST, jeSymbol *Qualifier, const char *Name, jeSymbol_Type Type);
	// Creates a symbol with the given qualifier in the given symbol table.  The symbol type
	// is primarily used to distinguish symbols that are going to be used as properties, so
	// that we can reliable retrieve the data from the property.

JETAPI	void JETCC jeSymbol_CreateRef(jeSymbol *S, jeSymbol **Result);
	// References a symbol.

JETAPI	void JETCC jeSymbol_Destroy(jeSymbol **S);
	// Destroys a symbol.

JETAPI	void JETCC jeSymbol_TableCollectGarbage(jeSymbol_Table *ST);
	// Sweeps for orphaned symbols, and reclaims the lost memory (garbage collects).

JETAPI	jeBoolean JETCC jeSymbol_Compare(const jeSymbol *S1, const jeSymbol *S2);
	// Compares two symbols to see if they are the same.  You must use this function,
	// since you are essentially guaranteed that references to the same symbol will always
	// be different pointers, even though the same symbols occupy the same storage.  This
	// is due to the reference model that we are keeping for the symbol table.

JETAPI	jeBoolean JETCC jeSymbol_Rename(jeSymbol *Symbol, const char *NewName);
	// Renames a symbol,  Returns JE_TRUE on success, JE_FALSE on failure.

JETAPI	jeSymbol *JETCC jeSymbol_TableGetSymbol(jeSymbol_Table *ST, jeSymbol *Qualifier, const char *Name);
	// Finds a symbol by name with a given qualifier from the given symbol table.

JETAPI	jeBoolean JETCC jeSymbol_TableRemoveSymbol(jeSymbol_Table *ST, jeSymbol *Symbol);
	// Removes a symbol from the symbol table

JETAPI	jeSymbol_Type JETCC jeSymbol_GetType(const jeSymbol *Sym);
	// Gets the type of a symbol.

JETAPI	jeSymbol *JETCC jeSymbol_GetQualifier(const jeSymbol *Sym);
	// Gets the qualifier of a symbol.

JETAPI	jeBoolean JETCC jeSymbol_SetProperty(jeSymbol *S, jeSymbol *Property, const void *Data, int DataLength, jeSymbol_Type Type);
	// Sets a property on a symbol.  The type of the Property symbol must match the passed type,
	// or this API will fail.

JETAPI	jeBoolean JETCC jeSymbol_GetProperty(
	jeSymbol *	Sym,
	jeSymbol *	Property,
	void *				Data,
	int 				DataLength,
	jeSymbol_Type 		Type);
	// Sets a property from a symbol.  The type of the Property symbol must match the passed type,
	// or this API will fail.

JETAPI	jeBoolean JETCC jeSymbol_CopyProperty(
	jeSymbol *Dest,
	jeSymbol *DestProp,
	jeSymbol *Src,
	jeSymbol *SrcProp);
	// Copies the property data from one property field to another.  The values are
	// copied directly, so if you are copying a list, you get a reference count bump
	// on the list, not a true copy of the list, so be careful here.

JETAPI	jeBoolean JETCC jeSymbol_GetEnumValue(const jeSymbol *S, int *Value);
	// Fast mechanism for getting the numeric enumeration value for a symbol.
JETAPI	jeBoolean JETCC jeSymbol_SetEnumValue(jeSymbol *S, int Value);
	// Fast mechanism for setting the numeric enumeration value for a symbol.
	
JETAPI	const char *JETCC jeSymbol_GetName(const jeSymbol *S);
	// Gets the name for the symbol.  You can hold this pointer for as long as
	// the symbol exists, but you must not alter it under any circumstances.

JETAPI	jeBoolean	JETCC jeSymbol_GetFullName(const jeSymbol *S, char *Buff, int MaxLen);
	// Gets the qualified name of a symbol.  This is mostly as a debugging/informational aid.

JETAPI	jeSymbol_List *	JETCC jeSymbol_ListCreate(jeSymbol_Table *ST);
	// Creates a symbol list.  Symbol lists that are set as property values on a symbol
	// will be written to file and streamed back in automatically by a symbol table.

JETAPI	void JETCC jeSymbol_ListCreateRef(jeSymbol_List *L, jeSymbol_List **Result);
	// References a symbol list.

JETAPI	void JETCC jeSymbol_ListDestroy(jeSymbol_List **L);
	// Destroys a symbol list.

//jeSymbol_List *jeSymbol_ListNext(const jeSymbol_List *L);
	// Iterates a symbol list.

JETAPI	jeSymbol *JETCC jeSymbol_ListGetSymbol(const jeSymbol_List *L, int Index);
	// Gets the current symbol from this list.

JETAPI	jeBoolean JETCC jeSymbol_ListAddSymbol(jeSymbol_List *L, jeSymbol *S);
	// Adds a symbol to the symbol list.

JETAPI	void JETCC jeSymbol_ListRemoveSymbol(jeSymbol_List *L, jeSymbol *S);
	// Removes a symbol from the given symbol list.

#if 0
// Pseudo code for what an editor might do to build classes:

typedef struct	FieldDefs
{
	char *			Name;
	jeSymbol_Type	Type;
	char *			DefaultValue;
}	FieldDefs;

static	FieldDefs	CoronaFields[] =
{
	{ "MinRadius", 	JE_SYMBOL_TYPE_INT, 	"20" },
	{ "MaxRadius", 	JE_SYMBOL_TYPE_INT, 	"40" },
	{ "FadeTime", 	JE_SYMBOL_TYPE_FLOAT, 	"0.5" },
	{ "Color", 		JE_SYMBOL_TYPE_COLOR, 	"255 255 255" },
	{ "Origin", 	JE_SYMBOL_TYPE_VEC3D, 	"0 0 0" },
};

jeSymbol *	CreateType(jeSymbol_Table *ST, const char *Name, const FieldDefs *Fields, int FieldCount)
{
	jeSymbol *			TypeSym;
	jeSymbol *			Qualifier;
	jeSymbol *			FieldSym;
	jeSymbol_List *		FieldList;
	int					i;

	// Create a qualifier for the type, and intern the type symbol in that package
	Qualifier = jeSymbol_Create(ST, NULL, Name, JE_SYMBOL_TYPE_VOID);
	TypeSym = jeSymbol_Create(ST, Qualifier, Name, JE_SYMBOL_TYPE_VOID);
	jeSymbol_Destroy(&Qualifier);

	// Create a symbol list to hold the field definition symbols.
	FieldList = jeSymbol_ListCreate();

	/*
		Rip through the fields, creating symbols in a list for each field name, and
		setting the default value property on each symbol.  The default value property
		will be used to initialize new entities that are added to the world.
	*/
	for	(i = 0; i < FieldCount; i++)
	{
		// Create and symbol for the current field.  Use the type symbol for the package for
		// namespace control.
		FieldSym = jeSymbol_Create(ST, TypeSym, Fields[i].Name, Fields[i].Type);
		switch	(Fields[i].Type)
		{
			int		Integer;
			gefloat	Float;
			jeVec3d	Vector;
			JE_RGBA	Color;

		case	JE_SYMBOL_TYPE_INT:
			Integer = atoi(Fields[i].DefaultValue);
			jeSymbol_SetProperty(FieldSym, 
									jeEclipseNames(ST, JE_ECLIPSENAMES_FIELDDEFAULTVALUE),
									&Integer, sizeof(Integer), JE_SYMBOL_TYPE_INT);
			break;

		case	JE_SYMBOL_TYPE_FLOAT:
			Integer = atof(Fields[i].DefaultValue);
			jeSymbol_SetProperty(FieldSym, 
									jeEclipseNames(ST, JE_ECLIPSENAMES_FIELDDEFAULTVALUE),
									&Float, sizeof(Float), JE_SYMBOL_TYPE_FLOAT);

		case	JE_SYMBOL_TYPE_COLOR:
			sscanf(Fields[i].DefaultValue, "%f %f %f", &Color.r, &Color.g, &Color.b);
			jeSymbol_SetProperty(FieldSym, 
									jeEclipseNames(ST, JE_ECLIPSENAMES_FIELDDEFAULTVALUE),
									&COlor, sizeof(Color), JE_SYMBOL_TYPE_COLOR);
			break;

		case	JE_SYMBOL_TYPE_VEC3D:
			sscanf(Fields[i].DefaultValue, "%f %f %f", &Vector.X, &Vector.Y, &Vector.Z);
			jeSymbol_SetProperty(FieldSym, 
									jeEclipseNames(ST, JE_ECLIPSENAMES_FIELDDEFAULTVALUE),
									&Vector, sizeof(Vector), JE_SYMBOL_TYPE_VEC3D);
			break;

		etc...
		}
		FieldList = jeSymbol_ListAddSymbol(FieldList, FieldSym);
	}
	jeSymbol_SetProperty(TypeSym,
							jeEclipseNames(ST, JE_ECLIPSENAMES_STRUCTUREFIELDS),
							&FieldList, sizeof(FieldList), JE_SYMBOL_TYPE_LIST);

	return TypeSym;
}

// Sample code for adding an entity as we know it to the world
AddEntity(jeSymbol_Table *ST, jeSymbol *Type, const char *Name)
{
	jeSymbol *		Entity;
	jeSymbol_List *	List;

	Entity = jeSymbol_Create(ST, jeSymbol_GetQualifier(Type), Name, JE_SYMBOL_TYPE_OBJECT);
	jeSymbol_GetProperty(Type,
							jeEclipseNames(ST, JE_ECLIPSENAMES_STRUCTUREFIELDS),
							&List, sizeof(List), JE_SYMBOL_TYPE_LIST);
	
	/*
		We've got the list of fields from the type symbol.  Run through each field,
		setting the default value for each field into the new entity symbol.  Here,
		we are using not the default value symbol name for the target, but the actual
		field name, since we are dealing with an instance, not a type now.
	*/
	while	(List)
	{
		jeSymbol *	FieldSym;

		FieldSym = jeSymbol_ListGetSymbol(List);
		jeSymbol_CopyProperty(Entity,
								 FieldSym,
								 FieldSym,
								 jeEclipseNames(ST, JE_ECLIPSENAMES_FIELDDEFAULTVALUE));
		
		List = jeSymbol_ListNext(List);
	}
}

//  Then, on the application side:

InitCoronas(jeSymbol *ST)
{
	jeSymbol *		TypeName;
	jeSymbol_List *	List;
	int				Count;
	int				i;

	TypeName = jeSymbol_TableGetSymbol(ST, NULL, "Corona");
	List = jeSymbol_TableGetQualifiedSymbolList(ST, TypeName);
	while	(List);
	{
		int				MinRadius;
		jeSymbol *		Entity;

		Entity = jeSymbol_ListGetSymbol(List);
		jeSymbol_GetProperty(Entity,
								jeSymbol_TableGetSymbol(ST, TypeName, "MinRadius"),
								&MinRadius, sizeof(MinRadius), JE_SYMBOL_TYPE_INT);
		// do something with the field values, ...
		List = jeSymbol_ListNext(List);
	}
}

#endif

#ifdef	__cplusplus
}
#endif

#endif

