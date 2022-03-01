#include "Ept.h"
#include "Util.hpp"
#include "PageAccessManager.h"

namespace whaledbg
{
	namespace vmm
	{
		namespace ept
		{
			extern EptControl eptCtrl = {0};
			extern PageEntry pageEntry = {0};

			NTSTATUS initialize()
			{
				InitializeListHead(&pageEntry.pageList);
				return STATUS_SUCCESS;
			}

			NTSTATUS enable()
			{
				NTSTATUS status = STATUS_SUCCESS;
				EptPointer eptp = { 0 };
				VmxCpuBasedControls primary = { 0 };
				VmxSecondaryCpuBasedControls secondary = { 0 };

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

				//pam::PASHidePage();
				return status;
			}

			EptEntry* allocEptMemory()
			{
				//EPT寻址结构
				//表名      容量        大小(位)
				//PML4T		256T		9
				//PDPT		512G		9
				//PDT		1G			9
				//PT		2M			9
				//PAGE		4K			12

				EptEntry* pml4t = 0;
				EptEntry* pdpt = 0;
				EptEntry* pdt = 0;
				EptEntry* pt = 0;

				const ULONG pm4tCount = 1;
				const ULONG pdptCount = 1;
				const ULONG pdtCount = 8;
				const ULONG ptCount = 512;
				const ULONG pageCount = 512;

				//ULONG pteCount = pm4tCount * pdptCount * pdtCount * ptCount * pageCount;


				//1张PML4T
				pml4t = (EptEntry*)(ExAllocatePoolWithTag(NonPagedPoolNx, PAGE_SIZE, 'pml4'));

				if (!pml4t)
				{
					return 0;
				}
				RtlZeroMemory(pml4t, PAGE_SIZE);

				//1张PDPT
				pdpt = (EptEntry*)(ExAllocatePoolWithTag(NonPagedPoolNx, PAGE_SIZE, 'pdpt'));
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
				//8张PDT
				for (ULONG i = 0; i < pdtCount; i++)
				{
					pdt = (EptEntry*)(ExAllocatePoolWithTag(NonPagedPoolNx, PAGE_SIZE, 'pdt'));
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
						pt = (EptEntry*)(ExAllocatePoolWithTag(NonPagedPoolNx, PAGE_SIZE, 'pt'));
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

			EptEntry* getPtEntry(EptEntry* pml4t, ULONG_PTR pa)
			{
				ULONG pml4teIndex = (pa & 0xFF8000000000ull) >> (12 + 9 + 9 + 9);
				ULONG pdpteIndex = (pa & 0x007FC0000000ull) >> (12 + 9 + 9);
				ULONG pdteIndex = (pa & 0x00003FE00000ull) >> (12 + 9);
				ULONG pteIndex = (pa & 0x0000001FF000ull) >> (12);

				EptEntry* pml4te = 0;
				EptEntry* pdpte = 0;
				EptEntry* pdte = 0;
				EptEntry* pte = 0;

				pml4te = &pml4t[pml4teIndex];
				pdpte = &((EptEntry*)Util::paToVa(GetPageHead(pml4t->all)))[pdpteIndex];
				pdte = &((EptEntry*)Util::paToVa(GetPageHead(pdpte->all)))[pdteIndex];
				pte = &((EptEntry*)Util::paToVa(GetPageHead(pdte->all)))[pteIndex];
				return pte;
			}

			NTSTATUS hidePage(PVOID targetAddress)
			{
				NTSTATUS status = STATUS_SUCCESS;
				// 判断目标页是否已经被权限分离过
				bool flag = false;
				for (LIST_ENTRY* pLink = ept::pageEntry.pageList.Flink; pLink != (PLIST_ENTRY)&ept::pageEntry.pageList.Flink; pLink = pLink->Flink)
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
				RtlZeroMemory(newPageEntry, sizeof(PageEntry));

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
				InsertHeadList(&ept::pageEntry.pageList, &newPageEntry->pageList);
				return status;
			}

			NTSTATUS recoverPage(PVOID targetAddress)
			{
				NTSTATUS status = STATUS_SUCCESS;
				for (LIST_ENTRY* pLink = ept::pageEntry.pageList.Flink; pLink != (PLIST_ENTRY)&ept::pageEntry.pageList.Flink; pLink = pLink->Flink)
				{
					PageEntry* tempPageEntry = CONTAINING_RECORD(pLink, PageEntry, pageList);
					if (tempPageEntry->pageAddress == (ULONG_PTR)PAGE_ALIGN(targetAddress))
					{

						ExFreePoolWithTag((PVOID)tempPageEntry->shadowPageAddress, 'fake');
						status = setPageAccess((ULONG_PTR)targetAddress, EptAccess::All);
						NT_CHECK();

						RemoveHeadList(&ept::pageEntry.pageList);
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
				EptEntry* pte = ept::getPtEntry(eptCtrl.pml4t, MmGetPhysicalAddress((PVOID)PAGE_ALIGN(pageAddress)).QuadPart);
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
		}
	}
}