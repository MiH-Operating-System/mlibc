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
	TcbSet
{};

template<typename Tag>
using Sysdeps = SysdepOf<MihosSysdepTags, Tag>;

struct SysdepTraits {
	static constexpr bool usesRtNetlink = false;
};

} // namespace mlibc
