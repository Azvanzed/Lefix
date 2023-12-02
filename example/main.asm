other:
	mov al, 128 ; var_36741
	ret
efi_main:
	sub rsp, 24 ; reserve locals
	call other
	mov qword [rsp+16], rax ; test
	mov rax, qword [rsp+16] ; test
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
