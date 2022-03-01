#pragma once
#include <Native.h>
#include "Common.h"

namespace whaledbg
{
	namespace vmm
	{
		namespace vmx
		{

			enum VmxState
			{
				Off,           // No virtualization
				Transition,    // Virtualized, context not yet restored
				On,            // Virtualized, running guest
			};

			struct Vcpu
			{
				KPROCESSOR_STATE guestState;
				VmxState vmxState;
				PVOID vmxonRegion;
				PVOID vmcsRegion;
				PVOID vmmStack;
				PVOID vmmStackBase;
				BOOLEAN isVmxEnable;
			};

			NTSTATUS initialize();

			/**
			 * 检查Vmx是否可用
			 * @return NTSTATUS:
			 */
			NTSTATUS checkVmxAvailable();

			/**
			 * 检查Ept是否可用
			 * @return override bool:
			 */
			NTSTATUS checkEptAvailable();

			/**
			 * 开启vmx标志
			 * @return NTSTATUS:
			 */
			NTSTATUS enableVmxFeature();

			/**
			 * 申请Vmx域空间
			 * @param Vcpu* vcpu
			 * @return NTSTATUS:
			 */
			NTSTATUS allocVcpu(Vcpu* vcpu);

			/**
			 * 启动vmx
			 * @param Vcpu* vcpu:
			 * @return NTSTATUS:
			 */
			extern "C" NTSTATUS launchVmx(Vcpu* vcpu, PVOID guestRsp, PVOID guestRip);

			/**
			 * 退出vmroot
			 * @return NTSTATUS:
			 */
			NTSTATUS quitVmx();

			/**
			 * 开启Root模式,执行vmxon,激活VMCS
			 * @param VmxContext * vmxContext:
			 * @return NTSTATUS:
			 */
			NTSTATUS enableRoot(Vcpu* vcpu);

			/**
			 * 装载vmcs
			 * @param Vcpu * vcpu:
			 * @return BOOLEAN:
			 */
			NTSTATUS setupVmcs(Vcpu* vcpu);

			extern Vcpu* vcpu;
		}
	}
}