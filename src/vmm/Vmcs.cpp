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
				// 1.�����״̬�� (Guest-state area) See: 24.4
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
				//����:IA32_DEBUGCTL,IA32_SYSENTER_CS,IA32_SYSENTER_ESP,and IA32_SYSENTER_EIP
				//���� MSR �Ĵ���������Ӧ�� VM-exit control λ����Ϊ 1 �����Ҫ�����״̬��
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

				//VMCS link pointer��ʹ�õĻ�������FFFFFFFF_FFFFFFFF See: 26.3.1.5 Checks on Guest Non-Register State
				__vmx_vmwrite(VMCS_LINK_POINTER, MAXULONG64);

				//��ѡ:VMX-preemption timer value,Page-directory-pointer-table-entries,Guest interrupt status
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
				// 2.������״̬�� (Host-state area) See: 24.5
				//

				// CR0 CR3,and CR4
				__vmx_vmwrite(HOST_CR0, __readcr0());
				__vmx_vmwrite(HOST_CR3, __readcr3());
				__vmx_vmwrite(HOST_CR4, __readcr4());

				// RSP and RIP
				__vmx_vmwrite(HOST_RSP, hostRsp);
				__vmx_vmwrite(HOST_RIP, hostRip);

				// Selector fileds
				// RPL��TI����Ϊ0  
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
				// ����:IA32_SYSENTER_CS,IA32_SYSENTER_ESP,IA32_SYSENTER_EIP,
				// ���� MSR �Ĵ���������Ӧ�� VM-exit control λ����Ϊ 1 �����Ҫ�����״̬��
				// IA32_PERF_GLOBAL_CTRL,IA32_PAT,IA32_EFER
				__vmx_vmwrite(HOST_IA32_SYSENTER_CS, __readmsr(MSR_IA32_SYSENTER_CS));
				__vmx_vmwrite(HOST_IA32_SYSENTER_ESP, __readmsr(MSR_IA32_SYSENTER_ESP));
				__vmx_vmwrite(HOST_IA32_SYSENTER_EIP, __readmsr(MSR_IA32_SYSENTER_EIP));//MSR[0x174] KiFastCallEntry�ĵ�ַ
				//__vmx_vmwrite(HOST_IA32_EFER, __readmsr(MSR_EFER));

				return STATUS_SUCCESS;
			}

			NTSTATUS setupVmExecCtrlFields(bool isUseTrueMsrs)
			{
				//
				// 3.�����ִ�п����� (VM-execution control fields) See:24.6 
				//
				VmxPinBasedControls vmPinCtlRequested = { 0 };
				VmxCpuBasedControls vmCpuCtlRequested = { 0 };
				VmxSecondaryCpuBasedControls vmCpuCtl2Requested = { 0 };


				// ͨ�� MSR_IA32_VMX_BASIC �Ĵ����� 55 λ���ж��Ƿ�ʹ�� True ���͵� Msrs
				// See: 31.5.1 Algorithms for Determining VMX Capabilities	

				// ���ִ�п�����,��Ҫ��������Ӳ���ж�
				// See: 24.6.1 Pin-Based VM-Execution Controls	
				__vmx_vmwrite(
					PIN_BASED_VM_EXEC_CONTROL,
					updateControlValue(
						isUseTrueMsrs ? MSR_IA32_VMX_TRUE_PINBASED_CTLS : MSR_IA32_VMX_PINBASED_CTLS,
						vmPinCtlRequested.all
					));

				// ������ִ�п�����,��Ҫ������������ָ��
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
				// 4.������˳������� (VM-exit control fields) See:24.7 
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
				// 5.�������������� (VM-entry control fields) See:24.8 
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
				// 6.������˳���Ϣ�� (VM-exit information fields)
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
				// �����ѡ���ӵ�T1 = 1��ʾ����LDT�е���, ����û��ʵ���������
				if (selector & 0x4)
				{
					return STATUS_INVALID_PARAMETER;
				}
				// ��GDT��ȡ��ԭʼ�Ķ�������
				segDesc = (SegmentDescriptor2*)((PUCHAR)gdtBase + (selector & ~0x7));
				// ��ѡ����
				segmentSelector->sel = selector;
				// �λ�ַ15-39λ 55-63λ
				segmentSelector->base = segDesc->base0 | segDesc->base1 << 16 | segDesc->base2 << 24;
				// ���޳�0-15λ  47-51λ, ������ȡ��
				segmentSelector->limit = segDesc->limit0 | (segDesc->limit1attr1 & 0xf) << 16;
				// ������39-47 51-55 ע��۲�ȡ��
				segmentSelector->attributes.UCHARs = segDesc->attr0 | (segDesc->limit1attr1 & 0xf0) << 4;
				// �����ж����Ե�DTλ, �ж��Ƿ���ϵͳ�����������Ǵ������ݶ�������
				if (!(segDesc->attr0 & LA_STANDARD))
				{
					ULONG64 tmp;
					// �����ʾ��ϵͳ��������������������, �о�����Ϊ64λ׼���İ�,
					// 32λ����λ�ַֻ��32λ��. �ѵ�64λ������ʲô������?
					tmp = (*(PULONG64)((PUCHAR)segDesc + 8));
					segmentSelector->base = (segmentSelector->base & 0xffffffff) | (tmp << 32);
				}

				// ���Ƕν��޵�����λ, 1Ϊ4K. 0Ϊ1BYTE
				if (segmentSelector->attributes.fields.g)
				{
					// �������λΪ1, ��ô�ͳ���4K. ���ƶ�12λ
					segmentSelector->limit = (segmentSelector->limit << 12) + 0xfff;
				}
				return STATUS_SUCCESS;
			}

			
		}
	}
}