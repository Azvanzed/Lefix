global _start

other:
	mov rax, 32
	ret

_start:
	mov [rsp+8], rcx
	mov [rsp+16], rdx
	sub rsp, 8
	mov rax, [rsp+16]
	mov [rsp+0], rax
	mov rax, [rsp+0]
	add rsp, 8
	mov rbx, rax
	mov rax, 1
	int 80h

