#include <ntddk.h>
#include "Common.hpp"
#include "Vmx.h"


EXTERN_C void DriverUnload(PDRIVER_OBJECT pDriver)
{
	NTSTATUS status = STATUS_SUCCESS;
	DbgLog(Common::LogLevel::Info, "������ж��");
	status = vmxStop();
	if (!NT_SUCCESS(status))
	{
		DbgLog(Common::LogLevel::Error, "Enable vmx failed.");
	}
}

EXTERN_C NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegStr)
{
	NTSTATUS status= STATUS_SUCCESS;
	
	DbgBreakPoint();
	DbgLog(Common::LogLevel::Info, "������װ��");
	pDriver->DriverUnload = DriverUnload;
		
	status = vmxStart();
	if (!NT_SUCCESS(status))
	{
		DbgLog(Common::LogLevel::Error,"VMX����ʧ��");
	}

	return STATUS_SUCCESS;
}