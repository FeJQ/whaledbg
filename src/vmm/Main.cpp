#include <ntddk.h>
#include "Common.hpp"
#include "Vmx.h"


Export void DriverUnload(PDRIVER_OBJECT pDriver)
{
	NTSTATUS status;
	DbgLog("������ж��", 0);
	status = vmxStop();
	if (!NT_SUCCESS(status))
	{
		Common::log(Common::LogLevel::Error, "Enable vmx failed.");
	}
}

Export NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegStr)
{
	NTSTATUS status;

	//DbgBreakPoint();
	DbgLog("������װ��", 0);
	pDriver->DriverUnload = DriverUnload;
	//DbgBreakPoint();	

	status = vmxStart();
	if (!NT_SUCCESS(status))
	{
		KdPrint(("VMX��ʼ��ʧ��"));
	}
	//status = VmStartVmx();

	if (!NT_SUCCESS(status))
	{
		DbgLog("VMX����ʧ��", 0);
	}


	return STATUS_SUCCESS;
}