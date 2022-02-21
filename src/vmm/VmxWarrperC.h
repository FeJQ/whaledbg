#pragma once

#include "VmxManager.hpp"
#include "VmExitCustomHandler.hpp"
EXTERN_C_START

//extern VmxManager vmxManager;
//extern VmExitHandler vmExitHandler;

/* 包装层,供asm里调用 */

NTSTATUS launchVmx(Vcpu* vcpu, PVOID rsp, PVOID rip);

NTSTATUS vmExitEntryPoint(Registers64* reg);



EXTERN_C_END