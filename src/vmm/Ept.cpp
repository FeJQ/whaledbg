#include "Ept.h"
#include "Util.hpp"
#include "PageAccessManager.h"
#include "asm.h"


namespace vmm
{
	namespace ept
	{
		EptControl eptCtrl = { 0 };
		LIST_ENTRY pageListHead;


		NTSTATUS enable()
		{
			NTSTATUS status = STATUS_SUCCESS;
			EptPointer eptp = { 0 };
			VmxCpuBasedControls primary = { 0 };
			VmxSecondaryCpuBasedControls secondary = { 0 };

			//pageEntry = (PageEntry*)ExAllocatePool(NonPagedPool, sizeof(PageEntry));
			//RtlZeroMemory(&pageEntry, sizeof(PageEntry));
			InitializeListHead(&pageListHead);

			eptCtrl.pml4t = allocEptMemory();

			// Set up the EPTP
			eptp.fields.physAddr = MmGetPhysicalAddress(eptCtrl.pml4t).QuadPart >> 12;
			eptp.fields.memoryType = MemoryType::WriteBack;
			eptp.fields.pageWalkLength = 3;
			__vmx_vmwrite(EPT_POINTER, eptp.all);

			__vmx_vmread(SECONDARY_VM_EXEC_CONTROL, (size_t*)&secondary.all);
			__vmx_vmread(CPU_BASED_VM_EXEC_CONTROL, (size_t*)&primary.all);
			primary.fields.activateSecondaryControl = TRUE;
			secondary.fields.enableEPT = TRUE;
			__vmx_vmwrite(SECONDARY_VM_EXEC_CONTROL, secondary.all);
			__vmx_vmwrite(CPU_BASED_VM_EXEC_CONTROL, primary.all);

			//PhRootine();

			pam::PASHidePage();
			
			return status;
		}

		PteEntry* allocEptMemory()
		{
			//EPT寻址结构
			//表名      容量        大小(位)
			//PML4T		256T		9
			//PDPT		512G		9
			//PDT		1G			9
			//PT		2M			9
			//PAGE		4K			12

			PteEntry* pml4t = 0;
			PteEntry* pdpt = 0;
			PteEntry* pdt = 0;
			PteEntry* pt = 0;

			const ULONG pm4tCount = 1;
			const ULONG pdptCount = 1;
			const ULONG pdtCount = 50;
			const ULONG ptCount = 512;
			const ULONG pageCount = 512;

			//ULONG pteCount = pm4tCount * pdptCount * pdtCount * ptCount * pageCount;


			//1张PML4T
			pml4t = (PteEntry*)(ExAllocatePoolWithTag(NonPagedPoolNx, PAGE_SIZE, 'pml4'));

			if (!pml4t)
			{
				return 0;
			}
			RtlZeroMemory(pml4t, PAGE_SIZE);

			//1张PDPT
			pdpt = (PteEntry*)(ExAllocatePoolWithTag(NonPagedPoolNx, PAGE_SIZE, 'pdpt'));
			if (!pdpt)
			{
				return 0;
			}
			RtlZeroMemory(pdpt, PAGE_SIZE);

			//PML4T存储PDPT的首地址(物理地址)
			pml4t[0].all = *(ULONG64*)&MmGetPhysicalAddress(pdpt);
			pml4t[0].fields.readAccess = true;
			pml4t[0].fields.writeAccess = true;
			pml4t[0].fields.executeAccess = true;
			// n张PDT (每一张可以支持1G内存寻址,根据内存容量来,最大512) 
			// 实际上由于分页内存可以交换到磁盘的缘故,这里申请的张数一定要比实际物理内存的容量要大
			for (ULONG i = 0; i < pdtCount; i++)
			{
				pdt = (PteEntry*)(ExAllocatePoolWithTag(NonPagedPoolNx, PAGE_SIZE, 'pdt'));
				if (!pdt)
				{
					return 0;
				}
				RtlZeroMemory(pdt, PAGE_SIZE);
				pdpt[i].all = *(ULONG64*)&MmGetPhysicalAddress(pdt);
				pdpt[i].fields.readAccess = true;
				pdpt[i].fields.writeAccess = true;
				pdpt[i].fields.executeAccess = true;
				for (ULONG j = 0; j < ptCount; j++)
				{
					pt = (PteEntry*)(ExAllocatePoolWithTag(NonPagedPoolNx, PAGE_SIZE, 'pt'));
					if (!pt)
					{
						return 0;
					}
					RtlZeroMemory(pt, PAGE_SIZE);
					pdt[j].all = *(ULONG64*)&MmGetPhysicalAddress(pt);
					pdt[j].fields.readAccess = true;
					pdt[j].fields.writeAccess = true;
					pdt[j].fields.executeAccess = true;
					for (ULONG k = 0; k < pageCount; k++)
					{
						pt[k].all = (i * (ULONG64)(1 << 30) + j * (ULONG64)(1 << 21) + k * (ULONG64)(1 << 12));
						pt[k].fields.readAccess = true;
						pt[k].fields.writeAccess = true;
						pt[k].fields.executeAccess = true;
						pt[k].fields.memoryType = MemoryType::WriteBack;
					}
				}
			}
			return pml4t;
		}

		PteEntry* getPtEntry(PteEntry* pml4t, ULONG_PTR pa)
		{
			/*
			  分页模式:9 9 9 9 12
			  pml4t指向一张大小为 512*8(2^8*8) 的表,表中每一项(pml4te)都指向一张pdpt表
			  pdpt指向一张大小为 512*8 的表,表中每一项(pdpte)都指向一张pdt表
			  pdt指向一张大小为 512*8 的表,表中每一项(pdte)都指向一张pt表
			  pt指向一张大小为 512*8 的表,表中每一项(pte)都指向一块 4KB 的物理页
			*/
			// pa长度为48bit
			// pml4teIndex = pa[39:47]   每一项8字节
			// pdpteIndex = pa[30:38]   每一项8字节
			// pdteIndex = pa[21:29]   每一项8字节
			// pteIndex = pa[12:20]    每一项8字节
			// 4kb page index= pa[0:11] & 0xfffff8  每一项1字节

			ULONG pml4teIndex1 = (pa & 0xff800000000) >> (9 + 9 + 9 + 12);
			ULONG pdpteIndex1 = (pa & 0x007fc0000000) >> (9 + 9 + 12);
			ULONG pdteIndex1 = (pa & 0x00003fe00000) >> (9 + 12);
			ULONG pteIndex1 = (pa & 0x0000001ff000) >> (12);


			ULONG pml4teIndex2 = (pa >> (12 + 9 + 9 + 9)) & 0x1ffull;
			ULONG pdpteIndex2 = (pa >> (12 + 9 + 9)) & 0x1ffull;
			ULONG pdteIndex2 = (pa >> (12 + 9)) & 0x1ffull;
			ULONG pteIndex2 = (pa >> (12)) & 0x1ffull;

			PteEntry* pml4te = 0;
			PteEntry* pdpte = 0;
			PteEntry* pdte = 0;
			PteEntry* pte = 0;

			pml4te = &pml4t[pml4teIndex1];
			pdpte = &((PteEntry*)Util::paToVa(GetPageHead(pml4te->all)))[pdpteIndex1];
			pdte = &((PteEntry*)Util::paToVa(GetPageHead(pdpte->all)))[pdteIndex1];
			pte = &((PteEntry*)Util::paToVa(GetPageHead(pdte->all)))[pteIndex1];
			return pte;
		}

		NTSTATUS hidePage(PVOID targetAddress)
		{
			NTSTATUS status = STATUS_SUCCESS;
			// 判断目标页是否已经被权限分离过
			bool flag = false;
			for (LIST_ENTRY* pLink = ept::pageListHead.Flink; pLink != &pageListHead; pLink = pLink->Flink)
			{
				PageEntry* tempPageEntry = CONTAINING_RECORD(pLink, PageEntry, pageList);
				if (tempPageEntry->pageAddress == (ULONG_PTR)PAGE_ALIGN(targetAddress))
				{
					flag = true;
					break;
				}
			}
			if (flag != true) return status;

			// 创建新的PageEntry项
			PageEntry* newPageEntry = (PageEntry*)ExAllocatePoolWithTag(NonPagedPool, sizeof(PageEntry), 'page');
			if (!newPageEntry)
			{
				return STATUS_MEMORY_NOT_ALLOCATED;
			}
			//RtlZeroMemory(newPageEntry, sizeof(PageEntry));

			// 记录目标地址, 目标页首地址, 目标页所在的pte, 以及用于替换的假页面
			newPageEntry->targetAddress = (ULONG_PTR)targetAddress;
			newPageEntry->pageAddress = (ULONG_PTR)PAGE_ALIGN(targetAddress);
			newPageEntry->pte = ept::getPtEntry(eptCtrl.pml4t, MmGetPhysicalAddress((PVOID)PAGE_ALIGN(targetAddress)).QuadPart);
			newPageEntry->shadowPageAddress = (ULONG_PTR)ExAllocatePoolWithTag(NonPagedPool, PAGE_SIZE, 'fake');
			if (!newPageEntry->shadowPageAddress)
			{
				return STATUS_MEMORY_NOT_ALLOCATED;
			}
			RtlMoveMemory((PVOID)newPageEntry->shadowPageAddress, (PVOID)newPageEntry->pageAddress, PAGE_SIZE);

			// 分别配置供guest读,写,执行的页面
			newPageEntry->readPage = newPageEntry->shadowPageAddress;
			newPageEntry->writePage = newPageEntry->shadowPageAddress;
			newPageEntry->executePage = newPageEntry->pageAddress;

			// 给供guest读写的假页面对应的pte读写权限
			status = setPageAccess(newPageEntry->shadowPageAddress, (EptAccess)(EptAccess::Read | EptAccess::Write));
			NT_CHECK();

			// 给予供guest执行代码的真实页面只执行的权限
			status = setPageAccess(newPageEntry->pageAddress, EptAccess::Execute);
			NT_CHECK();

			// 将目标PageEntry添加到page list
			InsertHeadList(&ept::pageListHead, &newPageEntry->pageList);
			return status;
		}

		NTSTATUS recoverPage(PVOID targetAddress)
		{
			NTSTATUS status = STATUS_SUCCESS;
			for (LIST_ENTRY* pLink = ept::pageListHead.Flink; pLink != &ept::pageListHead; pLink = pLink->Flink)
			{
				PageEntry* tempPageEntry = CONTAINING_RECORD(pLink, PageEntry, pageList);
				if (tempPageEntry->pageAddress == (ULONG_PTR)PAGE_ALIGN(targetAddress))
				{

					ExFreePoolWithTag((PVOID)tempPageEntry->shadowPageAddress, 'fake');
					status = setPageAccess((ULONG_PTR)targetAddress, EptAccess::All);
					NT_CHECK();

					RemoveHeadList(&ept::pageListHead);
				}
			}
		}


		NTSTATUS separatePageAccess(PageEntry* targetPageEntry, EptAccess eptAccess)
		{
			NTSTATUS status = STATUS_SUCCESS;



			/*PVOID NtLoadDriver = (PVOID)0xFFFFF800043594F0;
			Util::disableMemoryProtect();
			*(PCHAR)NtLoadDriver = 0xCC;
			Util::enableMemoryProtect();*/

			setPageAccess(targetPageEntry->pageAddress, eptAccess);
			return status;
		}


		NTSTATUS setPageAccess(ULONG_PTR pageAddress, EptAccess access)
		{
			//获取目标地址对应的pte	
			PteEntry* pte = ept::getPtEntry(eptCtrl.pml4t, MmGetPhysicalAddress((PVOID)PAGE_ALIGN(pageAddress)).QuadPart);
			//配置权限
			pte->fields.readAccess = (access & EptAccess::Read) >> 0;
			pte->fields.writeAccess = (access & EptAccess::Write) >> 1;
			pte->fields.executeAccess = (access & EptAccess::Execute) >> 2;
			pte->fields.memoryType = MemoryType::WriteBack;

			NTSTATUS checkStatus = pte->fields.readAccess & pte->fields.writeAccess & pte->fields.executeAccess;
			if (checkStatus == access && pte->fields.memoryType == MemoryType::WriteBack)
			{
				return STATUS_SUCCESS;
			}
			return STATUS_FAIL_CHECK;
		}

		void invalidGlobalEptCache()
		{
			unsigned __int64 descriptors[2];
			memset(descriptors, 0, sizeof(descriptors));
			__invept(static_cast<InveptType>(InveptType::GlobalContext), descriptors);
		}
	}
}
