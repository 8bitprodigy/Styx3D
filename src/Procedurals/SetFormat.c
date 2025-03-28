#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ErrorLog.h"
#include "GeBMUtil.h"
#include "Genesis.h"
#include "RAM.h"
#include "SetFormat.h"

/**************************************************************/

Procedural *SetFormat_Create(geBitmap **ppBitmap, const char *ParmStart);
void		SetFormat_Destroy(Procedural *Proc);
geBoolean	SetFormat_Animate(Procedural *SetFormat, float ElapsedTime);

static Procedural_Table SetFormat_Table = 
{
	Procedurals_Version,Procedurals_Tag,
	"SetFormat",
	SetFormat_Create,
	SetFormat_Destroy,
	SetFormat_Animate
};

Procedural_Table * SetFormat_GetProcedural_Table(void)
{
	return &SetFormat_Table;
}

/**********************************************************/

typedef struct Procedural
{
	geBitmap * Bitmap;
} Procedural;

/****/

Procedural *SetFormat_Create(geBitmap **ppBitmap, const char *InParams)
{
Procedural * P;
gePixelFormat Format;
geBitmap_Info Info;

	if ( ! ppBitmap || ! *ppBitmap )
		return NULL;

	if ( ! InParams )
	{
		Format = GE_PIXELFORMAT_16BIT_4444_ARGB;
	}
	else
	{
		Format = atol(InParams);
		if ( ! gePixelFormat_IsValid(Format) )
			return NULL;
	}

	P = geRam_Allocate(sizeof(Procedural));
	assert(P);
	memset(P,0,sizeof(Procedural));
	
	P->Bitmap = *ppBitmap;

	geBitmap_CreateRef(P->Bitmap);

	geBitmap_GetInfo(P->Bitmap,&Info,NULL);

	if ( gePixelFormat_HasAlpha(Format) && ! gePixelFormat_HasAlpha(Info.Format) )
	{
		geBitmapUtil_SetAlphaFromBrightness(P->Bitmap);
	}

	if ( ! geBitmap_SetFormatMin(P->Bitmap,Format) )
	{
		geErrorLog_AddString(-1,"Setformat failed!",NULL);
	}

return P;
}

void SetFormat_Destroy(Procedural * P)
{
	if ( ! P )
		return;	
	if ( P->Bitmap )
		geBitmap_Destroy(&(P->Bitmap));
	geRam_Free(P);
}

geBoolean SetFormat_Animate(Procedural * P,float time)
{
return GE_TRUE;
}
