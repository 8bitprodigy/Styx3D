/****************************************************************************************/
/*  UTILITY.C                                                                           */
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

//@@ #define DISABLED

#ifdef BUILD_BE
#define __fastcall
#endif

#include "Utility.h"
#include "Cpu.h"
#include "YUV.h"
#include "KatCache.h"
#include "Cache3DN.h"


#pragma warning(disable : 4799)	// I know we've got no emms; it's done in wavelet.c

void SetupUtility(void);

void (*cachetouch_r)	(const void * data,int num32s) = NULL;
void (*cachetouch_w)	(const void * data,int num32s) = NULL;

void (__fastcall * copy32)(char * to,const char *from) = NULL;
void (*copy32_8)(char * to,const char **froms) = NULL;

void (*fastmemclear32s)	(void *data,int num32s) = NULL;
void (*fastmemcpy32s)	(char * to,const char *from,int num32s) = NULL;

void cachetouch_nada(const void * data,int num32s) {}
void memclear_x86(char *data,int n);
void memclear_mmx(char *data,int n);
void memcpy32s_x86(char * to,const char *from,int len);
void memcpy32s_mmx(char * to,const char *from,int len);
void __fastcall copy32_mmx(char *to,const char *from);
void __fastcall copy32_x86(char *to,const char *from);
void copy32_8_c(char * to,const char **froms);
void copy32_8_katmai(char *to, const char **froms);

static jeBoolean IsSetup = JE_FALSE;

void UnSetupUtility(void)
{
	IsSetup = JE_FALSE;
	cachetouch_r	=NULL;
	cachetouch_w	=NULL;
	fastmemclear32s =NULL;
	fastmemcpy32s	=NULL;
	copy32			=NULL;
	copy32_8		=NULL;
}

void SetupUtility(void)
{
	if ( IsSetup )
		return;
	IsSetup = JE_TRUE;

	jeCPU_GetInfo();

	SetupYUV();

	if ( jeCPU_Features & JE_CPU_HAS_KATMAI )
	{
		cachetouch_r = cachetouch_katmai;
		cachetouch_w = cachetouch_katmai;
		fastmemclear32s = memclear_katmai;
		fastmemcpy32s = memcpy32s_katmai;
		copy32 = copy32_katmai;
		copy32_8 = copy32_8_katmai;
	}
	else if ( jeCPU_Features & JE_CPU_HAS_3DNOW )
	{
		cachetouch_r = cachetouch_r_3dnow;
		cachetouch_w = cachetouch_w_3dnow;

		// punt : just do it ala x86
		// on a K6 the prefetch is not good enough
		// on K7 we should get a win here

		//fastmemclear32s = memclear_3dnow;
		fastmemclear32s = memclear_x86;
		//fastmemcpy32s = memcpy32s_3dnow;
		fastmemcpy32s = memcpy32s_x86;


		copy32 = (void (__fastcall *)(char * to,const char *from)) copy32_3dnow_fastcall;
		//copy32_8 = copy32_8_3dnow;
		copy32_8 = copy32_8_c;
	}
	else
	{
		cachetouch_r = cachetouch_nada;
		cachetouch_w = cachetouch_nada;

	#if 0 //{ // just use the x86 versions !!
		if ( jeCPU_Features & JE_CPU_HAS_MMX )
		{
			fastmemclear32s = memclear_mmx;
			fastmemclear32s = memclear_x86;
			copy32 = copy32_mmx;
			fastmemcpy32s = memcpy32s_x86;
		}
		else
	#endif //}
		{
			fastmemclear32s = memclear_x86;
			copy32 = copy32_x86;
			fastmemcpy32s = memcpy32s_x86;
		}
		copy32_8 = copy32_8_c;
	}

}

void fastmemclear(char *data,int len)
{

#if 0
	// just do it like clib

	assert( ((int)data&3) == 0 );

	__asm
	{
		mov ecx,len
		xor eax,eax
		mov edi,data
		shr ecx,2
		rep stosd

		mov ecx,len
		and ecx,3
		rep stosb
	}

#else
int n;

	n = len>>5;

	if ( n )
	{
		assert(fastmemclear32s);
		fastmemclear32s(data,n);
	}
	
	len = len&31;	
	if ( len )
	{
		memset(data + (n<<5),0,len);
	}

#endif
}


void copy32_8_c(char * to,const char **froms)
{
	copy32(to,froms[0]); to += 32;
	copy32(to,froms[1]); to += 32;
	copy32(to,froms[2]); to += 32;
	copy32(to,froms[3]); to += 32;
	copy32(to,froms[4]); to += 32;
	copy32(to,froms[5]); to += 32;
	copy32(to,froms[6]); to += 32;
	copy32(to,froms[7]); to += 32;
}

void __fastcall copy32_x86(char *to,const char *from)
{
#ifdef WIN32
	__asm
	{
		mov esi,from
		mov ecx,8
		mov edi,to
		rep movsd
	}
#endif

#ifdef BUILD_BE
	__asm__ __volatile__ 
	(" 
		movl %0 , %%esi
		movl $8, %%ecx
		movl %1, %%edi
		rep movsl // movsd CHECK THIS!!!!!!
	 " : 
	   : "g" (from) , "g" (to)
	   : "%esi" , "%ecx" , "%edi" );
#endif

}

void __fastcall copy32_mmx(char *to,const char *from)
{
#ifdef WIN32
	__asm
	{
		// don't use esi & edi, those require extra pushes & pops
		mov eax,from
		movq mm0,[eax   ]
		mov ecx,to
		movq mm1,[eax+8 ]
		movq [ecx   ],mm0
		movq mm2,[eax+16]
		movq [ecx+8 ],mm1
		movq mm3,[eax+24]
		movq [ecx+16],mm2
		movq [ecx+24],mm3
	}
#endif

#ifdef BUILD_BE
        __asm__ __volatile__ ("
                movl %0, %%eax           //;mov eax,from
                movq (%%eax), %%mm0      //;movq mm0,[eax   ]
                movl %1, %%ecx           //;mov ecx,to
                movq 8(%%eax), %%mm1 //;movq mm1,[eax+8 ]
                movq %%mm0, (%%ecx)      //;movq [ecx   ],mm0
                movq 16(%%eax), %%mm2 //;movq mm2,[eax+16]
                movq %%mm1, 8(%%ecx)  //;movq [ecx+8 ],mm1
                movq 24(%%eax), %%mm3 //;movq mm3,[eax+24]
                movq %%mm2, 16(%%ecx) //;movq [ecx+16],mm2
                movq %%mm3, 24(%%ecx) //;movq [ecx+24],mm3
        "
         :
         : "g" (from), "g" (to)
         : "%ecx", "%eax");
#endif

}

void memclear_x86(char *data,int n)
{
#ifdef WIN32
	__asm 
	{
		mov edi,data
		mov ecx,n
		xor eax,eax
		shl ecx,3
		rep stosd
	}
#endif

#ifdef BUILD_BE
        __asm__ __volatile__ ("
                mov %0, %%edi
                mov %1, %%ecx
                xor %%eax, %%eax
                shl $3, %%eax
                rep stosl // stosd
                " 
                :
                : "g" (data), "g" (n) : "%edi", "%eax" , "%ecx");
#endif
}

void memclear_mmx(char *data,int n)
{
#ifdef WIN32
	__asm 
	{
	mov eax,data
	mov ecx,n

	pxor mm0,mm0

	More:

		movq [eax   ],mm0
		movq [eax+8 ],mm0
		movq [eax+16],mm0
		movq [eax+24],mm0
		add  eax,32

	dec ecx
	jnz More

	//emms
	}
#endif

#ifdef BUILD_BE
        __asm__ __volatile__ ("
        
        movl %0 , %%eax //;mov eax,data
        movl %1, %%ecx // ;mov ecx,n

        pxor %%mm0, %%mm0 //;pxor mm0,mm0

        memclear_mmx_More:

        movq %%mm0, (%%eax) //; movq [eax   ],mm0
        movq %%mm0, 8(%%eax) //;        movq [eax+8 ],mm0
        movq %%mm0, 16(%%eax) //;       movq [eax+16],mm0
        movq %%mm0, 24(%%eax) //;       movq [eax+24],mm0
        addl $32, %%eax //;     add  eax,32

        decl %%ecx      //;dec ecx
        jnz memclear_mmx_More
        "
        :
        : "g" (data), "g" (n)
        : "%eax" , "%ecx"
        );
#endif

}

void fastmemcpy		(char * to,const char *from,int len)
{
int n;
	n = len>>5;

	if ( n )
	{
		assert(fastmemcpy32s);
		fastmemcpy32s(to,from,n);
	}
	
	len = len&31;	
	if ( len )
	{
		memcpy(to + (n<<5),from,len);
	}
}

void memcpy32s_mmx(char * to,const char *from,int num32s)
{
#ifdef WIN32
	__asm
	{
	mov eax,to
	mov	ebx,from
	mov ecx,num32s

	;{
	MoreN:
		movq mm0,[ebx + 0]
		movq mm1,[ebx + 8]
		movq [eax + 0],mm0
		movq mm2,[ebx +16]
		movq [eax + 8],mm1
		movq mm3,[ebx +24]
		movq [eax +16],mm2
		add  ebx,32
		movq [eax +24],mm3

		add  eax,32
		dec  ecx		; ecx is n
	jg MoreN
	;}
	}
#endif

#ifdef BUILD_BE
__asm__ __volatile__ ("
        movl %0, %%eax          //;mov eax,to
        movl %1, %%ebx          //;mov  ebx,from
        movl %2, %%ecx          //;mov ecx,num32s

        MoreN:
                movq 0(%%ebx), %%mm0    //;     movq mm0,[ebx + 0]
                movq 8(%%ebx), %%mm1    //;movq mm1,[ebx + 8]
                movq %%mm0, (%%eax)             //;movq [eax + 0],mm0
                movq 16(%%ebx), %%mm2   //;movq mm2,[ebx +16]
                movq %%mm1, 8(%%eax)    //;movq [eax + 8],mm1
                movq 24(%%ebx), %%mm3   //;movq mm3,[ebx +24]
                movq %%mm2, 16(%%eax)   //;movq [eax +16],mm2
                addl $32, %%ebx                 //;add  ebx,32
                movq %%mm2, 24(%%eax)   //;movq [eax +24],mm3

                addl $32, %%eax                 //;add  eax,32
                decl %%ecx                              //;dec  ecx
        jg MoreN
        "
        :
        : "g" (to), "g" (from), "g" (num32s)
        : "%eax", "%ebx", "%ecx");
#endif

}

void memcpy32s_x86(char * to,const char *from,int num32s)
{
#ifdef WIN32
        __asm
        {
        mov edi,to
        mov     esi,from
        mov ecx,num32s
        shl ecx,3
        rep movsd
        }
#endif

#ifdef BUILD_BE
        __asm__ __volatile__ ("
        movl %0, %%edi
        movl %1, %%esi 
        movl %2, %%ecx
        shll $3, %%ecx
        rep movsl // movsd"
          :
          : "g" (to) , "g" (from) , "g" (num32s)
          : "%edi" , "%esi" , "%ecx" );
#endif
}
