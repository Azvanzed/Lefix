other:
	sub rsp, 7 ; reserve locals
	mov al, 32 ; var_41944
	mov byte [rsp+0], al ; hi
	mov al, 64 ; var_27242
	mov byte [rsp+3], al ; hey
	mov al, byte [rsp+0] ; hi
	mov byte [rsp+3], al ; hey
	mov eax, dword [rsp+3] ; hey
	add rsp, 7 ; free locals
	ret
efi_main:
	sub rsp, 8 ; reserve locals
	sub rsp, 8 ; reserve image_handle
	mov rax, qword [rsp+24] ; image_handle
	mov qword [rsp], rax
	call other
	add rsp, 8 ; free args
	mov qword [rsp+0], rax ; ret_14781
	mov rax, qword [rsp+0] ; ret_14781
	add rsp, 8 ; free locals
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
