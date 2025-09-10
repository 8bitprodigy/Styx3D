/****************************************************************************************/
/*  KATCACHE.C                                                                          */
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
//	Original Custom Build statements...
//	_DEBUG
//	"$(INTELC)" /c /Zl /Zi /Fo".\$(IntDir)\$(InputName).obj" /I support /MTd "$(InputPath)"
//	NDEBUG
//	"$(INTELC)" /c /Zl /Fo".\$(IntDir)\$(InputName).obj" /I support /MT "$(InputPath)"

// ..\intelc\icl.exe /c /Zl /Fo.\$(IntDir)\$(InputName).obj $(InputPath)
// $(IntDir)\$(InputName).obj

//#ifndef __ICL

#pragma message("Must compile with the Intel compiler!")

//Intentional_Error
//Intentional_Error

//#else

// todo : type versus rep stosd versions; those are probably better than mmx

#pragma message("I know there's no emms; how to disable warnings in ICL?")
#pragma warning(disable : 986) // I know there's no EMMS! !! ACK! How to turn off warnings in intel compiler !?
#pragma warning(disable : 4799)	// I know we've got no emms; it's done in wavelet.c

void __fastcall copy32_katmai(char *to,const char *from)
{
	// if you have katmai, you have mmx, right?
	__asm
	{
		prefetchnta from
		mov eax,from
		prefetchnta to
		mov ecx,to
		movq mm0,[eax   ]
		movq mm1,[eax+8 ]
		movq [ecx   ],mm0
		movq mm2,[eax+16]
		movq [ecx+8 ],mm1
		movq mm3,[eax+24]
		movq [ecx+16],mm2
		movq [ecx+24],mm3
	}
}

void copy32_8_katmai(char *to,char **froms)
{

	__asm
	{
	mov  ecx,to
	mov  ebx,froms

	prefetchnta [ecx]
	mov  eax,[ebx + 0]
	movq mm0,[eax   ]
	movq mm1,[eax+8 ]
	movq [ecx   ],mm0
	movq mm2,[eax+16]
	movq [ecx+8 ],mm1
	movq mm3,[eax+24]
	movq [ecx+16],mm2
	movq [ecx+24],mm3
	add  ecx,32

	prefetchnta [ecx]
	mov  eax,[ebx + 4]
	movq mm0,[eax   ]
	movq mm1,[eax+8 ]
	movq [ecx   ],mm0
	movq mm2,[eax+16]
	movq [ecx+8 ],mm1
	movq mm3,[eax+24]
	movq [ecx+16],mm2
	movq [ecx+24],mm3
	add  ecx,32

	prefetchnta [ecx]
	mov  eax,[ebx + 8]
	movq mm0,[eax   ]
	movq mm1,[eax+8 ]
	movq [ecx   ],mm0
	movq mm2,[eax+16]
	movq [ecx+8 ],mm1
	movq mm3,[eax+24]
	movq [ecx+16],mm2
	movq [ecx+24],mm3
	add  ecx,32
	
	prefetchnta [ecx]
	mov  eax,[ebx +12]
	movq mm0,[eax   ]
	movq mm1,[eax+8 ]
	movq [ecx   ],mm0
	movq mm2,[eax+16]
	movq [ecx+8 ],mm1
	movq mm3,[eax+24]
	movq [ecx+16],mm2
	movq [ecx+24],mm3
	add  ecx,32

	prefetchnta [ecx]
	mov  eax,[ebx +16]
	movq mm0,[eax   ]
	movq mm1,[eax+8 ]
	movq [ecx   ],mm0
	movq mm2,[eax+16]
	movq [ecx+8 ],mm1
	movq mm3,[eax+24]
	movq [ecx+16],mm2
	movq [ecx+24],mm3
	add  ecx,32

	prefetchnta [ecx]
	mov  eax,[ebx +20]
	movq mm0,[eax   ]
	movq mm1,[eax+8 ]
	movq [ecx   ],mm0
	movq mm2,[eax+16]
	movq [ecx+8 ],mm1
	movq mm3,[eax+24]
	movq [ecx+16],mm2
	movq [ecx+24],mm3
	add  ecx,32

	prefetchnta [ecx]
	mov  eax,[ebx +24]
	movq mm0,[eax   ]
	movq mm1,[eax+8 ]
	movq [ecx   ],mm0
	movq mm2,[eax+16]
	movq [ecx+8 ],mm1
	movq mm3,[eax+24]
	movq [ecx+16],mm2
	movq [ecx+24],mm3
	add  ecx,32
	
	prefetchnta [ecx]
	mov  eax,[ebx +28]
	movq mm0,[eax   ]
	movq mm1,[eax+8 ]
	movq [ecx   ],mm0
	movq mm2,[eax+16]
	movq [ecx+8 ],mm1
	movq mm3,[eax+24]
	movq [ecx+16],mm2
	movq [ecx+24],mm3
	}

}

void cachetouch_katmai(const void * data,int num32s)
{

	if ( num32s == 0 ) //<> should be merged into the asm !?
		return;

	__asm
	{
		//<> these moves would be gone if we used fastcall!
		mov eax,data
		mov ecx,num32s

		more:
			prefetchnta [eax] // !
			add eax,32
			dec ecx
		jnz more
	}
}

void memclear_katmai(unsigned int *data,int num32s)
{
	if ( num32s & 1)
	{
		*data++ = 0; *data++ = 0;
		*data++ = 0; *data++ = 0;
		*data++ = 0; *data++ = 0;
		*data++ = 0; *data++ = 0;
	}

	num32s >>= 1; 

	if ( num32s == 0 )
		return;

	// They're 64's now!!

	// this is microscopically faster than the plain clib version :
	//	162 megs/sec vs. 159 megs/sec

	// the MMX and STOSD versions are about the same...
#if 1
	__asm
	{

	// if you have katmai, you have mmx, right?

	mov eax,data
	mov ecx,num32s

	prefetchnta [eax]
	prefetchnta [eax+32]

	pxor mm0,mm0

	//{
	MoreN:
	
		// clear 64 bytes at eax

		movq [eax   ],mm0
		prefetchnta [eax+64]
		movq [eax+8 ],mm0
		mov  edx,eax
		movq [eax+16],mm0
		movq [eax+24],mm0
		prefetchnta [eax+96]
		movq [eax+32],mm0
		add  edx,64
		movq [eax+40],mm0
		movq [eax+48],mm0
		dec  ecx		// ecx is n
		movq [eax+56],mm0
		mov  eax,edx

	jnz MoreN
	//}
	}
#else

	__asm 
	{
	mov edi,data
	mov edx,num32s

	prefetchnta [edi]
	prefetchnta [edi+32]

	xor eax,eax

	//{
	MoreN:
		prefetchnta [edi+64]
		prefetchnta [edi+96]

		// clear 64 bytes at edi
		mov ecx,16
		rep stosd

		dec edx		// edx is n
	jnz MoreN
	//}
	}
#endif

}

void memcpy32s_katmai(unsigned int *to,const unsigned int *from,int num32s)
{
	if ( num32s & 1)
	{
		*to++ = *from++; *to++ = *from++;
		*to++ = *from++; *to++ = *from++;
		*to++ = *from++; *to++ = *from++;
		*to++ = *from++; *to++ = *from++;
	}

	num32s >>= 1; 

	if ( num32s == 0 )
		return;

	// They're 64's now!!

	__asm 
	{
	mov edi,to
	mov esi,from
	mov edx,num32s

	prefetchnta [edi]
	prefetchnta [edi+32]
	prefetchnta [esi]
	prefetchnta [esi+32]

	xor eax,eax

	//{
	MoreN:
		prefetchnta [edi+64]
		prefetchnta [esi+64]

		// mov 64 bytes from esi to edi
		mov ecx,16

		rep movsd

		dec edx		// edx is n

		prefetchnta [edi+32]
		prefetchnta [esi+32]

	jnz MoreN
	//}
	}

}

#pragma warning(default : 986) // I know there's no EMMS! !! ACK! How to turn off warnings in intel compiler !?
#pragma warning(default : 4799)	// I know we've got no emms; it's done in wavelet.c

//#endif
