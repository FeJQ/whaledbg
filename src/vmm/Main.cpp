#include "Common.h"
#include "VmxManager.h"
#include "global.h"
#include "Util.hpp"
#include "asm.h"

EXTERN_C_BEGIN

using namespace whaledbg::vmm;

void DriverUnload(PDRIVER_OBJECT pDriver)
{
	NTSTATUS status = STATUS_SUCCESS;

	status = Util::performForEachProcessor(vmx::quitVmx);

	for (auto i = 0; i < Util::getCpuCount(); i++)
	{
		ULONG index = Util::currentCpuIndex();
		if (vmx::vcpu->vmxonRegion)
		{
			ExFreePoolWithTag(vmx::vcpu->vmxonRegion, POOL_TAG_VMXON);
			vmx::vcpu->vmxonRegion = NULL;
		}
		if (vmx::vcpu->vmcsRegion)
		{
			ExFreePoolWithTag(vmx::vcpu->vmcsRegion, POOL_TAG_VMCS);
			vmx::vcpu->vmcsRegion = NULL;
		}
		if (vmx::vcpu->vmmStackBase)
		{
			ExFreePoolWithTag(vmx::vcpu->vmmStack, POOL_TAG_HOST_STACK);
			vmx::vcpu->vmmStack = NULL;
			vmx::vcpu->vmmStackBase = NULL;
		}
	}
	Util::free(vmx::vcpu);


	if (!NT_SUCCESS(status))
	{
		DbgLog(Common::LogLevel::Error, "Enable vmx failed.");
	}
	DbgLog(Common::LogLevel::Info, "驱动已卸载");
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegStr)
{
	DbgBreakPoint();

	vmx::vcpu = (vmx::Vcpu*)Util::alloc(sizeof(Vcpu) * Util::getCpuCount());

	Common::log(__FUNCTION__, Common::LogLevel::Info, "%s,%d", "hello world", 5);
	pDriver->DriverUnload = DriverUnload;
	NTSTATUS status = STATUS_SUCCESS;

	DbgLog(Common::LogLevel::Info, "驱动已装载");

	//检查是否支持VMX
	status = vmx::checkVmxAvailable();
	NT_CHECK(status);

	//检查是否支持EPT
	status = vmx::checkEptAvailable();
	NT_CHECK(status);

	//为每一个处理器开启VMX特征
	status = Util::performForEachProcessor(vmx::enableVmxFeature);
	NT_CHECK(status);


	for (auto i = 0; i < Util::getCpuCount(); i++)
	{
		//申请VMX域空间
		status = vmx::allocVcpu(&vmx::vcpu[i]);
		NT_CHECK(status);

		//__vmlaunch(VmxManager::launchVmx,&vcpu[i]);
	}

	//在每个处理器上开启VMX
	status = Util::performForEachProcessor(__vmlaunch, vmx::launchVmx, vmx::vcpu);



	NT_CHECK(status);

	DbgLog(Common::LogLevel::Info, "VMX开启成功");
	return STATUS_SUCCESS;
}

EXTERN_C_END