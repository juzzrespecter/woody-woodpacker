#include "format_elf.h"
#include <cpuid.h> // __get_cpuid
// Valid Set of Instructions
static int cpu_has_sse(int edx) {
	return (edx & (1 << 25)) != 0;
}

static int os_supports_xmm(int edx) {
	return (edx & (1 << 24)) != 0; // FXSR
}
//! FXSR floating-point context fast save and restore.

static int cpu_supports_cmov(int edx) {
	return (edx & (1 << 14)) != 0; // CMOV
}

bool valid_cpu_x86(void)
{
	unsigned int eax;
	unsigned int ebx;
	unsigned int ecx;
	unsigned int edx;

	if (!__get_cpuid(1, &eax, &ebx, &ecx, &edx))
		return (0);
	return (cpu_has_sse(edx)
		&& os_supports_xmm(edx)
		&& cpu_supports_cmov(edx));
}
