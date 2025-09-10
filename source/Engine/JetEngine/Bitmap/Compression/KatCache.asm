BITS 32

GLOBAL copy32_katmai
GLOBAL memclear_katmai_nasm
GLOBAL memcpy32s_katmai_nasm
GLOBAL copy32_8_katmai
GLOBAL cachetouch_katmai_more

SECTION .text

;cachetouch_katmai_more;
cachetouch_katmai_more:
		prefetchnta [eax] ;// !
		add eax,32
		dec ecx
jnz cachetouch_katmai_more

ret

;prototype void __fastcall copy32_katmai(char *to,const char *from);
copy32_katmai:
	push ebp
	mov ebp,esp
	sub esp,0x40
		
		prefetchnta [ebp + 8]
		mov eax,[ebp + 8];from
		prefetchnta [ebp + 12];to
		mov ecx,[ebp + 12];to
		movq mm0,[eax   ]
		movq mm1,[eax+8 ]
		movq [ecx   ],mm0
		movq mm2,[eax+16]
		movq [ecx+8 ],mm1
		movq mm3,[eax+24]
		movq [ecx+16],mm2
		movq [ecx+24],mm3

	leave
	ret
	
;
memclear_katmai_nasm:

	prefetchnta [eax]
	prefetchnta [eax+32]

	pxor mm0,mm0

	
	memclear_katmai_nasm_MoreN:
	
	;	// clear 64 bytes at eax

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
		dec  ecx		;// ecx is n
		movq [eax+56],mm0
		mov  eax,edx

	jnz memclear_katmai_nasm_MoreN
	
	ret
	
;
memcpy32s_katmai_nasm:
	prefetchnta [edi]
	prefetchnta [edi+32]
	prefetchnta [esi]
	prefetchnta [esi+32]

	xor eax,eax

	;//{
	MoreN:
		prefetchnta [edi+64]
		prefetchnta [esi+64]

		;// mov 64 bytes from esi to edi
		mov ecx,16

		rep movsd

		dec edx	;	// edx is n

		prefetchnta [edi+32]
		prefetchnta [esi+32]

	jnz MoreN

	ret
	
; prototype void copy32_8_katmai(char *to,char **froms)
copy32_8_katmai:

	push ebp
	mov ebp,esp
	sub esp,0x40

	mov  ecx,[ebp + 12]
	mov  ebx,[ebp + 8]

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

	leave
	ret
	
