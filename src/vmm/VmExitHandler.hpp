#pragma once
#include "Common.hpp"
#include "IA32.h"
#include <intrin.h>
#include "asm.h"
#include "VmxManager.hpp"

class VmExitHandler
{
public:

	/**
	 * vmexit事件入口
	 *
	 * @param Registers64 * pGuestRegisters:
	 * @return NTSTATUS:
	 */
	NTSTATUS vmExitEntryPoint(Registers64* pGuestRegisters)
	{
		VmExitInformation exitReson = { 0 };
		BOOLEAN isVmresume = TRUE;
		//查询导致vmexit的原因 
		// See:AppendixC VMX Basic Exit Reson ,Table C-1 Basic Exit Reson
		__vmx_vmread(VM_EXIT_REASON, (ULONG_PTR*)&exitReson);

		ULONG_PTR rip = 0;
		ULONG_PTR rsp = 0;
		ULONG_PTR guestPhysicalAddress = 0;
		__vmx_vmread(GUEST_RIP, &rip);
		__vmx_vmread(GUEST_RSP, &rsp);
		__vmx_vmread(GUEST_PHYSICAL_ADDRESS, &guestPhysicalAddress);


		switch (exitReson.fields.reason)
		{
		case VmExitReason::Cpuid:
			VmExitCpuid(pGuestRegisters);
			break;
		case VmExitReason::Invd:
			VmExitInvd(pGuestRegisters);
			break;
		case VmExitReason::Vmcall:
			VmExitVmcall(pGuestRegisters, isVmresume);
			break;
		case VmExitReason::CrAccess:
			VmExitCrAccess(pGuestRegisters);
			break;
		case VmExitReason::MsrRead:
			VmExitMsrRead(pGuestRegisters);
			break;
		case VmExitReason::MsrWrite:
			VmExitMsrWrite(pGuestRegisters);
			break;
		case VmExitReason::Mtf:
			VmExitMtf(pGuestRegisters);
			break;
		case VmExitReason::EptViolation:
			VmExitEptViolation(pGuestRegisters);
			break;
		case VmExitReason::EptMisconfig:
			VmExitEptMisconfiguration(pGuestRegisters);
			break;
		default:
			DbgBreakPoint();
			break;
		}

		return isVmresume;
	}

private:

	/**
	 * 恢复客户机的Rip,执行"导致vmexit的指令"的下一条指令
	 */
	void resumeGuestRip()
	{
		ULONG instLen = 0;
		ULONG_PTR rip = 0;
		__vmx_vmread(GUEST_RIP, &rip);
		__vmx_vmread(VM_EXIT_INSTRUCTION_LEN, (size_t*)&instLen);
		__vmx_vmwrite(GUEST_RIP, rip + instLen);
	}

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

	/**
	 * 执行cpuid指令而导致的vmexit事件的处理函数
	 *
	 * @param Registers64 * pGuestRegisters:客户机通用寄存器
	 */
	void VmExitCpuid(Registers64* pGuestRegisters)
	{
		CpuidField cpuInfo = { 0 };
		__cpuidex((int*)&cpuInfo, (int)pGuestRegisters->rax, (int)pGuestRegisters->rcx);
		pGuestRegisters->rax = cpuInfo.rax;
		pGuestRegisters->rbx = cpuInfo.rbx;
		pGuestRegisters->rcx = cpuInfo.rcx;
		pGuestRegisters->rdx = cpuInfo.rdx;
		resumeGuestRip();
	}
	/**
	 * 执行invd指令而导致的vmexit事件的处理函数
	 *
	 * @param Registers64 * pGuestRegisters:客户机通用寄存器
	 */
	void VmExitInvd(Registers64* pGuestRegisters)
	{
		__invd();
		resumeGuestRip();
	}

	/**
	 * 执行vmcall指令而导致的vmexit事件的处理函数
	 *
	 * @param Registers64 * pGuestRegisters: 客户机通用寄存器
	 * @param BOOLEAN & isVmresume: 是否继续执行vmresume
	 */
	void VmExitVmcall(Registers64* pGuestRegisters, BOOLEAN& isVmresume)
	{
	}

	/**
	 * 访问(读/写)控制寄存器而导致的vmexit事件的处理函数
	 *
	 * @param Registers64 * pGuestRegisters: 客户机通用寄存器
	 */
	void VmExitCrAccess(Registers64* pGuestRegisters)
	{
		ExitQualification data;
		__vmx_vmread(EXIT_QUALIFICATION, (size_t*)&data);
		ULONG_PTR* pReg = getUsedRegister(data.crAccess.generalRegister, pGuestRegisters);
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
				Common::log(Common::LogLevel::Error, "[%s]registerNumber:%d", __FUNCTION__, data.crAccess.registerNumber);
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
				Common::log(Common::LogLevel::Error, "[%s]accessType:%d", __FUNCTION__, data.crAccess.registerNumber);
				DbgBreakPoint();
				break;
			}
			break;
		default:
			Common::log(Common::LogLevel::Error, "[%s]registerNumber:%d", __FUNCTION__, data.crAccess.accessType);
			DbgBreakPoint();
			break;
		}
		resumeGuestRip();
	}

	/**
	 * 读取msr寄存器而导致的vmexit事件的处理函数
	 *
	 * @param Registers64 * pGuestRegisters: 客户机通用寄存器
	 */
	void VmExitMsrRead(Registers64* pGuestRegisters)
	{
		LARGE_INTEGER msrValue = { 0 };
		ULONG32 msrIndex = pGuestRegisters->rcx;
		VmxCpuContext* currentCpuContext = &VmxManager::getStaticVmxContext()[Util::currentCpuIndex()];
		switch (msrIndex)
		{
		case MSR_LSTAR:
			if (currentCpuContext->originalLstar)
			{
				msrValue.QuadPart = currentCpuContext->originalLstar;
			}
			else
			{
				msrValue.QuadPart = __readmsr(MSR_LSTAR);
			}
			break;
		default:
			msrValue.QuadPart = __readmsr(msrIndex);
			break;
		}
		pGuestRegisters->rax = msrValue.LowPart;
		pGuestRegisters->rdx = msrValue.HighPart;
		resumeGuestRip();
	}

	/**
	 * 写入msr寄存器而导致的vmexit事件的处理函数
	 *
	 * @param Registers64 * pGuestRegisters: 客户机通用寄存器
	 */
	void VmExitMsrWrite(Registers64* pGuestRegisters)
	{
		LARGE_INTEGER msrValue = { 0 };
		ULONG32 msrIndex = pGuestRegisters->rcx;
		VmxCpuContext* currentCpuContext = &VmxManager::getStaticVmxContext()[Util::currentCpuIndex()];
		msrValue.LowPart = (ULONG32)pGuestRegisters->rax;
		msrValue.HighPart = (ULONG32)pGuestRegisters->rdx;
		switch (msrIndex)
		{
		case MSR_LSTAR:
			if (currentCpuContext->originalLstar == NULL)
			{
				__writemsr(MSR_LSTAR, msrValue.QuadPart);
			}
			break;
		default:
			__writemsr(msrIndex, msrValue.QuadPart);
			break;
		}
		resumeGuestRip();
	}

	/**
	 * 开启MTF后执行指令所产生的vmexit处理函数
	 *
	 * @param Registers64 * pGuestRegisters: 客户机通用寄存器
	 */
	void VmExitMtf(Registers64* pGuestRegisters)
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
	void VmExitEptViolation(Registers64* pGuestRegisters)
	{
		ULONG64 guestPhysicalAddress = 0;
		ULONG_PTR guestVirtualAddress = 0;
		ExitQualification data;
		ULONG_PTR rip = 0;
		PageEntry* pageEntry = NULL;

		// 获取全局PageEntry对象
		PageEntry* staticPageEntry = Ept::getStaticPageEntry();

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
			ULONG_PTR pageAddressHeadPa = (ULONG_PTR)PAGE_ALIGN((PVOID)Util::vaToPa((PVOID)tempPgEntry->pageAddressVa));
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
			pageEntry->pte->all = (ULONG64)PAGE_ALIGN(Util::vaToPa((PVOID)pageEntry->excutePage));
			pageEntry->pte->fields.readAccess = false;
			pageEntry->pte->fields.writeAccess = false;
			pageEntry->pte->fields.executeAccess = true;
			pageEntry->pte->fields.memoryType = MemoryType::WriteBack;
		}
	}

	/**
	 * 内存属性错误导致的vmexit处理函数
	 *
	 * @param Registers64 * pGuestRegisters: 客户机通用寄存器
	 */
	void VmExitEptMisconfiguration(Registers64* pGuestRegisters)
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
	}

	private:
		

};