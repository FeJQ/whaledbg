#pragma once
#include <ntddk.h>
class Util
{
public:
	/**
	 * 在每个处理器上执行routine
	 *
	 * @param *routine: 要执行的函数指针
	 * @param context1: 参数1
	 * @param context2: 参数2
	 * @return NTSTATUS: 状态码
	 */
	static NTSTATUS performForEachProcessor(NTSTATUS(*routine)(void* arg1, void* arg2), void* context1, void* context2)
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
			//切换到i号处理器
			GROUP_AFFINITY affinity = { 0 };
			affinity.Group = processorNumber.Group;
			affinity.Mask = 1ull << processorNumber.Number;
			GROUP_AFFINITY preAffinity = { 0 };
			KeSetSystemGroupAffinityThread(&affinity, &preAffinity);

			//执行回调
			status = routine(context1, context2);

			KeRevertToUserGroupAffinityThread(&preAffinity);
			if (!NT_SUCCESS(status))
			{
				return status;
			}
		}
		return STATUS_SUCCESS;
	}



};

