smol_pause:

 ; Inlined assembly
	pause
 ; Inlined assembly

	mov al, 0 ; var_64410
	ret
other:
	sub rsp, 16 ; reserve locals
	mov al, 32 ; var_61354
	mov byte [rsp+8], al ; hi
	mov al, 64 ; var_16283
	mov byte [rsp+12], al ; hey

 ; Inlined assembly
	push rcx
	mov rcx, 40000000
	.sleep:
	test rcx, rcx
	jz .leave
	call smol_pause
	dec rcx
	jmp .sleep
	.leave:
	pop rcx
 ; Inlined assembly

	mov al, byte [rsp+8] ; hi
	mov byte [rsp+12], al ; hey
	mov eax, dword [rsp+12] ; hey
	add rsp, 16 ; free locals
	ret
efi_main:
	sub rsp, 24 ; reserve locals
	sub rsp, 8 ; reserve image_handle
	mov rax, qword [rsp+40] ; image_handle
	mov qword [rsp+0], rax
	call other
	add rsp, 8 ; free args
	mov qword [rsp+16], rax ; ret_50375
	mov rax, qword [rsp+16] ; ret_50375
	add rsp, 24 ; free locals
	ret
global _start; ; for testing
_start:
	mov rcx, 69
	mov rdx, 96
	push rdx ; SystemTable
	push rcx ; ImageHandle
	call efi_main
	add rsp, 16 ; free args
	mov rbx, rax ; exit code
	mov rax, 1 ; sys_exit
	int 0x80
