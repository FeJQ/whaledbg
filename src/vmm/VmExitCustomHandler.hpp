#pragma once
#include "Common.hpp"
#include "IA32.h"
#include <intrin.h>
#include "asm.h"
#include "VmxManager.hpp"
#include "PageAccessManager.hpp"
#include "VmExitPassHandler.hpp"

class VmExitCustomHandler :public VmExitPassHandler
{
public:



private:


	/**
	 * 执行vmcall指令而导致的vmexit事件的处理函数
	 *
	 * @param Registers64 * pGuestRegisters: 客户机通用寄存器
	 * @param BOOLEAN & isVmresume: 是否继续执行vmresume
	 */
	BOOLEAN handleVmcall(Registers64* reg) override
	{
		VmcallNumber vmcallNum = (VmcallNumber)reg->rcx;
		VmcallParam* param = (VmcallParam*)reg->rdx;

		BOOLEAN continueVmx = TRUE;
		switch (vmcallNum)
		{
		case Exit:
		{
			continueVmx = FALSE;

			/*
			当发生VM退出时，处理器将IDT和GDT的Limit设置为ffff。
			这里把它改回正确的值
			*/
			ULONG_PTR gdtLimit = 0;
			__vmx_vmread(GUEST_GDTR_LIMIT, &gdtLimit);

			ULONG_PTR gdtBase = 0;
			__vmx_vmread(GUEST_GDTR_BASE, &gdtBase);
			ULONG_PTR idtLimit = 0;
			__vmx_vmread(GUEST_IDTR_LIMIT, &idtLimit);
			ULONG_PTR idtBase = 0;
			__vmx_vmread(GUEST_IDTR_BASE, &idtBase);

			Gdtr gdtr = { (USHORT)gdtLimit, gdtBase };
			Idtr idtr = { (USHORT)(idtLimit), idtBase };
			__lgdt(&gdtr);
			__lidt(&idtr);


			//跳过VmCall指令
			ULONG instLen = 0;
			ULONG_PTR rip = 0;
			__vmx_vmread(GUEST_RIP, &rip);
			__vmx_vmread(VM_EXIT_INSTRUCTION_LEN, (size_t*)&instLen);
			ULONG_PTR returnAddress = rip + instLen;


			// Since the flag register is overwritten after VMXOFF, we should manually
			// indicates that VMCALL was successful by clearing those flags.
			// See: CONVENTIONS
			RFLAGS rflags = { 0 };
			__vmx_vmread(GUEST_RFLAGS, (SIZE_T*)&rflags);

			rflags.fields.CF = false;
			rflags.fields.PF = false;
			rflags.fields.AF = false;
			rflags.fields.ZF = false;
			rflags.fields.SF = false;
			rflags.fields.OF = false;


			// Set registers used after VMXOFF to recover the context. Volatile
			// registers must be used because those changes are reflected to the
			// guest's context after VMXOFF.
			reg->rcx = returnAddress;
			__vmx_vmread(GUEST_RSP, &reg->rdx);
			reg->r8 = rflags.all;

			break;
		}
		case VmcallLstarHookEnable:
			break;
		case VmcallLstarHookDisable:
			break;
		default:
			break;
		}
		return continueVmx;
	}


	/**
	 * 开启MTF后执行指令所产生的vmexit处理函数
	 *
	 * @param Registers64 * pGuestRegisters: 客户机通用寄存器
	 */
	void handleMtf(Registers64* reg) override
	{
		ULONG_PTR rip = 0;
		ULONG64 faultPagePa = 0;

		__vmx_vmread(GUEST_RIP, &rip);
		__vmx_vmread(GUEST_PHYSICAL_ADDRESS, &faultPagePa);
	}

	/**
	 * 内存属性不匹配导致的vmexit处理函数
	 *
	 * @param Registers64 * pGuestRegisters: 客户机通用寄存器
	 */
	void handleEptViolation(Registers64* reg) override
	{
		ULONG64 guestPhysicalAddress = 0;
		ULONG_PTR guestVirtualAddress = 0;
		ExitQualification data;
		ULONG_PTR rip = 0;
		PageEntry* pageEntry = NULL;

		// 获取全局PageEntry对象
		PageEntry* staticPageEntry = Ept::instance().getPageEntry();

		// GuestPhysicalAddress 是出错的地址,而 GuestRip 是导致出错的指令的地址
		// 如,地址 0x123456 为不可读的页面,而 0x654321 处尝试去读 0x123456 所在的页
		// 则 GuestRip=0x654321,GuestPhysicalAddress=0x123456

		__vmx_vmread(GUEST_PHYSICAL_ADDRESS, &guestPhysicalAddress);
		__vmx_vmread(GUEST_RIP, &rip);
		__vmx_vmread(EXIT_QUALIFICATION, (size_t*)&data);

		guestVirtualAddress = (ULONG_PTR)Util::paToVa(guestPhysicalAddress);
		DbgBreakPoint();
		//获取替换过的页面的相关数据
		for (LIST_ENTRY* pLink = staticPageEntry->pageList.Flink; pLink != (PLIST_ENTRY)&staticPageEntry->pageList; pLink = pLink->Flink)
		{
			PageEntry* tempPgEntry = CONTAINING_RECORD(pLink, PageEntry, pageList);
			ULONG_PTR pageAddressHeadPa = (ULONG_PTR)PAGE_ALIGN((PVOID)Util::vaToPa((PVOID)tempPgEntry->targetPageAddress));
			ULONG_PTR guestAddressHeadPa = (ULONG_PTR)PAGE_ALIGN(guestPhysicalAddress);
			if (pageAddressHeadPa == guestAddressHeadPa)
			{
				pageEntry = tempPgEntry;
				break;
			}
		}
		if (pageEntry == NULL)
		{
			DbgBreakPoint();
			return;
		}
		if (data.eptViolation.readAccess)
		{
			// 读不可读的内存页导致的vmexit		
			pageEntry->pte->all = (ULONG64)PAGE_ALIGN(Util::vaToPa((PVOID)pageEntry->readPage));
			pageEntry->pte->fields.readAccess = true;
			pageEntry->pte->fields.writeAccess = false;
			pageEntry->pte->fields.executeAccess = false;
			pageEntry->pte->fields.memoryType = MemoryType::WriteBack;
		}
		else if (data.eptViolation.writeAccess)
		{
			// 写不可写的内存页导致的vmexit	
			pageEntry->pte->all = (ULONG64)PAGE_ALIGN(Util::vaToPa((PVOID)pageEntry->writePage));
			pageEntry->pte->fields.readAccess = true;
			pageEntry->pte->fields.writeAccess = true;
			pageEntry->pte->fields.executeAccess = false;
			pageEntry->pte->fields.memoryType = MemoryType::WriteBack;
		}
		else if (data.eptViolation.executeAccess)
		{
			// 执行不可执行的内存页导致的vmexit
			pageEntry->pte->all = (ULONG64)PAGE_ALIGN(Util::vaToPa((PVOID)pageEntry->executePage));
			pageEntry->pte->fields.readAccess = false;
			pageEntry->pte->fields.writeAccess = false;
			pageEntry->pte->fields.executeAccess = true;
			pageEntry->pte->fields.memoryType = MemoryType::WriteBack;
		}
	}



private:


};