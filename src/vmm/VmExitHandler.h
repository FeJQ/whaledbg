#pragma once
#include "Common.h"
#include "iA32.h"

namespace whaledbg
{
	namespace vmm
	{
		namespace vmexit
		{
			/**
			 * vmexit�¼����
			 * @param Vcpu * vcpu:
			 * @return continue vmx?
			 */
			extern "C" BOOLEAN vmExitEntryPoint(Registers64* reg);

			/**
			 * �ָ��ͻ���rip������vmexitָ�����һ��ָ��,
			 */
			void resume();

			/**
			 * ִ��cpuidָ������µ�vmexit�¼��Ĵ�����
			 * @param Registers64 * pGuestRegisters:�ͻ���ͨ�üĴ���
			 */
			void handleCpuid(Registers64* reg);

			/**
			 * ִ��invdָ������µ�vmexit�¼��Ĵ�����
			 * @param Registers64 * reg:�ͻ���ͨ�üĴ���
			 */
			void handleInvd(Registers64* reg);

			/**
			 * ִ��vmcallָ������µ�vmexit�¼��Ĵ�����
			 * @param Registers64 * reg: �ͻ���ͨ�üĴ���
			 * @return �Ƿ����ִ��vmx
			 */
			BOOLEAN handleVmcall(Registers64* reg);

			/**
			 * ����(��/д)���ƼĴ��������µ�vmexit�¼��Ĵ�����
			 * @param Registers64 * reg: �ͻ���ͨ�üĴ���
			 */
			void handleCrAccess(Registers64* reg);

			/**
			 * ��ȡmsr�Ĵ��������µ�vmexit�¼��Ĵ�����
			 * @param Registers64 * reg: �ͻ���ͨ�üĴ���
			 */
			void handleMsrRead(Registers64* reg);

			/**
			 * д��msr�Ĵ��������µ�vmexit�¼��Ĵ�����
			 * @param Registers64 * reg: �ͻ���ͨ�üĴ���
			 */
			void handleMsrWrite(Registers64* reg);

			/**
			 * ����MTF��ִ��ָ����������vmexit������
			 * @param Registers64 * reg: �ͻ���ͨ�üĴ���
			 */
			void handleMtf(Registers64* reg);

			/**
			 * �ڴ����Բ�ƥ�䵼�µ�vmexit������
			 * @param Registers64 * reg: �ͻ���ͨ�üĴ���
			 */
			void handleEptViolation(Registers64* reg);

			/**
			 * �ڴ����Դ����µ�vmexit������
			 * @param Registers64 * pGuestRegisters: �ͻ���ͨ�üĴ���
			 */
			void handleEptMisconfig(Registers64* reg);
			
		}
	}
}