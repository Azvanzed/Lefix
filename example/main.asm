other:
	sub rsp, 2 ; reserve locals
	mov al, byte [rsp+10] ; hehe
	mov byte [rsp+1], al ; hehe2
	mov al, byte [rsp+1] ; hehe2
	add rsp, 2 ; free locals
	ret
efi_main:
	sub rsp, 24 ; reserve locals
	sub rsp, 1 ; reserve var_33378
	mov al, 32 ; var_33378
	mov byte [rsp], al
	call other
	add rsp, 1 ; free args
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
