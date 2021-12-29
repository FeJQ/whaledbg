#pragma once

#include "VmxManager.hpp"
#include "VmExitHandler.hpp"
EXTERN_C_START

extern VmxManager vmxManager;
extern VmExitHandler vmExitHandler;

/* ��װ��,��asm����� */
NTSTATUS launchVmx(PVOID guestStack, PVOID guestResumeRip);
NTSTATUS vmExitEntryPoint(Registers64* pGuestRegisters);

/**
 * ����vmx
 *
 * @return NTSTATUS: 
 */
 NTSTATUS vmxStart();

/**
 * �ر�vmx
 *
 * @return NTSTATUS: 
 */
 NTSTATUS vmxStop();

EXTERN_C_END