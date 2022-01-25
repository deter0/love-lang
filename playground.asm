section .text
	global _start

exit:
	mov rax, 60
	mov rdi, 0
	syscall


_start:
	mov rax, 1
	mov rdi, 1
	mov rsi, msg
	mov rdx, msg_len
	syscall
	jmp exit


section .data
	msg: db "Hello World", 10
	msg_len: equ $ - msg

