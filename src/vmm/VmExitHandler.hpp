#pragma once
#include "Common.hpp"
#include "iA32.h"


class VmExitHandler
{
public:
	/**
	 * vmexit�¼����
	 * @param Vcpu * vcpu:
	 * @return continue vmx?
	 */
	BOOLEAN vmExitEntryPoint(Registers64* reg)
	{
		VmExitInformation exitReson = { 0 };

		//��ѯ����vmexit��ԭ�� 
		// See:AppendixC VMX Basic Exit Reson ,Table C-1 Basic Exit Reson
		__vmx_vmread(VM_EXIT_REASON, (ULONG_PTR*)&exitReson);

		BOOLEAN continueVmx = TRUE;
		ULONG_PTR rip = 0;
		ULONG_PTR rsp = 0;
		ULONG_PTR guestPhysicalAddress = 0;
		__vmx_vmread(GUEST_RIP, &rip);
		__vmx_vmread(GUEST_RSP, &rsp);
		__vmx_vmread(GUEST_PHYSICAL_ADDRESS, &guestPhysicalAddress);

		switch (exitReson.fields.reason)
		{
		case VmExitReason::Cpuid:
			handleCpuid(reg);
			break;
		case VmExitReason::Invd:
			handleInvd(reg);
			break;
		case VmExitReason::Vmcall:
			continueVmx = handleVmcall(reg);
			if (continueVmx) resume();
			break;
		case VmExitReason::CrAccess:
			handleCrAccess(reg);
			break;
		case VmExitReason::MsrRead:
			handleMsrRead(reg);
			break;
		case VmExitReason::MsrWrite:
			handleMsrWrite(reg);
			break;
		case VmExitReason::Mtf:
			handleMtf(reg);
			break;
		case VmExitReason::EptViolation:
			handleEptViolation(reg);
			break;
		case VmExitReason::EptMisconfig:
			handleEptMisconfig(reg);
			break;
		default:
			DbgBreakPoint();
			break;
		}

		return continueVmx;
	}
protected:
	/**
	 * �ָ��ͻ���rip������vmexitָ�����һ��ָ��,
	 */
	void resume()
	{
		ULONG instLen = 0;
		ULONG_PTR rip = 0;
		__vmx_vmread(GUEST_RIP, &rip);
		__vmx_vmread(VM_EXIT_INSTRUCTION_LEN, (size_t*)&instLen);
		__vmx_vmwrite(GUEST_RIP, rip + instLen);
	}

	/**
	 * ִ��cpuidָ������µ�vmexit�¼��Ĵ�����
	 * @param Registers64 * pGuestRegisters:�ͻ���ͨ�üĴ���
	 */
	virtual void handleCpuid(Registers64* reg) = 0;

	/**
	 * ִ��invdָ������µ�vmexit�¼��Ĵ�����
	 * @param Registers64 * reg:�ͻ���ͨ�üĴ���
	 */
	virtual void handleInvd(Registers64* reg) = 0;

	/**
	 * ִ��vmcallָ������µ�vmexit�¼��Ĵ�����
	 * @param Registers64 * reg: �ͻ���ͨ�üĴ���
	 * @return �Ƿ����ִ��vmx
	 */
	virtual BOOLEAN handleVmcall(Registers64* reg) = 0;

	/**
	 * ����(��/д)���ƼĴ��������µ�vmexit�¼��Ĵ�����
	 * @param Registers64 * reg: �ͻ���ͨ�üĴ���
	 */
	virtual void handleCrAccess(Registers64* reg) = 0;

	/**
	 * ��ȡmsr�Ĵ��������µ�vmexit�¼��Ĵ�����
	 * @param Registers64 * reg: �ͻ���ͨ�üĴ���
	 */
	virtual void handleMsrRead(Registers64* reg) = 0;

	/**
	 * д��msr�Ĵ��������µ�vmexit�¼��Ĵ�����
	 * @param Registers64 * reg: �ͻ���ͨ�üĴ���
	 */
	virtual void handleMsrWrite(Registers64* reg) = 0;

	/**
	 * ����MTF��ִ��ָ����������vmexit������
	 * @param Registers64 * reg: �ͻ���ͨ�üĴ���
	 */
	virtual void handleMtf(Registers64* reg) = 0;

	/**
	 * �ڴ����Բ�ƥ�䵼�µ�vmexit������
	 * @param Registers64 * reg: �ͻ���ͨ�üĴ���
	 */
	virtual void handleEptViolation(Registers64* reg) = 0;

	/**
	 * �ڴ����Դ����µ�vmexit������
	 * @param Registers64 * pGuestRegisters: �ͻ���ͨ�üĴ���
	 */
	virtual void handleEptMisconfig(Registers64* reg) = 0;


protected:

	

};