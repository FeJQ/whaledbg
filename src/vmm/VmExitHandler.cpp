#include "VmExitHandler.h"
#include "asm.h"
#include "Ept.h"
#include "Util.hpp"
#include "VtxInstruction.h"
#include <ntddk.h>
#include "VmxManager.h"

namespace vmm
{
	namespace vmexit
	{
		using ept::PteEntry;
		using ept::HookedPage;

		BOOLEAN vmExitEntryPoint(Registers64* reg)
		{
			VmExitInformation exitReason = vtx::readVmexitReason();

			BOOLEAN continueVmx = TRUE;
			ULONG_PTR rip = vtx::readGuestRip();
			ULONG_PTR rsp = vtx::readGuestRsp();
			ULONG_PTR guestPhysicalAddress = 0;
			__vmx_vmread(GUEST_PHYSICAL_ADDRESS, &guestPhysicalAddress);

			switch (exitReason.fields.reason)
			{
			case VmExitReason::Cpuid:
				handleCpuid(reg);
				break;
			case VmExitReason::Invd:
				handleInvd(reg);
				break;
			case VmExitReason::Vmcall:
				continueVmx = handleVmcall(reg);
				if (continueVmx) resumeToNextInstruction();
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

		void resumeToNextInstruction()
		{
			ULONG instLen = 0;
			ULONG_PTR rip = vtx::readGuestRip();

			__vmx_vmread(VM_EXIT_INSTRUCTION_LEN, (size_t*)&instLen);
			vtx::setGuestRip(rip + instLen);
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
		ULONG_PTR* getUsedRegister(ULONG index, Registers64* guestRegisters)
		{
			ULONG_PTR* registerUsed = nullptr;
			// clang-format off
			switch (index)
			{
			case 0: registerUsed = &guestRegisters->rax; break;
			case 1: registerUsed = &guestRegisters->rcx; break;
			case 2: registerUsed = &guestRegisters->rdx; break;
			case 3: registerUsed = &guestRegisters->rbx; break;
			case 4: registerUsed = &guestRegisters->rsp; break;
			case 5: registerUsed = &guestRegisters->rbp; break;
			case 6: registerUsed = &guestRegisters->rsi; break;
			case 7: registerUsed = &guestRegisters->rdi; break;
#if defined(_AMD64_)
			case 8: registerUsed = &guestRegisters->r8; break;
			case 9: registerUsed = &guestRegisters->r9; break;
			case 10: registerUsed = &guestRegisters->r10; break;
			case 11: registerUsed = &guestRegisters->r11; break;
			case 12: registerUsed = &guestRegisters->r12; break;
			case 13: registerUsed = &guestRegisters->r13; break;
			case 14: registerUsed = &guestRegisters->r14; break;
			case 15: registerUsed = &guestRegisters->r15; break;
#endif
			default: DbgBreakPoint(); break;
			}

			// clang-format on
			return registerUsed;
		}


		void handleCpuid(Registers64* reg)
		{
			CpuidField cpuInfo = { 0 };
			__cpuidex((int*)&cpuInfo, (int)reg->rax, (int)reg->rcx);
			reg->rax = cpuInfo.rax;
			reg->rbx = cpuInfo.rbx;
			reg->rcx = cpuInfo.rcx;
			reg->rdx = cpuInfo.rdx;
			resumeToNextInstruction();
		}

		void handleInvd(Registers64* reg)
		{
			__invd();
			resumeToNextInstruction();
		}

		BOOLEAN handleVmcall(Registers64* reg)
		{
			VmcallReason reson = (VmcallReason)reg->rcx;
			VmcallParam* param = (VmcallParam*)reg->rdx;

			BOOLEAN continueVmx = TRUE;
			switch (reson)
			{
			case VmcallReason::Exit:
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

				// 获取guest的返回地址
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
			case VmcallReason::kHookNtLoadDriver:
			{
				DbgBreakPoint();
				NTSTATUS status = STATUS_SUCCESS;
				//UNICODE_STRING strRoutineName;
				//RtlInitUnicodeString(&strRoutineName, L"NtLoadDriver");
				//PVOID t_NtLoadDriver = MmGetSystemRoutineAddress(&strRoutineName);

				PVOID t_NtLoadDriver = (PVOID)0xFFFFF80002EF51F0;

				status = ept::hidePage(t_NtLoadDriver);
				ASSERT(status == STATUS_SUCCESS);
				Util::disableWriteProtect();
				*(char*)t_NtLoadDriver = 0xCC;
				Util::enableWriteProtect();
				break;
			}
			default:
				break;
			}
			return continueVmx;
		}

		void handleCrAccess(Registers64* reg)
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
			resumeToNextInstruction();
		}

		void handleMsrRead(Registers64* reg)
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
			resumeToNextInstruction();
		}

		void handleMsrWrite(Registers64* reg)
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
			resumeToNextInstruction();
		}

		void handleMtf(Registers64* reg)
		{
			ULONG_PTR rip = vtx::readGuestRip();
			PteEntry* pte = ept::getPtEntry(Util::vaToPa(rip));
			pte->fields
		}

		void handleEptViolation(Registers64* reg)
		{
			ULONG64 violationPa = 0;
			ExitQualification data;
			ULONG_PTR rip = 0;
			ULONG_PTR linearAddr = 0;


			// violationPage 是出错的地址,而 rip 是导致出错的指令的地址
			// 如,地址 0x123456 为不可读的页面,而 0x654321 处尝试去读 0x123456 所在的页
			// 则 rip=0x654321,violationPage=0x123456
			__vmx_vmread(EXIT_QUALIFICATION, (size_t*)&data);
			__vmx_vmread(GUEST_PHYSICAL_ADDRESS, &violationPa);
			__vmx_vmread(GUEST_RIP, &rip);


			if (data.eptViolation.validGuestLinearAddress)
			{
				__vmx_vmread(GUEST_LINEAR_ADDRESS, &linearAddr);
			}

			DbgBreakPoint();
			ULONG_PTR violationAddress = (ULONG_PTR)Util::paToVa(violationPa);
			PteEntry* pte = ept::getPtEntry((ULONG_PTR)PAGE_ALIGN(violationPa));


			HookedPage* hookedPage = NULL;

			// 获取替换过的页面的相关数据
			for (LIST_ENTRY* pLink = ept::eptState.hookedPage.listEntry.Flink; pLink != &ept::eptState.hookedPage.listEntry; pLink = pLink->Flink)
			{
				HookedPage* tempHookedPage = CONTAINING_RECORD(pLink, HookedPage, listEntry);
				ULONG_PTR hookedPa = Util::vaToPa((PVOID)tempHookedPage->hookedPageAddress);

				// 判断发生ept violation的页是否与hooked页为同一个页
				if ((ULONG_PTR)PAGE_ALIGN(violationPa) == (ULONG_PTR)PAGE_ALIGN(hookedPa))
				{
					hookedPage = tempHookedPage;
					break;
				}
			}
			if (hookedPage == NULL)
			{
				DbgBreakPoint();
				return;
			}
			DbgBreakPoint();

			if (data.eptViolation.readAccess)
			{
				// 读不可读的内存页导致的vmexit		
				hookedPage->pte->all = (ULONG64)PAGE_ALIGN(Util::vaToPa((PVOID)hookedPage->readPage));
			}
			else if (data.eptViolation.writeAccess)
			{
				// 写不可写的内存页导致的vmexit	
				hookedPage->pte->all = (ULONG64)PAGE_ALIGN(Util::vaToPa((PVOID)hookedPage->writePage));
			}
			else if (data.eptViolation.executeAccess)
			{
				// 执行不可执行的内存页导致的vmexit
				hookedPage->pte->all = (ULONG64)PAGE_ALIGN(Util::vaToPa((PVOID)hookedPage->executePage));
			}
			PteEntry* ripPte = ept::getPtEntry((ULONG_PTR)PAGE_ALIGN(Util::vaToPa(rip)));
			EptAccess oldViolationPageAccess = static_cast<EptAccess>(hookedPage->pte->fields.readAccess | hookedPage->pte->fields.writeAccess | hookedPage->pte->fields.executeAccess);
			EptAccess oldViolationRipPageAccess = static_cast<EptAccess>(ripPte->fields.readAccess | ripPte->fields.writeAccess | ripPte->fields.executeAccess);

			_disable();
			// 分别保存发生Ept violation时的rip和violationPa所在的物理页的地址, 以及他们的EptAccess
			// 之后给EptAccess::all权限, 并返回guest,执行一行指令后会陷入mtfHanlder, 此时再将这两个页面的EptAccess复原,并执行invept(或许只需要操作violationPa就可以了)
			vmx::vcpu->eptViolationPage= (ULONG_PTR)PAGE_ALIGN(violationPa);
			vmx::vcpu->eptViolationRipPage = (ULONG_PTR)PAGE_ALIGN(Util::vaToPa(rip));
			vmx::vcpu->oldEptViolationPageAccess = oldViolationPageAccess;
			vmx::vcpu->oldEptViolationRipPageAccess = oldViolationRipPageAccess;

			hookedPage->pte->fields.readAccess = true;
			hookedPage->pte->fields.writeAccess = true;
			hookedPage->pte->fields.executeAccess = true;
			hookedPage->pte->fields.memoryType = MemoryType::WriteBack;

			ept::invalidGlobalEptCache();

			

			vtx::setMonitorTrapFlag(true);
		}

		void handleEptMisconfig(Registers64* reg)
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
	}
}
