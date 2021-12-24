#pragma once
#include "Common.hpp"
#include "IA32.h"
#include <intrin.h>
#include "Util.hpp"

#define POOL_TAG_VMXON 'vmon'
#define POOL_TAG_VMCS 'vmcs'
#define POOL_TAG_HOST_STACK 'hstk'

enum VmcallReason
{
	VmcallVmxOff = 54886475,
	VmcallLstarHookEnable,
	VmcallLstarHookDisable,
};

struct VmxContext
{
	PVOID pVmxonRegion;
	PVOID pVmcsRegion;
	PVOID pVmStack;
	PVOID pVmStackBase;

	ULONG64 originalLstar;
	PVOID newKiSystemCall64;
	BOOLEAN bIsVmxEnable;
};

class VmxManager
{
public:
	NTSTATUS start()
	{
		NTSTATUS status;

		//检查是否支持VMX
		status = checkVmxAvailable();
		NT_CHECK(status);

		//检查是否支持EPT
		status = checkEptAvailable();
		NT_CHECK(status);

		//为每一个处理器开启VMX特征
		status = Util::performForEachProcessor(enableVmxFeature);
		NT_CHECK(status);

		//申请VMX域空间
		status = allocateRegion();
		NT_CHECK(status);

		//在每个处理器上开启VMX
		status = Util::performForEachProcessor(__vmlaunch);
		if (!NT_SUCCESS(status))
		{
			return status;
		}
	}
	override NTSTATUS stop()
	{

	}

private:
	VmxContext*& getStaticVmxContext()
	{
		static VmxContext* vmxContext = NULL;
		return vmxContext;
	}
	override bool checkVmxAvailable()
	{
		CpuIdField cpuIdField = { 0 };
		Cr0 cr0;
		Cr4 cr4;
		ControlMsr controlMsr = { 0 };
		BasicMsr basicMsr = { 0 };

		//1.CPUID
		__cpuid((int*)&cpuIdField, 1);
		CpuIdFiledEcx* cpuIdFiledEcx = (CpuIdFiledEcx*)(&cpuIdField.rcx);
		if (cpuIdFiledEcx->fields.vmx != 1)
		{
			Common::log(Common::LogLevel::Error, "Vt is not supported on this machine.");
			return STATUS_HV_FEATURE_UNAVAILABLE;
		}

		//2.检测cr0
		cr0.all = __readcr0();
		if (!cr0.fields.pg || !cr0.fields.ne || !cr0.fields.pe)
		{
			Common::log(Common::LogLevel::Error, "Cr0 not supported to be virtualizaion.");
			return STATUS_HV_FEATURE_UNAVAILABLE;
		}

		//2.检查 BASIC_MSR,判断是否支持回写内存
		//See 24.2 Fromat of the VMCX region
		basicMsr.all = __readmsr(MSR_IA32_VMX_BASIC);
		if (basicMsr.fields.memory_type != MemoryType::WriteBack)
		{
			Common::log(Common::LogLevel::Error, "Write-back cache type is not supported.");
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
		return TRUE;
	}

	/**
	 * 检查Ept是否可用
	 *
	 * @return override bool:
	 */
	bool checkEptAvailable()
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
			Common::log(Common::LogLevel::Warnning, "Ept is unavailable.");
			return false;
		}
		return true;
	}

	/**
	 * 开启vmx标志
	 *
	 * @param void * arg1:
	 * @param void * arg2:
	 * @return override NTSTATUS:
	 */
	static NTSTATUS enableVmxFeature(void* arg1, void* arg2)
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
			Common::log(Common::LogLevel::Error, "Virtualization is not enabled in the BIOS.");
			return STATUS_HV_FEATURE_UNAVAILABLE;
		}
		return STATUS_SUCCESS;
	}

	override NTSTATUS allocateRegion()
	{
		VmxContext* vmxContext = getStaticVmxContext();
		vmxContext = (VmxContext*)Util::alloc(sizeof(VmxContext) * Util::getCpuCount());

		for (int i = 0; i < Util::getCpuCount(); i++)
		{
			PVOID pVmxonRegion;
			PVOID pVmcsRegion;
			PVOID pVmStack;

			pVmxonRegion = ExAllocatePoolWithTag(NonPagedPool, 0x1000, POOL_TAG_VMXON); //4KB
			if (!pVmxonRegion)
			{
				Common::log(Common::LogLevel::Error, "Allocate vmxon memory failed.");
				return STATUS_MEMORY_NOT_ALLOCATED;
			}
			RtlZeroMemory(pVmxonRegion, 0x1000);

			pVmcsRegion = ExAllocatePoolWithTag(NonPagedPool, 0x1000, POOL_TAG_VMCS);
			if (!pVmcsRegion)
			{
				Common::log(Common::LogLevel::Error, "Allocate vmcs memory failed.");
				ExFreePoolWithTag(pVmxonRegion, 0x1000);
				return STATUS_MEMORY_NOT_ALLOCATED;
			}
			RtlZeroMemory(pVmcsRegion, 0x1000);

			pVmStack = ExAllocatePoolWithTag(NonPagedPool, KERNEL_STACK_SIZE, POOL_TAG_HOST_STACK);
			if (!pVmStack)
			{
				Common::log(Common::LogLevel::Error, "Allocate host stack memory failed.");
				ExFreePoolWithTag(pVmxonRegion, 0x1000);
				ExFreePoolWithTag(pVmcsRegion, 0x1000);
				return STATUS_MEMORY_NOT_ALLOCATED;
			}
			RtlZeroMemory(pVmStack, KERNEL_STACK_SIZE);

			Common::log(Common::LogLevel::Info, "Vmxon region:0x%08X.", pVmxonRegion);
			Common::log(Common::LogLevel::Info, "Vmcs region:0x%08X.", pVmcsRegion);
			Common::log(Common::LogLevel::Info, "Host stack region:0x%08X.", pVmStack);

			vmxContext[i].pVmxonRegion = pVmxonRegion;
			vmxContext[i].pVmcsRegion = pVmcsRegion;
			vmxContext[i].pVmStack = pVmStack;
			vmxContext[i].pVmStackBase = (CHAR*)pVmStack + KERNEL_STACK_SIZE;
		}
		return STATUS_SUCCESS;
	}
private:



};

