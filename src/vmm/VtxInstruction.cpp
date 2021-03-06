#include "VtxInstruction.h"


namespace vmm
{
	namespace vtx
	{
		void enableMTF()
		{
			VmxCpuBasedControls cpuBasedControls = { 0 };
			__vmx_vmread(CPU_BASED_VM_EXEC_CONTROL, (size_t*)&cpuBasedControls);
			cpuBasedControls.fields.monitorTrapFlag = true;
			__vmx_vmwrite(CPU_BASED_VM_EXEC_CONTROL, cpuBasedControls.all);
		}

		void disableMTF()
		{
			VmxCpuBasedControls cpuBasedControls = { 0 };
			__vmx_vmread(CPU_BASED_VM_EXEC_CONTROL, (size_t*)&cpuBasedControls);
			cpuBasedControls.fields.monitorTrapFlag = false;
			__vmx_vmwrite(CPU_BASED_VM_EXEC_CONTROL, cpuBasedControls.all);
		}

		VmExitInformation readVmexitReason()
		{
			VmExitInformation exitReason = { 0 };

			//查询导致vmexit的原因 
			// See:AppendixC VMX Basic Exit Reson ,Table C-1 Basic Exit Reson
			__vmx_vmread(VM_EXIT_REASON, (ULONG_PTR*)&exitReason);
			return exitReason;
		}

		ULONG_PTR readGuestRip()
		{
			ULONG_PTR rip = 0;
			__vmx_vmread(GUEST_RIP, &rip);
			return rip;
		}

		void setGuestRip(ULONG_PTR rip)
		{
			__vmx_vmwrite(GUEST_RIP, rip);
		}

		ULONG_PTR readGuestRsp()
		{
			ULONG_PTR rsp = 0;
			__vmx_vmread(GUEST_RSP, &rsp);
			return rsp;
		}

		void setGuestRsp(ULONG_PTR rsp)
		{
			__vmx_vmwrite(GUEST_RSP, rsp);
		}

		ULONG_PTR readHostRip()
		{
			ULONG_PTR rip = 0;
			__vmx_vmread(HOST_RIP, &rip);
			return rip;
		}

		void setHostRip(ULONG_PTR rip)
		{
			__vmx_vmwrite(HOST_RIP, rip);
		}

		ULONG_PTR readHostRsp()
		{
			ULONG_PTR rip = 0;
			__vmx_vmread(HOST_RSP, &rip);
			return rip;
		}

		void setHostRsp(ULONG_PTR rsp)
		{
			__vmx_vmwrite(HOST_RSP, rsp);
		}

		
		
		void setInterruptWindowExiting(bool set)
		{
			 size_t cpuBasedVmExecControls = 0;
			__vmx_vmread(CPU_BASED_VM_EXEC_CONTROL, &cpuBasedVmExecControls);
			if (set)
			{
				cpuBasedVmExecControls |= CPU_BASED_VIRTUAL_INTR_PENDING;
			}
			else
			{
				cpuBasedVmExecControls &= ~CPU_BASED_VIRTUAL_INTR_PENDING;
			}
			__vmx_vmwrite(CPU_BASED_VM_EXEC_CONTROL, cpuBasedVmExecControls);
		}

		ULONG64 readGuestPhysicalAddress()
		{
			size_t guestPa = 0;
			__vmx_vmread(GUEST_PHYSICAL_ADDRESS, &guestPa);
			return guestPa;
		}

	}
}
