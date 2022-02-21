#pragma once
#include "Common.hpp"
#include "iA32.h"


class VmExitHandler
{
public:
	/**
	 * vmexit事件入口
	 * @param Vcpu * vcpu:
	 * @return continue vmx?
	 */
	BOOLEAN vmExitEntryPoint(Registers64* reg)
	{
		VmExitInformation exitReson = { 0 };

		//查询导致vmexit的原因 
		// See:AppendixC VMX Basic Exit Reson ,Table C-1 Basic Exit Reson
		__vmx_vmread(VM_EXIT_REASON, (ULONG_PTR*)&exitReson);

		BOOLEAN continueVmx = TRUE;
		ULONG_PTR rip = 0;
		ULONG_PTR rsp = 0;
		ULONG_PTR guestPhysicalAddress = 0;
		__vmx_vmread(GUEST_RIP, &rip);
		__vmx_vmread(GUEST_RSP, &rsp);
		__vmx_vmread(GUEST_PHYSICAL_ADDRESS, &guestPhysicalAddress);

		switch (exitReson.fields.reason)
		{
		case VmExitReason::Cpuid:
			handleCpuid(reg);
			break;
		case VmExitReason::Invd:
			handleInvd(reg);
			break;
		case VmExitReason::Vmcall:
			continueVmx = handleVmcall(reg);
			if (continueVmx) resume();
			break;
		case VmExitReason::CrAccess:
			handleCrAccess(reg);
			break;
		case VmExitReason::MsrRead:
			handleMsrRead(reg);
			break;
		case VmExitReason::MsrWrite:
			handleMsrWrite(reg);
			break;
		case VmExitReason::Mtf:
			handleMtf(reg);
			break;
		case VmExitReason::EptViolation:
			handleEptViolation(reg);
			break;
		case VmExitReason::EptMisconfig:
			handleEptMisconfig(reg);
			break;
		default:
			DbgBreakPoint();
			break;
		}

		return continueVmx;
	}
protected:
	/**
	 * 恢复客户机rip到导致vmexit指令的下一条指令,
	 */
	void resume()
	{
		ULONG instLen = 0;
		ULONG_PTR rip = 0;
		__vmx_vmread(GUEST_RIP, &rip);
		__vmx_vmread(VM_EXIT_INSTRUCTION_LEN, (size_t*)&instLen);
		__vmx_vmwrite(GUEST_RIP, rip + instLen);
	}

	/**
	 * 执行cpuid指令而导致的vmexit事件的处理函数
	 * @param Registers64 * pGuestRegisters:客户机通用寄存器
	 */
	virtual void handleCpuid(Registers64* reg) = 0;

	/**
	 * 执行invd指令而导致的vmexit事件的处理函数
	 * @param Registers64 * reg:客户机通用寄存器
	 */
	virtual void handleInvd(Registers64* reg) = 0;

	/**
	 * 执行vmcall指令而导致的vmexit事件的处理函数
	 * @param Registers64 * reg: 客户机通用寄存器
	 * @return 是否继续执行vmx
	 */
	virtual BOOLEAN handleVmcall(Registers64* reg) = 0;

	/**
	 * 访问(读/写)控制寄存器而导致的vmexit事件的处理函数
	 * @param Registers64 * reg: 客户机通用寄存器
	 */
	virtual void handleCrAccess(Registers64* reg) = 0;

	/**
	 * 读取msr寄存器而导致的vmexit事件的处理函数
	 * @param Registers64 * reg: 客户机通用寄存器
	 */
	virtual void handleMsrRead(Registers64* reg) = 0;

	/**
	 * 写入msr寄存器而导致的vmexit事件的处理函数
	 * @param Registers64 * reg: 客户机通用寄存器
	 */
	virtual void handleMsrWrite(Registers64* reg) = 0;

	/**
	 * 开启MTF后执行指令所产生的vmexit处理函数
	 * @param Registers64 * reg: 客户机通用寄存器
	 */
	virtual void handleMtf(Registers64* reg) = 0;

	/**
	 * 内存属性不匹配导致的vmexit处理函数
	 * @param Registers64 * reg: 客户机通用寄存器
	 */
	virtual void handleEptViolation(Registers64* reg) = 0;

	/**
	 * 内存属性错误导致的vmexit处理函数
	 * @param Registers64 * pGuestRegisters: 客户机通用寄存器
	 */
	virtual void handleEptMisconfig(Registers64* reg) = 0;


protected:

	

};