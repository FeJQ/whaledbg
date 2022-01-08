#pragma once
#include "Common.hpp"
#include "IA32.h"

enum EptAccess
{
	All = 0b111,
	Read = 0b001,
	Write = 0b010,
	Execute = 0b100
};

union Pml4Entry
{
	ULONG64 all;
	struct
	{
		ULONG64 readAccess : 1;       //!< [0]
		ULONG64 writeAccess : 1;      //!< [1]
		ULONG64 executeAccess : 1;    //!< [2]
		ULONG64 reserved1 : 5;       //!< [3:7]
		ULONG64 accessFlag : 1;         //!< [8]
		ULONG64 ignored : 3;  //!< [9:11]
		ULONG64 physAddr : 40;  //   [12:51]
		ULONG64 reserved2 : 12;       //!< [52:63]
	}fields;
};

union EptEntry
{
	ULONG64 all;
	struct
	{
		ULONG64 readAccess : 1;       //!< [0]
		ULONG64 writeAccess : 1;      //!< [1]
		ULONG64 executeAccess : 1;    //!< [2]
		ULONG64 memoryType : 3;       //!< [3:5]
		ULONG64 reserved1 : 6;         //!< [6:11]
		ULONG64 physialAddress : 36;  //!< [12:48-1]
		ULONG64 reserved2 : 16;        //!< [48:63]
	} fields;
};
union EptPointer
{
	ULONG64 all;
	struct
	{
		ULONG64 memoryType : 3;         // EPT Paging structure memory type (0 for UC)
		ULONG64 pageWalkLength : 3;     // Page-walk length
		ULONG64 reserved1 : 6;
		ULONG64 physAddr : 40;          // Physical address of the EPT PML4 table
		ULONG64 reserved2 : 12;
	} fields;
};
struct EptControl
{
	EptEntry* pml4t;
};

class PageEntry
{
public:
	LIST_ENTRY pageList;
	//目标指令所在的地址
	ULONG_PTR targetAddress;
	//目标页首地址
	ULONG_PTR targetPageAddress;
	//假页首地址
	ULONG_PTR shadowPageAddress;

	//目标页所对应的pte
	EptEntry* pte;

	//ULONG_PTR readPage;
	//ULONG_PTR writePage;
	//ULONG_PTR excutePage;
};

class Ept
{
public:
	Ept()
	{
		this->eptCtrl = { 0 };
		this->pageEntry = { 0 };
		InitializeListHead(&pageEntry.pageList);
	}
	/**
	 * 开启Ept
	 *
	 * @return void:
	 */
	NTSTATUS enable()
	{
		DbgBreakPoint();
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
		return status;
	}


	/**
	 * 分配页表内存
	 *
	 * @return EptEntry*: 1级页表(PML4T)的首地址
	 */
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



	/**
	 * 获取物理地址所对应的PTE
	 *
	 * @param EptEntry * pml4t: pml4t首地址
	 * @param ULONG_PTR pa: 要查询的物理地址
	 * @return EptEntry*: PTE
	 */
	static EptEntry* getPtEntry(EptEntry* pml4t, ULONG_PTR pa)
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

	/**
	 * 隐藏页面
	 *
	 * @param PVOID targetAddress:
	 * @return NTSTATUS:
	 */
	NTSTATUS hidePage(PVOID targetAddress)
	{
		NTSTATUS status = STATUS_SUCCESS;
		// 判断目标页是否已经被权限分离过
		bool flag = false;
		for (LIST_ENTRY* pLink = this->pageEntry.pageList.Flink; pLink != (PLIST_ENTRY)&this->pageEntry.pageList.Flink; pLink = pLink->Flink)
		{
			PageEntry* tempPageEntry = CONTAINING_RECORD(pLink, PageEntry, pageList);
			if (tempPageEntry->targetPageAddress == (ULONG_PTR)PAGE_ALIGN(targetAddress))
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
		newPageEntry->targetPageAddress = (ULONG_PTR)PAGE_ALIGN(targetAddress);	
		newPageEntry->pte = Ept::getPtEntry(eptCtrl.pml4t, MmGetPhysicalAddress((PVOID)PAGE_ALIGN(targetAddress)).QuadPart);
		newPageEntry->shadowPageAddress = (ULONG_PTR)ExAllocatePoolWithTag(NonPagedPool, PAGE_SIZE, 'fake');
		if (!newPageEntry->shadowPageAddress)
		{
			return STATUS_MEMORY_NOT_ALLOCATED;
		}
		RtlMoveMemory((PVOID)newPageEntry->shadowPageAddress, (PVOID)newPageEntry->targetPageAddress, PAGE_SIZE);	

		// 给假页面对应的pte读写权限
		status = setPageAccess(newPageEntry->shadowPageAddress, (EptAccess)(EptAccess::Read | EptAccess::Write));
		NT_CHECK();

		// 给予真实页面只执行的权限
		status = setPageAccess(newPageEntry->targetPageAddress, EptAccess::Execute);
		NT_CHECK();

		// 将目标PageEntry添加到page list
		InsertHeadList(&this->pageEntry.pageList, &newPageEntry->pageList);
		return status;
	}

	/**
	 * 恢复页面
	 *      
	 * @param PVOID targetAddress: 
	 * @return NTSTATUS: 
	 */
	 NTSTATUS recoverPage(PVOID targetAddress)
	{
		NTSTATUS status = STATUS_SUCCESS;
		for (LIST_ENTRY* pLink = this->pageEntry.pageList.Flink; pLink != (PLIST_ENTRY)&this->pageEntry.pageList.Flink; pLink = pLink->Flink)
		{
			PageEntry* tempPageEntry = CONTAINING_RECORD(pLink, PageEntry, pageList);
			if (tempPageEntry->targetPageAddress == (ULONG_PTR)PAGE_ALIGN(targetAddress))
			{
				
				ExFreePoolWithTag((PVOID)tempPageEntry->shadowPageAddress, 'fake');
				status = setPageAccess((ULONG_PTR)targetAddress, EptAccess::All);
				NT_CHECK();

				RemoveHeadList();
			}
		}
	}

private:
	/**
	 * 页表权限分离
	 *
	 * @param PVOID targetPageEntry: 目标页表项
	 * @return NTSTATUS:
	 */
	NTSTATUS separatePageAccess(PageEntry* targetPageEntry, EptAccess eptAccess)
	{
		NTSTATUS status = STATUS_SUCCESS;



		/*PVOID NtLoadDriver = (PVOID)0xFFFFF800043594F0;
		Util::disableMemoryProtect();
		*(PCHAR)NtLoadDriver = 0xCC;
		Util::enableMemoryProtect();*/

		setPageAccess(targetPageEntry->targetPageAddress, eptAccess);
		return status;
	}

	/**
	 * 设置页访问权限
	 *
	 * @param ULONG_PTR pageAddress:
	 * @param EptAccess access:
	 * @return NTSTATUS:
	 */
	NTSTATUS setPageAccess(ULONG_PTR pageAddress, EptAccess access)
	{
		//获取目标地址对应的pte	
		EptEntry* pte = Ept::getPtEntry(eptCtrl.pml4t, MmGetPhysicalAddress((PVOID)PAGE_ALIGN(pageAddress)).QuadPart);
		//配置权限
		pte->fields.readAccess = (access & EptAccess::Read) >> 0;
		pte->fields.writeAccess = (access & EptAccess::Write) >> 1;
		pte->fields.executeAccess = (access & EptAccess::Execute) >> 2;
		pte->fields.memoryType = MemoryType::WriteBack;
	}

public:

private:
	EptControl eptCtrl;
	PageEntry pageEntry;
};