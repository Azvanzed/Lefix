efi_main:
	sub rsp, 24 ; reserve locals
	mov al, 40 ; var_10773
	mov qword [rsp+16], rax ; result
	mov al, 25 ; var_56263
	mov rbx, rax
	xor rdx, rdx
	mov rax, qword [rsp+16] ; result
	div rbx ; var_56263
	mov qword [rsp+16], rdx ; result
	mov al, 2 ; var_19223
	mov rbx, rax
	mov rax, qword [rsp+16] ; result
	mul rbx ; var_19223
	mov qword [rsp+16], rax ; result
	mov al, 5
	shl qword [rsp+16], 2 ; result
	shr qword [rsp+16], 1 ; result
	mov rax, qword [rsp+16] ; result
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
