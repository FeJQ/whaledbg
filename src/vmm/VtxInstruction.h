#pragma once
#include "IA32.h"

namespace vmm
{
	namespace vtx
	{
		/**
		 * 设置MTF
		 * @param bool value: 开启或关闭
		 */
		void setMonitorTrapFlag(bool value);

		/**
		 * 查询导致vmexit的原因
		 * @return VmExitInformation:
		 */
		VmExitInformation readVmexitReason();

		/**
		 * 读取VMCS 中 GuestRip字段
		 * @return ULONG_PTR:guest rip
		 */
		ULONG_PTR readGuestRip();

		/**
		 * 设置Guest rip
		 * @param ULONG_PTR rip: 
		 * @return void: 
		 */
		 void setGuestRip(ULONG_PTR rip);

		 /**
		  * 获取Guest rsp
		  * @return ULONG_PTR: 
		  */
		  ULONG_PTR readGuestRsp();

		 /**
		  * 设置Guest rsp
		  * @param ULONG_PTR rsp: 
		  * @return void: 
		  */
		  void setGuestRsp(ULONG_PTR rsp);

		 /**
		  * 获取Host rip
		  * @return ULONG_PTR: 
		  */
		  ULONG_PTR readHostRip();

		 /**
		  * 设置Host rip
		  * @param ULONG_PTR rip: 
		  * @return void: 
		  */
		  void setHostRip(ULONG_PTR rip);

		 /**
		  * 获取Host rsp
		  * @return ULONG_PTR: 
		  */
		  ULONG_PTR readHostRsp();

		 /**
		  * 设置Host rsp
		  * @param ULONG_PTR rsp: 
		  * @return void: 
		  */
		  void setHostRsp(ULONG_PTR rsp);

	}
}