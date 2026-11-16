#include <bits/syscall.h>

// MiHOS system calls: int $0x80, the number in rax, arguments in rdi, rsi, rdx,
// r10, r8, r9 (Linux's registers); the result comes back in rax, -errno on failure
extern "C" long __do_syscall_ret(unsigned long ret) {
	return ret; // as is: each sysdep checks for < 0 itself
}

__sc_word_t __do_syscall0(long n) {
	__sc_word_t ret;
	asm volatile("int $0x80" : "=a"(ret) : "a"(n) : "memory");
	return ret;
}

__sc_word_t __do_syscall1(long n, __sc_word_t a1) {
	__sc_word_t ret;
	asm volatile("int $0x80" : "=a"(ret) : "a"(n), "D"(a1) : "memory");
	return ret;
}

__sc_word_t __do_syscall2(long n, __sc_word_t a1, __sc_word_t a2) {
	__sc_word_t ret;
	asm volatile("int $0x80" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2) : "memory");
	return ret;
}

__sc_word_t __do_syscall3(long n, __sc_word_t a1, __sc_word_t a2, __sc_word_t a3) {
	__sc_word_t ret;
	asm volatile("int $0x80" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2), "d"(a3) : "memory");
	return ret;
}

__sc_word_t __do_syscall4(long n, __sc_word_t a1, __sc_word_t a2, __sc_word_t a3, __sc_word_t a4) {
	register __sc_word_t r10 asm("r10") = a4;
	__sc_word_t ret;
	asm volatile("int $0x80" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2), "d"(a3), "r"(r10) : "memory");
	return ret;
}

__sc_word_t __do_syscall5(
    long n, __sc_word_t a1, __sc_word_t a2, __sc_word_t a3, __sc_word_t a4, __sc_word_t a5
) {
	register __sc_word_t r10 asm("r10") = a4;
	register __sc_word_t r8 asm("r8") = a5;
	__sc_word_t ret;
	asm volatile("int $0x80"
	             : "=a"(ret)
	             : "a"(n), "D"(a1), "S"(a2), "d"(a3), "r"(r10), "r"(r8)
	             : "memory");
	return ret;
}

__sc_word_t __do_syscall6(
    long n,
    __sc_word_t a1,
    __sc_word_t a2,
    __sc_word_t a3,
    __sc_word_t a4,
    __sc_word_t a5,
    __sc_word_t a6
) {
	register __sc_word_t r10 asm("r10") = a4;
	register __sc_word_t r8 asm("r8") = a5;
	register __sc_word_t r9 asm("r9") = a6;
	__sc_word_t ret;
	asm volatile("int $0x80"
	             : "=a"(ret)
	             : "a"(n), "D"(a1), "S"(a2), "d"(a3), "r"(r10), "r"(r8), "r"(r9)
	             : "memory");
	return ret;
}
