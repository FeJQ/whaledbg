#pragma once
#include "Common.hpp"
#include "IA32.h"

enum EptAccess
{
	EptAccessAll = 0b111,
	EptAccessRead = 0b001,
	EptAccessWrite = 0b010,
	EptAccessExecute = 0b100
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

struct PageEntry
{
	LIST_ENTRY pageList;
	//目标指令所在的地址
	ULONG_PTR targetAddressVa;
	//目标页首地址
	ULONG_PTR pageAddressVa;
	//假页首地址
	ULONG_PTR shadowPageAddressVa;
	//目标页所对应的pte
	EptEntry* pte;

	ULONG_PTR readPage;
	ULONG_PTR writePage;
	ULONG_PTR excutePage;

};

class Ept
{
public:
	/**
	 * 开启Ept
	 *
	 * @return void:
	 */
	NTSTATUS enable()
	{
		NTSTATUS status = STATUS_SUCCESS;
		EptPointer eptp = { 0 };
		VmxCpuBasedControls primary = { 0 };
		VmxSecondaryCpuBasedControls secondary = { 0 };

		eptCtrl.pml4t = EptAllocateTable();

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
	EptEntry* EptAllocateTable()
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

public:
	static PageEntry*& getStaticPageEntry()
	{
		static PageEntry* pageEntry = nullptr;
		return pageEntry;
	}
private:
	EptControl eptCtrl = { 0 };
};