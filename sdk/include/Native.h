/*++

Copyright (c) Alex Ionescu.  All rights reserved.

Header Name:

    ntint.h

Abstract:

    This header contains selected NT structures and functions from ntosp.h

Author:

    Alex Ionescu (@aionescu) 16-Mar-2016 - Initial version

Environment:

    Kernel mode only.

--*/

#pragma once
#include <ntddk.h>


typedef enum _SYSTEM_INFORMATION_CLASS
{
    SystemBasicInformation = 0x0,
    SystemProcessorInformation = 0x1,
    SystemPerformanceInformation = 0x2,
    SystemTimeOfDayInformation = 0x3,
    SystemPathInformation = 0x4,
    SystemProcessInformation = 0x5,
    SystemCallCountInformation = 0x6,
    SystemDeviceInformation = 0x7,
    SystemProcessorPerformanceInformation = 0x8,
    SystemFlagsInformation = 0x9,
    SystemCallTimeInformation = 0xa,
    SystemModuleInformation = 0xb,
    SystemLocksInformation = 0xc,
    SystemStackTraceInformation = 0xd,
    SystemPagedPoolInformation = 0xe,
    SystemNonPagedPoolInformation = 0xf,
    SystemHandleInformation = 0x10,
    SystemObjectInformation = 0x11,
    SystemPageFileInformation = 0x12,
    SystemVdmInstemulInformation = 0x13,
    SystemVdmBopInformation = 0x14,
    SystemFileCacheInformation = 0x15,
    SystemPoolTagInformation = 0x16,
    SystemInterruptInformation = 0x17,
    SystemDpcBehaviorInformation = 0x18,
    SystemFullMemoryInformation = 0x19,
    SystemLoadGdiDriverInformation = 0x1a,
    SystemUnloadGdiDriverInformation = 0x1b,
    SystemTimeAdjustmentInformation = 0x1c,
    SystemSummaryMemoryInformation = 0x1d,
    SystemMirrorMemoryInformation = 0x1e,
    SystemPerformanceTraceInformation = 0x1f,
    SystemObsolete0 = 0x20,
    SystemExceptionInformation = 0x21,
    SystemCrashDumpStateInformation = 0x22,
    SystemKernelDebuggerInformation = 0x23,
    SystemContextSwitchInformation = 0x24,
    SystemRegistryQuotaInformation = 0x25,
    SystemExtendServiceTableInformation = 0x26,
    SystemPrioritySeperation = 0x27,
    SystemVerifierAddDriverInformation = 0x28,
    SystemVerifierRemoveDriverInformation = 0x29,
    SystemProcessorIdleInformation = 0x2a,
    SystemLegacyDriverInformation = 0x2b,
    SystemCurrentTimeZoneInformation = 0x2c,
    SystemLookasideInformation = 0x2d,
    SystemTimeSlipNotification = 0x2e,
    SystemSessionCreate = 0x2f,
    SystemSessionDetach = 0x30,
    SystemSessionInformation = 0x31,
    SystemRangeStartInformation = 0x32,
    SystemVerifierInformation = 0x33,
    SystemVerifierThunkExtend = 0x34,
    SystemSessionProcessInformation = 0x35,
    SystemLoadGdiDriverInSystemSpace = 0x36,
    SystemNumaProcessorMap = 0x37,
    SystemPrefetcherInformation = 0x38,
    SystemExtendedProcessInformation = 0x39,
    SystemRecommendedSharedDataAlignment = 0x3a,
    SystemComPlusPackage = 0x3b,
    SystemNumaAvailableMemory = 0x3c,
    SystemProcessorPowerInformation = 0x3d,
    SystemEmulationBasicInformation = 0x3e,
    SystemEmulationProcessorInformation = 0x3f,
    SystemExtendedHandleInformation = 0x40,
    SystemLostDelayedWriteInformation = 0x41,
    SystemBigPoolInformation = 0x42,
    SystemSessionPoolTagInformation = 0x43,
    SystemSessionMappedViewInformation = 0x44,
    SystemHotpatchInformation = 0x45,
    SystemObjectSecurityMode = 0x46,
    SystemWatchdogTimerHandler = 0x47,
    SystemWatchdogTimerInformation = 0x48,
    SystemLogicalProcessorInformation = 0x49,
    SystemWow64SharedInformationObsolete = 0x4a,
    SystemRegisterFirmwareTableInformationHandler = 0x4b,
    SystemFirmwareTableInformation = 0x4c,
    SystemModuleInformationEx = 0x4d,
    SystemVerifierTriageInformation = 0x4e,
    SystemSuperfetchInformation = 0x4f,
    SystemMemoryListInformation = 0x50,
    SystemFileCacheInformationEx = 0x51,
    SystemThreadPriorityClientIdInformation = 0x52,
    SystemProcessorIdleCycleTimeInformation = 0x53,
    SystemVerifierCancellationInformation = 0x54,
    SystemProcessorPowerInformationEx = 0x55,
    SystemRefTraceInformation = 0x56,
    SystemSpecialPoolInformation = 0x57,
    SystemProcessIdInformation = 0x58,
    SystemErrorPortInformation = 0x59,
    SystemBootEnvironmentInformation = 0x5a,
    SystemHypervisorInformation = 0x5b,
    SystemVerifierInformationEx = 0x5c,
    SystemTimeZoneInformation = 0x5d,
    SystemImageFileExecutionOptionsInformation = 0x5e,
    SystemCoverageInformation = 0x5f,
    SystemPrefetchPatchInformation = 0x60,
    SystemVerifierFaultsInformation = 0x61,
    SystemSystemPartitionInformation = 0x62,
    SystemSystemDiskInformation = 0x63,
    SystemProcessorPerformanceDistribution = 0x64,
    SystemNumaProximityNodeInformation = 0x65,
    SystemDynamicTimeZoneInformation = 0x66,
    SystemCodeIntegrityInformation = 0x67,
    SystemProcessorMicrocodeUpdateInformation = 0x68,
    SystemProcessorBrandString = 0x69,
    SystemVirtualAddressInformation = 0x6a,
    SystemLogicalProcessorAndGroupInformation = 0x6b,
    SystemProcessorCycleTimeInformation = 0x6c,
    SystemStoreInformation = 0x6d,
    SystemRegistryAppendString = 0x6e,
    SystemAitSamplingValue = 0x6f,
    SystemVhdBootInformation = 0x70,
    SystemCpuQuotaInformation = 0x71,
    SystemNativeBasicInformation = 0x72,
    SystemErrorPortTimeouts = 0x73,
    SystemLowPriorityIoInformation = 0x74,
    SystemBootEntropyInformation = 0x75,
    SystemVerifierCountersInformation = 0x76,
    SystemPagedPoolInformationEx = 0x77,
    SystemSystemPtesInformationEx = 0x78,
    SystemNodeDistanceInformation = 0x79,
    SystemAcpiAuditInformation = 0x7a,
    SystemBasicPerformanceInformation = 0x7b,
    SystemQueryPerformanceCounterInformation = 0x7c,
    SystemSessionBigPoolInformation = 0x7d,
    SystemBootGraphicsInformation = 0x7e,
    SystemScrubPhysicalMemoryInformation = 0x7f,
    SystemBadPageInformation = 0x80,
    SystemProcessorProfileControlArea = 0x81,
    SystemCombinePhysicalMemoryInformation = 0x82,
    SystemEntropyInterruptTimingInformation = 0x83,
    SystemConsoleInformation = 0x84,
    SystemPlatformBinaryInformation = 0x85,
    SystemThrottleNotificationInformation = 0x86,
    SystemHypervisorProcessorCountInformation = 0x87,
    SystemDeviceDataInformation = 0x88,
    SystemDeviceDataEnumerationInformation = 0x89,
    SystemMemoryTopologyInformation = 0x8a,
    SystemMemoryChannelInformation = 0x8b,
    SystemBootLogoInformation = 0x8c,
    SystemProcessorPerformanceInformationEx = 0x8d,
    SystemSpare0 = 0x8e,
    SystemSecureBootPolicyInformation = 0x8f,
    SystemPageFileInformationEx = 0x90,
    SystemSecureBootInformation = 0x91,
    SystemEntropyInterruptTimingRawInformation = 0x92,
    SystemPortableWorkspaceEfiLauncherInformation = 0x93,
    SystemFullProcessInformation = 0x94,
    SystemKernelDebuggerInformationEx = 0x95,
    SystemBootMetadataInformation = 0x96,
    SystemSoftRebootInformation = 0x97,
    SystemElamCertificateInformation = 0x98,
    SystemOfflineDumpConfigInformation = 0x99,
    SystemProcessorFeaturesInformation = 0x9a,
    SystemRegistryReconciliationInformation = 0x9b,
    MaxSystemInfoClass = 0x9c,
} SYSTEM_INFORMATION_CLASS;

typedef struct _SYSTEM_BASIC_INFORMATION
{
    ULONG reserved;
    ULONG timerResolution;
    ULONG pageSize;
    ULONG numberOfPhysicalPages;
    ULONG lowestPhysicalPageNumber;
    ULONG highestPhysicalPageNumber;
    ULONG allocationGranularity;
    ULONG_PTR minimumUserModeAddress;
    ULONG_PTR maximumUserModeAddress;
    ULONG_PTR activeProcessorsAffinityMask;
    CCHAR numberOfProcessors;
} SYSTEM_BASIC_INFORMATION, *PSYSTEM_BASIC_INFORMATION;

typedef struct _SYSTEM_SERVICE_DESCRIPTOR_TABLE
{
    PULONG_PTR serviceTableBase;
    PULONG serviceCounterTableBase;
    ULONG_PTR numberOfServices;
    PUCHAR paramTableBase;
} SYSTEM_SERVICE_DESCRIPTOR_TABLE, *PSYSTEM_SERVICE_DESCRIPTOR_TABLE;

typedef struct _RTL_PROCESS_MODULE_INFORMATION
{
    HANDLE section;         // Not filled in
    PVOID mappedBase;
    PVOID imageBase;
    ULONG imageSize;
    ULONG flags;
    USHORT loadOrderIndex;
    USHORT initOrderIndex;
    USHORT loadCount;
    USHORT offsetToFileName;
    UCHAR  fullPathName[256];
} RTL_PROCESS_MODULE_INFORMATION, *PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES
{
    ULONG numberOfModules;
    RTL_PROCESS_MODULE_INFORMATION modules[1];
} RTL_PROCESS_MODULES, *PRTL_PROCESS_MODULES;

typedef struct _PHYSICAL_MEMORY_RUN
{
    PFN_NUMBER basePage;
    PFN_NUMBER pageCount;
} PHYSICAL_MEMORY_RUN, *PPHYSICAL_MEMORY_RUN;

typedef struct _PHYSICAL_MEMORY_DESCRIPTOR
{
    ULONG numberOfRuns;
    PFN_NUMBER numberOfPages;
    PHYSICAL_MEMORY_RUN run[1];
} PHYSICAL_MEMORY_DESCRIPTOR, *PPHYSICAL_MEMORY_DESCRIPTOR;


//
// Define pseudo descriptor structures for both 64- and 32-bit mode.
//

typedef struct _KDESCRIPTOR 
{
    USHORT pad[3];
    USHORT limit;
    PVOID base;
} KDESCRIPTOR, *PKDESCRIPTOR;

typedef struct _KDESCRIPTOR32 
{
    USHORT pad[3];
    USHORT limit;
    ULONG base;
} KDESCRIPTOR32, *PKDESCRIPTOR32;

//
// Define special kernel registers and the initial MXCSR value.
//

typedef struct _KSPECIAL_REGISTERS 
{
    ULONG64 cr0;
    ULONG64 cr2;
    ULONG64 cr3;
    ULONG64 cr4;
    ULONG64 kernelDr0;
    ULONG64 kernelDr1;
    ULONG64 kernelDr2;
    ULONG64 kernelDr3;
    ULONG64 kernelDr6;
    ULONG64 kernelDr7;
    KDESCRIPTOR gdtr;
    KDESCRIPTOR idtr;
    USHORT tr;
    USHORT ldtr;
    ULONG mxCsr;
    ULONG64 debugControl;
    ULONG64 lastBranchToRip;
    ULONG64 lastBranchFromRip;
    ULONG64 lastExceptionToRip;
    ULONG64 lastExceptionFromRip;
    ULONG64 cr8;
    ULONG64 msrGsBase;
    ULONG64 msrGsSwap;
    ULONG64 msrStar;
    ULONG64 msrLStar;
    ULONG64 msrCStar;
    ULONG64 msrSyscallMask;
    ULONG64 xcr0;
} KSPECIAL_REGISTERS, *PKSPECIAL_REGISTERS;

//
// Define processor state structure.
//

typedef struct _KPROCESSOR_STATE 
{
    KSPECIAL_REGISTERS specialRegisters;
    CONTEXT contextFrame;
} KPROCESSOR_STATE, *PKPROCESSOR_STATE;

//
// Define descriptor privilege levels for user and system.
//

#define DPL_USER 3
#define DPL_SYSTEM 0

//
// Define limit granularity.
//

#define GRANULARITY_BYTE 0
#define GRANULARITY_PAGE 1

//
// Define processor number packing constants.
//
// The compatibility processor number is encoded in the FS segment descriptor.
//
// Bits 19:14 of the segment limit encode the compatible processor number.
// Bits 13:10 are set to ones to ensure that segment limit is at least 15360.
// Bits 9:0 of the segment limit encode the extended processor number.
//

#define KGDT_LEGACY_LIMIT_SHIFT 14
#define KGDT_LIMIT_ENCODE_MASK (0xf << 10)

#define SELECTOR_TABLE_INDEX 0x04

#define KGDT64_NULL         0x00
#define KGDT64_R0_CODE      0x10
#define KGDT64_R0_DATA      0x18
#define KGDT64_R3_CMCODE    0x20
#define KGDT64_R3_DATA      0x28
#define KGDT64_R3_CODE      0x30
#define KGDT64_SYS_TSS      0x40
#define KGDT64_R3_CMTEB     0x50
#define KGDT64_R0_LDT       0x60

#define RPL_MASK 3


#pragma warning(disable: 4214 4201)
typedef union _KGDTENTRY64 
{
    struct 
    {
        USHORT limitLow;
        USHORT baseLow;
        union 
        {
            struct 
            {
                UCHAR baseMiddle;
                UCHAR flags1;
                UCHAR flags2;
                UCHAR baseHigh;
            } bytes;

            struct 
            {
                ULONG baseMiddle : 8;
                ULONG type : 5;
                ULONG dpl : 2;
                ULONG present : 1;
                ULONG limitHigh : 4;
                ULONG system : 1;
                ULONG longMode : 1;
                ULONG defaultBig : 1;
                ULONG granularity : 1;
                ULONG baseHigh : 8;
            } bits;
        };

        ULONG baseUpper;
        ULONG mustBeZero;
    };

    struct 
    {
        LONG64 dataLow;
        LONG64 dataHigh;
    };

} KGDTENTRY64, *PKGDTENTRY64;
#pragma warning(default: 4214 4201)

NTKERNELAPI
_IRQL_requires_max_(APC_LEVEL)
_IRQL_requires_min_(PASSIVE_LEVEL)
_IRQL_requires_same_
VOID
KeGenericCallDpc (
    _In_ PKDEFERRED_ROUTINE routine,
    _In_opt_ PVOID context
    );

NTKERNELAPI
_IRQL_requires_(DISPATCH_LEVEL)
_IRQL_requires_same_
VOID
KeSignalCallDpcDone (
    _In_ PVOID systemArgument1
    );

NTKERNELAPI
_IRQL_requires_(DISPATCH_LEVEL)
_IRQL_requires_same_
LOGICAL
KeSignalCallDpcSynchronize (
    _In_ PVOID systemArgument2
    );

DECLSPEC_NORETURN
NTSYSAPI
VOID
RtlRestoreContext(
    _In_ PCONTEXT contextRecord,
    _In_opt_ struct _EXCEPTION_RECORD * exceptionRecord
    );

NTKERNELAPI
VOID
KeSaveStateForHibernate (
    _In_ PKPROCESSOR_STATE state
    );

NTSYSAPI
VOID
NTAPI
RtlCaptureContext(
    _Out_ PCONTEXT contextRecord
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwQuerySystemInformation(
    IN SYSTEM_INFORMATION_CLASS systemInformationClass,
    OUT PVOID systemInformation,
    IN ULONG systemInformationLength,
    OUT PULONG returnLength OPTIONAL
    );

NTSYSAPI
PIMAGE_NT_HEADERS
NTAPI
RtlImageNtHeader( 
    _In_ PVOID base
    );