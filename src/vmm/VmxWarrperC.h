#pragma once

#include "VmxManager.hpp"
#include "VmExitCustomHandler.hpp"
EXTERN_C_START

//extern VmxManager vmxManager;
//extern VmExitHandler vmExitHandler;

/* ��װ��,��asm����� */

NTSTATUS launchVmx(Vcpu* vcpu, PVOID rsp, PVOID rip);

NTSTATUS vmExitEntryPoint(Registers64* reg);



EXTERN_C_END