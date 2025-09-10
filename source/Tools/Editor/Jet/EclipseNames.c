/****************************************************************************************/
/*  ECLIPSENAMES.C                                                                      */
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
#include	"symbol.h"
#include	"eclipsenames.h"

static	jeSymbol *JETCC AcquireSymbol(jeSymbol_Table *ST, jeSymbol *Qualifier, const char *Name, jeSymbol_Type Type)
{
	jeSymbol *	Symbol;

	Symbol = jeSymbol_TableFindSymbol(ST, Qualifier, Name);
	if	(Symbol)
		return Symbol;

	Symbol = jeSymbol_Create(ST, Qualifier, Name, Type);
	if	(Symbol)
	{
		jeSymbol_Destroy(&Symbol);
		Symbol = jeSymbol_TableFindSymbol(ST, Qualifier, Name);
	}
	return Symbol;
}

static	const char *Names[] =
{
	"StructureFields",
	"DefaultValue",
	"Types",
	"TypeDefinitions"
};

static	jeSymbol_Type Types[] =
{
	JE_SYMBOL_TYPE_LIST,
	JE_SYMBOL_TYPE_VOID,
	JE_SYMBOL_TYPE_VOID,
	JE_SYMBOL_TYPE_LIST,
};

jeSymbol *jeEclipseNames(jeSymbol_Table *ST, jeEclipseNames_Id Id)
{
	jeSymbol *	Qualifier;

	Qualifier = AcquireSymbol(ST, NULL, "Eclipse", JE_SYMBOL_TYPE_VOID);
	if	(!Qualifier)
		return NULL;

	return AcquireSymbol(ST, Qualifier, Names[Id], Types[Id]);
}

