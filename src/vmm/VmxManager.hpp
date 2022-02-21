#pragma once
#include "Common.hpp"
#include "IA32.h"
#include <intrin.h>
#include "Util.hpp"
#include "asm.h"
#include "Ept.hpp"

#define POOL_TAG_VMXON 'vmon'
#define POOL_TAG_VMCS 'vmcs'
#define POOL_TAG_HOST_STACK 'hstk'
#include "global.h"


class VmxManager
{
public:
	VmxManager() = default;

	static VmxManager& instance()
	{
		static VmxManager vmxManager;
		return vmxManager;
	}

	/**
	 * 检查Vmx是否可用
	 * @return NTSTATUS:
	 */
	NTSTATUS checkVmxAvailable()
	{
		CpuidField cpuIdField = { 0 };
		Cr0 cr0;
		Cr4 cr4;
		ControlMsr controlMsr = { 0 };
		BasicMsr basicMsr = { 0 };

		//1.CPUID
		__cpuid((int*)&cpuIdField, 1);
		CpuIdFiledEcx* cpuIdFiledEcx = (CpuIdFiledEcx*)(&cpuIdField.rcx);
		if (cpuIdFiledEcx->fields.vmx != 1)
		{
			DbgLog(Common::LogLevel::Error, "Vt is not supported on this machine.");
			return STATUS_HV_FEATURE_UNAVAILABLE;
		}

		//2.检测cr0
		cr0.all = __readcr0();
		if (!cr0.fields.pg || !cr0.fields.ne || !cr0.fields.pe)
		{
			DbgLog(Common::LogLevel::Error, "Cr0 not supported to be virtualizaion.");
			return STATUS_HV_FEATURE_UNAVAILABLE;
		}

		//2.检查 BASIC_MSR,判断是否支持回写内存
		//See 24.2 Fromat of the VMCX region
		basicMsr.all = __readmsr(MSR_IA32_VMX_BASIC);
		if (basicMsr.fields.memory_type != MemoryType::WriteBack)
		{
			DbgLog(Common::LogLevel::Error, "Write-back cache type is not supported.");
			return STATUS_HV_FEATURE_UNAVAILABLE;
		}

		////3.检查 CONTROL_MSR
		//controlMsr.all = __readmsr(MSR_IA32_FEATURE_CONTROL);
		////controlMsr.fields.enable_vmxon = TRUE;
		//if (!controlMsr.fields.lock)
		//{
		//	controlMsr.fields.lock = TRUE;
		//	//将每一个处理器的lock位设为 1
		//	UtilForEachProcessor([](void* context) {
		//		IA32_FEATURE_CONTROL_MSR* tempControlMsr = (IA32_FEATURE_CONTROL_MSR*)context;
		//		__writemsr(MSR_IA32_FEATURE_CONTROL, tempControlMsr->all);
		//		return STATUS_SUCCESS;
		//		}, &controlMsr);
		//}
		//if (!controlMsr.fields.enable_vmxon)
		//{
		//	Log("Error:CPU %d: %s: VMX 不支持\n", KeGetCurrentProcessorIndex(), __FUNCTION__);
		//	return STATUS_HV_FEATURE_UNAVAILABLE;
		//}
		return STATUS_SUCCESS;
	}

	/**
	 * 检查Ept是否可用
	 * @return override bool:
	 */
	NTSTATUS checkEptAvailable()
	{
		// Check the followings:
		// - page walk length is 4 steps
		// - extended page tables can be laid out in write-back memory
		// - INVEPT instruction with all possible types is supported
		// - INVVPID instruction with all possible types is supported

		EptVpidCapMsr capability = { __readmsr(MSR_IA32_VMX_EPT_VPID_CAP) };
		if (!capability.fields.support_page_walk_length4 ||
			!capability.fields.support_write_back_memory_type ||
			!capability.fields.support_invept ||
			!capability.fields.support_single_context_invept ||
			!capability.fields.support_all_context_invept ||
			!capability.fields.support_invvpid ||
			!capability.fields.support_individual_address_invvpid ||
			!capability.fields.support_single_context_invvpid ||
			!capability.fields.support_all_context_invvpid ||
			!capability.fields.support_single_context_retaining_globals_invvpid)
		{
			DbgLog(Common::LogLevel::Warnning, "Ept is unavailable.");
			return STATUS_FAIL_CHECK;
		}
		return STATUS_SUCCESS;
	}

	/**
	 * 开启vmx标志
	 * @return NTSTATUS:
	 */
	static NTSTATUS enableVmxFeature()
	{
		//开启cr4.vmxe
		Cr4 cr4 = { 0 };

		cr4.all = __readcr4();
		cr4.fields.vmxe = TRUE;
		__writecr4(cr4.all);

		//对每个cpu开启vmxon指令的限制
		ControlMsr controlMsr = { 0 };
		controlMsr.all = __readmsr(MSR_IA32_FEATURE_CONTROL);
		if (!controlMsr.fields.lock)
		{
			controlMsr.fields.lock = TRUE;
			controlMsr.fields.enable_vmxon = TRUE;
			__writemsr(MSR_IA32_FEATURE_CONTROL, controlMsr.all);
			controlMsr.all = __readmsr(MSR_IA32_FEATURE_CONTROL);
		}
		if (!controlMsr.fields.lock && !controlMsr.fields.enable_vmxon)
		{
			DbgLog(Common::LogLevel::Error, "Virtualization is not enabled in the BIOS.");
			return STATUS_HV_FEATURE_UNAVAILABLE;
		}
		return STATUS_SUCCESS;
	}

	/**
	 * 申请Vmx域空间
	 * @param Vcpu* vcpu
	 * @return NTSTATUS:
	 */
	NTSTATUS allocVcpu(Vcpu* vcpu)
	{
		PVOID pVmxonRegion;
		PVOID pVmcsRegion;
		PVOID pVmStack;

		pVmxonRegion = ExAllocatePoolWithTag(NonPagedPool, 0x1000, POOL_TAG_VMXON); //4KB
		if (!pVmxonRegion)
		{
			DbgLog(Common::LogLevel::Error, "Allocate vmxon memory failed.");
			return STATUS_MEMORY_NOT_ALLOCATED;
		}
		RtlZeroMemory(pVmxonRegion, 0x1000);

		pVmcsRegion = ExAllocatePoolWithTag(NonPagedPool, 0x1000, POOL_TAG_VMCS);
		if (!pVmcsRegion)
		{
			DbgLog(Common::LogLevel::Error, "Allocate vmcs memory failed.");
			ExFreePoolWithTag(pVmxonRegion, 0x1000);
			return STATUS_MEMORY_NOT_ALLOCATED;
		}
		RtlZeroMemory(pVmcsRegion, 0x1000);

		pVmStack = ExAllocatePoolWithTag(NonPagedPool, KERNEL_STACK_SIZE, POOL_TAG_HOST_STACK);
		if (!pVmStack)
		{
			DbgLog(Common::LogLevel::Error, "Allocate host stack memory failed.");
			ExFreePoolWithTag(pVmxonRegion, 0x1000);
			ExFreePoolWithTag(pVmcsRegion, 0x1000);
			return STATUS_MEMORY_NOT_ALLOCATED;
		}
		RtlZeroMemory(pVmStack, KERNEL_STACK_SIZE);

		DbgLog(Common::LogLevel::Info, "Vmxon region:0x%08X.", pVmxonRegion);
		DbgLog(Common::LogLevel::Info, "Vmcs region:0x%08X.", pVmcsRegion);
		DbgLog(Common::LogLevel::Info, "Host stack region:0x%08X.", pVmStack);

		vcpu->vmxonRegion = pVmxonRegion;
		vcpu->vmcsRegion = pVmcsRegion;
		vcpu->vmmStack = pVmStack;
		vcpu->vmmStackBase = (CHAR*)pVmStack + KERNEL_STACK_SIZE;
		return STATUS_SUCCESS;
	}

	


	/**
	 * 启动vmx
	 * @param Vcpu* vcpu:
	 * @return NTSTATUS:
	 */
	static NTSTATUS launchVmx(Vcpu* vcpu,PVOID guestRsp, PVOID guestRip)
	{
		NTSTATUS status;
		DbgBreakPoint();

		Vcpu* currentVcpu = &vcpu[Util::currentCpuIndex()];

		currentVcpu->guestState.contextFrame.Rsp = (ULONG_PTR)guestRsp;
		currentVcpu->guestState.contextFrame.Rip = (ULONG_PTR)guestRip;

		//开启Root模式
		status = VmxManager::instance().enableRoot(currentVcpu);
		NT_CHECK(status);

		currentVcpu->isVmxEnable = TRUE;

		//设置VMCS 
		//DbgBreakPoint();
		status = VmxManager::instance().setupVmcs(currentVcpu);
		NT_CHECK(status);

		Ept::instance().enable();

		//开启虚拟机
		__vmx_vmlaunch();

		// See:30.4 VM Instruction Error Numbers
		int error = 0;
		DbgBreakPoint();
		if (__vmx_vmread(VM_INSTRUCTION_ERROR, (size_t*)&error) != 0)
		{
			DbgLog(Common::LogLevel::Error, "read error code failed");
			return FALSE;
		}
		DbgLog(Common::LogLevel::Error, "vmlaunch failed,error:%d", error);
		return FALSE;
	}

	/**
	 * 退出vmroot
	 * @return NTSTATUS:
	 */
	static NTSTATUS quitVmx()
	{
		Cr4 cr4;

		//以Guest身份执行vmoff会导致26号vmexit
		//__vmx_off();

		VmcallParam param = { 0 };
		__vmcall(VmcallNumber::Exit, &param);


		//Cr4.VMXE置0
		cr4.all = __readcr4();
		cr4.fields.vmxe = FALSE;
		__writecr4(cr4.all);

		
		return STATUS_SUCCESS;

	}

private:
	/**
	 * 开启Root模式,执行vmxon,激活VMCS
	 * @param VmxContext * vmxContext:
	 * @return NTSTATUS:
	 */
	NTSTATUS enableRoot(Vcpu* vcpu)
	{
		Cr0 cr0;
		Cr4 cr4;
		BasicMsr basicMsr = { 0 };
		ULONG_PTR tmpVmxonRegionPa;
		ULONG uRet;
		ULONG_PTR tmpVmcsRegionPa;
		NTSTATUS status = STATUS_SUCCESS;

		cr0 = { __readcr0() };
		cr0.all &= __readmsr(MSR_IA32_VMX_CR0_FIXED1);
		cr0.all |= __readmsr(MSR_IA32_VMX_CR0_FIXED0);
		__writecr0(cr0.all);

		// See: VMX-FIXED BITS IN CR4
		cr4 = { __readcr4() };
		cr4.all &= __readmsr(MSR_IA32_VMX_CR4_FIXED1);
		cr4.all |= __readmsr(MSR_IA32_VMX_CR4_FIXED0);
		__writecr4(cr4.all);


		basicMsr.all = __readmsr(MSR_IA32_VMX_BASIC);

		// See: 31.5 VMM setup 
		*(ULONG*)vcpu->vmxonRegion = basicMsr.fields.revision_identifier;

		// See: 24.2 Format of the VMCX region
		*(ULONG*)vcpu->vmcsRegion = basicMsr.fields.revision_identifier;

		//vmxon
		tmpVmxonRegionPa = Util::vaToPa(vcpu->vmxonRegion);
		uRet = __vmx_on(&tmpVmxonRegionPa);
		if (uRet != 0)
		{
			DbgLog(Common::LogLevel::Error, "perform _vmx_on failed");
			return STATUS_UNSUCCESSFUL;
		}

		//vmclear
		tmpVmcsRegionPa = Util::vaToPa(vcpu->vmcsRegion);
		uRet = __vmx_vmclear(&tmpVmcsRegionPa);
		if (uRet != 0)
		{
			DbgLog(Common::LogLevel::Error, "perform __vmx_vmclear failed");
			return STATUS_UNSUCCESSFUL;
		}

		//vmptrld
		tmpVmcsRegionPa = Util::vaToPa(vcpu->vmcsRegion);
		uRet = __vmx_vmptrld(&tmpVmcsRegionPa);
		if (uRet != 0)
		{
			DbgLog(Common::LogLevel::Error, "perform __vmx_vmptrld failed");
			return STATUS_UNSUCCESSFUL;
		}
		return status;
	}

	/**
	 * 装载vmcs
	 * @param Vcpu * vcpu:
	 * @return BOOLEAN:
	 */
	NTSTATUS setupVmcs(Vcpu* vcpu)
	{
		NTSTATUS status = STATUS_SUCCESS;
		Gdtr gdtr = { 0 };
		Idtr idtr = { 0 };
		SegmentSelector segmentSelector;
		VmxPinBasedControls vmPinCtlRequested = { 0 };
		VmxCpuBasedControls vmCpuCtlRequested = { 0 };
		VmxVmEnterControls vmEnterCtlRequested = { 0 };
		VmxVmExitControls vmExitCtlRequested = { 0 };
		VmxSecondaryCpuBasedControls vmCpuCtl2Requested = { 0 };

		gdtr.base = __getgdtbase();
		gdtr.limit = __getgdtlimit();
		idtr.base = __getidtbase();
		idtr.limit = __getidtlimit();
		//_sgdt(&gdtr);
		//__sidt(&idtr);

		/********************************************
		 填充VMCS See: 24.3 Organization of VMCX data
		********************************************/

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
		__vmx_vmwrite(GUEST_RSP, (ULONG_PTR)vcpu->guestState.contextFrame.Rsp);
		__vmx_vmwrite(GUEST_RIP, (ULONG_PTR)vcpu->guestState.contextFrame.Rip);
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
		loadSementDescriptor(&segmentSelector, __readldtr(), gdtr.base);
		__vmx_vmwrite(GUEST_LDTR_BASE, segmentSelector.base);
		loadSementDescriptor(&segmentSelector, __readtr(), gdtr.base);
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


		//
		// 2.宿主机状态域 (Host-state area) See: 24.5
		//

		// CR0 CR3,and CR4
		__vmx_vmwrite(HOST_CR0, __readcr0());
		__vmx_vmwrite(HOST_CR3, __readcr3());
		__vmx_vmwrite(HOST_CR4, __readcr4());

		// RSP and RIP
		__vmx_vmwrite(HOST_RSP, (ULONG_PTR)vcpu->vmmStackBase);
		__vmx_vmwrite(HOST_RIP, (ULONG_PTR)__vmm_entry_point);

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
		loadSementDescriptor(&segmentSelector, __readtr(), gdtr.base);
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

		//
		// 3.虚拟机执行控制域 (VM-execution control fields) See:24.6 
		//

		// 通过 MSR_IA32_VMX_BASIC 寄存器的 55 位来判断是否使用 True 类型的 Msrs
		// See: 31.5.1 Algorithms for Determining VMX Capabilities	
		BasicMsr basicMsr;
		basicMsr.all = __readmsr(MSR_IA32_VMX_BASIC);
		BOOLEAN isUseTrueMsrs = basicMsr.fields.vmx_capability_hint;

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

		//
		// 4.虚拟机退出控制域 (VM-exit control fields) See:24.7 
		//

		// See: 24.7.1 VM-Exit Controls: Table 24-10	
		vmExitCtlRequested.fields.hostAddressSpaceSize = Util::checkAmd64();
		vmExitCtlRequested.fields.acknowledgeInterruptOnExit = TRUE;
		__vmx_vmwrite(
			VM_EXIT_CONTROLS,
			updateControlValue(
				isUseTrueMsrs ? MSR_IA32_VMX_TRUE_EXIT_CTLS : MSR_IA32_VMX_EXIT_CTLS,
				vmExitCtlRequested.all
			));

		//
		// 5.虚拟机进入控制域 (VM-entry control fields) See:24.8 
		//

		// See: 24.8.1 VM-Entry Controls: Table 24-12 
		vmEnterCtlRequested.fields.ia32eModeGuest = TRUE;
		//vmEnterCtlRequested.Fields.LoadIA32_EFER = TRUE;
		__vmx_vmwrite(
			VM_ENTRY_CONTROLS,
			updateControlValue(
				isUseTrueMsrs ? MSR_IA32_VMX_TRUE_ENTRY_CTLS : MSR_IA32_VMX_ENTRY_CTLS,
				vmEnterCtlRequested.all
			));

		//
		// 6.虚拟机退出信息域 (VM-exit information fields)
		//

		return status;
	}

	/**
	 * 加载段描述符
	 * @param OUT SegmentSelector * segmentSelector:
	 * @param USHORT selector:
	 * @param ULONG_PTR gdtBase:
	 * @return NTSTATUS:
	 */
	NTSTATUS loadSementDescriptor(OUT SegmentSelector* segmentSelector, USHORT selector, ULONG_PTR gdtBase)
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

	/**
	 * 获取段描述符访问权限
	 * @param USHORT selector: 段选择子
	 * @return ULONG:
	 */
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

	/**
	 * 调整Msr寄存器值
	 * See:24.6.1 Pin-Base VM-Execution Controls
	 * vmwrite虚拟机控制域时,有些位必须置为0,有些位必须置为1
	 * 通过读取相应的MSR寄存器来确定哪些位必须置0,哪些位必须置1
	 * @param ULONG msr:
	 * @param ULONG ctl:
	 * @return ULONG:
	 */
	ULONG updateControlValue(ULONG msr, ULONG ctl)
	{

		LARGE_INTEGER MsrValue = { 0 };
		MsrValue.QuadPart = __readmsr(msr);
		ctl &= MsrValue.HighPart;     /* bit == 0 in high word ==> must be zero */
		ctl |= MsrValue.LowPart;      /* bit == 1 in low word  ==> must be one  */
		return ctl;
	}



public:
	///**
	// * 获取全局VmxContext对象数组的指针
	// *
	// * @return VmxCpuContext**:
	// */
	//VmxCpuContext* getStaticVmxContext()
	//{
	//	static VmxCpuContext* vmxCpuContext=nullptr;
	//	if (vmxCpuContext == nullptr)
	//	{
	//		int size = sizeof(VmxCpuContext) * Util::getCpuCount();
	//		vmxCpuContext = (VmxCpuContext*)Util::alloc(size);
	//		RtlZeroMemory(vmxCpuContext, size);
	//	}
	//	return vmxCpuContext;
	//}

	///**
	// * 获取当前VmxContext对象的地址
	// *
	// * @return VmxCpuContext*:
	// */
	//VmxCpuContext& getCurrentVmxContext()
	//{
	//	VmxCpuContext currentCpuContext = VmxManager::getStaticVmxContext()[Util::currentCpuIndex()];
	//	return currentCpuContext;
	//}
private:

};
