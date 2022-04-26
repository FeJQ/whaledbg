#include "PageAccessManager.h"
#include "Ept.h"
#include "Util.hpp"


namespace vmm
{
	namespace pam
	{

		using ept::PteEntry;
		using ept::HookedPage;
		NTSTATUS PASHidePage()
		{
			//DbgBreakPoint();
			//_disable();
			KIRQL oldIrql;
			ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
			KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);
			// NtLoadDriver
			PVOID t_NtLoadDriver = (PVOID)0xfffff80002f441f0;


			//获取目标地址对应的pte
			PteEntry* pte = ept::getPtEntry(MmGetPhysicalAddress((PVOID)t_NtLoadDriver).QuadPart);

			//ept::pageEntry.pte = pte;
			//目标页首地址
			PVOID targetPageHeadVa = PAGE_ALIGN(t_NtLoadDriver);
			PVOID shadowPageAddress = ExAllocatePoolWithTag(NonPagedPool, PAGE_SIZE, 'fake');
			if (!shadowPageAddress)
			{
				return STATUS_MEMORY_NOT_ALLOCATED;
			}
			RtlMoveMemory(shadowPageAddress, targetPageHeadVa, PAGE_SIZE);
			//phData->readWritePagePa = UtilVaToPa(phData->readWritePageVa);

			//PVOID NtLoadDriver = (PVOID)0xFFFFF800043594F0;
			Util::disableWriteProtect();
			*(PCHAR)t_NtLoadDriver = 0xCC;
			Util::enableWriteProtect();

			//配置pte权限
			pte->fields.readAccess = 0;
			pte->fields.writeAccess = 0;
			pte->fields.executeAccess = 1;
			pte->fields.memoryType = MemoryType::WriteBack;
			ept::invalidGlobalEptCache();

			// 配置页表数据
			HookedPage* targetPageEntry = (HookedPage*)Util::alloc(sizeof(HookedPage));
			//targetPageEntry->hookedPageAddress = (ULONG_PTR)t_NtLoadDriver;
			targetPageEntry->originalPageAddress = (ULONG_PTR)PAGE_ALIGN(t_NtLoadDriver);
			targetPageEntry->pte = pte;
			targetPageEntry->shadowPageAddress = (ULONG_PTR)shadowPageAddress;

			// 插入数据到链表
			InsertHeadList(&ept::eptState.hookedPage.listEntry, &targetPageEntry->listEntry);
			//_enable();
			KeLowerIrql(oldIrql);
			return STATUS_SUCCESS;

		}
	}
}


