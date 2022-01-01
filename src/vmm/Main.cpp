#include <ntddk.h>
#include "Common.hpp"
#include "Vmx.h"


EXTERN_C void DriverUnload(PDRIVER_OBJECT pDriver)
{
	NTSTATUS status = STATUS_SUCCESS;
	DbgPrint("Hello driver!");
	//DbgLog(Common::LogLevel::Info, "������ж��");
	//status = vmxStop();
	if (!NT_SUCCESS(status))
	{
		DbgLog(Common::LogLevel::Error, "Enable vmx failed.");
	}
}

EXTERN_C NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegStr)
{
	NTSTATUS status;
	
	DbgPrint("Hello world!");
	DbgBreakPoint();
	DbgPrint("Hello 1!");
	DbgBreakPoint();
	DbgPrint("Hello 2!");
	DbgBreakPoint();
	DbgPrint("Hello 3!");
	DbgBreakPoint();
	//DbgLog(Common::LogLevel::Info, "������װ��");
	pDriver->DriverUnload = DriverUnload;
	
	return STATUS_SUCCESS;
	//status = vmxStart();
	//if (!NT_SUCCESS(status))
	//{
	//	KdPrint(("VMX��ʼ��ʧ��"));
	//}
	////status = VmStartVmx();

	//if (!NT_SUCCESS(status))
	//{
	//	DbgLog(Common::LogLevel::Error,"VMX����ʧ��");
	//}


	//return STATUS_SUCCESS;
}