#pragma once

#include "VmxManager.hpp"
#include "VmExitHandler.hpp"
EXTERN_C_START

extern VmxManager vmxManager;
extern VmExitHandler vmExitHandler;

/* 包装层,供asm里调用 */
NTSTATUS launchVmx(PVOID guestStack, PVOID guestResumeRip);
NTSTATUS vmExitEntryPoint(Registers64* pGuestRegisters);

/**
 * 启动vmx
 *
 * @return NTSTATUS: 
 */
 NTSTATUS vmxStart();

/**
 * 关闭vmx
 *
 * @return NTSTATUS: 
 */
 NTSTATUS vmxStop();

EXTERN_C_END