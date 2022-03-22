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

union PteEntry
{
	ULONG64 all;
	struct
	{
		ULONG64 readAccess : 1;       //!< [0]
		ULONG64 writeAccess : 1;      //!< [1]
		ULONG64 executeAccess : 1;    //!< [2]
		ULONG64 memoryType : 3;       //!< [3:5]
		ULONG64 reserved1 : 6;         //!< [6:11]
		ULONG64 physialAddress : 36;  //!< [12:48-1]
		ULONG64 reserved2 : 16;        //!< [48:63]
	} fields;
};
union EptPointer
{
	ULONG64 all;
	struct
	{
		ULONG64 memoryType : 3;         // EPT Paging structure memory type (0 for UC)
		ULONG64 pageWalkLength : 3;     // Page-walk length
		ULONG64 reserved1 : 6;
		ULONG64 physAddr : 40;          // Physical address of the EPT PML4 table
		ULONG64 reserved2 : 12;
	} fields;
};
struct EptControl
{
	PteEntry* pml4t;
};

struct PageEntry
{
    LIST_ENTRY pageList;             // 指向下一个PageEntry
    ULONG_PTR targetAddress;         // 目标地址
    ULONG_PTR pageAddress;           // 目标页首地址
    ULONG_PTR shadowPageAddress;     // 假页页首地址
	PteEntry* pte;                   // 目标页所对应的pte

    ULONG_PTR readPage;
    ULONG_PTR writePage;
    ULONG_PTR executePage;
};


