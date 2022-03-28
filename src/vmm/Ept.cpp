#include "Ept.h"
#include "Util.hpp"
#include "PageAccessManager.h"
#include "asm.h"
#include "Log.h"

namespace vmm
{
	namespace ept
	{
		EptState eptState;

		NTSTATUS enable()
		{
			NTSTATUS status = STATUS_SUCCESS;


			InitializeListHead(&eptState.hookedPage.listEntry);

			eptState.pml4t = (PteEntry*)allocEptMemory();
			ASSERT(eptState.pml4t != NULL);

			// Set up the EPTP
			eptState.eptp.fields.physAddr = MmGetPhysicalAddress(eptState.pml4t).QuadPart >> 12;
			eptState.eptp.fields.memoryType = MemoryType::WriteBack;
			eptState.eptp.fields.pageWalkLength = 3;




			//PhRootine();

			//pam::PASHidePage();

			return status;
		}


		PVOID allocEptMemory()
		{
			//EPTѰַ�ṹ
			//����      ����        ��С(λ)
			//PML4T		256T		9
			//PDPT		512G		9
			//PDT		1G			9
			//PT		2M			9
			//PAGE		4K			12

			PteEntry* pml4t = 0;
			PteEntry* pdpt = 0;
			PteEntry* pdt = 0;
			PteEntry* pt = 0;

			//1��PML4T  EPT_PML4T_COUNT
			pml4t = (PteEntry*)(ExAllocatePoolWithTag(NonPagedPoolNx, PAGE_SIZE, 'pml4'));
			if (!pml4t)
			{
				return NULL;
			}
			RtlZeroMemory(pml4t, PAGE_SIZE);

			//1��PDPT  EPT_PDPT_COUNT
			pdpt = (PteEntry*)(ExAllocatePoolWithTag(NonPagedPoolNx, PAGE_SIZE, 'pdpt'));
			if (!pdpt)
			{
				return NULL;
			}
			RtlZeroMemory(pdpt, PAGE_SIZE);

			//PML4T�洢PDPT���׵�ַ(�����ַ)
			pml4t[0].all = *(ULONG64*)&MmGetPhysicalAddress(pdpt);
			pml4t[0].fields.readAccess = true;
			pml4t[0].fields.writeAccess = true;
			pml4t[0].fields.executeAccess = true;
			// n��PDT (ÿһ�ſ���֧��1G�ڴ�Ѱַ,�����ڴ�������,���512) 
			// ʵ�������ڷ�ҳ�ڴ���Խ��������̵�Ե��,�������������һ��Ҫ��ʵ�������ڴ������Ҫ��
			for (ULONG i = 0; i < EPT_PDT_COUNT; i++)
			{
				pdt = (PteEntry*)(ExAllocatePoolWithTag(NonPagedPoolNx, PAGE_SIZE, 'pdt'));
				if (!pdt)
				{
					return NULL;
				}
				RtlZeroMemory(pdt, PAGE_SIZE);
				pdpt[i].all = *(ULONG64*)&MmGetPhysicalAddress(pdt);
				pdpt[i].fields.readAccess = true;
				pdpt[i].fields.writeAccess = true;
				pdpt[i].fields.executeAccess = true;
				for (ULONG j = 0; j < EPT_PT_COUNT; j++)
				{
					pt = (PteEntry*)(ExAllocatePoolWithTag(NonPagedPoolNx, PAGE_SIZE, 'pt'));
					if (!pt)
					{
						return NULL;
					}
					RtlZeroMemory(pt, PAGE_SIZE);
					pdt[j].all = *(ULONG64*)&MmGetPhysicalAddress(pt);
					pdt[j].fields.readAccess = true;
					pdt[j].fields.writeAccess = true;
					pdt[j].fields.executeAccess = true;
					for (ULONG k = 0; k < EPT_PAGE_COUNT; k++)
					{
						pt[k].all = (i * (ULONG64)(1 << 30) + j * (ULONG64)(1 << 21) + k * (ULONG64)(1 << 12));
						pt[k].fields.readAccess = true;
						pt[k].fields.writeAccess = true;
						pt[k].fields.executeAccess = true;
						pt[k].fields.memoryType = MemoryType::WriteBack;
					}
				}
			}
			return (PVOID)pml4t;
		}

		NTSTATUS freeEptMemory()
		{
			NTSTATUS status = STATUS_SUCCESS;
			for (LIST_ENTRY* pLink = eptState.hookedPage.listEntry.Flink; pLink != &eptState.hookedPage.listEntry; pLink = pLink->Flink)
			{
				HookedPage* tempHookedPage = CONTAINING_RECORD(pLink, HookedPage, listEntry);
				ExFreePoolWithTag((PVOID)tempHookedPage->shadowPageAddress, 'fake');
				status = setPageAccess((ULONG_PTR)tempHookedPage->hookedPageAddress, EptAccess::All);
				ASSERT(status == STATUS_SUCCESS);
				RemoveHeadList(&eptState.hookedPage.listEntry);
				ExFreePoolWithTag(tempHookedPage, 'hkpg');
			}

			for (auto i = 0; i < EPT_PML4T_COUNT; i++)
			{
				PteEntry* pml4te = (PteEntry*)eptState.pml4t[i].all;
				for (auto j = 0; j < EPT_PDPT_COUNT; j++)
				{
					PteEntry* pdpte = (PteEntry*)pml4te[j].all;
					for (auto k = 0; k < EPT_PDT_COUNT; k++)
					{
						PteEntry* pdte = (PteEntry*)pdpte[k].all;
						for (auto l = 0; l < EPT_PT_COUNT; l++)
						{
							PteEntry* pte = (PteEntry*)pdte[l].all;
							ExFreePoolWithTag((PVOID)(pte->all & 0xFFFFFFFFFFFFF000ull), 'pt');
						}
						ExFreePoolWithTag((PVOID)(pdte->all & 0xFFFFFFFFFFFFF000ull), 'pdt');
					}
					ExFreePoolWithTag((PVOID)(pdpte->all & 0xFFFFFFFFFFFFF000ull), 'pdpt');
				}
				ExFreePoolWithTag((PVOID)(pml4te->all & 0xFFFFFFFFFFFFF000ull), 'pml4');
			}
			RtlZeroMemory(&eptState, sizeof(EptState));
			return status;
		}

		PteEntry* getPtEntry(ULONG_PTR pa)
		{
			/*
			  ��ҳģʽ:9 9 9 9 12
			  pml4tָ��һ�Ŵ�СΪ 512*8(2^8*8) �ı�,����ÿһ��(pml4te)��ָ��һ��pdpt��
			  pdptָ��һ�Ŵ�СΪ 512*8 �ı�,����ÿһ��(pdpte)��ָ��һ��pdt��
			  pdtָ��һ�Ŵ�СΪ 512*8 �ı�,����ÿһ��(pdte)��ָ��һ��pt��
			  ptָ��һ�Ŵ�СΪ 512*8 �ı�,����ÿһ��(pte)��ָ��һ�� 4KB ������ҳ
			*/
			// pa����Ϊ48bit
			// pml4teIndex = pa[39:47]   ÿһ��8�ֽ�
			// pdpteIndex = pa[30:38]   ÿһ��8�ֽ�
			// pdteIndex = pa[21:29]   ÿһ��8�ֽ�
			// pteIndex = pa[12:20]    ÿһ��8�ֽ�
			// 4kb page index= pa[0:11] & 0xfffff8  ÿһ��1�ֽ�

			ULONG pml4teIndex1 = (pa & 0xff800000000ull) >> (9 + 9 + 9 + 12);
			ULONG pdpteIndex1 = (pa & 0x007fc0000000ull) >> (9 + 9 + 12);
			ULONG pdteIndex1 = (pa & 0x00003fe00000ull) >> (9 + 12);
			ULONG pteIndex1 = (pa & 0x0000001ff000ull) >> (12);


			ULONG pml4teIndex2 = (pa >> (12 + 9 + 9 + 9)) & 0x1ffull;
			ULONG pdpteIndex2 = (pa >> (12 + 9 + 9)) & 0x1ffull;
			ULONG pdteIndex2 = (pa >> (12 + 9)) & 0x1ffull;
			ULONG pteIndex2 = (pa >> (12)) & 0x1ffull;

			PteEntry* pml4te = 0;
			PteEntry* pdpte = 0;
			PteEntry* pdte = 0;
			PteEntry* pte = 0;

			pml4te = &eptState.pml4t[pml4teIndex1];
			pdpte = &((PteEntry*)Util::paToVa(GetPageHead(pml4te->all)))[pdpteIndex1];
			pdte = &((PteEntry*)Util::paToVa(GetPageHead(pdpte->all)))[pdteIndex1];
			pte = &((PteEntry*)Util::paToVa(GetPageHead(pdte->all)))[pteIndex1];
			return pte;
		}

		NTSTATUS hidePage(PVOID targetAddress)
		{
			DbgBreakPoint();
			_disable();
			NTSTATUS status = STATUS_SUCCESS;
			for (LIST_ENTRY* pLink = eptState.hookedPage.listEntry.Flink; pLink != &eptState.hookedPage.listEntry; pLink = pLink->Flink)
			{
				HookedPage* tempHookedPage = CONTAINING_RECORD(pLink, HookedPage, listEntry);
				if (tempHookedPage->hookedPageAddress == (ULONG_PTR)PAGE_ALIGN(targetAddress))
				{
					// �Ѿ�Hook�����ҳ�棬����ֱ�ӷ��سɹ�
					return STATUS_SUCCESS;
				}
			}

			// �����µ�HookedPage��
			HookedPage* newHookedPage = (HookedPage*)ExAllocatePoolWithTag(NonPagedPool, sizeof(HookedPage), 'hkpg');
			if (!newHookedPage)
			{
				return STATUS_MEMORY_NOT_ALLOCATED;
			}
			RtlZeroMemory(newHookedPage, sizeof(HookedPage));

			// ��¼Ŀ��ҳ�׵�ַ, Ŀ��ҳ���ڵ�pte, �Լ������滻�ļ�ҳ��
			newHookedPage->hookedPageAddress = (ULONG_PTR)PAGE_ALIGN(targetAddress);
			newHookedPage->pte = ept::getPtEntry(eptState.pml4t, MmGetPhysicalAddress((PVOID)PAGE_ALIGN(targetAddress)).QuadPart);
			newHookedPage->shadowPageAddress = (ULONG_PTR)ExAllocatePoolWithTag(NonPagedPool, PAGE_SIZE, 'fake');
			if (!newHookedPage->shadowPageAddress)
			{
				return STATUS_MEMORY_NOT_ALLOCATED;
			}
			RtlMoveMemory((PVOID)newHookedPage->shadowPageAddress, (PVOID)newHookedPage->hookedPageAddress, PAGE_SIZE);

			// �ֱ����ù�guest��,д,ִ�е�ҳ��
			newHookedPage->readPage = newHookedPage->shadowPageAddress;
			newHookedPage->writePage = newHookedPage->shadowPageAddress;
			newHookedPage->executePage = newHookedPage->hookedPageAddress;

			// ����guest��д�ļ�ҳ���Ӧ��pte��дȨ��
			status = setPageAccess(newHookedPage->shadowPageAddress, (EptAccess)(EptAccess::Read | EptAccess::Write));
			NT_CHECK();

			// ���蹩guestִ�д������ʵҳ��ִֻ�е�Ȩ��
			status = setPageAccess(newHookedPage->hookedPageAddress, EptAccess::Execute);
			NT_CHECK();

			// ��eptѰַ����
			ept::invalidGlobalEptCache();

			// ��Ŀ��PageEntry��ӵ�page list
			InsertHeadList(&eptState.hookedPage.listEntry, &newHookedPage->listEntry);
			_enable();
			return status;
		}

		NTSTATUS recoverPage(PVOID targetAddress)
		{
			NTSTATUS status = STATUS_SUCCESS;
			for (LIST_ENTRY* pLink = eptState.hookedPage.listEntry.Flink; pLink != &eptState.hookedPage.listEntry; pLink = pLink->Flink)
			{
				HookedPage* tempHookedPage = CONTAINING_RECORD(pLink, HookedPage, listEntry);
				if (tempHookedPage->hookedPageAddress == (ULONG_PTR)PAGE_ALIGN(targetAddress))
				{
					ExFreePoolWithTag((PVOID)tempHookedPage->shadowPageAddress, 'fake');
					status = setPageAccess((ULONG_PTR)targetAddress, EptAccess::All);
					ASSERT(status == STATUS_SUCCESS);
					RemoveHeadList(&eptState.hookedPage.listEntry);
					ExFreePoolWithTag(tempHookedPage, 'hkpg');
				}
			}
			return status;
		}



		NTSTATUS setPageAccess(ULONG_PTR pageAddress, EptAccess access)
		{
			//��ȡĿ���ַ��Ӧ��pte	
			PteEntry* pte = ept::getPtEntry(eptState.pml4t, MmGetPhysicalAddress((PVOID)PAGE_ALIGN(pageAddress)).QuadPart);
			//����Ȩ��
			pte->fields.readAccess = (access & EptAccess::Read) >> 0;
			pte->fields.writeAccess = (access & EptAccess::Write) >> 1;
			pte->fields.executeAccess = (access & EptAccess::Execute) >> 2;
			pte->fields.memoryType = MemoryType::WriteBack;
			return STATUS_SUCCESS;
		}

		void invalidGlobalEptCache()
		{
			unsigned __int64 descriptors[2];
			memset(descriptors, 0, sizeof(descriptors));
			__invept(static_cast<InveptType>(InveptType::GlobalContext), descriptors);
		}



	}
}
