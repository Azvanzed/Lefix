efi_main:
	sub rsp, 17 ; reserve locals
	mov al, 0 ; var_51
	mov byte [rsp+16], al ; mask
	mov al, byte [rsp+16] ; mask
	not byte [rsp+16] ; mask
	mov al, byte [rsp+16] ; mask
	add rsp, 17 ; free locals
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
