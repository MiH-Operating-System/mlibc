.section .text
.global _start
_start:
	xor %ebp, %ebp          # end of the frame-pointer chain
	mov %rsp, %rdi          # argc, argv..., envp..., auxv: mlibc reads them all
	mov $main, %rsi
	call __mlibc_entry      # never returns
	ud2
.section .note.GNU-stack,"",%progbits