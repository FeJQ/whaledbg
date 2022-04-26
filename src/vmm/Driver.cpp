#include "VmxManager.h"
#include "Util.hpp"
#include "asm.h"
#include "Ept.h"
#include "Log.h"
EXTERN_C_BEGIN

using namespace vmm;
using vmm::vmx::Vcpu;

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
		LOG(log::ERROR, "Failed to enable vmx");
	}
	LOG(log::INFO, "������ж��");
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegStr)
{
	

	vmx::vcpu = (vmx::Vcpu*)Util::alloc(sizeof(Vcpu) * Util::getCpuCount());

	LOG(log::INFO, "%s,%d", "hello world", 5);
	pDriver->DriverUnload = DriverUnload;
	NTSTATUS status = STATUS_SUCCESS;

	
	LOG(log::INFO, "������װ��");

	//����Ƿ�֧��VMX
	status = vmx::checkVmxAvailable();
	NT_CHECK(status);

	// ����Ƿ�֧��EPT
	status = vmx::checkEptAvailable();
	NT_CHECK(status);

	// Ϊÿһ������������VMX����
	status = Util::performForEachProcessor(vmx::enableVmxFeature);
	NT_CHECK(status);


	for (auto i = 0; i < Util::getCpuCount(); i++)
	{
		// ����VMX��ռ�
		status = vmx::allocVcpu(&vmx::vcpu[i]);
		NT_CHECK(status);

		//__vmlaunch(VmxManager::launchVmx,&vcpu[i]);
	}
	// ����ept
	ept::enable();

	// ��ÿ���������Ͽ���VMX
	status = Util::performForEachProcessor(__vmlaunch, vmx::launchVmx, vmx::vcpu);
	NT_CHECK(status);


	__vmcall(VmcallReason::HOOK_NT_LOAD_DRIVER, 0);
	

	LOG(log::INFO, "VMX�����ɹ�");
	return STATUS_SUCCESS;
}

EXTERN_C_END