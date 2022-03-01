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
			 * vmexit事件入口
			 * @param Vcpu * vcpu:
			 * @return continue vmx?
			 */
			extern "C" BOOLEAN vmExitEntryPoint(Registers64* reg);

			/**
			 * 恢复客户机rip到导致vmexit指令的下一条指令,
			 */
			void resume();

			/**
			 * 执行cpuid指令而导致的vmexit事件的处理函数
			 * @param Registers64 * pGuestRegisters:客户机通用寄存器
			 */
			void handleCpuid(Registers64* reg);

			/**
			 * 执行invd指令而导致的vmexit事件的处理函数
			 * @param Registers64 * reg:客户机通用寄存器
			 */
			void handleInvd(Registers64* reg);

			/**
			 * 执行vmcall指令而导致的vmexit事件的处理函数
			 * @param Registers64 * reg: 客户机通用寄存器
			 * @return 是否继续执行vmx
			 */
			BOOLEAN handleVmcall(Registers64* reg);

			/**
			 * 访问(读/写)控制寄存器而导致的vmexit事件的处理函数
			 * @param Registers64 * reg: 客户机通用寄存器
			 */
			void handleCrAccess(Registers64* reg);

			/**
			 * 读取msr寄存器而导致的vmexit事件的处理函数
			 * @param Registers64 * reg: 客户机通用寄存器
			 */
			void handleMsrRead(Registers64* reg);

			/**
			 * 写入msr寄存器而导致的vmexit事件的处理函数
			 * @param Registers64 * reg: 客户机通用寄存器
			 */
			void handleMsrWrite(Registers64* reg);

			/**
			 * 开启MTF后执行指令所产生的vmexit处理函数
			 * @param Registers64 * reg: 客户机通用寄存器
			 */
			void handleMtf(Registers64* reg);

			/**
			 * 内存属性不匹配导致的vmexit处理函数
			 * @param Registers64 * reg: 客户机通用寄存器
			 */
			void handleEptViolation(Registers64* reg);

			/**
			 * 内存属性错误导致的vmexit处理函数
			 * @param Registers64 * pGuestRegisters: 客户机通用寄存器
			 */
			void handleEptMisconfig(Registers64* reg);
			
		}
	}
}