;/****************************************************************************************/
;/*  ASMXFORM3D.ASM                                                                      */
;/*                                                                                      */
;/*  Author: Christopher Plymire, asm code from asmxform3d.cpp                           */
;/*  Description: Nasm 98 'c callable' replacement code for the inline intel asm in asmxform3d.cpp                                            	                           */
;/*                                                                                      */
;/*  The contents of this file are subject to the Jet3D Public License                   */
;/*  Version 1.02 (the "License"); you may not use this file except in                   */
;/*  compliance with the License. You may obtain a copy of the License at                */
;/*  http://www.jet3d.com                                                                */
;/*                                                                                      */
;/*  Software distributed under the License is distributed on an "AS IS"                 */
;/*  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See                */
;/*  the License for the specific language governing rights and limitations              */
;/*  under the License.                                                                  */
;/*                                                                                      */
;/*  The Original Code is Jet3D, released December 12, 1999.                             */
;/*  Copyright (C) 1996-1999 Eclipse Entertainment, L.L.C. All Rights Reserved           */
;/*                                                                                      */
;/****************************************************************************************/

BITS 32

SECTION .text

; prototype: void jeXForm3d_TransformVecArrayKatmai(const jeXForm3d *XForm, const jeVec3d *Source, jeVec3d *Dest, int32 Count);
jeXForm3d_TransformVecArrayKatmai:
; variables passed from right to left i believe..

global jeXForm3d_TransformArrayKatmai
global Asm_TransVecArrayKetmai

Asm_TransVecArrayKetmai:
		prefetchnta	[eax]
		prefetchnta	[esi]
		prefetchnta	[edi]
													;// |  High |       |       |  Low  |
		movups		xmm0, [eax]					;	// |  pad  | M->AZ | M->AY | M->AX | use movaps if 16-byte aligned
		movups		xmm1, [eax+16]					;// |  pad  | M->BZ | M->BY | M->BX |
		movups		xmm2, [eax+32]					;// |  pad  | M->CZ | M->CY | M->CX |

	TransformArrayLoop:
		prefetchnta	[edi]

		movups		xmm4, [esi]						;// |  pad  | VL.Z  | VL.Y  | VL.X  |
		movups		xmm3, [eax+48]					;// |  pad  | M->TZ | M->TY | M->TX |
		movaps		xmm5, xmm4						;// |  pad  | VL.Z  | VL.Y  | VL.X  |
		movaps		xmm6, xmm4						;// |  pad  | VL.Z  | VL.Y  | VL.X  |

		add			esi, 16							;// source buffer is x, y, z, pad

		mulps		xmm4, xmm0						;// |  ???  | VL.Z * M->AZ | VL.Y * M->AY | VL.X * M->AX |
		mulps		xmm5, xmm1						;// |  ???  | VL.Z * M->BZ | VL.Y * M->BY | VL.X * M->BX |
		mulps		xmm6, xmm2						;// |  ???  | VL.Z * M->CZ | VL.Y * M->CY | VL.X * M->CX |

		prefetchnta	[esi]

		movaps		xmm7, xmm4						;// |  ???  | VL.Z * M->AZ | VL.Y * M->AY | VL.X * M->AX |
		shufps		xmm4, xmm4, 0x39  ; // (00 11 10 01)| VL.X * M->AX |  ???  | VL.Z * M->AZ | VL.Y * M->AY |
		addss		xmm7, xmm4	;					// ...|(VL.X * M->AX)+(VL.Y * M->AY)|
		shufps		xmm4, xmm4, 0x39   ;// (00 11 10 01)| VL.Y * M->AY | VL.X * M->AX |  ???  | VL.Z * M->AZ |
		addss		xmm7, xmm4	;					// ...|(VL.X * M->AX)+(VL.Y * M->AY)+(VL.Z * M->AZ)|
		addss		xmm7, xmm3	;					// ...|(VL.X * M->AX)+(VL.Y * M->AY)+(VL.Z * M->AZ)+M->Translation.X|
		movss		[edi], xmm7

		shufps		xmm3, xmm3, 0x39	;			// | M->TX |  pad  | M->TZ | M->TY |
		movaps		xmm7, xmm5		;				// |  ???  | VL.Z * M->BZ | VL.Y * M->BY | VL.X * M->BX |
		shufps		xmm5, xmm5, 0x39  ; // (00 11 10 01)| VL.X * M->BX |  ???  | VL.Z * M->BZ | VL.Y * M->BY |
		addss		xmm7, xmm5		;				// ...|(VL.X * M->BX)+(VL.Y * M->BY)|
		shufps		xmm5, xmm5, 0x39 ;  // (00 11 10 01)| VL.Y * M->BY | VL.X * M->BX |  ???  | VL.Z * M->BZ |
		addss		xmm7, xmm5	;// ...|(VL.X * M->BX)+(VL.Y * M->BY)+(VL.Z * M->BZ)|
		addss		xmm7, xmm3	;					// ...|(VL.X * M->BX)+(VL.Y * M->BY)+(VL.Z * M->BZ)+M->Translation.Y|
		movss		[edi+4], xmm7

		shufps		xmm3, xmm3, 0x39		;		// | M->TY | M->TX |  pad  | M->TZ |
		movaps		xmm7, xmm6			;			// |  ???  | VL.Z * M->CZ | VL.Y * M->CY | VL.X * M->CX |
		shufps		xmm6, xmm6, 0x39  ; // (00 11 10 01)| VL.X * M->CX |  ???  | VL.Z * M->CZ | VL.Y * M->CY |
		addss		xmm7, xmm6	;					// ...|(VL.X * M->CX)+(VL.Y * M->CY)|
		shufps		xmm6, xmm6, 0x39;   // (00 11 10 01)| VL.Y * M->CY | VL.X * M->CX |  ???  | VL.Z * M->CZ |
		addss		xmm7, xmm6	;					// ...|(VL.X * M->CX)+(VL.Y * M->CY)+(VL.Z * M->CZ)|
		addss		xmm7, xmm3	;					// ...|(VL.X * M->CX)+(VL.Y * M->CY)+(VL.Z * M->CZ)+M->Translation.Y|
		movss		[edi+8], xmm7
		
		add			edi, 16	;						// dest buffer is x, y, z, pad

		dec			ecx
		jnz			NEAR TransformArrayLoop

Done:

ret

jeXForm3d_TransformArrayKatmai:
		push ebp
		mov ebp,esp
		sub esp,0x40

		mov			ecx,	[ebp+8] ;//Count						// number of jeVec3d's to process
		test			ecx, ecx
		jz NEAR			TransArray_TransformArrayLoop_Done

		mov			esi, [ebp+24] ;//Source						// pointer to Source
		mov			edi, [ebp+20]  ;//Dest						// pointer to Dest
		mov			eax, [ebp+28] ;//XForm						// pointer to matrix

		prefetchnta	[esi]						
												;	// |  High |       |       |  Low  |
		movups		xmm0, [eax]			;			// |  pad  | M->AZ | M->AY | M->AX | use movaps if 16-byte aligned
		movups		xmm1, [eax+16]			;		// |  pad  | M->BZ | M->BY | M->BX |
		movups		xmm2, [eax+32]			;		// |  pad  | M->CZ | M->CY | M->CX |

	TransArray_TransformArrayLoop:
		prefetchnta	[edi]

		movups		xmm4, [esi]				;		// |  pad  | VL.Z  | VL.Y  | VL.X  |
		movups		xmm3, [eax+48]			;		// |  pad  | M->TZ | M->TY | M->TX |
		movaps		xmm5, xmm4			;			// |  pad  | VL.Z  | VL.Y  | VL.X  |
		movaps		xmm6, xmm4			;			// |  pad  | VL.Z  | VL.Y  | VL.X  |

		mulps		xmm4, xmm0				;		// |  ???  | VL.Z * M->AZ | VL.Y * M->AY | VL.X * M->AX |
		mulps		xmm5, xmm1				;		// |  ???  | VL.Z * M->BZ | VL.Y * M->BY | VL.X * M->BX |
		mulps		xmm6, xmm2				;		// |  ???  | VL.Z * M->CZ | VL.Y * M->CY | VL.X * M->CX |

		movaps		xmm7, xmm4			;			// |  ???  | VL.Z * M->AZ | VL.Y * M->AY | VL.X * M->AX |
		shufps		xmm4, xmm4, 0x39 ; // (00 11 10 01)| VL.X * M->AX |  ???  | VL.Z * M->AZ | VL.Y * M->AY |
		addss		xmm7, xmm4		;				// ...|(VL.X * M->AX)+(VL.Y * M->AY)|
		shufps		xmm4, xmm4, 0x39 ;  // (00 11 10 01)| VL.Y * M->AY | VL.X * M->AX |  ???  | VL.Z * M->AZ |
		addss		xmm7, xmm4		;				// ...|(VL.X * M->AX)+(VL.Y * M->AY)+(VL.Z * M->AZ)|
		addss		xmm7, xmm3		;				// ...|(VL.X * M->AX)+(VL.Y * M->AY)+(VL.Z * M->AZ)+M->Translation.X|
		movss		[edi], xmm7

		add			esi, [ebp+16] ;//SourceStride

		shufps		xmm3, xmm3, 0x39			;	// | M->TX |  pad  | M->TZ | M->TY |
		movaps		xmm7, xmm5			;			// |  ???  | VL.Z * M->BZ | VL.Y * M->BY | VL.X * M->BX |
		shufps		xmm5, xmm5, 0x39 ;  // (00 11 10 01)| VL.X * M->BX |  ???  | VL.Z * M->BZ | VL.Y * M->BY |
		addss		xmm7, xmm5	;					// ...|(VL.X * M->BX)+(VL.Y * M->BY)|
		shufps		xmm5, xmm5, 0x39  ; // (00 11 10 01)| VL.Y * M->BY | VL.X * M->BX |  ???  | VL.Z * M->BZ |
		addss		xmm7, xmm5		;				// ...|(VL.X * M->BX)+(VL.Y * M->BY)+(VL.Z * M->BZ)|
		addss		xmm7, xmm3		;				// ...|(VL.X * M->BX)+(VL.Y * M->BY)+(VL.Z * M->BZ)+M->Translation.Y|
		movss		[edi+4], xmm7

		prefetchnta	[esi]

		shufps		xmm3, xmm3, 0x39		;		// | M->TY | M->TX |  pad  | M->TZ |
		movaps		xmm7, xmm6			;			// |  ???  | VL.Z * M->CZ | VL.Y * M->CY | VL.X * M->CX |
		shufps		xmm6, xmm6, 0x39   ;// (00 11 10 01)| VL.X * M->CX |  ???  | VL.Z * M->CZ | VL.Y * M->CY |
		addss		xmm7, xmm6	;					// ...|(VL.X * M->CX)+(VL.Y * M->CY)|
		shufps		xmm6, xmm6, 0x39  ; // (00 11 10 01)| VL.Y * M->CY | VL.X * M->CX |  ???  | VL.Z * M->CZ |
		addss		xmm7, xmm6	;					// ...|(VL.X * M->CX)+(VL.Y * M->CY)+(VL.Z * M->CZ)|
		addss		xmm7, xmm3		;				// ...|(VL.X * M->CX)+(VL.Y * M->CY)+(VL.Z * M->CZ)+M->Translation.Y|
		movss		[edi+8], xmm7
		
		add			edi, [ebp+12] ;//DestStride

		dec			ecx
		jnz			TransArray_TransformArrayLoop

TransArray_TransformArrayLoop_Done:

leave
ret

;this is to prevent BeIDE from crashing=)
