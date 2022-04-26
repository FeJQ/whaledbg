#pragma once
#include <ntddk.h>
#include "IA32.h"




namespace vmm
{
	namespace vmcs
	{

		/**
		 * ���ÿͻ���״̬��
		 * @param ULONG_PTR guestRsp:
		 * @param ULONG_PTR guestRip:
		 * @return NTSTATUS:
		 */
		NTSTATUS setupGuestState(ULONG_PTR guestRsp, ULONG_PTR guestRip);

		/**
		 * ����������״̬��
		 * @param ULONG_PTR hostRsp:
		 * @param ULONG_PTR hostRip:
		 * @return NTSTATUS:
		 */
		NTSTATUS setupHostState(ULONG_PTR hostRsp, ULONG_PTR hostRip);

		/**
		 * ���������ִ�п�����
		 * @param bool isUseTrueMsrs:
		 * @return NTSTATUS:
		 */
		NTSTATUS setupVmExecCtrlFields(bool isUseTrueMsrs);

		/**
		 * ����vmexit������
		 * @param bool isUseTrueMsrs:
		 * @return NTSTATUS:
		 */
		NTSTATUS setupVmExitCtrlFields(bool isUseTrueMsrs);

		/**
		 * ����vmentry������
		 * @param bool isUseTrueMsrs:
		 * @return NTSTATUS:
		 */
		NTSTATUS setupVmEntryCtrlFields(bool isUseTrueMsrs);

		/**
		 * ����vmexit��Ϣ��
		 * @return NTSTATUS:
		 */
		NTSTATUS setupVmExitInfoFields();

		/**
		 * ����EPT��Ϣ��
		 * @return NTSTATUS:
		 */
		NTSTATUS setupEptFields();

		/**
		 * ��ȡ������������Ȩ��
		 * @param USHORT selector: ��ѡ����
		 * @return ULONG:
		 */
		ULONG getSegmentAccessRight(USHORT selector);


		/**
		 * ����Msr�Ĵ���ֵ
		 * See:24.6.1 Pin-Base VM-Execution Controls
		 * vmwrite�����������ʱ,��Щλ������Ϊ0,��Щλ������Ϊ1
		 * ͨ����ȡ��Ӧ��MSR�Ĵ�����ȷ����Щλ������0,��Щλ������1
		 * @param ULONG msr:
		 * @param ULONG ctl:
		 * @return ULONG:
		 */
		ULONG updateControlValue(ULONG msr, ULONG ctl);

		/**
		 * ���ض�������
		 * @param OUT SegmentSelector * segmentSelector:
		 * @param USHORT selector:
		 * @param ULONG_PTR gdtBase:
		 * @return NTSTATUS:
		 */
		NTSTATUS loadSegmentDescriptor(SegmentSelector* segmentSelector, USHORT selector, ULONG_PTR gdtBase);
	}
}
