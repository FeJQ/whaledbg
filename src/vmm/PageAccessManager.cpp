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

				//����pgae��
				ept::pageEntry.targetAddress = (ULONG_PTR)t_NtLoadDriver;
				ept::pageEntry.pageAddress = (ULONG_PTR)PAGE_ALIGN(t_NtLoadDriver);
				//DbgBreakPoint();


				//��ȡĿ���ַ��Ӧ��pte
				EptEntry* pte = ept::getPtEntry(ept::eptCtrl.pml4t, MmGetPhysicalAddress((PVOID)t_NtLoadDriver).QuadPart);
				ept::pageEntry.pte = pte;
				//Ŀ��ҳ�׵�ַ
				PVOID targetPageHeadVa = PAGE_ALIGN(t_NtLoadDriver);
				//���ü�ҳ��
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

				//����Ȩ��
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

