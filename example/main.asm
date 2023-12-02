other:
	sub rsp, 16 ; reserve locals
	mov rax, qword [rsp+24] ; a1
	mov qword [rsp+8], rax ; a
	mov rax, qword [rsp+8] ; a
	add rsp, 16 ; free locals
	ret
efi_main:
	sub rsp, 24 ; reserve locals
	sub rsp, 8 ; reserve image_handle
	mov rax, qword [rsp+40] ; image_handle
	mov qword [rsp], rax
	call other
	add rsp, 8 ; free args
	mov qword [rsp+16], rax ; ret_25357
	mov rax, qword [rsp+16] ; ret_25357
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
