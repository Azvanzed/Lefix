global _start

other:
	mov rax, 32
	ret

_start:
	call other
	mov rax, 64
	mov rbx, rax
	mov rax, 1
	int 80h

