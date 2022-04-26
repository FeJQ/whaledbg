#pragma once
#include <ntddk.h>
namespace vmm
{
	namespace hooker
	{
		_IRQL_requires_max_(APC_LEVEL)
			NTSTATUS hookProc(
				_In_ PVOID	 originalProc,
				_In_ PVOID	 proxyProc,
				_Out_ PVOID* OriginalTrampoline
			);

		_IRQL_requires_max_(APC_LEVEL)
			NTSTATUS unhookProc(
				_In_ PVOID	 HookedFunction,
				_In_ PVOID	 OriginalTrampoline
			);
	}
}

