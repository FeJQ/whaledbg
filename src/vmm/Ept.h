#pragma once
#include "Common.h"
#include "IA32.h"
#include "global.h"
#include "PageAccessManager.h"

namespace whaledbg
{
	namespace vmm
	{
		namespace ept
		{
			NTSTATUS initialize();

			/**
			 * 开启Ept
			 */
			NTSTATUS enable();

			/**
			 * 分配页表内存
			 * @return EptEntry*: 1级页表(PML4T)的首地址
			 */
			EptEntry* allocEptMemory();

			/**
			 * 获取物理地址所对应的PTE
			 * @param EptEntry * pml4t: pml4t首地址
			 * @param ULONG_PTR pa: 要查询的物理地址
			 * @return EptEntry*: PTE
			 */
			EptEntry* getPtEntry(EptEntry* pml4t, ULONG_PTR pa);

			/**
			 * 隐藏页面
			 * @param PVOID targetAddress:
			 * @return NTSTATUS:
			 */
			NTSTATUS hidePage(PVOID targetAddress);

			/**
			 * 恢复页面
			 * @param PVOID targetAddress:
			 * @return NTSTATUS:
			 */
			NTSTATUS recoverPage(PVOID targetAddress);

			/**
			 * 页表权限分离
			 * @param PVOID targetPageEntry: 目标页表项
			 * @return NTSTATUS:
			 */
			NTSTATUS separatePageAccess(PageEntry* targetPageEntry, EptAccess eptAccess);

			/**
			 * 设置页访问权限
			 * @param ULONG_PTR pageAddress:
			 * @param EptAccess access:
			 * @return NTSTATUS:
			 */
			NTSTATUS setPageAccess(ULONG_PTR pageAddress, EptAccess access);


			extern EptControl eptCtrl;
			extern PageEntry pageEntry;
		}
	}
}
