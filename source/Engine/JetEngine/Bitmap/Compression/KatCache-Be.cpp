
#ifndef BUILD_BE
Intentional Error("Only designed to be compiled on BeOS / GCC compatible systems!")
#endif

#define __fastcall

#include "KatCache.h"


void cachetouch_katmai(const void * data,int num32s)
{

	if ( num32s == 0 ) //<> should be merged into the asm !?
		return;

	__asm__ __volatile__
	("
		//<> these moves would be gone if we used fastcall!
		mov %0, %%eax  //;eax,data
		mov %1, %%ecx	//;ecx,num32s

		jmp cachetouch_katmai_more"
	: 
	: "g" (data), "g" (data)
	: "%ecx" , "%eax"
	);
}

void memclear_katmai(void *dataPtr,int num32s)
{
	char *data = (char *)dataPtr;
	
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
	__asm__ __volatile__ ("

	;// if you have katmai, you have mmx, right?

	mov %0, %%eax //;eax,data
	mov %1, %%ecx //;ecx,num32s

	jmp memclear_katmai_nasm
	"
	:
	: "g" (data), "g" (num32s) 
	: "%eax", "%ecx");
	
	/* 
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
	*/
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

void memcpy32s_katmai( char *to,const char *from,int num32s)
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

	__asm__ __volatile__ ("
	mov %0, %%edi //;edi,to
	mov %1, %%esi //;esi,from
	mov %2, %%edx //;edx,num32s

	//; Pass through to the nasm - katmai code
	jmp memcpy32s_katmai_nasm "
	:
	: "g" (to) , "g" (from) , "g" (num32s)
	: "%esi" , "%edi" , "%edx");
	
}
