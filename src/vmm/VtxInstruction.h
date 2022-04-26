#pragma once
#include "IA32.h"

namespace vmm
{
	namespace vtx
	{
		/**
		 * 开启MTF
		 * @return void:
		 */
		void enableMTF();

		/**
		 * 关闭MTF
		 * @return void:
		 */
		void disableMTF();

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

		/**
		 * setInterruptWindowExiting
		 * @param set
		 */
		void setInterruptWindowExiting(bool set);

		ULONG64 readGuestPhysicalAddress();
	}
}