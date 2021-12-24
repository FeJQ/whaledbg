#pragma once
#include "Common.hpp"

EXTERN_C_BEGIN

EXTERN_C  PVOID __stdcall __getPebAddress();

EXTERN_C inline void __stdcall __devideByZero();

ASM ULONG64 __readcs();
ASM ULONG64 __readds();
ASM ULONG64 __reades();
ASM ULONG64 __readss();
ASM ULONG64 __readfs();
ASM ULONG64 __readgs();
ASM ULONG64 __readldtr();
ASM ULONG64 __readtr();
ASM ULONG64 __getidtbase();
ASM ULONG64 __getidtlimit();
ASM ULONG64 __getgdtbase();
ASM ULONG64 __getgdtlimit();
ASM void __invd();
ASM void __vmcall(VmcallReason vmcallReason, VmxoffContext* context);
ASM void __lgdt(void* gdtr);
ASM void __pushall();
ASM void __popall();

ASM void __svreg(Registers64* reg);
ASM void __ldreg(Registers64* reg);

EXTERN_C_END