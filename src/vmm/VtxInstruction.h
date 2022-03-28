#pragma once
#include "IA32.h"

namespace vmm
{
	namespace vtx
	{
		/**
		 * ����MTF
		 * @param bool value: ������ر�
		 */
		void setMonitorTrapFlag(bool value);

		/**
		 * ��ѯ����vmexit��ԭ��
		 * @return VmExitInformation:
		 */
		VmExitInformation readVmexitReason();

		/**
		 * ��ȡVMCS �� GuestRip�ֶ�
		 * @return ULONG_PTR:guest rip
		 */
		ULONG_PTR readGuestRip();

		/**
		 * ����Guest rip
		 * @param ULONG_PTR rip: 
		 * @return void: 
		 */
		 void setGuestRip(ULONG_PTR rip);

		 /**
		  * ��ȡGuest rsp
		  * @return ULONG_PTR: 
		  */
		  ULONG_PTR readGuestRsp();

		 /**
		  * ����Guest rsp
		  * @param ULONG_PTR rsp: 
		  * @return void: 
		  */
		  void setGuestRsp(ULONG_PTR rsp);

		 /**
		  * ��ȡHost rip
		  * @return ULONG_PTR: 
		  */
		  ULONG_PTR readHostRip();

		 /**
		  * ����Host rip
		  * @param ULONG_PTR rip: 
		  * @return void: 
		  */
		  void setHostRip(ULONG_PTR rip);

		 /**
		  * ��ȡHost rsp
		  * @return ULONG_PTR: 
		  */
		  ULONG_PTR readHostRsp();

		 /**
		  * ����Host rsp
		  * @param ULONG_PTR rsp: 
		  * @return void: 
		  */
		  void setHostRsp(ULONG_PTR rsp);

	}
}