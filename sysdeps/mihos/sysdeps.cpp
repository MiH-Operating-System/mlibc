#include <abi-bits/errno.h>
#include <bits/ensure.h>
#include <bits/syscall.h>
#include <fcntl.h>
#include <limits.h> // INT_MAX
#include <mihos/syscall.h>
#include <mlibc/all-sysdeps.hpp>
#include <signal.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h> // PROT_*, MAP_*
#include <sys/reboot.h>
#include <termios.h>
#include <time.h>

// Where a signal handler returns to: sigaction gives the kernel this address
// (sa_restorer), and it asks the kernel to remove the handler's frame
// clang-format off
#define MIHOS_STRING(x) #x
#define MIHOS_EXPAND(x) MIHOS_STRING(x)
asm(".pushsection .text\n"
    ".globl __mihos_signal_restore\n"
    ".type __mihos_signal_restore, @function\n"
    "__mihos_signal_restore:\n"
    "\tmov $" MIHOS_EXPAND(SYS_SIGRETURN) ", %rax\n"
	"\tint $0x80\n"
	"\tud2\n"
	".size __mihos_signal_restore, .-__mihos_signal_restore\n"
	".popsection\n");
extern "C" void __mihos_signal_restore(void);
// clang-format on

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

int Sysdeps<Fork>::operator()(pid_t *child) {
	long ret = syscall(SYS_FORK);
	if (ret < 0)
		return -ret;
	*child = ret;
	return 0;
}

int Sysdeps<Execve>::operator()(const char *path, char *const argv[], char *const envp[]) {
	long ret = syscall(SYS_EXEC, path, argv, envp);
	return ret < 0 ? -ret : 0;
}

int
Sysdeps<Waitpid>::operator()(pid_t pid, int *status, int flags, struct rusage *ru, pid_t *ret_pid) {
	(void)ru;
	long ret = syscall(SYS_WAIT, pid, status, flags);
	if (ret < 0)
		return -ret;
	*ret_pid = ret;
	return 0;
}

int Sysdeps<Ioctl>::operator()(int fd, unsigned long request, void *arg, int *result) {
	long ret = syscall(SYS_IOCTL, fd, request, arg);
	if (ret < 0)
		return -ret;
	*result = ret;
	return 0;
}

int Sysdeps<SetSid>::operator()(pid_t *sid) {
	long ret = syscall(SYS_SETSID);
	if (ret < 0)
		return -ret;
	*sid = ret;
	return 0;
}

int Sysdeps<OpenDir>::operator()(const char *path, int *handle) {
	long ret = syscall(SYS_OPEN, path, O_RDONLY | O_DIRECTORY | O_CLOEXEC, 0);
	if (ret < 0)
		return -ret;
	*handle = ret;
	return 0;
}

int
Sysdeps<ReadEntries>::operator()(int handle, void *buffer, size_t max_size, size_t *bytes_read) {
	long ret = syscall(SYS_GETDENTS, handle, buffer, max_size);
	if (ret < 0)
		return -ret;
	*bytes_read = ret;
	return 0;
}

int Sysdeps<Reboot>::operator()(int cmd) {
	// The kernel can only halt (no power off or restart yet). Another command
	// still goes to the kernel, which refuses a caller other than init (EPERM)
	// before it looks at the command (EINVAL), as Linux does
	long command = (unsigned int)cmd == RB_HALT_SYSTEM ? REBOOT_HALT : -1;
	long ret = syscall(SYS_REBOOT, command);
	return ret < 0 ? -ret : 0;
}

int Sysdeps<Sigprocmask>::operator()(int how, const sigset_t *set, sigset_t *retrieve) {
	uint64_t incoming, previous;
	if (set)
		memcpy(&incoming, set, sizeof(incoming));
	long ret =
	    syscall(SYS_SIGPROCMASK, how, set ? &incoming : nullptr, retrieve ? &previous : nullptr);
	if (ret < 0)
		return -ret;
	if (retrieve) {
		memset(retrieve, 0, sizeof(*retrieve));
		memcpy(retrieve, &previous, sizeof(previous));
	}
	return 0;
}

int Sysdeps<Sigpending>::operator()(sigset_t *set) {
	uint64_t waiting;
	long ret = syscall(SYS_SIGPENDING, &waiting);
	if (ret < 0)
		return -ret;
	memset(set, 0, sizeof(*set));
	memcpy(set, &waiting, sizeof(waiting));
	return 0;
}

// The kernel's struct sigaction (<mihos/signal.h>, whose names clash with mlibc's)
struct KernelSigaction {
	void (*handler)(int);
	unsigned long flags;
	void (*restorer)(void);
	uint64_t mask;
};

int
Sysdeps<Sigaction>::operator()(int signal, const struct sigaction *act, struct sigaction *oldact) {
	// No siginfo_t yet; ENOSYS makes mlibc skip pthread cancellation's handler
	if (act && (act->sa_flags & SA_SIGINFO))
		return ENOSYS;

	KernelSigaction incoming, previous;
	if (act) {
		incoming.handler = act->sa_handler;
		incoming.flags = (unsigned int)act->sa_flags | SA_RESTORER;
		incoming.restorer = __mihos_signal_restore;
		memcpy(&incoming.mask, &act->sa_mask, sizeof(incoming.mask));
	}
	long ret =
	    syscall(SYS_SIGACTION, signal, act ? &incoming : nullptr, oldact ? &previous : nullptr);
	if (ret < 0)
		return -ret;
	if (oldact) {
		memset(oldact, 0, sizeof(*oldact));
		oldact->sa_handler = previous.handler;
		oldact->sa_flags = (int)previous.flags;
		oldact->sa_restorer = previous.restorer;
		memcpy(&oldact->sa_mask, &previous.mask, sizeof(previous.mask));
	}
	return 0;
}

int Sysdeps<Tcgetattr>::operator()(int fd, struct termios *attr) {
	// The kernel's termios is the start of mlibc's (19 c_cc slots, no speeds)
	memset(attr, 0, sizeof(*attr));
	long ret = syscall(SYS_IOCTL, fd, TCGETS, attr);
	return ret < 0 ? -ret : 0;
}

int Sysdeps<Tcsetattr>::operator()(int fd, int when, const struct termios *attr) {
	int request;
	switch (when) {
		case TCSANOW:
			request = TCSETS;
			break;
		case TCSADRAIN:
			request = TCSETSW;
			break;
		case TCSAFLUSH:
			request = TCSETSF;
			break;
		default:
			return EINVAL;
	}

	long ret = syscall(SYS_IOCTL, fd, request, attr);
	return ret < 0 ? -ret : 0;
}

int Sysdeps<Tcgetpgrp>::operator()(int fd, pid_t *pgrp_out) {
	int32_t group;
	long ret = syscall(SYS_IOCTL, fd, TIOCGPGRP, &group);
	if (ret < 0)
		return -ret;
	*pgrp_out = group;
	return 0;
}

int Sysdeps<Tcsetpgrp>::operator()(int fd, pid_t pgrp) {
	// The kernel reads the group through a pointer
	int32_t group = pgrp;
	long ret = syscall(SYS_IOCTL, fd, TIOCSPGRP, &group);
	return ret < 0 ? -ret : 0;
}

int Sysdeps<Pipe>::operator()(int *fds, int flags) {
	if (flags & ~O_CLOEXEC)
		return EINVAL;
	long ret = syscall(SYS_PIPE, fds);
	if (ret < 0)
		return -ret;
	if (flags & O_CLOEXEC) {
		ret = syscall(SYS_FCNTL, fds[0], F_SETFD, FD_CLOEXEC);
		if (ret == 0)
			ret = syscall(SYS_FCNTL, fds[1], F_SETFD, FD_CLOEXEC);

		if (ret < 0) {
			syscall(SYS_CLOSE, fds[0]);
			syscall(SYS_CLOSE, fds[1]);
			return -ret;
		}
	}
	return 0;
}

int Sysdeps<Dup>::operator()(int fd, int flags, int *newfd) {
	long ret =
	    (flags & O_CLOEXEC) ? syscall(SYS_FCNTL, fd, F_DUPFD_CLOEXEC, 0) : syscall(SYS_DUP, fd);
	if (ret < 0)
		return -ret;
	*newfd = ret;
	return 0;
}

int Sysdeps<Fcntl>::operator()(int fd, int request, va_list args, int *result) {
	long arg = 0;
	switch (request) {
		case F_GETFD:
		case F_GETFL:
			break;
		case F_SETFD:
		case F_SETFL:
		case F_DUPFD:
		case F_DUPFD_CLOEXEC:
			arg = va_arg(args, int);
			break;
		default:
			return EINVAL; // the kernel knows only these six
	}
	long ret = syscall(SYS_FCNTL, fd, request, arg);
	if (ret < 0)
		return -ret;
	*result = ret;
	return 0;
}

int Sysdeps<Kill>::operator()(pid_t pid, int signal) {
	long ret = syscall(SYS_KILL, pid, signal);
	return ret < 0 ? -ret : 0;
}

int Sysdeps<Pause>::operator()() {
	long ret = syscall(SYS_PAUSE);
	return -ret;
}

int Sysdeps<SetPgid>::operator()(pid_t pid, pid_t pgid) {
	long ret = syscall(SYS_SETPGID, pid, pgid);
	return ret < 0 ? -ret : 0;
}

int Sysdeps<GetPgid>::operator()(pid_t pid, pid_t *pgid) {
	long ret = syscall(SYS_GETPGID, pid);
	if (ret < 0)
		return -ret;
	*pgid = ret;
	return 0;
}

int Sysdeps<GetSid>::operator()(pid_t pid, pid_t *sid) {
	long ret = syscall(SYS_GETSID, pid);
	if (ret < 0)
		return -ret;
	*sid = ret;
	return 0;
}

int Sysdeps<Chdir>::operator()(const char *path) {
	long ret = syscall(SYS_CHDIR, path);
	return ret < 0 ? -ret : 0;
}

int Sysdeps<GetCwd>::operator()(char *buffer, size_t size) {
	long ret = syscall(SYS_GETCWD, buffer, size);
	return ret < 0 ? -ret : 0;
}

pid_t Sysdeps<GetPid>::operator()() { return syscall(SYS_GETPID); }

pid_t Sysdeps<GetPpid>::operator()() { return syscall(SYS_GETPPID); }

uid_t Sysdeps<GetUid>::operator()() { return syscall(SYS_GETUID); }

uid_t Sysdeps<GetEuid>::operator()() { return syscall(SYS_GETEUID); }

gid_t Sysdeps<GetGid>::operator()() { return syscall(SYS_GETGID); }

gid_t Sysdeps<GetEgid>::operator()() { return syscall(SYS_GETEGID); }

// The kernel's struct mihos_stat is the start of mlibc's x86-64 struct stat
// (which adds __unused[3]); checked here so a change on either side fails the build
static_assert(offsetof(struct stat, st_ctim) == 104);

int Sysdeps<Stat>::operator()(
    fsfd_target fsfdt, int fd, const char *path, int flags, struct stat *statbuf
) {
	bool by_fd = fsfdt == fsfd_target::fd
	             || (fsfdt == fsfd_target::fd_path && (flags & AT_EMPTY_PATH) && !path[0]);
	if (fsfdt == fsfd_target::fd_path && !by_fd && fd != AT_FDCWD && path[0] != '/')
		return ENOSYS; // relative to another directory: the kernel has no *at calls yet

	memset(statbuf, 0, sizeof(*statbuf));
	// lstat (and fstatat with AT_SYMLINK_NOFOLLOW): the link itself
	long ret = by_fd ? syscall(SYS_FSTAT, fd, statbuf)
	                 : syscall(flags & AT_SYMLINK_NOFOLLOW ? SYS_LSTAT : SYS_STAT, path, statbuf);
	return ret < 0 ? -ret : 0;
}

int Sysdeps<Unlinkat>::operator()(int dirfd, const char *path, int flags) {
	if (dirfd != AT_FDCWD && path[0] != '/')
		return ENOSYS;
	if (flags & ~AT_REMOVEDIR)
		return EINVAL;
	long ret = syscall(flags & AT_REMOVEDIR ? SYS_RMDIR : SYS_UNLINK, path);
	return ret < 0 ? -ret : 0;
}

int Sysdeps<Sleep>::operator()(time_t *secs, long *nanos) {
	struct timespec request = {*secs, *nanos};
	struct timespec left = {0, 0};
	long ret = syscall(SYS_NANOSLEEP, &request, &left);
	// The time left goes back in both cases: 0 when the sleep finished,
	// what a signal cut short otherwise (the kernel writes it with -EINTR)
	*secs = left.tv_sec;
	*nanos = left.tv_nsec;
	return ret < 0 ? -ret : 0;
}

int Sysdeps<Mkdir>::operator()(const char *path, mode_t mode) {
	long ret = syscall(SYS_MKDIR, path, mode);
	return ret < 0 ? -ret : 0;
}

int Sysdeps<Rmdir>::operator()(const char *path) {
	long ret = syscall(SYS_RMDIR, path);
	return ret < 0 ? -ret : 0;
}

int Sysdeps<Rename>::operator()(const char *path, const char *new_path) {
	long ret = syscall(SYS_RENAME, path, new_path);
	return ret < 0 ? -ret : 0;
}

int Sysdeps<Utimensat>::operator()(
    int dirfd, const char *pathname, const struct timespec times[2], int flags
) {
	long ret = syscall(SYS_UTIMENSAT, dirfd, pathname, times, flags);
	return ret < 0 ? -ret : 0;
}

int
Sysdeps<Readlink>::operator()(const char *path, void *buffer, size_t max_size, ssize_t *length) {
	long ret = syscall(SYS_READLINK, path, buffer, max_size);
	if (ret < 0)
		return -ret;
	*length = ret;
	return 0;
}

int Sysdeps<Symlink>::operator()(const char *target_path, const char *link_path) {
	long ret = syscall(SYS_SYMLINK, target_path, link_path);
	return ret < 0 ? -ret : 0;
}

int Sysdeps<Chmod>::operator()(const char *pathname, mode_t mode) {
	long ret = syscall(SYS_FCHMODAT, AT_FDCWD, pathname, mode, 0);
	return ret < 0 ? -ret : 0;
}

int Sysdeps<Fchmod>::operator()(int fd, mode_t mode) {
	long ret = syscall(SYS_FCHMODAT, fd, "", mode, AT_EMPTY_PATH);
	return ret < 0 ? -ret : 0;
}

int Sysdeps<Fchmodat>::operator()(int fd, const char *pathname, mode_t mode, int flags) {
	long ret = syscall(SYS_FCHMODAT, fd, pathname, mode, flags);
	return ret < 0 ? -ret : 0;
}

int Sysdeps<Fchownat>::operator()(
    int dirfd, const char *pathname, uid_t owner, gid_t group, int flags
) {
	long ret = syscall(SYS_FCHOWNAT, dirfd, pathname, owner, group, flags);
	return ret < 0 ? -ret : 0;
}

int Sysdeps<Umask>::operator()(mode_t mode, mode_t *old) {
	*old = syscall(SYS_UMASK, mode);
	return 0;
}

int Sysdeps<Truncate>::operator()(const char *path, off_t length) {
	long ret = syscall(SYS_TRUNCATE, path, length);
	return ret < 0 ? -ret : 0;
}

int Sysdeps<Ftruncate>::operator()(int fd, size_t size) {
	long ret = syscall(SYS_FTRUNCATE, fd, size);
	return ret < 0 ? -ret : 0;
}

int Sysdeps<VmProtect>::operator()(void *pointer, size_t size, int prot) {
	long ret = syscall(SYS_MPROTECT, pointer, size, prot);
	return ret < 0 ? -ret : 0;
}

int Sysdeps<Brk>::operator()(void **out) {
	long ret = syscall(SYS_BRK, 0);
	if (ret < 0)
		return -ret;
	*out = (void *)ret;
	return 0;
}

} // namespace mlibc
