/****************************************************************************************/
/*  ASMXFORM3D.C                                                                        */
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
//	original custom build command line
//	_DEBUG
//"$(INTELC)" /c /Zl /Zi /Fo".\$(IntDir)\$(InputName).obj" /I support /MTd "$(InputPath)"
//	NDEBUG
//"$(INTELC)" /c /Zl /Fo".\$(IntDir)\$(InputName).obj" /I support /MT "$(InputPath)"


#include "asmxform3d.h"
#include <assert.h>

//========================================================================================
//	jeXForm3d_TransformVecArrayKatmai
//	Katmai asm version 
//	cb added prefetches
//========================================================================================
void JETCC jeXForm3d_TransformVecArrayKatmai(const jeXForm3d *XForm, const jeVec3d *Source, jeVec3d *Dest, int32 Count)
{
	assert( XForm != NULL );
	assert( Source != NULL );
	assert( Dest != NULL );

	if (Count <= 0)								// Early out if possible
		return;

	_asm
	 {
		mov			ecx, Count						// number of jeVec3d's to process
		test		ecx, ecx
		jz			Done

		mov			esi, Source						// pointer to Source
		mov			edi, Dest						// pointer to Dest
		mov			eax, XForm						// pointer to matrix
		
		prefetchnta	[eax]
		prefetchnta	[esi]
		prefetchnta	[edi]
													// |  High |       |       |  Low  |
		movups		xmm0, [eax]						// |  pad  | M->AZ | M->AY | M->AX | use movaps if 16-byte aligned
		movups		xmm1, [eax+16]					// |  pad  | M->BZ | M->BY | M->BX |
		movups		xmm2, [eax+32]					// |  pad  | M->CZ | M->CY | M->CX |

	TransformArrayLoop:
		prefetchnta	[edi]

		movups		xmm4, [esi]						// |  pad  | VL.Z  | VL.Y  | VL.X  |
		movups		xmm3, [eax+48]					// |  pad  | M->TZ | M->TY | M->TX |
		movaps		xmm5, xmm4						// |  pad  | VL.Z  | VL.Y  | VL.X  |
		movaps		xmm6, xmm4						// |  pad  | VL.Z  | VL.Y  | VL.X  |

		add			esi, 16							// source buffer is x, y, z, pad

		mulps		xmm4, xmm0						// |  ???  | VL.Z * M->AZ | VL.Y * M->AY | VL.X * M->AX |
		mulps		xmm5, xmm1						// |  ???  | VL.Z * M->BZ | VL.Y * M->BY | VL.X * M->BX |
		mulps		xmm6, xmm2						// |  ???  | VL.Z * M->CZ | VL.Y * M->CY | VL.X * M->CX |

		prefetchnta	[esi]

		movaps		xmm7, xmm4						// |  ???  | VL.Z * M->AZ | VL.Y * M->AY | VL.X * M->AX |
		shufps		xmm4, xmm4, 0x39   // (00 11 10 01)| VL.X * M->AX |  ???  | VL.Z * M->AZ | VL.Y * M->AY |
		addss		xmm7, xmm4						// ...|(VL.X * M->AX)+(VL.Y * M->AY)|
		shufps		xmm4, xmm4, 0x39   // (00 11 10 01)| VL.Y * M->AY | VL.X * M->AX |  ???  | VL.Z * M->AZ |
		addss		xmm7, xmm4						// ...|(VL.X * M->AX)+(VL.Y * M->AY)+(VL.Z * M->AZ)|
		addss		xmm7, xmm3						// ...|(VL.X * M->AX)+(VL.Y * M->AY)+(VL.Z * M->AZ)+M->Translation.X|
		movss		[edi], xmm7

		shufps		xmm3, xmm3, 0x39				// | M->TX |  pad  | M->TZ | M->TY |
		movaps		xmm7, xmm5						// |  ???  | VL.Z * M->BZ | VL.Y * M->BY | VL.X * M->BX |
		shufps		xmm5, xmm5, 0x39   // (00 11 10 01)| VL.X * M->BX |  ???  | VL.Z * M->BZ | VL.Y * M->BY |
		addss		xmm7, xmm5						// ...|(VL.X * M->BX)+(VL.Y * M->BY)|
		shufps		xmm5, xmm5, 0x39   // (00 11 10 01)| VL.Y * M->BY | VL.X * M->BX |  ???  | VL.Z * M->BZ |
		addss		xmm7, xmm5						// ...|(VL.X * M->BX)+(VL.Y * M->BY)+(VL.Z * M->BZ)|
		addss		xmm7, xmm3						// ...|(VL.X * M->BX)+(VL.Y * M->BY)+(VL.Z * M->BZ)+M->Translation.Y|
		movss		[edi+4], xmm7

		shufps		xmm3, xmm3, 0x39				// | M->TY | M->TX |  pad  | M->TZ |
		movaps		xmm7, xmm6						// |  ???  | VL.Z * M->CZ | VL.Y * M->CY | VL.X * M->CX |
		shufps		xmm6, xmm6, 0x39   // (00 11 10 01)| VL.X * M->CX |  ???  | VL.Z * M->CZ | VL.Y * M->CY |
		addss		xmm7, xmm6						// ...|(VL.X * M->CX)+(VL.Y * M->CY)|
		shufps		xmm6, xmm6, 0x39   // (00 11 10 01)| VL.Y * M->CY | VL.X * M->CX |  ???  | VL.Z * M->CZ |
		addss		xmm7, xmm6						// ...|(VL.X * M->CX)+(VL.Y * M->CY)+(VL.Z * M->CZ)|
		addss		xmm7, xmm3						// ...|(VL.X * M->CX)+(VL.Y * M->CY)+(VL.Z * M->CZ)+M->Translation.Y|
		movss		[edi+8], xmm7
		
		add			edi, 16							// dest buffer is x, y, z, pad

		dec			ecx
		jnz			TransformArrayLoop

Done:
	}
}

//========================================================================================
//	jeXForm3d_TransformArrayKatmai  Strides must be aligned!
//	Katmai asm version 
//========================================================================================
void JETCC jeXForm3d_TransformArrayKatmai(const jeXForm3d *XForm,
										   const jeVec3d *Source,
										   jeVec3d *Dest,
										   int32 SourceStride,
										   int32 DestStride,
										   int32 Count)
{
	assert( XForm != NULL );
	assert( Source != NULL );
	assert( Dest != NULL );
	assert(SourceStride >= 16);
	assert(DestStride >= 16);
	assert(!(SourceStride & 0xF));
	assert(!(DestStride & 0xF));

	if (Count <= 0)								// Early out if possible
		return;

	_asm
	 {
		mov			ecx, Count						// number of jeVec3d's to process
		test		ecx, ecx
		jz			Done

		mov			esi, Source						// pointer to Source
		mov			edi, Dest						// pointer to Dest
		mov			eax, XForm						// pointer to matrix
		
		prefetchnta	[esi]						
													// |  High |       |       |  Low  |
		movups		xmm0, [eax]						// |  pad  | M->AZ | M->AY | M->AX | use movaps if 16-byte aligned
		movups		xmm1, [eax+16]					// |  pad  | M->BZ | M->BY | M->BX |
		movups		xmm2, [eax+32]					// |  pad  | M->CZ | M->CY | M->CX |

	TransformArrayLoop:
		prefetchnta	[edi]

		movups		xmm4, [esi]						// |  pad  | VL.Z  | VL.Y  | VL.X  |
		movups		xmm3, [eax+48]					// |  pad  | M->TZ | M->TY | M->TX |
		movaps		xmm5, xmm4						// |  pad  | VL.Z  | VL.Y  | VL.X  |
		movaps		xmm6, xmm4						// |  pad  | VL.Z  | VL.Y  | VL.X  |

		mulps		xmm4, xmm0						// |  ???  | VL.Z * M->AZ | VL.Y * M->AY | VL.X * M->AX |
		mulps		xmm5, xmm1						// |  ???  | VL.Z * M->BZ | VL.Y * M->BY | VL.X * M->BX |
		mulps		xmm6, xmm2						// |  ???  | VL.Z * M->CZ | VL.Y * M->CY | VL.X * M->CX |

		movaps		xmm7, xmm4						// |  ???  | VL.Z * M->AZ | VL.Y * M->AY | VL.X * M->AX |
		shufps		xmm4, xmm4, 0x39   // (00 11 10 01)| VL.X * M->AX |  ???  | VL.Z * M->AZ | VL.Y * M->AY |
		addss		xmm7, xmm4						// ...|(VL.X * M->AX)+(VL.Y * M->AY)|
		shufps		xmm4, xmm4, 0x39   // (00 11 10 01)| VL.Y * M->AY | VL.X * M->AX |  ???  | VL.Z * M->AZ |
		addss		xmm7, xmm4						// ...|(VL.X * M->AX)+(VL.Y * M->AY)+(VL.Z * M->AZ)|
		addss		xmm7, xmm3						// ...|(VL.X * M->AX)+(VL.Y * M->AY)+(VL.Z * M->AZ)+M->Translation.X|
		movss		[edi], xmm7

		add			esi, SourceStride

		shufps		xmm3, xmm3, 0x39				// | M->TX |  pad  | M->TZ | M->TY |
		movaps		xmm7, xmm5						// |  ???  | VL.Z * M->BZ | VL.Y * M->BY | VL.X * M->BX |
		shufps		xmm5, xmm5, 0x39   // (00 11 10 01)| VL.X * M->BX |  ???  | VL.Z * M->BZ | VL.Y * M->BY |
		addss		xmm7, xmm5						// ...|(VL.X * M->BX)+(VL.Y * M->BY)|
		shufps		xmm5, xmm5, 0x39   // (00 11 10 01)| VL.Y * M->BY | VL.X * M->BX |  ???  | VL.Z * M->BZ |
		addss		xmm7, xmm5						// ...|(VL.X * M->BX)+(VL.Y * M->BY)+(VL.Z * M->BZ)|
		addss		xmm7, xmm3						// ...|(VL.X * M->BX)+(VL.Y * M->BY)+(VL.Z * M->BZ)+M->Translation.Y|
		movss		[edi+4], xmm7

		prefetchnta	[esi]

		shufps		xmm3, xmm3, 0x39				// | M->TY | M->TX |  pad  | M->TZ |
		movaps		xmm7, xmm6						// |  ???  | VL.Z * M->CZ | VL.Y * M->CY | VL.X * M->CX |
		shufps		xmm6, xmm6, 0x39   // (00 11 10 01)| VL.X * M->CX |  ???  | VL.Z * M->CZ | VL.Y * M->CY |
		addss		xmm7, xmm6						// ...|(VL.X * M->CX)+(VL.Y * M->CY)|
		shufps		xmm6, xmm6, 0x39   // (00 11 10 01)| VL.Y * M->CY | VL.X * M->CX |  ???  | VL.Z * M->CZ |
		addss		xmm7, xmm6						// ...|(VL.X * M->CX)+(VL.Y * M->CY)+(VL.Z * M->CZ)|
		addss		xmm7, xmm3						// ...|(VL.X * M->CX)+(VL.Y * M->CY)+(VL.Z * M->CZ)+M->Translation.Y|
		movss		[edi+8], xmm7
		
		add			edi, DestStride

		dec			ecx
		jnz			TransformArrayLoop

Done:
	}
}

//========================================================================================
//	jeXForm3d_TransformArrayX86  Strides should be aligned
//	Assembly version 
//========================================================================================
void JETCC jeXForm3d_TransformArrayX86(const jeXForm3d *XForm,
										const jeVec3d *Source,
										jeVec3d *Dest,
										int32 SourceStride,
										int32 DestStride,
										int32 Count)
{
	#define FSIZE	4

	assert( XForm != NULL );
	assert( Source != NULL );
	assert( Dest != NULL );
	assert(SourceStride >= 16);
	assert(DestStride >= 16);
	assert(!(SourceStride & 0xF));
	assert(!(DestStride & 0xF));

	if (Count <= 0)								// Early out if possible
		return;

	_asm 
	{
	mov     ecx,Count							// get item count
	mov		esi,DestStride
	mov		edi,SourceStride
	mov		eax,Count
	imul	ecx,esi
	mov     esi,Source							// get source array pointer
	mov     ebx,Dest							// get dest array pointer
	imul	eax,edi
	mov     edi,XForm							// point to matrix
	add     esi,eax								// esi points to source end
	add     ebx,ecx								// edi pointe to dest end
	neg     eax
	neg		ecx
          
Again:	
	// Multiply
	fld   dword ptr [esi+eax+0*FSIZE]			// 1;i1
	fmul  dword ptr [edi+(0+0*4)*FSIZE]			// 1;m11
	fld   dword ptr [esi+eax+1*FSIZE]			// 1;m11 i2
	fmul  dword ptr [edi+(1+0*4)*FSIZE]			// 1;m11 m12
	fld   dword ptr [esi+eax+2*FSIZE]			// 1;m11 m12 i3
	fmul  dword ptr [edi+(2+0*4)*FSIZE]			// 1;m11 m12 m13
	fxch  st(1)									// 0;m11 m13 m12
	faddp st(2),st								// 1;s1a m13
	fld   dword ptr [esi+eax+0*FSIZE]			// 1;s1a m13 i1
	fmul  dword ptr [edi+(0+1*4)*FSIZE]			// 1;s1a m13 m21
	fxch  st(1)									// 0;s1a m21 m13
	faddp st(2),st								// 1;s1b m21
	fld   dword ptr [esi+eax+1*FSIZE]			// 1;s1b m21 i2
	fmul  dword ptr [edi+(1+1*4)*FSIZE]			// 1;s1b m21 m22
	fld   dword ptr [esi+eax+2*FSIZE]			// 1;s1b m21 m22 i3
	fmul  dword ptr [edi+(2+1*4)*FSIZE]			// 1;s1b m21 m22 m23
	fxch  st(1)									// 0;s1b m21 m23 m22
	faddp st(2),st								// 1;s1b s2a m23
	fld   dword ptr [esi+eax+0*FSIZE]			// 1;s1b s2a m23 i1
	fmul  dword ptr [edi+(0+2*4)*FSIZE]			// 1;s1b s2a m23 m31
	fxch  st(1)									// 0;s1b s2a m31 m23
	faddp st(2),st								// 1;s1b s2b m31
	fld   dword ptr [esi+eax+1*FSIZE]			// 1;s1b s2b m31 i2
	fmul  dword ptr [edi+(1+2*4)*FSIZE]			// 1;s1b s2b m31 m32
	fld   dword ptr [esi+eax+2*FSIZE]			// 1;s1b s2b m31 m32 i3
	fmul  dword ptr [edi+(2+2*4)*FSIZE]			// 1;s1b s2b m31 m32 m33
	// Add translation
	fxch  st(1)									// 0;s1b s2b m31 m33 m32
	faddp st(2),st								// 1;s1b s2b s3a m33
	fxch  st(3)									// 0;m33 s2b s3a s1b
	fadd  dword ptr [edi+(12+0)*FSIZE]			// 1;m33 s2b s3a s1c
	fxch  st(1)									// 0;m33 s2b s1c s3a 
	faddp st(3),st								// 1;s3b s2b s1c
	fxch  st(1)									// 0;s3b s1c s2b
	fadd  dword ptr [edi+(12+1)*FSIZE]			// 1;s3b s1c s2c
	fxch  st(2)									// 0;s2c s1c s3b
	fadd  dword ptr [edi+(12+2)*FSIZE]			// 1;s2c s1c s3c
	fxch  st(1)									// 0;s2c s3c s1c
	fstp  dword ptr [ebx+ecx+0*FSIZE]			// 2;s2c s3c    
	fxch  st(1)									// 0;s3c s2c    
	fstp  dword ptr [ebx+ecx+1*FSIZE]			// 2;s3c
	fstp  dword ptr [ebx+ecx+2*FSIZE]			// 2;
	add	  eax,SourceStride
	add   ecx,DestStride

	cmp ecx, 0
	jne Again
	}
}

//========================================================================================
//	jeXForm3d_TransformVecArrayX86
//	Assembly version 
//========================================================================================
void JETCC jeXForm3d_TransformVecArrayX86(const jeXForm3d *XForm, const jeVec3d *Source, jeVec3d *Dest, int32 Count)
{
	#define FSIZE	4

	assert( XForm != NULL );
	assert( Source != NULL );
	assert( Dest != NULL );

	if (Count <= 0)								// Early out if possible
		return;

	_asm 
	{
	mov     ecx,Count							// get item count
	mov     esi,Source							// get source array pointer
	mov     ebx,Dest							// get dest array pointer
	mov     edi,XForm							// point to matrix
	imul    ecx,ecx,4*FSIZE						// ecx is size of array
	add     esi,ecx								// esi points to source end
	add     ebx,ecx								// edi pointe to dest end
	neg     ecx									// ecx ready for count-up
          
Again:	
	// Multiply
	fld   dword ptr [esi+ecx+0*FSIZE]			// 1;i1
	fmul  dword ptr [edi+(0+0*4)*FSIZE]			// 1;m11
	fld   dword ptr [esi+ecx+1*FSIZE]			// 1;m11 i2
	fmul  dword ptr [edi+(1+0*4)*FSIZE]			// 1;m11 m12
	fld   dword ptr [esi+ecx+2*FSIZE]			// 1;m11 m12 i3
	fmul  dword ptr [edi+(2+0*4)*FSIZE]			// 1;m11 m12 m13
	fxch  st(1)									// 0;m11 m13 m12
	faddp st(2),st								// 1;s1a m13
	fld   dword ptr [esi+ecx+0*FSIZE]			// 1;s1a m13 i1
	fmul  dword ptr [edi+(0+1*4)*FSIZE]			// 1;s1a m13 m21
	fxch  st(1)									// 0;s1a m21 m13
	faddp st(2),st								// 1;s1b m21
	fld   dword ptr [esi+ecx+1*FSIZE]			// 1;s1b m21 i2
	fmul  dword ptr [edi+(1+1*4)*FSIZE]			// 1;s1b m21 m22
	fld   dword ptr [esi+ecx+2*FSIZE]			// 1;s1b m21 m22 i3
	fmul  dword ptr [edi+(2+1*4)*FSIZE]			// 1;s1b m21 m22 m23
	fxch  st(1)									// 0;s1b m21 m23 m22
	faddp st(2),st								// 1;s1b s2a m23
	fld   dword ptr [esi+ecx+0*FSIZE]			// 1;s1b s2a m23 i1
	fmul  dword ptr [edi+(0+2*4)*FSIZE]			// 1;s1b s2a m23 m31
	fxch  st(1)									// 0;s1b s2a m31 m23
	faddp st(2),st								// 1;s1b s2b m31
	fld   dword ptr [esi+ecx+1*FSIZE]			// 1;s1b s2b m31 i2
	fmul  dword ptr [edi+(1+2*4)*FSIZE]			// 1;s1b s2b m31 m32
	fld   dword ptr [esi+ecx+2*FSIZE]			// 1;s1b s2b m31 m32 i3
	fmul  dword ptr [edi+(2+2*4)*FSIZE]			// 1;s1b s2b m31 m32 m33
	// Add translation
	fxch  st(1)									// 0;s1b s2b m31 m33 m32
	faddp st(2),st								// 1;s1b s2b s3a m33
	fxch  st(3)									// 0;m33 s2b s3a s1b
	fadd  dword ptr [edi+(12+0)*FSIZE]			// 1;m33 s2b s3a s1c
	fxch  st(1)									// 0;m33 s2b s1c s3a 
	faddp st(3),st								// 1;s3b s2b s1c
	fxch  st(1)									// 0;s3b s1c s2b
	fadd  dword ptr [edi+(12+1)*FSIZE]			// 1;s3b s1c s2c
	fxch  st(2)									// 0;s2c s1c s3b
	fadd  dword ptr [edi+(12+2)*FSIZE]			// 1;s2c s1c s3c
	fxch  st(1)									// 0;s2c s3c s1c
	fstp  dword ptr [ebx+ecx+0*FSIZE]			// 2;s2c s3c    
	fxch  st(1)									// 0;s3c s2c    
	fstp  dword ptr [ebx+ecx+1*FSIZE]			// 2;s3c
	fstp  dword ptr [ebx+ecx+2*FSIZE]			// 2;
	add   ecx,4*FSIZE							// 1;

	cmp ecx, 0
	jne Again
	}

	// 34 cycles predicted (per loop)
	// 39 cycles measured
}

#if 0	//thought intel compiler did these but it doesn't
//========================================================================================
//	jeXForm3d_TransformVecArray3DNow
//	Assembly version 
//========================================================================================
void JETCC jeXForm3d_TransformVecArray3DNow(const jeXForm3d *XForm, const jeVec3d *Source, jeVec3d *Dest, int32 Count)
{
	assert( XForm != NULL );
	assert( Source != NULL );
	assert( Dest != NULL );

	if (Count <= 0)								// Early out if possible
		return;

	_asm 
	{
		mov     ecx,Count							// get item count
		mov     esi,Source							// get source array pointer
		mov     ebx,Dest							// get dest array pointer
		mov     edi,XForm							// point to matrix

		femms

LVector_Array_Transform4_Loop_Next:

		movq		mm0,[esi]		; ay, ax	in mm0
		movq		mm1,[esi+8]		; aw, az	in mm1

		movq		mm4,[edi]		; m01, m00      in mm4
		punpckhdq	mm2,mm0			; ay, undef	in mm2

		movq		mm5,[edi+16]	; m11, m10      in mm5
		punpckldq	mm0,	mm0		; ax, ax	in mm2

		movq		mm6,    [edi+32]; m21, m20      in mm6
		pfmul		mm4,	mm0		; ax*m01, ax*m00	in mm4

		movq		mm7,    [edi+48]; m31, m30      in mm7
		punpckhdq	mm2,	mm2		; ay, ay	in mm2

		punpckhdq	mm3,	mm1		; aw, undef	in mm3
		pfmul		mm5,	mm2		; ay*m11, ay*m10

		punpckldq	mm1,	mm1		; az, az	in mm3
		pfmul		mm0,    [edi+8]	; ax*m03, ax*m02	in mm0

		punpckhdq	mm3,	mm3		; aw, aw	in mm3
		pfmul		mm2,    [edi+24]; ay*13, ay*m12	in mm2

		pfmul		mm6,	mm1		; az*m21, az*m20	in mm6
		pfadd		mm5,	mm4

		pfmul		mm1,    [edi+40]; az*23, az*22	in mm1
		pfadd		mm2,	mm0

		pfmul		mm7,	mm3		; aw*m31, aw*m30	in mm7
		pfadd		mm6,	mm5

		pfmul		mm3,    [edi+56]; aw*m33, aw*m32	in mm3
		pfadd		mm2,	mm1

		pfadd		mm7,	mm6		; ry, rx	in mm7
		pfadd		mm3,	mm2		; rw,rz		in mm3

		add			esi, 16			; 00000090H
		movq		[ebx],	 mm7

		movq		[ebx+8], mm3
		add			ebx, 16			; 00000090H

		dec			ecx
		jne			LVector_Array_Transform4_Loop_Next

		femms
	}
}
//========================================================================================
//	jeXForm3d_TransformArray3DNow
//	Assembly version 
//========================================================================================
void JETCC jeXForm3d_TransformArray3DNow(const jeXForm3d *XForm,
										  const jeVec3d *Source,
										  jeVec3d *Dest,
										  int32 SourceStride,
										  int32 DestStride,
										  int32 Count)
{
	assert( XForm != NULL );
	assert( Source != NULL );
	assert( Dest != NULL );
	assert(SourceStride >= 16);
	assert(DestStride >= 16);
	assert(!(SourceStride & 0xF));
	assert(!(DestStride & 0xF));

	if (Count <= 0)								// Early out if possible
		return;

	_asm 
	{
		mov     ecx,Count							// get item count
		mov     esi,Source							// get source array pointer
		mov     ebx,Dest							// get dest array pointer
		mov     edi,XForm							// point to matrix

		femms

LVector_Array_Transform4_Loop_Next:

		movq		mm0,[esi]		; ay, ax	in mm0
		movq		mm1,[esi+8]		; aw, az	in mm1

		movq		mm4,[edi]		; m01, m00      in mm4
		punpckhdq	mm2,mm0			; ay, undef	in mm2

		movq		mm5,[edi+16]	; m11, m10      in mm5
		punpckldq	mm0,	mm0		; ax, ax	in mm2

		movq		mm6,    [edi+32]; m21, m20      in mm6
		pfmul		mm4,	mm0		; ax*m01, ax*m00	in mm4

		movq		mm7,    [edi+48]; m31, m30      in mm7
		punpckhdq	mm2,	mm2		; ay, ay	in mm2

		punpckhdq	mm3,	mm1		; aw, undef	in mm3
		pfmul		mm5,	mm2		; ay*m11, ay*m10

		punpckldq	mm1,	mm1		; az, az	in mm3
		pfmul		mm0,    [edi+8]	; ax*m03, ax*m02	in mm0

		punpckhdq	mm3,	mm3		; aw, aw	in mm3
		pfmul		mm2,    [edi+24]; ay*13, ay*m12	in mm2

		pfmul		mm6,	mm1		; az*m21, az*m20	in mm6
		pfadd		mm5,	mm4

		pfmul		mm1,    [edi+40]; az*23, az*22	in mm1
		pfadd		mm2,	mm0

		pfmul		mm7,	mm3		; aw*m31, aw*m30	in mm7
		pfadd		mm6,	mm5

		pfmul		mm3,    [edi+56]; aw*m33, aw*m32	in mm3
		pfadd		mm2,	mm1

		pfadd		mm7,	mm6		; ry, rx	in mm7
		pfadd		mm3,	mm2		; rw,rz		in mm3

		add			esi,SourceStride			; 00000090H
		movq		[ebx],	 mm7

		movq		[ebx+8], mm3
		add			ebx,DestStride			; 00000090H

		dec			ecx
		jne			LVector_Array_Transform4_Loop_Next

		femms
	}
}
#endif
