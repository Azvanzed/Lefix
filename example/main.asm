global _start

some_other_function:
	mov rax, 32
	ret

_start:
	sub rsp, 4
	mov rax, 128
	mov [rsp+0], rax
	sub rsp, 20
	mov rax, 16
	mov [rsp+0], rax
	mov rax, 32
	mov [rsp+4], rax
	mov rax, [rsp+0]
	mov [rsp+12], rax
	call some_other_function
	mov rax, 64
	add rsp, 4
	mov rbx, rax
	mov rax, 1
	int 80h

