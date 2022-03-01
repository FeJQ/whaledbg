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
			 * ���Vmx�Ƿ����
			 * @return NTSTATUS:
			 */
			NTSTATUS checkVmxAvailable();

			/**
			 * ���Ept�Ƿ����
			 * @return override bool:
			 */
			NTSTATUS checkEptAvailable();

			/**
			 * ����vmx��־
			 * @return NTSTATUS:
			 */
			NTSTATUS enableVmxFeature();

			/**
			 * ����Vmx��ռ�
			 * @param Vcpu* vcpu
			 * @return NTSTATUS:
			 */
			NTSTATUS allocVcpu(Vcpu* vcpu);

			/**
			 * ����vmx
			 * @param Vcpu* vcpu:
			 * @return NTSTATUS:
			 */
			extern "C" NTSTATUS launchVmx(Vcpu* vcpu, PVOID guestRsp, PVOID guestRip);

			/**
			 * �˳�vmroot
			 * @return NTSTATUS:
			 */
			NTSTATUS quitVmx();

			/**
			 * ����Rootģʽ,ִ��vmxon,����VMCS
			 * @param VmxContext * vmxContext:
			 * @return NTSTATUS:
			 */
			NTSTATUS enableRoot(Vcpu* vcpu);

			/**
			 * װ��vmcs
			 * @param Vcpu * vcpu:
			 * @return BOOLEAN:
			 */
			NTSTATUS setupVmcs(Vcpu* vcpu);

			extern Vcpu* vcpu;
		}
	}
}