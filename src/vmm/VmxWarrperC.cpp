#include "VmxWarrperC.h"

EXTERN_C_START
VmExitCustomHandler customHandler;
VmExitHandler* vmexitHandle = &customHandler;

NTSTATUS launchVmx(Vcpu* vcpu, PVOID rsp, PVOID rip)
{
	 return VmxManager::instance().launchVmx(vcpu, rsp, rip);
}

NTSTATUS vmExitEntryPoint(Registers64* reg)
{
	return vmexitHandle->vmExitEntryPoint(reg);
}

EXTERN_C_END