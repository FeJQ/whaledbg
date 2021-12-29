#include <ntddk.h>
#include "Common.hpp"
#include "Vmx.h"


Export void DriverUnload(PDRIVER_OBJECT pDriver)
{
	NTSTATUS status;
	DbgLog("驱动已卸载", 0);
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
	DbgLog("驱动已装载", 0);
	pDriver->DriverUnload = DriverUnload;
	//DbgBreakPoint();	

	status = vmxStart();
	if (!NT_SUCCESS(status))
	{
		KdPrint(("VMX初始化失败"));
	}
	//status = VmStartVmx();

	if (!NT_SUCCESS(status))
	{
		DbgLog("VMX开启失败", 0);
	}


	return STATUS_SUCCESS;
}