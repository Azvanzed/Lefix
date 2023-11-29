global _start

_start:
	sub rsp, 4
	mov rax, 64
	mov [rsp+0], rax
	mov rax, 2
	add rsp, 4
	mov rbx, rax
	mov rax, 1
	int 80h

some_other_function:
	mov rax, 32
	ret

