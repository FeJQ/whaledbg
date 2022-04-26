#include "VmxManager.h"
#include "IA32.h"
#include "Vmcs.h"
#include "asm.h"
#include "Util.hpp"
#include "Log.h"
#include "Ept.h"




namespace vmm
{
	namespace vmx
	{
		Vcpu* vcpu = nullptr;

		NTSTATUS initialize()
		{
			return NTSTATUS();
		}

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
				LOG(log::ERROR, "Vt is not supported on this machine.");
				return STATUS_HV_FEATURE_UNAVAILABLE;
			}

			//2.检测cr0
			cr0.all = __readcr0();
			if (!cr0.fields.pg || !cr0.fields.ne || !cr0.fields.pe)
			{
				LOG(log::ERROR, "Cr0 not supported to be virtualizaion.");
				return STATUS_HV_FEATURE_UNAVAILABLE;
			}

			//2.检查 BASIC_MSR,判断是否支持回写内存
			//See 24.2 Fromat of the VMCX region
			basicMsr.all = __readmsr(MSR_IA32_VMX_BASIC);
			if (basicMsr.fields.memory_type != MemoryType::WriteBack)
			{
				LOG(log::ERROR, "Write-back cache type is not supported.");
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
				LOG(log::WARNNING, "Ept is unavailable.");
				return STATUS_FAIL_CHECK;
			}
			return STATUS_SUCCESS;
		}

		NTSTATUS enableVmxFeature()
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
				LOG(log::ERROR, "Virtualization is not enabled in the BIOS.");
				return STATUS_HV_FEATURE_UNAVAILABLE;
			}
			return STATUS_SUCCESS;
		}

		NTSTATUS allocVcpu(Vcpu* vcpu)
		{
			PVOID pVmxonRegion;
			PVOID pVmcsRegion;
			PVOID pVmStack;

			pVmxonRegion = ExAllocatePoolWithTag(NonPagedPool, 0x1000, POOL_TAG_VMXON); //4KB
			if (!pVmxonRegion)
			{
				LOG(log::ERROR, "Allocate vmxon memory failed.");
				return STATUS_MEMORY_NOT_ALLOCATED;
			}
			RtlZeroMemory(pVmxonRegion, 0x1000);

			pVmcsRegion = ExAllocatePoolWithTag(NonPagedPool, 0x1000, POOL_TAG_VMCS);
			if (!pVmcsRegion)
			{
				LOG(log::ERROR, "Allocate vmcs memory failed.");
				ExFreePoolWithTag(pVmxonRegion, 0x1000);
				return STATUS_MEMORY_NOT_ALLOCATED;
			}
			RtlZeroMemory(pVmcsRegion, 0x1000);

			pVmStack = ExAllocatePoolWithTag(NonPagedPool, KERNEL_STACK_SIZE, POOL_TAG_HOST_STACK);
			if (!pVmStack)
			{
				LOG(log::ERROR, "Allocate host stack memory failed.");
				ExFreePoolWithTag(pVmxonRegion, 0x1000);
				ExFreePoolWithTag(pVmcsRegion, 0x1000);
				return STATUS_MEMORY_NOT_ALLOCATED;
			}
			RtlZeroMemory(pVmStack, KERNEL_STACK_SIZE);

			LOG(log::INFO, "Vmxon region:0x%08X.", pVmxonRegion);
			LOG(log::INFO, "Vmcs region:0x%08X.", pVmcsRegion);
			LOG(log::INFO, "Host stack region:0x%08X.", pVmStack);

			vcpu->vmxonRegion = pVmxonRegion;
			vcpu->vmcsRegion = pVmcsRegion;
			vcpu->vmmStack = pVmStack;
			vcpu->vmmStackBase = (CHAR*)pVmStack + KERNEL_STACK_SIZE;
			return STATUS_SUCCESS;
		}

		NTSTATUS launchVmx(Vcpu* vcpu, PVOID guestRsp, PVOID guestRip)
		{
			NTSTATUS status;

			Vcpu* currentVcpu = &vcpu[Util::currentCpuIndex()];

			currentVcpu->guestState.contextFrame.Rsp = (ULONG_PTR)guestRsp;
			currentVcpu->guestState.contextFrame.Rip = (ULONG_PTR)guestRip;

			//开启Root模式
			status = enableRoot(currentVcpu);
			NT_CHECK(status);

			currentVcpu->isVmxEnable = TRUE;

			//设置VMCS 
			//DbgBreakPoint();
			status = setupVmcs(currentVcpu);
			NT_CHECK(status);



			//开启虚拟机
			__vmx_vmlaunch();

			// See:30.4 VM Instruction Error Numbers
			int error = 0;
			if (__vmx_vmread(VM_INSTRUCTION_ERROR, (size_t*)&error) != 0)
			{
				LOG(log::ERROR, "read error code failed");
				return FALSE;
			}
			LOG(log::ERROR, "vmlaunch failed,error:%d", error);
			return FALSE;
		}

		NTSTATUS quitVmx()
		{
			Cr4 cr4;

			//以Guest身份执行vmoff会导致26号vmexit
			//__vmx_off();

			VmcallParam param = { 0 };
			__vmcall(VmcallReason::EXIT, &param);


			//Cr4.VMXE置0
			cr4.all = __readcr4();
			cr4.fields.vmxe = FALSE;
			__writecr4(cr4.all);


			return STATUS_SUCCESS;
		}

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
				LOG(log::ERROR, "perform _vmx_on failed");
				return STATUS_UNSUCCESSFUL;
			}

			//vmclear
			tmpVmcsRegionPa = Util::vaToPa(vcpu->vmcsRegion);
			uRet = __vmx_vmclear(&tmpVmcsRegionPa);
			if (uRet != 0)
			{
				LOG(log::ERROR, "perform __vmx_vmclear failed");
				return STATUS_UNSUCCESSFUL;
			}

			//vmptrld
			tmpVmcsRegionPa = Util::vaToPa(vcpu->vmcsRegion);
			uRet = __vmx_vmptrld(&tmpVmcsRegionPa);
			if (uRet != 0)
			{
				LOG(log::ERROR, "perform __vmx_vmptrld failed");
				return STATUS_UNSUCCESSFUL;
			}
			return status;
		}

		NTSTATUS setupVmcs(Vcpu* vcpu)
		{
			NTSTATUS status;
			status = vmcs::setupGuestState(vcpu->guestState.contextFrame.Rsp, vcpu->guestState.contextFrame.Rip);
			NT_CHECK(status);
			status = vmcs::setupHostState((ULONG_PTR)vcpu->vmmStackBase, (ULONG_PTR)__vmm_entry_point);
			NT_CHECK(status);

			BasicMsr basicMsr;
			basicMsr.all = __readmsr(MSR_IA32_VMX_BASIC);
			bool isUseTrueMsrs = basicMsr.fields.vmx_capability_hint;
			status = vmcs::setupVmExecCtrlFields(isUseTrueMsrs);
			NT_CHECK(status);
			status = vmcs::setupVmExitCtrlFields(isUseTrueMsrs);
			NT_CHECK(status);
			status = vmcs::setupVmEntryCtrlFields(isUseTrueMsrs);
			NT_CHECK(status);
			status = vmcs::setupEptFields();
			NT_CHECK(status);

			return status;
		}
	}
}
