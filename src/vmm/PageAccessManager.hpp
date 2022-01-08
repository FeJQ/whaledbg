#pragma once
#include "Common.hpp"
#include "Ept.hpp"



class PageAccessManager
{
public:
	PageAccessManager()
	{
		this->pageEntry = { 0 };
		InitializeListHead(&pageEntry.pageList);
	}

	NTSTATUS separatePageAccess(PVOID targetAddress)
	{
		NTSTATUS status = STATUS_SUCCESS;

		// 判断目标页是否已经被权限分离过
		bool flag = false;
		for (LIST_ENTRY* pLink = this->pageEntry.pageList.Flink; pLink != (PLIST_ENTRY)&this->pageEntry.pageList.Flink; pLink = pLink->Flink)
		{
			PageEntry* tempPageEntry = CONTAINING_RECORD(pLink, PageEntry, pageList);		
			if (tempPageEntry->targetPageAddress == (ULONG_PTR)PAGE_ALIGN((ULONG_PTR)targetAddress))
			{
				flag = true;
				break;
			}
		}
		if (flag != true) return status;

		//配置pgae项
		PageEntry* newPageEntry = (PageEntry*)ExAllocatePoolWithTag(NonPagedPool, sizeof(PageEntry), 'page');
		if (!newPageEntry)
		{
			return STATUS_MEMORY_NOT_ALLOCATED;
		}
		RtlZeroMemory(newPageEntry, sizeof(PageEntry));
		newPageEntry->targetAddress = (ULONG_PTR)targetAddress;
		newPageEntry->targetPageAddress = (ULONG_PTR)PAGE_ALIGN(targetAddress);
		InsertHeadList(&this->pageEntry.pageList, &newPageEntry->pageList);

		//获取目标地址对应的pte
		EptEntry* pte = Ept::getPtEntry(eptCtrl.pml4t, MmGetPhysicalAddress(targetAddressVa).QuadPart);
		pgEntry->pte = pte;
		//目标页首地址
		PVOID targetPageHeadVa = PAGE_ALIGN(targetAddressVa);
		//配置假页面
		pgEntry->shadowPageAddressVa = (ULONG_PTR)ExAllocatePoolWithTag(NonPagedPool, PAGE_SIZE, 'fake');
		if (!pgEntry->shadowPageAddressVa)
		{
			return STATUS_MEMORY_NOT_ALLOCATED;
		}
		RtlMoveMemory((PVOID)pgEntry->shadowPageAddressVa, targetPageHeadVa, PAGE_SIZE);
		//phData->readWritePagePa = UtilVaToPa(phData->readWritePageVa);

		PVOID NtLoadDriver = (PVOID)0xFFFFF800043594F0;
		mem_protect_close();
		*(PCHAR)NtLoadDriver = 0xCC;
		mem_protect_open();

		//配置权限
		pte->fields.readAccess = (eptAccess & EptAccess::EptAccessRead) >> 0;
		pte->fields.writeAccess = (eptAccess & EptAccess::EptAccessWrite) >> 1;
		pte->fields.executeAccess = (eptAccess & EptAccess::EptAccessExecute) >> 2;
		pte->fields.memoryType = kWriteBack;
		*outPgEntry = pgEntry;

	}

public:
	
};