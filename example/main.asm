hehe:
	sub rsp, 4 ; reserve locals
	mov al, 0 ; var_18027
	mov byte [rsp+0], al ; i
	mov eax, dword [rsp+0] ; i
	add rsp, 4 ; free locals
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
