ehhh:
	mov al, 32 ; var_54414
	ret
ehhh0:
	mov al, 64 ; var_14720
	ret
ehhh1:
	mov al, 128 ; var_33600
	ret
efi_main:
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
