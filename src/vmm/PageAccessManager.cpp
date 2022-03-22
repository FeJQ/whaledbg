#include "PageAccessManager.h"
#include "Ept.h"
#include "Util.hpp"


namespace vmm
{
	namespace pam
	{
		NTSTATUS PASHidePage()
		{
			//DbgBreakPoint();
			_disable();
			// NtLoadDriver
			PVOID t_NtLoadDriver = (PVOID)0xfffff80002f441f0;
			

			DbgBreakPoint();


			//��ȡĿ���ַ��Ӧ��pte
			PteEntry* pte = ept::getPtEntry(ept::eptCtrl.pml4t, MmGetPhysicalAddress((PVOID)t_NtLoadDriver).QuadPart);


			//ept::pageEntry.pte = pte;
			//Ŀ��ҳ�׵�ַ
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

			KIRQL oldIrql;
			ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
			KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);
			//����pteȨ��
			pte->fields.readAccess = 0;
			pte->fields.writeAccess = 0;
			pte->fields.executeAccess = 1;
			pte->fields.memoryType = MemoryType::WriteBack;
			ept::invalidGlobalEptCache();
			KeLowerIrql(oldIrql);


			// ����ҳ������
			PageEntry* targetPageEntry = (PageEntry*)Util::alloc(sizeof(PageEntry));
			targetPageEntry->targetAddress = (ULONG_PTR)t_NtLoadDriver;
			targetPageEntry->pageAddress = (ULONG_PTR)PAGE_ALIGN(t_NtLoadDriver);
			targetPageEntry->pte = pte;
			targetPageEntry->shadowPageAddress = (ULONG_PTR)shadowPageAddress;
			targetPageEntry->readPage = targetPageEntry->shadowPageAddress;
			targetPageEntry->writePage = targetPageEntry->shadowPageAddress;
			targetPageEntry->executePage = targetPageEntry->pageAddress;

			// �������ݵ�����
			InsertHeadList(&ept::pageListHead, &targetPageEntry->pageList);
			_enable();
			return STATUS_SUCCESS;

		}
	}
}


