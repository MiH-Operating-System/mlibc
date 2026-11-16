#pragma once

#include <mlibc/sysdep-signatures.hpp>

namespace mlibc {

// The sysdeps MiHOS implements: a tag listed here must have its
// Sysdeps<Tag>::operator() defined in sysdeps.cpp; one left out is reported
// to mlibc as not implemented (the function returns ENOSYS or falls back).
struct MihosSysdepTags :
	// The mandatory ones (all-sysdeps.hpp)
	Exit,
	FutexWait,
	FutexWake,
	Open,
	Read,
	Write,
	Seek,
	Close,
	ClockGet,
	LibcLog,
	LibcPanic,
	AnonAllocate,
	AnonFree,
	VmMap,
	VmUnmap,
	TcbSet,
	// Called unconditionally by options this port enables
	Isatty, // stdio, for every stream (ansi)
	Dup2,   // openpty, login_tty (bsd)
	// D10 batch 1: init and the test runner
	Fork,
	Execve,
	Waitpid,
	Ioctl,
	SetSid,
	Reboot,
	OpenDir,
	ReadEntries
{};

template<typename Tag>
using Sysdeps = SysdepOf<MihosSysdepTags, Tag>;

struct SysdepTraits {
	static constexpr bool usesRtNetlink = false;
};

} // namespace mlibc
