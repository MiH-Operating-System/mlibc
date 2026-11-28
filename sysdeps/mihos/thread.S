// Where pthread_create's threads start (SYS_THREAD_CREATE's entry). The stack is the one
// PrepareStack built: the function, its argument and the new thread's TCB on top, which
// become __mlibc_enter_thread's three arguments (rdi, rsi, rdx in the SysV ABI)
.section .text
.global __mlibc_start_thread
.type __mlibc_start_thread, @function
__mlibc_start_thread:
	pop %rdi // the thread's function
	pop %rsi // its argument
	pop %rdx // its TCB
	call __mlibc_enter_thread
	ud2 // it never returns
.size __mlibc_start_thread, .-__mlibc_start_thread

.section .note.GNU-stack,"",%progbits