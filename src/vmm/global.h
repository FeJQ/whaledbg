#pragma once
#include <Native.h>



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

extern Vcpu* vcpu;