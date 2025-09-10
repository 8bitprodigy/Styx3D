;..\masm\bin\ml -Zi -c -coff  /Fo $(IntDir)\$(InputName).obj $(InputPath)
;$(IntDir)\$(InputName).obj

GLOBAL cachetouch_w_3dnow;
GLOBAL cachetouch_r_3dnow
GLOBAL copy32_3dnow_fastcall

SECTION .text

;memclear_3dnow_mmx PROC C PUBLIC		data : DWORD, num32s : DWORD
;{

memclear_3dnow_mmx:
	push ebp
	mov ebp,esp
	sub esp,0x40

	mov eax, [ebp + 12] ;//data
	mov edx, [ebp + 8] ;//num32s

	prefetchw [eax]

	test edx,1
	jz memclear_3dnow_mmx_SkipOne

		prefetchw [eax+32]

		movq [eax + 0],mm0
		movq [eax + 8],mm0
		movq [eax +16],mm0
		movq [eax +24],mm0

		add eax,32

memclear_3dnow_mmx_SkipOne:

	shr edx,1

	; the speed of this code appears to be motherboard limited.
	; we get about 87 megs/sec on my home K6
	;	(my MB is clocked at 87.5 MHz for a 350 CPU ; a relation? )

	pxor mm0,mm0

	;{
	memclear_3dnow_mmx_MoreN:
		prefetchw [eax+32]

		movq [eax + 0],mm0
		movq [eax + 8],mm0
		movq [eax +16],mm0
		movq [eax +24],mm0

		prefetchw [eax+64]

		movq [eax +32],mm0
		movq [eax +40],mm0
		movq [eax +48],mm0
		movq [eax +56],mm0

		add eax,64
		dec edx		; edx is n
	jg memclear_3dnow_mmx_MoreN
	;}
	
	leave	
	ret
;}
;memclear_3dnow_mmx ENDP

;memclear_3dnow PROC C PUBLIC uses edi eax ecx edx,	data : DWORD, num32s : DWORD
;{
memclear_3dnow:
	push ebp
	mov ebp,esp
	sub esp,0x40

	mov edi, [ebp + 12] ;data
	mov edx,[ebp + 8] ;num32s
	xor eax,eax

	prefetchw [edi]

	test edx,1
	jz memclear_3dnow_SkipOne

		prefetchw [edi+32]

		mov ecx,8
		rep stosd

memclear_3dnow_SkipOne:

	shr edx,1

	;{
	MoreN:
		prefetchw [edi+32]

		mov ecx,16
		rep stosd

		prefetchw [edi]

		dec edx		; edx is n
	jg MoreN
	;}
	
	leave
	ret
;}
;memclear_3dnow ENDP

;memcpy32s_3dnow PROC C PUBLIC uses ebx,		to : DWORD, from : DWORD, num32s : DWORD
;{
memcpy32s_3dnow:
	push ebp
	mov ebp,esp
	sub esp,0x40

	mov eax,[ebp + 16] ;to
	mov	ebx,[ebp + 12] ;from
	mov edx,[ebp + 8] ;num32s

	prefetchw [eax]

	;{
	memcpy32s_3dnow_MoreN:
		prefetchw [eax+32]

		movq mm0,[ebx + 0]
		movq mm1,[ebx + 8]
		movq [eax + 0],mm0
		movq mm2,[ebx +16]
		movq [eax + 8],mm1
		movq mm3,[ebx +24]
		prefetch [ebx+32]
		movq [eax +16],mm2
		add  ebx,32
		movq [eax +24],mm3

		add  eax,32
		dec  edx		; edx is n
	jg memcpy32s_3dnow_MoreN
	;}
	
	leave
	ret
;}
;memcpy32s_3dnow ENDP

; MINIMAL K6 VERSIONS :

;cachetouch_w_3dnow PROC C PUBLIC data : DWORD, num32s : DWORD

cachetouch_w_3dnow:
;{
	push ebp
	mov ebp,esp
	sub esp,0x40

	prefetchw [ebp + 8] ;data
	leave
	ret
;}
;cachetouch_w_3dnow ENDP

;cachetouch_r_3dnow PROC C PUBLIC data : DWORD, num32s : DWORD
cachetouch_r_3dnow:
;{
	push ebp
	mov ebp,esp
	sub esp,0x40

	prefetch [ebp + 12] ;data]
	leave
	ret
;}
;cachetouch_r_3dnow ENDP

;copy32_3dnow PROC C PUBLIC to : DWORD, from : DWORD
;{
copy32_3dnow:

	push ebp
	mov ebp,esp
	sub esp,0x40

	; if you have 3dnow, you have mmx, right?
	prefetchw [ebp+12];to
	mov eax,[ebp+8];from
	mov ecx,[ebp+12];to
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
;}
;copy32_3dnow ENDP

;copy32_8_3dnow PROC C PUBLIC uses ebx, to : DWORD, froms : DWORD
;{
	copy32_8_3dnow:

	push ebp
	mov ebp,esp
	sub esp,0x40

	; if you have 3dnow, you have mmx, right?
;	int 3

	mov  ebx,[ebp + 8];froms
	mov  ecx,[ebp + 12];to

	prefetchw [ecx]
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

	prefetchw [ecx]
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

	prefetchw [ecx]
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
	
	prefetchw [ecx]
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

	prefetchw [ecx]
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

	prefetchw [ecx]
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

	prefetchw [ecx]
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
	
	prefetchw [ecx]
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
;}
;copy32_8_3dnow ENDP

;copy32_3dnow_fastcall PROC C PUBLIC	; to, from

copy32_3dnow_fastcall:
;{
	push ebp
	mov ebp,esp
	sub esp,0x40

	; if you have 3dnow, you have mmx, right?
; These two lines were commented out.. but we dont have a fastcall.. so we need to set them in the registers.
	mov eax,[ebp+12]
	mov ecx,[ebp+8];from
	prefetchw [ecx]
	movq mm0,[edx   ]
	movq mm1,[edx+8 ]
	movq [ecx   ],mm0
	movq mm2,[edx+16]
	movq [ecx+8 ],mm1
	movq mm3,[edx+24]
	movq [ecx+16],mm2
	movq [ecx+24],mm3

	leave
	ret
;}
;copy32_3dnow_fastcall ENDP
