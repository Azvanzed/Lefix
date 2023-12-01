other:
	sub rsp, 9 ; reserve locals
	mov al, 2 ; var_44683
	mov byte [rsp+8], al ; test2
	mov al, byte [rsp+8] ; test2
	add rsp, 9 ; free locals
	ret
efi_main:
	sub rsp, 17 ; reserve locals
	mov al, 64 ; var_25831
	mov byte [rsp+16], al ; test
	mov al, byte [rsp+16] ; test
	add rsp, 17 ; free locals
	ret
global _start; ; for testing
_start:
	push rcx ; ImageHandle
	push rdx ; SystemTable
	call efi_main
	mov rbx, rax ; exit code
	mov rax, 1 ; sys_exit
	int 0x80
