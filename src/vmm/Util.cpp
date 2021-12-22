#include "Util.h"

NTSTATUS Util::performForEachProcessor(NTSTATUS(*routine)(void* arg1, void* arg2), void* context1, void* context2)
{
	NTSTATUS status = STATUS_SUCCESS;
	ULONG processorCount = KeQueryActiveProcessorCountEx(ALL_PROCESSOR_GROUPS);
	for (ULONG i = 0; i < processorCount; i++)
	{
		PROCESSOR_NUMBER processorNumber = { 0 };
		status = KeGetProcessorNumberFromIndex(i, &processorNumber);
		if (!NT_SUCCESS(status))
		{
			return status;
		}
		//�л���i�Ŵ�����
		GROUP_AFFINITY affinity = { 0 };
		affinity.Group = processorNumber.Group;
		affinity.Mask = 1ull << processorNumber.Number;
		GROUP_AFFINITY preAffinity = { 0 };
		KeSetSystemGroupAffinityThread(&affinity, &preAffinity);

		//ִ�лص�
		status = routine(context1, context2);

		KeRevertToUserGroupAffinityThread(&preAffinity);
		if (!NT_SUCCESS(status))
		{
			return status;
		}
	}
	return STATUS_SUCCESS;
}
