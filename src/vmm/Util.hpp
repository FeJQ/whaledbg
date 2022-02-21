#pragma once
#include <ntddk.h>
#include "Common.hpp"

typedef NTSTATUS(*Routine_t)();
typedef NTSTATUS(*Routine_t_arg1)(void* arg1);
typedef NTSTATUS(*Routine_t_arg2)(void* arg1, void* arg2);

class Util
{
public:
	static NTSTATUS performForEachProcessor(Routine_t routine)
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
			status = routine();

			KeRevertToUserGroupAffinityThread(&preAffinity);
			NT_CHECK(status);
		}
		return STATUS_SUCCESS;

	}

	static NTSTATUS performForEachProcessor(Routine_t_arg1 routine, void* context = nullptr)
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
			status = routine(context);

			KeRevertToUserGroupAffinityThread(&preAffinity);
			NT_CHECK(status);
		}
		return STATUS_SUCCESS;
	}
	/**
	 * ��ÿ����������ִ��routine
	 *
	 * @param *routine: Ҫִ�еĺ���ָ��
	 * @param context1: ����1
	 * @param context2: ����2
	 * @return NTSTATUS: ״̬��
	 */
	static NTSTATUS performForEachProcessor(Routine_t_arg2 routine, void* context1 = nullptr, void* context2 = nullptr)
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
			NT_CHECK(status);
		}
		return STATUS_SUCCESS;
	}



	/**
	 * ����Ƿ�ҳ�ڴ�
	 *
	 * @param size:��С
	 * @return PVOID:
	 */
	static PVOID alloc(ULONG_PTR size)
	{
		// pa:�����߿���ʹ�õ������Ч�����ַ
		PHYSICAL_ADDRESS pa = { -1 };
		return MmAllocateContiguousMemory(size, pa);
	}

	/**
	 * �ͷŷǷ�ҳ�ڴ�
	 *
	 * @param p:
	 * @return void:
	 */
	static void free(PVOID p)
	{
		MmFreeContiguousMemory(p);
	}

	/**
	 * ��ȡCPU��������
	 *
	 * @return ULONG:
	 */
	static ULONG getCpuCount()
	{
		return KeQueryActiveProcessorCountEx(ALL_PROCESSOR_GROUPS);
	}

	/**
	 * ��ȡ��ǰcpu����
	 *
	 * @return ULONG:
	 */
	static ULONG currentCpuIndex()
	{
		return KeGetCurrentProcessorNumberEx(NULL);
	}

	/**
	 * �����ַת�����ַ
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
	 * �����ַת�����ַ
	 *
	 * @param pa:�����ַ
	 * @return void*:
	 */
	static void* paToVa(ULONG64 pa)
	{
		PHYSICAL_ADDRESS paddr = { 0 };
		paddr.QuadPart = pa;
		return MmGetVirtualForPhysical(paddr);
	}

	/**
	 * ���ƽ̨�ܹ��Ƿ�Ϊx64
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

	/**
	 * �����ڴ汣��
	 *
	 */
	static void disableWriteProtect()
	{
		//_disable();
		__writecr0(__readcr0() & (~(0x10000)));
	}

	/**
	 * �����ڴ汣��
	 *
	 */
	static void enableWriteProtect()
	{
		__writecr0(__readcr0() ^ 0x10000);
		//_enable();
	}

};

