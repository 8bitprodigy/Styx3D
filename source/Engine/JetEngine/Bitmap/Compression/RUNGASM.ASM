; original custom build line, _DEBUG and NDEBUG
; "$(MASM)" -Zi -c -coff  /Fo "$(IntDir)\$(InputName).obj" "$(InputPath)"


; build : ..\masm\bin\ml -Zi -c -coff  /Fo $(IntDir)\$(InputName).obj $(InputPath)
; output: $(IntDir)\$(InputName).obj

.586
.model FLAT,STDCALL
option language:c

.DATA

.CODE

asmRungModelDecBit PROC C PUBLIC USES esi edi ebp, ari : DWORD, rung : DWORD, ladder : DWORD

	mov ecx,ladder
	mov esi,rung		; esi == rung
	mov ebp,ari			; ebp == ari

	push ecx

	mov ebx,[ebp+0]		; ebx == code
	mov eax,[ebp+4]		; eax == range

	cmp eax,800000h		; ? ( range - MinRange )
	ja NoMoreOut		; cmp & jcc pair !
MoreOut:
	
	mov ecx,[ebp+8]		; ecx == ari->queue
	shl ebx,8			; code <<=8
	shl ecx,7			; ecx == (ari->queue<<EXTRA_BITS)

	mov edi,[ebp+12]	; edi == ari->outptr

	and ecx,00FFh		; ecx == (((ari->queue)<<EXTRA_BITS)&0xFF)

	add ebx,ecx			; code = (code<<8) + (((ari->queue)<<EXTRA_BITS)&0xFF)

	shl eax,8			; range<<=8
	
	movzx ecx,BYTE PTR [edi]	; ecx = *(ari->outptr)
	inc edi				; outptr++
	mov [ebp+8],ecx

	shr ecx, 1			; ecx = (ari->queue) >> (TAIL_EXTRA_BITS);
	add ebx, ecx
	mov [ebp+12],edi	; ari->outptr = outptr
	cmp eax,800000h		; ? ( range - MinRange )
	jbe MoreOut
	NoMoreOut:

	mov edi,[esi]
	shl edi,4
	pop ecx				; ecx == ladder
	add edi, ecx		; edi = ladder + (*rung) * sizeof(srung)

	mov ecx, [edi + 8 ]
	mov edx,eax
	shr edx,10			; edx = range >> rung_shifts
	imul edx, ecx		; 'r' = edx = (range >> rung_shifts) * p0	

	cmp ebx,edx			; (ebx - edx) == (range - r)
	jl CODE_LESS_THAN_R
	

	sub eax,edx			; range -= r;
	sub ebx,edx			; code  -= r;

	mov [ebp+4],eax
	mov [ebp+0],ebx
	
	mov edx,[edi + 4]
	mov eax,1
	mov [esi],edx		; *rung = s->r1

	ret

CODE_LESS_THAN_R:

	mov [ebp+4],edx		; range = r

	mov ecx,[edi + 0]
	mov eax,0
	mov [esi],ecx		; *rung = s->r0

	mov [ebp+0],ebx

	ret

asmRungModelDecBit ENDP

END

;CODE_BITS		31
;SHIFT_BITS		23
;CODE_BYTES		4
;PRECISION_BITS	9	

;MinRange 		800000h
;One   			80000000h
;CODE_MASK		7FFFFFFFh

;EXTRA_BITS		7
;TAIL_EXTRA_BITS	1

;RUNG_SHIFTS		10

;ARI_CODE	0
;ARI_RANGE	4
;ARI_QUEUE	8
;ARI_OUTPTR	12

;RUNG_R0		0
;RUNG_R1		4
;RUNG_P0		8