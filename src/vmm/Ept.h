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
			LIST_ENTRY listEntry;             // ָ����һ��PageEntry
			ULONG_PTR hookedPageAddress;  // Ŀ��ҳ�׵�ַ
			ULONG_PTR shadowPageAddress; // ��ҳҳ�׵�ַ
			PteEntry* pte;                   // Ŀ��ҳ����Ӧ��pte

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
		 * ����Ept
		 */
		NTSTATUS enable();


		/**
		 * ����ҳ���ڴ�
		 * @return EptEntry*: 1��ҳ��(PML4T)���׵�ַ
		 */
		PVOID allocEptMemory();

		/**
		 * �ͷ�������EPTģ����������ڴ�
		 * tips:���Ҫ��ִ����vmmoff�����
		 * @return NTSTATUS: 
		 */
		 NTSTATUS freeEptMemory();

		/**
		 * ��ȡ�����ַ����Ӧ��PTE
		 * @param ULONG_PTR pa: Ҫ��ѯ�������ַ
		 * @return EptEntry*: PTE
		 */
		PteEntry* getPtEntry(ULONG_PTR pa);

		/**
		 * ����ҳ�棬����ԭʼҳ�ķ���Ȩ������Ϊִֻ��
		 * @param PVOID targetAddress:
		 * @return NTSTATUS:
		 */
		NTSTATUS hidePage(PVOID targetAddress);

		/**
		 * �ָ����ص�ҳ������ԭ����ҳ����Ȩ������ΪALL
		 * @param PVOID targetAddress:
		 * @return NTSTATUS:
		 */
		NTSTATUS recoverPage(PVOID targetAddress);

		/**
		 * ����ҳ����Ȩ��
		 * @param ULONG_PTR pageAddress:
		 * @param EptAccess access:
		 * @return NTSTATUS:
		 */
		NTSTATUS setPageAccess(ULONG_PTR pageAddress, EptAccess access);

		/**
		 * ʹ�߼������������� EPTP ������ӳ��ʧЧ
		 * see:https://www.felixcloutier.com/x86/invept
		 */
		void invalidGlobalEptCache();


		extern EptState eptState;
	}
}

