#pragma once
#include "Common.h"
#include "IA32.h"
#include "global.h"
#include "PageAccessManager.h"


namespace vmm
{
	namespace ept
	{
		/**
		 * ����Ept
		 */
		NTSTATUS enable();

		/**
		 * ����ҳ���ڴ�
		 * @return EptEntry*: 1��ҳ��(PML4T)���׵�ַ
		 */
		PteEntry* allocEptMemory();

		/**
		 * ��ȡ�����ַ����Ӧ��PTE
		 * @param EptEntry * pml4t: pml4t�׵�ַ
		 * @param ULONG_PTR pa: Ҫ��ѯ�������ַ
		 * @return EptEntry*: PTE
		 */
		PteEntry* getPtEntry(PteEntry* pml4t, ULONG_PTR pa);

		/**
		 * ����ҳ��
		 * @param PVOID targetAddress:
		 * @return NTSTATUS:
		 */
		NTSTATUS hidePage(PVOID targetAddress);

		/**
		 * �ָ�ҳ��
		 * @param PVOID targetAddress:
		 * @return NTSTATUS:
		 */
		NTSTATUS recoverPage(PVOID targetAddress);

		/**
		 * ҳ��Ȩ�޷���
		 * @param PVOID targetPageEntry: Ŀ��ҳ����
		 * @return NTSTATUS:
		 */
		NTSTATUS separatePageAccess(PageEntry* targetPageEntry, EptAccess eptAccess);

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


		extern EptControl eptCtrl;
		extern LIST_ENTRY pageListHead;
	}
}

