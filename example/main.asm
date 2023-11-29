global _start

_start:
	sub rsp, 22
	mov rax, 268435455
	mov [rsp+0], rax
	mov rax, 1
	mov [rsp+4], rax
	mov rax, 14343
	mov [rsp+5], rax
	mov rax, 321
	mov [rsp+7], rax
	mov rax, 16
	mov [rsp+9], rax
	mov rax, [rsp+9]
	mov [rsp+0], rax
	mov rax, 18446744073709551615
	mov [rsp+10], rax
	mov rax, 256
	mov [rsp+18], rax
	mov rax, [rsp+9]
	mov [rsp+10], rax
	mov rax, [rsp+9]
	mov [rsp+4], rax
	mov rax, [rsp+7]
	mov [rsp+18], rax
	mov rax, [rsp+7]
	mov [rsp+5], rax
	mov rax, 51
	mov [rsp+9], rax
	mov rax, [rsp+9]
	add rsp, 22
	mov rbx, rax
	mov rax, 1
	int 80h

some_other_function:
	mov rax, 32
	ret

