#include "PageAccessManager.h"
#include "Ept.h"
#include "Util.hpp"

namespace whaledbg
{
	namespace vmm
	{
		namespace pam
		{
			NTSTATUS PASHidePage()
			{
				DbgBreakPoint();
				_disable();
				// NtLoadDriver
				PVOID t_NtLoadDriver = (PVOID)0xFFFFF800042FD010;

				//配置pgae项
				ept::pageEntry.targetAddress = (ULONG_PTR)t_NtLoadDriver;
				ept::pageEntry.pageAddress = (ULONG_PTR)PAGE_ALIGN(t_NtLoadDriver);
				//DbgBreakPoint();


				//获取目标地址对应的pte
				EptEntry* pte = ept::getPtEntry(ept::eptCtrl.pml4t, MmGetPhysicalAddress((PVOID)t_NtLoadDriver).QuadPart);
				ept::pageEntry.pte = pte;
				//目标页首地址
				PVOID targetPageHeadVa = PAGE_ALIGN(t_NtLoadDriver);
				//配置假页面
				ept::pageEntry.shadowPageAddress = (ULONG_PTR)ExAllocatePoolWithTag(NonPagedPool, PAGE_SIZE, 'fake');
				if (!ept::pageEntry.shadowPageAddress)
				{
					return STATUS_MEMORY_NOT_ALLOCATED;
				}
				RtlMoveMemory((PVOID)ept::pageEntry.shadowPageAddress, targetPageHeadVa, PAGE_SIZE);
				//phData->readWritePagePa = UtilVaToPa(phData->readWritePageVa);

				//PVOID NtLoadDriver = (PVOID)0xFFFFF800043594F0;
				Util::disableWriteProtect();
				*(PCHAR)t_NtLoadDriver = 0xCC;
				Util::enableWriteProtect();

				//配置权限
				pte->fields.readAccess = 0;
				pte->fields.writeAccess = 0;
				pte->fields.executeAccess = 1;
				pte->fields.memoryType = MemoryType::WriteBack;

				ept::pageEntry.readPage = ept::pageEntry.shadowPageAddress;
				ept::pageEntry.writePage = ept::pageEntry.shadowPageAddress;
				ept::pageEntry.executePage = ept::pageEntry.pageAddress;

				_enable();
				return STATUS_SUCCESS;

			}
		}
	}
}

