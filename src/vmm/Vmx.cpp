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
 * ����vmx
 *
 * @return NTSTATUS:
 */
 NTSTATUS vmxStart()
{
	NTSTATUS status = STATUS_SUCCESS;
	//����Ƿ�֧��VMX
	status = vmxManager.checkVmxAvailable();
	NT_CHECK(status);

	//����Ƿ�֧��EPT
	status = vmxManager.checkEptAvailable();
	NT_CHECK(status);

	//Ϊÿһ������������VMX����
	status = Util::performForEachProcessor(VmxManager::enableVmxFeature);
	NT_CHECK(status);

	//����VMX��ռ�
	status = vmxManager.allocateVmxRegion();
	NT_CHECK(status);

	//��ÿ���������Ͽ���VMX
	status = Util::performForEachProcessor(__vmlaunch);
	if (!NT_SUCCESS(status))
	{
		return status;
	}
	return status;
}

/**
 * �ر�vmx
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
