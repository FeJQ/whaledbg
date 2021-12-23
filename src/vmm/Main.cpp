#include <ntddk.h>
#include "Common.hpp"

void DriverUnload(PDRIVER_OBJECT pDriver)
{
	NTSTATUS status;
	DbgLog("驱动已卸载", 0);
	status = VmStopVmx();
	if (!NT_SUCCESS(status))
	{
		DbgLog("VMX开启失败", 0);
	}
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegStr)
{
	NTSTATUS status;
	//DbgBreakPoint();
	DbgLog("驱动已装载", 0);
	pDriver->DriverUnload = DriverUnload;
	//DbgBreakPoint();	

	status = VmInitializeVmx();
	if (!NT_SUCCESS(status))
	{
		DbgLog("VMX初始化失败", 0);
	}
	status = VmStartVmx();

	if (!NT_SUCCESS(status))
	{
		DbgLog("VMX开启失败", 0);
	}


	return STATUS_SUCCESS;
}