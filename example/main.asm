other:
	sub rsp, 8 ; reserve locals
	mov rax, [rsp+0] ; c
	add rsp, 8 ; free locals
	ret
efi_main:
	sub rsp, 32 ; reserve locals
	push byte 128 ; var_47852
	call other
	mov rax, 32 ; var_23842
	mov [rsp+16], rax ; test
	mov rax, 128332 ; var_50217
	mov [rsp+24], rax ; i_swear
	mov rax, [rsp+24] ; i_swear
	mov [rsp+16], rax ; test
	mov rax, [rsp+40] ; image_handle
	mov [rsp+24], rax ; i_swear
	mov rax, [rsp+24] ; i_swear
	add rsp, 32 ; free locals
	ret
global _start; ; for testing
_start:
	push rcx ; ImageHandle
	push rdx ; SystemTable
	call efi_main
	mov rbx, rax ; exit code
	mov rax, 1 ; sys_exit
	int 0x80
