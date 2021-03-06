#pragma once
#include <Native.h>
#include "Common.h"
#include "Ept.h"


namespace vmm
{
	namespace vmx
	{
		using ept::EptAccess;

		enum VmxState
		{
			OFF,           // No virtualization
			TRANSITION,    // Virtualized, context not yet restored
			ON,            // Virtualized, running guest
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

			// 导致ept violation 的指令所在的页
			//ULONG_PTR eptViolationRipPage;
			//EptAccess oldEptViolationRipPageAccess;

			// 权限不够而引发ept violation 的页
			ULONG_PTR eptViolationPage;

			bool eptViolationPageReadAccess;
			bool eptViolationPageWriteAccess;
			bool eptViolationPageExecuteAccess;
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
		extern "C" NTSTATUS launchVmx(Vcpu * vcpu, PVOID guestRsp, PVOID guestRip);

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
