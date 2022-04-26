#pragma once
#include <ntddk.h>
#include "IA32.h"




namespace vmm
{
	namespace vmcs
	{

		/**
		 * 设置客户机状态域
		 * @param ULONG_PTR guestRsp:
		 * @param ULONG_PTR guestRip:
		 * @return NTSTATUS:
		 */
		NTSTATUS setupGuestState(ULONG_PTR guestRsp, ULONG_PTR guestRip);

		/**
		 * 设置宿主机状态域
		 * @param ULONG_PTR hostRsp:
		 * @param ULONG_PTR hostRip:
		 * @return NTSTATUS:
		 */
		NTSTATUS setupHostState(ULONG_PTR hostRsp, ULONG_PTR hostRip);

		/**
		 * 设置虚拟机执行控制欲
		 * @param bool isUseTrueMsrs:
		 * @return NTSTATUS:
		 */
		NTSTATUS setupVmExecCtrlFields(bool isUseTrueMsrs);

		/**
		 * 设置vmexit控制域
		 * @param bool isUseTrueMsrs:
		 * @return NTSTATUS:
		 */
		NTSTATUS setupVmExitCtrlFields(bool isUseTrueMsrs);

		/**
		 * 设置vmentry控制域
		 * @param bool isUseTrueMsrs:
		 * @return NTSTATUS:
		 */
		NTSTATUS setupVmEntryCtrlFields(bool isUseTrueMsrs);

		/**
		 * 设置vmexit信息域
		 * @return NTSTATUS:
		 */
		NTSTATUS setupVmExitInfoFields();

		/**
		 * 设置EPT信息域
		 * @return NTSTATUS:
		 */
		NTSTATUS setupEptFields();

		/**
		 * 获取段描述符访问权限
		 * @param USHORT selector: 段选择子
		 * @return ULONG:
		 */
		ULONG getSegmentAccessRight(USHORT selector);


		/**
		 * 调整Msr寄存器值
		 * See:24.6.1 Pin-Base VM-Execution Controls
		 * vmwrite虚拟机控制域时,有些位必须置为0,有些位必须置为1
		 * 通过读取相应的MSR寄存器来确定哪些位必须置0,哪些位必须置1
		 * @param ULONG msr:
		 * @param ULONG ctl:
		 * @return ULONG:
		 */
		ULONG updateControlValue(ULONG msr, ULONG ctl);

		/**
		 * 加载段描述符
		 * @param OUT SegmentSelector * segmentSelector:
		 * @param USHORT selector:
		 * @param ULONG_PTR gdtBase:
		 * @return NTSTATUS:
		 */
		NTSTATUS loadSegmentDescriptor(SegmentSelector* segmentSelector, USHORT selector, ULONG_PTR gdtBase);
	}
}
