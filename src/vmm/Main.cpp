#include "Common.hpp"
#include "VmxManager.hpp"
#include "global.h"

EXTERN_C_BEGIN

void DriverUnload(PDRIVER_OBJECT pDriver)
{
	NTSTATUS status = STATUS_SUCCESS;

	status = Util::performForEachProcessor(VmxManager::quitVmx);

	for (auto i = 0; i < Util::getCpuCount(); i++)
	{
		ULONG index = Util::currentCpuIndex();
		if (vcpu->vmxonRegion)
		{
			ExFreePoolWithTag(vcpu->vmxonRegion, POOL_TAG_VMXON);
			vcpu->vmxonRegion = NULL;
		}
		if (vcpu->vmcsRegion)
		{
			ExFreePoolWithTag(vcpu->vmcsRegion, POOL_TAG_VMCS);
			vcpu->vmcsRegion = NULL;
		}
		if (vcpu->vmmStackBase)
		{
			ExFreePoolWithTag(vcpu->vmmStack, POOL_TAG_HOST_STACK);
			vcpu->vmmStack = NULL;
			vcpu->vmmStackBase = NULL;
		}
	}
	Util::free(vcpu);


	if (!NT_SUCCESS(status))
	{
		DbgLog(Common::LogLevel::Error, "Enable vmx failed.");
	}
	DbgLog(Common::LogLevel::Info, "������ж��");
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegStr)
{
	DbgBreakPoint();

	vcpu = (Vcpu*)Util::alloc(sizeof(Vcpu) * Util::getCpuCount());

	Common::log(__FUNCTION__, Common::LogLevel::Info, "%s,%d", "hello world", 5);
	pDriver->DriverUnload = DriverUnload;
	NTSTATUS status = STATUS_SUCCESS;

	DbgLog(Common::LogLevel::Info, "������װ��");

	//����Ƿ�֧��VMX
	status = VmxManager::instance().checkVmxAvailable();
	NT_CHECK(status);

	//����Ƿ�֧��EPT
	status = VmxManager::instance().checkEptAvailable();
	NT_CHECK(status);

	//Ϊÿһ������������VMX����
	status = Util::performForEachProcessor(VmxManager::enableVmxFeature);
	NT_CHECK(status);


	for (auto i = 0; i < Util::getCpuCount(); i++)
	{
		//����VMX��ռ�
		status = VmxManager::instance().allocVcpu(&vcpu[i]);
		NT_CHECK(status);

		//__vmlaunch(VmxManager::launchVmx,&vcpu[i]);
	}

	//��ÿ���������Ͽ���VMX
	status = Util::performForEachProcessor(__vmlaunch, VmxManager::launchVmx, vcpu);



	NT_CHECK(status);

	DbgLog(Common::LogLevel::Info, "VMX�����ɹ�");
	return STATUS_SUCCESS;
}

EXTERN_C_END