/* $Id: //depot/blt/kernel/cpuid.S#3 $
**
** Copyright 1998 Sidney Cammeresi.  All rights reserved.
** Distributed under the terms of the OpenBLT License.
*/

#include "smp.h"

.section ".text.init"

.globl smp_check_cpu

smp_check_cpu:
		#.byte 0x66
		pushfl
		#.byte 0x66
		popl %eax
		movl %eax, %ebx
		andl $0x00200000, %eax
		jz not_pentium_or_newer

		xor %eax, %eax
		cpuid
		mov %eax, cpuid_max_level

		cmp %ebx, CPUID_GEN_EBX
		jne not_genuine_intel
		cmp %ecx, CPUID_GEN_ECX
		jne not_genuine_intel
		cmp %edx, CPUID_GEN_EDX
		jne not_genuine_intel

		mov %eax, cpuid_eax
		mov %edx, cpuid_edx
		mov $1, %eax
		ret

not_pentium_or_newer:
not_genuine_intel:
		xor %eax, %eax
		mov %ebx, %eax
		ret

