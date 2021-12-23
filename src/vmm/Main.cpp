#include <ntddk.h>
#include "Common.hpp"

void DriverUnload(PDRIVER_OBJECT pDriver)
{
	NTSTATUS status;
	DbgLog("������ж��", 0);
	status = VmStopVmx();
	if (!NT_SUCCESS(status))
	{
		DbgLog("VMX����ʧ��", 0);
	}
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegStr)
{
	NTSTATUS status;
	//DbgBreakPoint();
	DbgLog("������װ��", 0);
	pDriver->DriverUnload = DriverUnload;
	//DbgBreakPoint();	

	status = VmInitializeVmx();
	if (!NT_SUCCESS(status))
	{
		DbgLog("VMX��ʼ��ʧ��", 0);
	}
	status = VmStartVmx();

	if (!NT_SUCCESS(status))
	{
		DbgLog("VMX����ʧ��", 0);
	}


	return STATUS_SUCCESS;
}