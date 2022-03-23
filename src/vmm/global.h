#pragma once
#include <Native.h>
#include "Common.h"

#define POOL_TAG_VMXON 'vmon'
#define POOL_TAG_VMCS 'vmcs'
#define POOL_TAG_HOST_STACK 'hstk'

enum VmxState
{
    Off,           // No virtualization
    Transition,    // Virtualized, context not yet restored
    On,            // Virtualized, running guest
};

enum EptAccess
{
	All = 0b111,
	Read = 0b001,
	Write = 0b010,
	Execute = 0b100
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



union Pml4Entry
{
	ULONG64 all;
	struct
	{
		ULONG64 readAccess : 1;       //!< [0]
		ULONG64 writeAccess : 1;      //!< [1]
		ULONG64 executeAccess : 1;    //!< [2]
		ULONG64 reserved1 : 5;       //!< [3:7]
		ULONG64 accessFlag : 1;         //!< [8]
		ULONG64 ignored : 3;  //!< [9:11]
		ULONG64 physAddr : 40;  //   [12:51]
		ULONG64 reserved2 : 12;       //!< [52:63]
	}fields;
};






