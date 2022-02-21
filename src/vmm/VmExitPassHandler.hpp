#pragma once
#include "VmExitHandler.hpp"
#include "Common.hpp"
#include <intrin.h>
#include "asm.h"
#include "VmxManager.hpp"
#include "PageAccessManager.hpp"

class VmExitPassHandler :public VmExitHandler
{
public:
	/**
	 * 获取目标寄存器的地址
	 * 从保存在堆栈的客户机上下文中,取出目标寄存器的地址
	 * see:Table 27-3 Exit Qualification for Control-Register Access
	 *
	 * @param ULONG index: 目标寄存器索引
	 * @param Registers64 * pGuestRegisters: 客户机上下文
	 * @return ULONG_PTR*: 目标寄存器地址
	 */
	ULONG_PTR* getUsedRegister(ULONG index, Registers64* pGuestRegisters)
	{
		ULONG_PTR* registerUsed = nullptr;
		// clang-format off
		switch (index)
		{
		case 0: registerUsed = &pGuestRegisters->rax; break;
		case 1: registerUsed = &pGuestRegisters->rcx; break;
		case 2: registerUsed = &pGuestRegisters->rdx; break;
		case 3: registerUsed = &pGuestRegisters->rbx; break;
		case 4: registerUsed = &pGuestRegisters->rsp; break;
		case 5: registerUsed = &pGuestRegisters->rbp; break;
		case 6: registerUsed = &pGuestRegisters->rsi; break;
		case 7: registerUsed = &pGuestRegisters->rdi; break;
#if defined(_AMD64_)
		case 8: registerUsed = &pGuestRegisters->r8; break;
		case 9: registerUsed = &pGuestRegisters->r9; break;
		case 10: registerUsed = &pGuestRegisters->r10; break;
		case 11: registerUsed = &pGuestRegisters->r11; break;
		case 12: registerUsed = &pGuestRegisters->r12; break;
		case 13: registerUsed = &pGuestRegisters->r13; break;
		case 14: registerUsed = &pGuestRegisters->r14; break;
		case 15: registerUsed = &pGuestRegisters->r15; break;
#endif
		default: DbgBreakPoint(); break;
		}

		// clang-format on
		return registerUsed;
	}

protected:

	virtual void handleCpuid(Registers64* reg) override
	{
		CpuidField cpuInfo = { 0 };
		__cpuidex((int*)&cpuInfo, (int)reg->rax, (int)reg->rcx);
		reg->rax = cpuInfo.rax;
		reg->rbx = cpuInfo.rbx;
		reg->rcx = cpuInfo.rcx;
		reg->rdx = cpuInfo.rdx;
		resume();
	};


	virtual void handleInvd(Registers64* reg) override
	{
		__invd();
		resume();
	};


	virtual BOOLEAN handleVmcall(Registers64* reg) override
	{
		return TRUE;
	};


	virtual void handleCrAccess(Registers64* reg) override
	{
		ExitQualification data;
		__vmx_vmread(EXIT_QUALIFICATION, (size_t*)&data);
		ULONG_PTR* pReg = getUsedRegister(data.crAccess.generalRegister, reg);
		switch (data.crAccess.accessType)
		{
		case AccessType::MOV_TO_CR:
			switch (data.crAccess.registerNumber)
			{
			case 0:
				__vmx_vmwrite(GUEST_CR0, *pReg);
				break;
			case 3:
				__vmx_vmwrite(GUEST_CR3, *pReg);
				break;
			case 4:
				__vmx_vmwrite(GUEST_CR4, *pReg);
				break;
			default:
				DbgLog(Common::LogLevel::Error, "[%s]registerNumber:%d", __FUNCTION__, data.crAccess.registerNumber);
				DbgBreakPoint();
				break;
			}
		case AccessType::MOV_FROM_CR:
			switch (data.crAccess.registerNumber)
			{
			case 0:
				__vmx_vmread(GUEST_CR0, pReg);
				break;
			case 3:
				__vmx_vmread(GUEST_CR3, pReg);
				break;
			case 4:
				__vmx_vmread(GUEST_CR4, pReg);
				break;
			default:
				DbgLog(Common::LogLevel::Error, "[%s]accessType:%d", __FUNCTION__, data.crAccess.registerNumber);
				DbgBreakPoint();
				break;
			}
			break;
		default:
			DbgLog(Common::LogLevel::Error, "[%s]registerNumber:%d", __FUNCTION__, data.crAccess.accessType);
			DbgBreakPoint();
			break;
		}
		resume();
	};


	virtual void handleMsrRead(Registers64* reg) override
	{
		LARGE_INTEGER msrValue = { 0 };
		ULONG32 msrIndex = reg->rcx;

		switch (msrIndex)
		{
		case MSR_LSTAR:
			msrValue.QuadPart = __readmsr(MSR_LSTAR);
			break;
		default:
			msrValue.QuadPart = __readmsr(msrIndex);
			break;
		}
		reg->rax = msrValue.LowPart;
		reg->rdx = msrValue.HighPart;
		resume();
	};


	virtual void handleMsrWrite(Registers64* reg) override
	{
		LARGE_INTEGER msrValue = { 0 };
		ULONG32 msrIndex = reg->rcx;

		msrValue.LowPart = (ULONG32)reg->rax;
		msrValue.HighPart = (ULONG32)reg->rdx;
		switch (msrIndex)
		{
		case MSR_LSTAR:
			__writemsr(MSR_LSTAR, msrValue.QuadPart);
			break;
		default:
			__writemsr(msrIndex, msrValue.QuadPart);
			break;
		}
		resume();
	};


	virtual void handleMtf(Registers64* reg) override
	{
		ULONG_PTR rip = 0;
		ULONG64 faultPagePa = 0;

		__vmx_vmread(GUEST_RIP, &rip);
		__vmx_vmread(GUEST_PHYSICAL_ADDRESS, &faultPagePa);
	};


	virtual void handleEptViolation(Registers64* reg) override
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
	};


	virtual void handleEptMisconfig(Registers64* reg) override
	{
		/*
		EPT Misconfigurations
			如果 guest - physical address的转换遇到满足以下任一条件的EPT分页结构，则会发生EPT Misconfigurations：
			• 该条目的位0清除（指示不允许进行数据读取），并且将位1置1（指示允许进行数据写入）。

			• 如果处理器不支持以下任一操作，则仅执行translations ：
			— 该条目的位0被清除（指示不允许进行数据读取），并且位2被置位（指示允许进行指令提取）。
			— “用于EPT的基于模式的执行控制” VM - execution control 为1，该条目的位0被清除（指示不允许进行数据读取），
			并且已设置位10（指示允许从用户提取指令）模式线性地址）。
			软件应阅读VMX功能MSR IA32_VMX_EPT_VPID_CAP，以确定是否支持仅执行转换。

			• 该条目存在，并且具有以下条件之一：
			— 保留位被置位。这包括设置超出逻辑处理器的物理地址宽度的范围为51 : 12的位。
			有关在哪些EPT page struceture条目中保留哪些位的详细信息。
			— 该条目是最后一个用于转换guest - physical address（第7位设置为1的EPT PDE或EPT PTE），
			而第5：3位（EPT存储器类型）的值是2、3或7 （这些值是保留的）。
			当为EPT paging - structure条目配置了保留用于将来功能的设置时，会导致EPT misconfigurations 。
			developer应注意，将来可能会使用此类设置，并且导致一个处理器上的EPT配置错误的EPT paging - structure条目将来可能不会使用。
		*/
		DbgBreakPoint();
		KdPrint(("EptMisconfiguration\n"));
	};

};