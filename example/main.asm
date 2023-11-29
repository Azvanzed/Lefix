global _start

_start:
	sub rsp, 16
	mov rax, 64
	mov [rsp+0], rax
	mov rax, 32
	mov [rsp+8], rax
	mov rax, [rsp+8]
	mov [rsp+0], rax
	mov rax, 1228
	add rsp, 16
	mov rbx, rax
	mov rax, 1
	int 80h
