#include "Vmcs.h"
#include "IA32.h"
#include <intrin.h>
#include "asm.h"
#include "Util.hpp"


namespace whaledbg
{
	namespace vmm
	{
		namespace vmcs
		{	
			NTSTATUS setupGuestState(ULONG_PTR guestRsp,ULONG_PTR guestRip)
			{
				SegmentSelector segmentSelector;
				Gdtr gdtr = { 0 };
				Idtr idtr = { 0 };

				gdtr.base = __getgdtbase();
				gdtr.limit = __getgdtlimit();
				idtr.base = __getidtbase();
				idtr.limit = __getidtlimit();
				//
				// 1.虚拟机状态域 (Guest-state area) See: 24.4
				// 	

				// See: 24.4.1 Guest Register State
				//CR0,CR3 and CR4
				__vmx_vmwrite(GUEST_CR0, __readcr0());
				__vmx_vmwrite(GUEST_CR3, __readcr3());
				__vmx_vmwrite(GUEST_CR4, __readcr4());

				//Debug register DR7
				__vmx_vmwrite(GUEST_DR7, __readdr(7));

				//RSP,RIP and RFLAGS
				__vmx_vmwrite(GUEST_RSP, guestRsp);
				__vmx_vmwrite(GUEST_RIP, guestRip);
				__vmx_vmwrite(GUEST_RFLAGS, __readeflags() & ~0x200);//cli

				//Selector,Base address,Segment limit,Access rights for each of following registers:
				//CS,SS,DS,ES,FS,GS,LDTR and TR
				__vmx_vmwrite(GUEST_ES_SELECTOR, __reades());
				__vmx_vmwrite(GUEST_CS_SELECTOR, __readcs());
				__vmx_vmwrite(GUEST_SS_SELECTOR, __readss());
				__vmx_vmwrite(GUEST_DS_SELECTOR, __readds());
				__vmx_vmwrite(GUEST_FS_SELECTOR, __readfs());
				__vmx_vmwrite(GUEST_GS_SELECTOR, __readgs());
				__vmx_vmwrite(GUEST_LDTR_SELECTOR, __readldtr());
				__vmx_vmwrite(GUEST_TR_SELECTOR, __readtr());

				__vmx_vmwrite(GUEST_ES_BASE, 0);
				__vmx_vmwrite(GUEST_CS_BASE, 0);
				__vmx_vmwrite(GUEST_SS_BASE, 0);
				__vmx_vmwrite(GUEST_DS_BASE, 0);
				__vmx_vmwrite(GUEST_FS_BASE, __readmsr(MSR_FS_BASE));
				__vmx_vmwrite(GUEST_GS_BASE, __readmsr(MSR_GS_BASE));
				loadSegmentDescriptor(&segmentSelector, __readldtr(), gdtr.base);
				__vmx_vmwrite(GUEST_LDTR_BASE, segmentSelector.base);
				loadSegmentDescriptor(&segmentSelector, __readtr(), gdtr.base);
				__vmx_vmwrite(GUEST_TR_BASE, segmentSelector.base);

				__vmx_vmwrite(GUEST_ES_LIMIT, GetSegmentLimit(__reades()));
				__vmx_vmwrite(GUEST_CS_LIMIT, GetSegmentLimit(__readcs()));
				__vmx_vmwrite(GUEST_SS_LIMIT, GetSegmentLimit(__readss()));
				__vmx_vmwrite(GUEST_DS_LIMIT, GetSegmentLimit(__readds()));
				__vmx_vmwrite(GUEST_FS_LIMIT, GetSegmentLimit(__readfs()));
				__vmx_vmwrite(GUEST_GS_LIMIT, GetSegmentLimit(__readgs()));
				__vmx_vmwrite(GUEST_LDTR_LIMIT, GetSegmentLimit(__readldtr()));
				__vmx_vmwrite(GUEST_TR_LIMIT, GetSegmentLimit(__readtr()));

				__vmx_vmwrite(GUEST_ES_AR_BYTES, getSegmentAccessRight(__reades()));
				__vmx_vmwrite(GUEST_CS_AR_BYTES, getSegmentAccessRight(__readcs()));
				__vmx_vmwrite(GUEST_SS_AR_BYTES, getSegmentAccessRight(__readss()));
				__vmx_vmwrite(GUEST_DS_AR_BYTES, getSegmentAccessRight(__readds()));
				__vmx_vmwrite(GUEST_FS_AR_BYTES, getSegmentAccessRight(__readfs()));
				__vmx_vmwrite(GUEST_GS_AR_BYTES, getSegmentAccessRight(__readgs()));
				__vmx_vmwrite(GUEST_LDTR_AR_BYTES, getSegmentAccessRight(__readldtr()));
				__vmx_vmwrite(GUEST_TR_AR_BYTES, getSegmentAccessRight(__readtr()));


				//Base address,limit for each of following registers:
				//GDTR,IDTR
				__vmx_vmwrite(GUEST_GDTR_BASE, gdtr.base);
				__vmx_vmwrite(GUEST_GDTR_LIMIT, gdtr.limit);
				__vmx_vmwrite(GUEST_IDTR_BASE, idtr.base);
				__vmx_vmwrite(GUEST_IDTR_LIMIT, idtr.limit);

				//MSRs
				//必填:IA32_DEBUGCTL,IA32_SYSENTER_CS,IA32_SYSENTER_ESP,and IA32_SYSENTER_EIP
				//以下 MSR 寄存器仅当对应的 VM-exit control 位被置为 1 后才需要填充其状态域
				//IA32_PERF_GLOBAL_CTRL,IA32_PAT,IA32_EFER
				__vmx_vmwrite(GUEST_IA32_DEBUGCTL, __readmsr(MSR_IA32_DEBUGCTL));
				__vmx_vmwrite(GUEST_SYSENTER_CS, __readmsr(MSR_IA32_SYSENTER_CS));
				__vmx_vmwrite(GUEST_SYSENTER_ESP, __readmsr(MSR_IA32_SYSENTER_ESP));
				__vmx_vmwrite(GUEST_SYSENTER_EIP, __readmsr(MSR_IA32_SYSENTER_EIP)); // KiFastCallEntry
				//__vmx_vmwrite(GUEST_IA32_EFER,__readmsr(MSR_EFER));

				//SMBASE 0
				__vmx_vmwrite(GUEST_SMBASE, 0);

				//See: 24.2.2 Guest Non-Register State
				//Active state 0
				__vmx_vmwrite(GUEST_ACTIVITY_STATE, 0);

				//Interruptibility state 0
				__vmx_vmwrite(GUEST_INTERRUPTIBILITY_INFO, 0);

				//Pending debug exceptions 0
				__vmx_vmwrite(GUEST_PENDING_DBG_EXCEPTIONS, 0);

				//VMCS link pointer不使用的话必须填FFFFFFFF_FFFFFFFF See: 26.3.1.5 Checks on Guest Non-Register State
				__vmx_vmwrite(VMCS_LINK_POINTER, MAXULONG64);

				//可选:VMX-preemption timer value,Page-directory-pointer-table-entries,Guest interrupt status
				//...

				return STATUS_SUCCESS;
			}

			NTSTATUS setupHostState(ULONG_PTR hostRsp,ULONG_PTR hostRip)
			{
				SegmentSelector segmentSelector;
				Gdtr gdtr = { 0 };
				Idtr idtr = { 0 };

				gdtr.base = __getgdtbase();
				gdtr.limit = __getgdtlimit();
				idtr.base = __getidtbase();
				idtr.limit = __getidtlimit();
				//
				// 2.宿主机状态域 (Host-state area) See: 24.5
				//

				// CR0 CR3,and CR4
				__vmx_vmwrite(HOST_CR0, __readcr0());
				__vmx_vmwrite(HOST_CR3, __readcr3());
				__vmx_vmwrite(HOST_CR4, __readcr4());

				// RSP and RIP
				__vmx_vmwrite(HOST_RSP, hostRsp);
				__vmx_vmwrite(HOST_RIP, hostRip);

				// Selector fileds
				// RPL和TI必须为0  
				// See: 26.2.3 Check on Host Segment and Descriptor-Table Registers
				__vmx_vmwrite(HOST_ES_SELECTOR, __reades() & 0xf8);
				__vmx_vmwrite(HOST_CS_SELECTOR, __readcs() & 0xf8);
				__vmx_vmwrite(HOST_SS_SELECTOR, __readss() & 0xf8);
				__vmx_vmwrite(HOST_DS_SELECTOR, __readds() & 0xf8);
				__vmx_vmwrite(HOST_FS_SELECTOR, __readfs() & 0xf8);
				__vmx_vmwrite(HOST_GS_SELECTOR, __readgs() & 0xf8);
				__vmx_vmwrite(HOST_TR_SELECTOR, __readtr() & 0xf8);

				// Base-address fileds for FS,GS,TR,GDTR,and IDTR	
				__vmx_vmwrite(HOST_FS_BASE, __readmsr(MSR_FS_BASE));
				__vmx_vmwrite(HOST_GS_BASE, __readmsr(MSR_GS_BASE));
				loadSegmentDescriptor(&segmentSelector, __readtr(), gdtr.base);
				__vmx_vmwrite(HOST_TR_BASE, segmentSelector.base);
				__vmx_vmwrite(HOST_GDTR_BASE, gdtr.base);
				__vmx_vmwrite(HOST_IDTR_BASE, idtr.base);

				// MSRs
				// 必填:IA32_SYSENTER_CS,IA32_SYSENTER_ESP,IA32_SYSENTER_EIP,
				// 以下 MSR 寄存器仅当对应的 VM-exit control 位被置为 1 后才需要填充其状态域
				// IA32_PERF_GLOBAL_CTRL,IA32_PAT,IA32_EFER
				__vmx_vmwrite(HOST_IA32_SYSENTER_CS, __readmsr(MSR_IA32_SYSENTER_CS));
				__vmx_vmwrite(HOST_IA32_SYSENTER_ESP, __readmsr(MSR_IA32_SYSENTER_ESP));
				__vmx_vmwrite(HOST_IA32_SYSENTER_EIP, __readmsr(MSR_IA32_SYSENTER_EIP));//MSR[0x174] KiFastCallEntry的地址
				//__vmx_vmwrite(HOST_IA32_EFER, __readmsr(MSR_EFER));

				return STATUS_SUCCESS;
			}

			NTSTATUS setupVmExecCtrlFields(bool isUseTrueMsrs)
			{
				//
				// 3.虚拟机执行控制域 (VM-execution control fields) See:24.6 
				//
				VmxPinBasedControls vmPinCtlRequested = { 0 };
				VmxCpuBasedControls vmCpuCtlRequested = { 0 };
				VmxSecondaryCpuBasedControls vmCpuCtl2Requested = { 0 };


				// 通过 MSR_IA32_VMX_BASIC 寄存器的 55 位来判断是否使用 True 类型的 Msrs
				// See: 31.5.1 Algorithms for Determining VMX Capabilities	

				// 针脚执行控制域,主要用于拦截硬件中断
				// See: 24.6.1 Pin-Based VM-Execution Controls	
				__vmx_vmwrite(
					PIN_BASED_VM_EXEC_CONTROL,
					updateControlValue(
						isUseTrueMsrs ? MSR_IA32_VMX_TRUE_PINBASED_CTLS : MSR_IA32_VMX_PINBASED_CTLS,
						vmPinCtlRequested.all
					));

				// 处理器执行控制域,主要用于拦截特殊指令
				// Primary Processor
				// See: 24.6.2 Processor-Based VM-Execution Controls:Table 24-6
				__vmx_vmwrite(
					CPU_BASED_VM_EXEC_CONTROL,
					updateControlValue(
						isUseTrueMsrs ? MSR_IA32_VMX_TRUE_PROCBASED_CTLS : MSR_IA32_VMX_PROCBASED_CTLS,
						vmCpuCtlRequested.all
					));

				// Secondary Processor
				// See: 24.6.2 Processor-Based VM-Execution Controls:Table 24-7
				__vmx_vmwrite(
					SECONDARY_VM_EXEC_CONTROL,
					updateControlValue(
						MSR_IA32_VMX_PROCBASED_CTLS2,
						vmCpuCtl2Requested.all
					));
				return STATUS_SUCCESS;
			}

			NTSTATUS setupVmExitCtrlFields(bool isUseTrueMsrs)
			{
				//
				// 4.虚拟机退出控制域 (VM-exit control fields) See:24.7 
				//
				VmxVmExitControls vmExitCtlRequested = { 0 };
				// See: 24.7.1 VM-Exit Controls: Table 24-10	
				vmExitCtlRequested.fields.hostAddressSpaceSize = Util::checkAmd64();
				vmExitCtlRequested.fields.acknowledgeInterruptOnExit = TRUE;
				__vmx_vmwrite(
					VM_EXIT_CONTROLS,
					updateControlValue(
						isUseTrueMsrs ? MSR_IA32_VMX_TRUE_EXIT_CTLS : MSR_IA32_VMX_EXIT_CTLS,
						vmExitCtlRequested.all
					));
				return STATUS_SUCCESS;
			}

			NTSTATUS setupVmEntryCtrlFields(bool isUseTrueMsrs)
			{
				//
				// 5.虚拟机进入控制域 (VM-entry control fields) See:24.8 
				//
				VmxVmEnterControls vmEnterCtlRequested = { 0 };
				// See: 24.8.1 VM-Entry Controls: Table 24-12 
				vmEnterCtlRequested.fields.ia32eModeGuest = TRUE;
				//vmEnterCtlRequested.Fields.LoadIA32_EFER = TRUE;
				__vmx_vmwrite(
					VM_ENTRY_CONTROLS,
					updateControlValue(
						isUseTrueMsrs ? MSR_IA32_VMX_TRUE_ENTRY_CTLS : MSR_IA32_VMX_ENTRY_CTLS,
						vmEnterCtlRequested.all
					));
				return STATUS_SUCCESS;
			}

			NTSTATUS setupVmExitInfoFields()
			{
				//
				// 6.虚拟机退出信息域 (VM-exit information fields)
				//
				return STATUS_SUCCESS;
			}

			
			ULONG getSegmentAccessRight(USHORT selector)
			{
				VmxRegmentDescriptorAccessRight accessRight = { 0 };
				if (selector)
				{
					ULONG_PTR nativeAccessRight = __load_access_rights_byte(selector);
					nativeAccessRight >>= 8;
					accessRight.all = (ULONG)(nativeAccessRight);
					accessRight.fields.reserved1 = 0;
					accessRight.fields.reserved2 = 0;
					accessRight.fields.unusable = FALSE;
				}
				else
				{
					accessRight.fields.unusable = TRUE;
				}
				return accessRight.all;
			}


			ULONG updateControlValue(ULONG msr, ULONG ctl)
			{

				LARGE_INTEGER MsrValue = { 0 };
				MsrValue.QuadPart = __readmsr(msr);
				ctl &= MsrValue.HighPart;     /* bit == 0 in high word ==> must be zero */
				ctl |= MsrValue.LowPart;      /* bit == 1 in low word  ==> must be one  */
				return ctl;
			}

			NTSTATUS loadSegmentDescriptor(SegmentSelector* segmentSelector, USHORT selector, ULONG_PTR gdtBase)
			{
				SegmentDescriptor2* segDesc;
				if (!segmentSelector)
				{
					return STATUS_INVALID_PARAMETER;
				}
				// 如果段选择子的T1 = 1表示索引LDT中的项, 这里没有实现这个功能
				if (selector & 0x4)
				{
					return STATUS_INVALID_PARAMETER;
				}
				// 在GDT中取出原始的段描述符
				segDesc = (SegmentDescriptor2*)((PUCHAR)gdtBase + (selector & ~0x7));
				// 段选择子
				segmentSelector->sel = selector;
				// 段基址15-39位 55-63位
				segmentSelector->base = segDesc->base0 | segDesc->base1 << 16 | segDesc->base2 << 24;
				// 段限长0-15位  47-51位, 看它的取法
				segmentSelector->limit = segDesc->limit0 | (segDesc->limit1attr1 & 0xf) << 16;
				// 段属性39-47 51-55 注意观察取法
				segmentSelector->attributes.UCHARs = segDesc->attr0 | (segDesc->limit1attr1 & 0xf0) << 4;
				// 这里判断属性的DT位, 判断是否是系统段描述符还是代码数据段描述符
				if (!(segDesc->attr0 & LA_STANDARD))
				{
					ULONG64 tmp;
					// 这里表示是系统段描述符或者门描述符, 感觉这是为64位准备的吧,
					// 32位下面段基址只有32位啊. 难道64位下面有什么区别了?
					tmp = (*(PULONG64)((PUCHAR)segDesc + 8));
					segmentSelector->base = (segmentSelector->base & 0xffffffff) | (tmp << 32);
				}

				// 这是段界限的粒度位, 1为4K. 0为1BYTE
				if (segmentSelector->attributes.fields.g)
				{
					// 如果粒度位为1, 那么就乘以4K. 左移动12位
					segmentSelector->limit = (segmentSelector->limit << 12) + 0xfff;
				}
				return STATUS_SUCCESS;
			}

			
		}
	}
}