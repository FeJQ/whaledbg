#include "Vmx.h"


VmxManager vmxManager;
VmExitHandler vmExitHandler;

NTSTATUS launchVmx(PVOID guestStack, PVOID guestResumeRip)
{
	return vmxManager.launchVmx(guestStack, guestResumeRip);
}

NTSTATUS vmExitEntryPoint(Registers64* pGuestRegisters)
{
	return vmExitHandler.vmExitEntryPoint(pGuestRegisters);
}

/**
 * 启动vmx
 *
 * @return NTSTATUS:
 */
 NTSTATUS vmxStart()
{
	NTSTATUS status = STATUS_SUCCESS;
	//检查是否支持VMX
	status = vmxManager.checkVmxAvailable();
	NT_CHECK(status);

	//检查是否支持EPT
	status = vmxManager.checkEptAvailable();
	NT_CHECK(status);

	//为每一个处理器开启VMX特征
	status = Util::performForEachProcessor(VmxManager::enableVmxFeature);
	NT_CHECK(status);

	//申请VMX域空间
	status = vmxManager.allocateVmxRegion();
	NT_CHECK(status);

	//在每个处理器上开启VMX
	status = Util::performForEachProcessor(__vmlaunch);
	if (!NT_SUCCESS(status))
	{
		return status;
	}
	return status;
}

/**
 * 关闭vmx
 *
 * @return NTSTATUS: 
 */
 NTSTATUS vmxStop()
{
	NTSTATUS status = STATUS_SUCCESS;

	VmxCpuContext* vmxCpuContext = VmxManager::getStaticVmxContext();
	Util::performForEachProcessor(VmxManager::quitVmx);
	Util::free(vmxCpuContext);
	return status;
}
