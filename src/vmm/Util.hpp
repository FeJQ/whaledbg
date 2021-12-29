#pragma once
#include <ntddk.h>
class Util
{
public:
	static NTSTATUS performForEachProcessor(NTSTATUS(*routine)())
	{
		return performForEachProcessor(routine);
	}

	static NTSTATUS performForEachProcessor(NTSTATUS(*routine)(void* arg), void* context = nullptr)
	{
		return performForEachProcessor(routine, context);
	}
	/**
	 * 在每个处理器上执行routine
	 *
	 * @param *routine: 要执行的函数指针
	 * @param context1: 参数1
	 * @param context2: 参数2
	 * @return NTSTATUS: 状态码
	 */
	static NTSTATUS performForEachProcessor(NTSTATUS(*routine)(void* arg1, void* arg2), void* context1 = nullptr, void* context2 = nullptr)
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
			NT_CHECK(status);
		}
		return STATUS_SUCCESS;
	}

	

	/**
	 * 申请非分页内存
	 *
	 * @param size:大小
	 * @return PVOID:
	 */
	static PVOID alloc(ULONG_PTR size)
	{
		PHYSICAL_ADDRESS p = { 0 };
		return MmAllocateContiguousMemory(size, p);
	}

	/**
	 * 释放非分页内存
	 *
	 * @param p:
	 * @return void:
	 */
	static void free(PVOID p)
	{
		MmFreeContiguousMemory(p);
	}

	/**
	 * 获取CPU核心数量
	 *
	 * @return ULONG:
	 */
	static ULONG getCpuCount()
	{
		return KeQueryActiveProcessorCountEx(ALL_PROCESSOR_GROUPS);
	}

	/**
	 * 获取当前cpu索引
	 *
	 * @return ULONG:
	 */
	static ULONG currentCpuIndex()
	{
		return KeGetCurrentProcessorNumberEx(NULL);
	}

	/**
	 * 虚拟地址转物理地址
	 *
	 * @param void * virtualAddress:
	 * @return ULONG_PTR:
	 */
	static ULONG_PTR vaToPa(void* virtualAddress)
	{
		PHYSICAL_ADDRESS pa = MmGetPhysicalAddress(virtualAddress);
		return pa.QuadPart;
	}

	/**
	 * 物理地址转虚拟地址
	 *
	 * @param pa:物理地址
	 * @return void*:
	 */
	static void* paToVa(ULONG64 pa)
	{
		PHYSICAL_ADDRESS paddr = { 0 };
		paddr.QuadPart = pa;
		return MmGetVirtualForPhysical(paddr);
	}

	/**
	 * 检查平台架构是否为x64
	 *
	 * @return bool:
	 */
	static bool checkAmd64()
	{
#ifdef AMD64
		return true;
#else
		return false;
#endif // AMD64
	}



};

