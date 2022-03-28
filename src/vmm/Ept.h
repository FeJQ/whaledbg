#pragma once
#include "Common.h"
#include "IA32.h"
#include "global.h"
#include "PageAccessManager.h"


#define EPT_PML4T_COUNT 1 
#define EPT_PDPT_COUNT 1
#define EPT_PDT_COUNT 50
#define EPT_PT_COUNT 512
#define EPT_PAGE_COUNT 512

namespace vmm
{
	namespace ept
	{
		union PteEntry
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


		struct HookedPage
		{
			LIST_ENTRY listEntry;             // 指向下一个PageEntry
			ULONG_PTR hookedPageAddress;  // 目标页首地址
			ULONG_PTR shadowPageAddress; // 假页页首地址
			PteEntry* pte;                   // 目标页所对应的pte

			ULONG_PTR readPage;
			ULONG_PTR writePage;
			ULONG_PTR executePage;
		};
		struct EptState
		{
			EptPointer eptp;
			PteEntry* pml4t;
			HookedPage hookedPage;
		};
		/**
		 * 开启Ept
		 */
		NTSTATUS enable();


		/**
		 * 分配页表内存
		 * @return EptEntry*: 1级页表(PML4T)的首地址
		 */
		PVOID allocEptMemory();

		/**
		 * 释放所有在EPT模块中申请的内存
		 * tips:务必要在执行完vmmoff后调用
		 * @return NTSTATUS: 
		 */
		 NTSTATUS freeEptMemory();

		/**
		 * 获取物理地址所对应的PTE
		 * @param ULONG_PTR pa: 要查询的物理地址
		 * @return EptEntry*: PTE
		 */
		PteEntry* getPtEntry(ULONG_PTR pa);

		/**
		 * 隐藏页面，并将原始页的访问权限设置为只执行
		 * @param PVOID targetAddress:
		 * @return NTSTATUS:
		 */
		NTSTATUS hidePage(PVOID targetAddress);

		/**
		 * 恢复隐藏的页，并将原本的页访问权限设置为ALL
		 * @param PVOID targetAddress:
		 * @return NTSTATUS:
		 */
		NTSTATUS recoverPage(PVOID targetAddress);

		/**
		 * 设置页访问权限
		 * @param ULONG_PTR pageAddress:
		 * @param EptAccess access:
		 * @return NTSTATUS:
		 */
		NTSTATUS setPageAccess(ULONG_PTR pageAddress, EptAccess access);

		/**
		 * 使逻辑处理器与所有 EPTP 关联的映射失效
		 * see:https://www.felixcloutier.com/x86/invept
		 */
		void invalidGlobalEptCache();


		extern EptState eptState;
	}
}

