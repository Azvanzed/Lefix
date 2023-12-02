efi_main:
	sub rsp, 20 ; reserve locals
	mov al, 25 ; var_48598
	mov byte [rsp+16], al ; hehe

 ; Inlined assembly
	mov rax, qword [@efi_main]
	add rsp, 20 ; @stack_size
	ret
 ; Inlined assembly

	add rsp, 20 ; free locals
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
