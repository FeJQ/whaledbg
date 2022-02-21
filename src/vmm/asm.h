#pragma once
#include "Common.hpp"
#include "IA32.h"

EXTERN_C_BEGIN

enum VmcallNumber
{
	Exit = 54886475,
	VmcallLstarHookEnable,
	VmcallLstarHookDisable,
};

struct VmcallParam
{
	ULONG_PTR eptHookAddress; // 填写要hook的函数的地址

};

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
ASM void __vmcall(VmcallNumber vmcallNumber, VmcallParam* param);
ASM void __lgdt(void* gdtr);
ASM void __pushall();
ASM void __popall();

ASM NTSTATUS __stdcall __vmlaunch(void* routine,void* vcpu);

ASM void __svreg(Registers64* reg);
ASM void __ldreg(Registers64* reg);


/**
 * asm64.asm里的vmm入口点
 *
 * @return ASM void __stdcall: 
 */
ASM void __stdcall __vmm_entry_point();

/**
 * asm64.asm里的加载段描述符访问权限
 *
 * @param _In_ ULONG_PTR segmentSelector: 
 * @return ULONG_PTR __stdcall: 
 */
 ULONG_PTR __stdcall __load_access_rights_byte(_In_ ULONG_PTR segmentSelector);

EXTERN_C_END