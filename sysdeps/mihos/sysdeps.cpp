#include <abi-bits/errno.h>
#include <bits/ensure.h>
#include <bits/syscall.h>
#include <fcntl.h>
#include <limits.h> // INT_MAX
#include <mihos/syscall.h>
#include <mlibc/all-sysdeps.hpp>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h> // PROT_*, MAP_*
#include <termios.h>
#include <time.h>

namespace mlibc {

int Sysdeps<Write>::operator()(int fd, const void *buffer, size_t size, ssize_t *written) {
	long ret = syscall(SYS_WRITE, fd, buffer, size);
	if (ret < 0)
		return -ret; // -EBADF → EBADF
	*written = ret;
	return 0;
}

void Sysdeps<Exit>::operator()(int status) {
	syscall(SYS_EXIT, status);
	__builtin_unreachable();
}

int Sysdeps<Open>::operator()(const char *pathname, int flags, mode_t mode, int *fd) {
	long ret = syscall(SYS_OPEN, pathname, flags, mode);
	if (ret < 0)
		return -ret;
	*fd = ret;
	return 0;
}

int Sysdeps<Read>::operator()(int fd, void *buf, size_t count, ssize_t *bytes_read) {
	long ret = syscall(SYS_READ, fd, buf, count);
	if (ret < 0)
		return -ret;
	*bytes_read = ret;
	return 0;
}

int Sysdeps<Seek>::operator()(int fd, off_t offset, int whence, off_t *new_offset) {
	long ret = syscall(SYS_LSEEK, fd, offset, whence);
	if (ret < 0)
		return -ret;
	*new_offset = ret;
	return 0;
}

int Sysdeps<Close>::operator()(int fd) {
	long ret = syscall(SYS_CLOSE, fd);
	return ret < 0 ? -ret : 0;
}

int Sysdeps<AnonFree>::operator()(void *pointer, size_t size) {
	long ret = syscall(SYS_MUNMAP, pointer, size);
	return ret < 0 ? -ret : 0;
}

int Sysdeps<VmUnmap>::operator()(void *pointer, size_t size) {
	long ret = syscall(SYS_MUNMAP, pointer, size);
	return ret < 0 ? -ret : 0;
}

int Sysdeps<TcbSet>::operator()(void *pointer) {
	long ret = syscall(SYS_SET_THREAD_POINTER, pointer);
	return ret < 0 ? -ret : 0;
}

int Sysdeps<FutexWait>::operator()(int *pointer, int expected, const struct timespec *time) {
	long ret = syscall(SYS_FUTEX_WAIT, pointer, expected, time);
	return ret < 0 ? -ret : 0;
}

int Sysdeps<FutexWake>::operator()(int *pointer, bool all) {
	long ret = syscall(SYS_FUTEX_WAKE, pointer, all ? INT_MAX : 1);
	return ret < 0 ? -ret : 0;
}

int Sysdeps<ClockGet>::operator()(int clock, time_t *secs, long *nanos) {
	struct timespec time;
	long ret = syscall(SYS_CLOCK_GETTIME, clock, &time);
	if (ret < 0)
		return -ret;
	*secs = time.tv_sec;
	*nanos = time.tv_nsec;
	return 0;
}

int Sysdeps<AnonAllocate>::operator()(size_t size, void **pointer) {
	long ret =
	    syscall(SYS_MMAP, 0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (ret < 0)
		return -ret;
	*pointer = (void *)ret;
	return 0;
}

int Sysdeps<VmMap>::operator()(
    void *hint, size_t size, int prot, int flags, int fd, off_t offset, void **window
) {
	long ret = syscall(SYS_MMAP, hint, size, prot, flags, fd, offset);
	if (ret < 0)
		return -ret;
	*window = (void *)ret;
	return 0;
}

void Sysdeps<LibcLog>::operator()(const char *message) {
	ssize_t unused;
	sysdep<Write>(2, message, strlen(message), &unused);
	sysdep<Write>(2, "\n", 1, &unused);
}

void Sysdeps<LibcPanic>::operator()() {
	sysdep<LibcLog>("mlibc: panic");
	sysdep<Exit>(1);
}

int Sysdeps<Isatty>::operator()(int fd) {
	struct winsize size;
	long ret = syscall(SYS_IOCTL, fd, TIOCGWINSZ, &size);
	return ret < 0 ? -ret : 0;
}

int Sysdeps<Dup2>::operator()(int fd, int flags, int newfd) {
	long ret = syscall(SYS_DUP2, fd, newfd);
	if (ret < 0)
		return -ret;
	if (flags & O_CLOEXEC) {
		ret = syscall(SYS_FCNTL, newfd, F_SETFD, FD_CLOEXEC);
		if (ret < 0)
			return -ret;
	}
	return 0;
}

} // namespace mlibc
