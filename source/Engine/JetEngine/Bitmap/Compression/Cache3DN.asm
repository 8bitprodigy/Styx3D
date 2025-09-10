;	original custom build command line _DEBUG and NDEBUG
; "$(MASM)" -c -coff  /Fo "$(IntDir)\$(InputName).obj" "$(InputPath)"

;..\masm\bin\ml -Zi -c -coff  /Fo $(IntDir)\$(InputName).obj $(InputPath)
;$(IntDir)\$(InputName).obj

.586
;.K3D
.model FLAT,C

; option language:c

.DATA

.CODE

memclear_3dnow_mmx PROC C PUBLIC		data : DWORD, num32s : DWORD
;{
	ret
;}
memclear_3dnow_mmx ENDP

memclear_3dnow PROC C PUBLIC uses edi eax ecx edx,	data : DWORD, num32s : DWORD
;{
	ret
;}
memclear_3dnow ENDP

memcpy32s_3dnow PROC C PUBLIC uses ebx,		to : DWORD, from : DWORD, num32s : DWORD
;{
	ret
;}
memcpy32s_3dnow ENDP

; MINIMAL K6 VERSIONS :

cachetouch_w_3dnow PROC C PUBLIC data : DWORD, num32s : DWORD
;{
	ret
;}
cachetouch_w_3dnow ENDP

cachetouch_r_3dnow PROC C PUBLIC data : DWORD, num32s : DWORD
;{
	ret
;}
cachetouch_r_3dnow ENDP

copy32_3dnow PROC C PUBLIC to : DWORD, from : DWORD
;{
	; if you have 3dnow, you have mmx, right?
	ret
;}
copy32_3dnow ENDP

copy32_8_3dnow PROC C PUBLIC uses ebx, to : DWORD, froms : DWORD
;{
	; if you have 3dnow, you have mmx, right?
;	int 3

	ret
;}
copy32_8_3dnow ENDP

copy32_3dnow_fastcall PROC C PUBLIC	; to, from
;{
	; if you have 3dnow, you have mmx, right?
;	mov eax,to
;	mov ecx,from
	ret
;}
copy32_3dnow_fastcall ENDP

END


cachetouch_w_3dnow PROC C PUBLIC data : DWORD, num32s : DWORD
;{

	ret
;}
cachetouch_w_3dnow ENDP

cachetouch_r_3dnow PROC C PUBLIC data : DWORD, num32s : DWORD
;{

	ret
;}
cachetouch_r_3dnow ENDP

END