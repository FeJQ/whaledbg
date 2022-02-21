#pragma once
#include "Common.hpp"


#define MSR_APIC_BASE                       0x01B

#define MSR_IA32_VMX_BASIC                  0x480
#define MSR_IA32_VMX_PINBASED_CTLS          0x481
#define MSR_IA32_VMX_PROCBASED_CTLS         0x482
#define MSR_IA32_VMX_EXIT_CTLS              0x483
#define MSR_IA32_VMX_ENTRY_CTLS             0x484
#define MSR_IA32_VMX_MISC                   0x485
#define MSR_IA32_VMX_CR0_FIXED0             0x486
#define MSR_IA32_VMX_CR0_FIXED1             0x487
#define MSR_IA32_VMX_CR4_FIXED0             0x488
#define MSR_IA32_VMX_CR4_FIXED1             0x489
#define MSR_IA32_VMX_VMCS_ENUM              0x48A
#define MSR_IA32_VMX_PROCBASED_CTLS2        0x48B
#define MSR_IA32_VMX_EPT_VPID_CAP           0x48C
#define MSR_IA32_VMX_TRUE_PINBASED_CTLS     0x48D
#define MSR_IA32_VMX_TRUE_PROCBASED_CTLS    0x48E
#define MSR_IA32_VMX_TRUE_EXIT_CTLS         0x48F
#define MSR_IA32_VMX_TRUE_ENTRY_CTLS        0x490
#define MSR_IA32_VMX_VMFUNC                 0x491

#define MSR_IA32_SYSENTER_CS                0x174
#define MSR_IA32_SYSENTER_ESP               0x175
#define MSR_IA32_SYSENTER_EIP               0x176
#define MSR_IA32_DEBUGCTL                   0x1D9


/*
 * Intel CPU  MSR
 */
 /* MSRs & bits used for VMX enabling */

#define CPU_BASED_VIRTUAL_INTR_PENDING          0x00000004
#define CPU_BASED_USE_TSC_OFFSETING             0x00000008
#define CPU_BASED_HLT_EXITING                   0x00000080
#define CPU_BASED_INVLPG_EXITING                0x00000200
#define CPU_BASED_MWAIT_EXITING                 0x00000400
#define CPU_BASED_RDPMC_EXITING                 0x00000800
#define CPU_BASED_RDTSC_EXITING                 0x00001000
#define CPU_BASED_CR3_LOAD_EXITING		        0x00008000
#define CPU_BASED_CR3_STORE_EXITING		       0x00010000
#define CPU_BASED_CR8_LOAD_EXITING              0x00080000
#define CPU_BASED_CR8_STORE_EXITING             0x00100000
#define CPU_BASED_TPR_SHADOW                    0x00200000
#define CPU_BASED_VIRTUAL_NMI_PENDING		     0x00400000
#define CPU_BASED_MOV_DR_EXITING                0x00800000
#define CPU_BASED_UNCOND_IO_EXITING             0x01000000
#define CPU_BASED_USE_IO_BITMAPS                0x02000000
#define CPU_BASED_ACTIVATE_MSR_BITMAP           0x10000000
#define CPU_BASED_MTF_TRAP_EXITING              0x08000000
#define CPU_BASED_USE_MSR_BITMAPS               0x10000000
#define CPU_BASED_MONITOR_EXITING               0x20000000
#define CPU_BASED_PAUSE_EXITING                 0x40000000
#define CPU_BASED_ACTIVATE_SECONDARY_CONTROLS   0x80000000

#define PIN_BASED_ALWAYSON_WITHOUT_TRUE_MSR	0x00000016
#define VM_EXIT_SAVE_DEBUG_CONTROLS      0x00000004
#define VM_EXIT_IA32E_MODE              0x00000200
#define VM_EXIT_ACK_INTR_ON_EXIT        0x00008000
#define VM_EXIT_SAVE_IA32_PAT			0x00040000
#define VM_EXIT_LOAD_IA32_PAT			0x00080000
#define VM_EXIT_SAVE_IA32_EFER          0x00100000
#define VM_EXIT_LOAD_IA32_EFER          0x00200000
#define VM_EXIT_SAVE_VMX_PREEMPTION_TIMER       0x00400000
#define VM_EXIT_CLEAR_BNDCFGS                   0x00800000

#define VM_ENTRY_LOAD_DEBUG_CONTROLS            0x00000004
#define VM_ENTRY_IA32E_MODE             0x00000200
#define VM_ENTRY_SMM                    0x00000400
#define VM_ENTRY_DEACT_DUAL_MONITOR     0x00000800
#define VM_ENTRY_LOAD_IA32_PAT			0x00004000
#define VM_ENTRY_LOAD_IA32_EFER         0x00008000
#define VM_ENTRY_LOAD_BNDCFGS           0x00010000

#define MSR_IA32_VMX_BASIC   		0x480
#define MSR_IA32_FEATURE_CONTROL 		0x03a
#define MSR_IA32_VMX_PINBASED_CTLS		0x481
#define MSR_IA32_VMX_PROCBASED_CTLS		0x482
#define MSR_IA32_VMX_EXIT_CTLS		0x483
#define MSR_IA32_VMX_ENTRY_CTLS		0x484


#define MSR_EFER 0xc0000080           /* extended feature register */
#define MSR_STAR 0xc0000081           /* legacy mode SYSCALL target */
#define MSR_LSTAR 0xc0000082          /* long mode SYSCALL target */
#define MSR_CSTAR 0xc0000083          /* compatibility mode SYSCALL target */
#define MSR_SYSCALL_MASK 0xc0000084   /* EFLAGS mask for syscall */
#define MSR_FS_BASE 0xc0000100                /* 64bit FS base */
#define MSR_GS_BASE 0xc0000101                /* 64bit GS base */
#define MSR_SHADOW_GS_BASE  0xc0000102        /* SwapGS GS shadow */


struct CpuidField
{
	int rax;
	int rbx;
	int rcx;
	int rdx;
};

union Cr0
{
	ULONG_PTR all;
	struct
	{
		unsigned pe : 1;          //!< [0] Protected Mode Enabled
		unsigned mp : 1;          //!< [1] Monitor Coprocessor FLAG
		unsigned em : 1;          //!< [2] Emulate FLAG
		unsigned ts : 1;          //!< [3] Task Switched FLAG
		unsigned et : 1;          //!< [4] Extension Type FLAG
		unsigned ne : 1;          //!< [5] Numeric Error
		unsigned reserved1 : 10;  //!< [6:15]
		unsigned wp : 1;          //!< [16] Write Protect
		unsigned reserved2 : 1;   //!< [17]
		unsigned am : 1;          //!< [18] Alignment Mask
		unsigned reserved3 : 10;  //!< [19:28]
		unsigned nw : 1;          //!< [29] Not Write-Through
		unsigned cd : 1;          //!< [30] Cache Disable
		unsigned pg : 1;          //!< [31] Paging Enabled
	} fields;
};

union Cr4
{
	ULONG_PTR all;
	struct
	{
		unsigned vme : 1;         //!< [0] Virtual Mode Extensions
		unsigned pvi : 1;         //!< [1] Protected-Mode Virtual Interrupts
		unsigned tsd : 1;         //!< [2] Time Stamp Disable
		unsigned de : 1;          //!< [3] Debugging Extensions
		unsigned pse : 1;         //!< [4] Page Size Extensions
		unsigned pae : 1;         //!< [5] Physical Address Extension
		unsigned mce : 1;         //!< [6] Machine-Check Enable
		unsigned pge : 1;         //!< [7] Page Global Enable
		unsigned pce : 1;         //!< [8] Performance-Monitoring Counter Enable
		unsigned osfxsr : 1;      //!< [9] OS Support for FXSAVE/FXRSTOR
		unsigned osxmmexcpt : 1;  //!< [10] OS Support for Unmasked SIMD Exceptions
		unsigned reserved1 : 2;   //!< [11:12]
		unsigned vmxe : 1;        //!< [13] Virtual Machine Extensions Enabled
		unsigned smxe : 1;        //!< [14] SMX-Enable Bit
		unsigned reserved2 : 2;   //!< [15:16]
		unsigned pcide : 1;       //!< [17] PCID Enable
		unsigned osxsave : 1;  //!< [18] XSAVE and Processor Extended States-Enable
		unsigned reserved3 : 1;  //!< [19]
		unsigned smep : 1;  //!< [20] Supervisor Mode Execution Protection Enable
		unsigned smap : 1;  //!< [21] Supervisor Mode Access Protection Enable
	} fields;
};

union CpuIdFiledEcx
{
	ULONG32 all;
	struct
	{
		ULONG32 sse3 : 1;       //!< [0] Streaming SIMD Extensions 3 (SSE3)
		ULONG32 pclmulqdq : 1;  //!< [1] PCLMULQDQ
		ULONG32 dtes64 : 1;     //!< [2] 64-bit DS Area
		ULONG32 monitor : 1;    //!< [3] MONITOR/WAIT
		ULONG32 ds_cpl : 1;     //!< [4] CPL qualified Debug Store
		ULONG32 vmx : 1;        //!< [5] Virtual Machine Technology
		ULONG32 smx : 1;        //!< [6] Safer Mode Extensions
		ULONG32 est : 1;        //!< [7] Enhanced Intel Speedstep Technology
		ULONG32 tm2 : 1;        //!< [8] Thermal monitor 2
		ULONG32 ssse3 : 1;      //!< [9] Supplemental Streaming SIMD Extensions 3
		ULONG32 cid : 1;        //!< [10] L1 context ID
		ULONG32 sdbg : 1;       //!< [11] IA32_DEBUG_INTERFACE MSR
		ULONG32 fma : 1;        //!< [12] FMA extensions using YMM state
		ULONG32 cx16 : 1;       //!< [13] CMPXCHG16B
		ULONG32 xtpr : 1;       //!< [14] xTPR Update Control
		ULONG32 pdcm : 1;       //!< [15] Performance/Debug capability MSR
		ULONG32 reserved : 1;   //!< [16] Reserved
		ULONG32 pcid : 1;       //!< [17] Process-context identifiers
		ULONG32 dca : 1;        //!< [18] prefetch from a memory mapped device
		ULONG32 sse4_1 : 1;     //!< [19] SSE4.1
		ULONG32 sse4_2 : 1;     //!< [20] SSE4.2
		ULONG32 x2_apic : 1;    //!< [21] x2APIC feature
		ULONG32 movbe : 1;      //!< [22] MOVBE instruction
		ULONG32 popcnt : 1;     //!< [23] POPCNT instruction
		ULONG32 reserved3 : 1;  //!< [24] one-shot operation using a TSC deadline
		ULONG32 aes : 1;        //!< [25] AESNI instruction
		ULONG32 xsave : 1;      //!< [26] XSAVE/XRSTOR feature
		ULONG32 osxsave : 1;    //!< [27] enable XSETBV/XGETBV instructions
		ULONG32 avx : 1;        //!< [28] AVX instruction extensions
		ULONG32 f16c : 1;       //!< [29] 16-bit floating-point conversion
		ULONG32 rdrand : 1;     //!< [30] RDRAND instruction
		ULONG32 not_used : 1;   //!< [31] Always 0 (a.k.a. HypervisorPresent)
	} fields;
};


union BasicMsr
{
	ULONG64 all;
	struct
	{
		unsigned revision_identifier : 31;    //!< [0:30]
		unsigned reserved1 : 1;               //!< [31]
		unsigned region_size : 12;            //!< [32:43]
		unsigned region_clear : 1;            //!< [44]
		unsigned reserved2 : 3;               //!< [45:47]
		unsigned supported_ia64 : 1;          //!< [48]
		unsigned supported_dual_moniter : 1;  //!< [49]
		unsigned memory_type : 4;             //!< [50:53]
		unsigned vm_exit_report : 1;          //!< [54]
		unsigned vmx_capability_hint : 1;     //!< [55]
		unsigned reserved3 : 8;               //!< [56:63]
	} fields;
};

union ControlMsr
{
	ULONG64 all;
	struct
	{
		unsigned lock : 1;                  //!< [0]
		unsigned enable_smx : 1;            //!< [1]
		unsigned enable_vmxon : 1;          //!< [2]
		unsigned reserved1 : 5;             //!< [3:7]
		unsigned enable_local_senter : 7;   //!< [8:14]
		unsigned enable_global_senter : 1;  //!< [15]
		unsigned reserved2 : 16;            //!<
		unsigned reserved3 : 32;            //!< [16:63]
	} fields;
};

/// See: Memory Types That Can Be Encoded With PAT Memory Types Recommended for
/// VMCS and Related Data Structures
enum  MemoryType : unsigned __int8
{
	Uncacheable = 0,
	WriteCombining = 1,
	WriteThrough = 4,
	WriteProtected = 5,
	WriteBack = 6,
	Uncached = 7,
};

/// See: VPID AND EPT CAPABILITIES
union EptVpidCapMsr
{
	ULONG64 all;
	struct {
		unsigned support_execute_only_pages : 1;                        //!< [0]
		unsigned reserved1 : 5;                                         //!< [1:5]
		unsigned support_page_walk_length4 : 1;                         //!< [6]
		unsigned reserved2 : 1;                                         //!< [7]
		unsigned support_uncacheble_memory_type : 1;                    //!< [8]
		unsigned reserved3 : 5;                                         //!< [9:13]
		unsigned support_write_back_memory_type : 1;                    //!< [14]
		unsigned reserved4 : 1;                                         //!< [15]
		unsigned support_pde_2mb_pages : 1;                             //!< [16]
		unsigned support_pdpte_1_gb_pages : 1;                          //!< [17]
		unsigned reserved5 : 2;                                         //!< [18:19]
		unsigned support_invept : 1;                                    //!< [20]
		unsigned support_accessed_and_dirty_flag : 1;                   //!< [21]
		unsigned reserved6 : 3;                                         //!< [22:24]
		unsigned support_single_context_invept : 1;                     //!< [25]
		unsigned support_all_context_invept : 1;                        //!< [26]
		unsigned reserved7 : 5;                                         //!< [27:31]
		unsigned support_invvpid : 1;                                   //!< [32]
		unsigned reserved8 : 7;                                         //!< [33:39]
		unsigned support_individual_address_invvpid : 1;                //!< [40]
		unsigned support_single_context_invvpid : 1;                    //!< [41]
		unsigned support_all_context_invvpid : 1;                       //!< [42]
		unsigned support_single_context_retaining_globals_invvpid : 1;  //!< [43]
		unsigned reserved9 : 20;                                        //!< [44:63]
	} fields;
};

struct VmxoffContext
{
	ULONG_PTR rflags;
	ULONG_PTR rsp;
	ULONG_PTR rip;
};

union RFLAGS
{
	ULONG64 all;
	struct
	{
		unsigned CF : 1;
		unsigned Unknown_1 : 1;	//Always 1
		unsigned PF : 1;
		unsigned Unknown_2 : 1;	//Always 0
		unsigned AF : 1;
		unsigned Unknown_3 : 1;	//Always 0
		unsigned ZF : 1;
		unsigned SF : 1;
		unsigned TF : 1;
		unsigned IF : 1;
		unsigned DF : 1;
		unsigned OF : 1;
		unsigned TOPL : 2;
		unsigned NT : 1;
		unsigned Unknown_4 : 1;
		unsigned RF : 1;
		unsigned VM : 1;
		unsigned AC : 1;
		unsigned VIF : 1;
		unsigned VIP : 1;
		unsigned ID : 1;
		unsigned Reserved : 10;	//Always 0
		unsigned Reserved_64 : 32;	//Always 0
	}fields;
};

struct Registers64
{
	RFLAGS rflags;
	ULONG_PTR r15;
	ULONG_PTR r14;
	ULONG_PTR r13;
	ULONG_PTR r12;
	ULONG_PTR r11;
	ULONG_PTR r10;
	ULONG_PTR r9;
	ULONG_PTR r8;
	ULONG_PTR rdi;
	ULONG_PTR rsi;
	ULONG_PTR rbp;
	ULONG_PTR rsp;
	ULONG_PTR rbx;
	ULONG_PTR rdx;
	ULONG_PTR rcx;
	ULONG_PTR rax;
};

class VmxCpuContext
{
public:
	VmxCpuContext() = default;
	VmxCpuContext(const VmxCpuContext& other)
	{
		this->vmxonRegion = other.vmxonRegion;
		this->vmcsRegion = other.vmcsRegion;
		this->vmStack = other.vmStack;
		this->vmStackBase = other.vmStackBase;
		this->originalLstar = other.originalLstar;
		this->newKiSystemCall64 = other.newKiSystemCall64;
		this->isVmxEnable = other.isVmxEnable;
	}
	PVOID vmxonRegion;
	PVOID vmcsRegion;
	PVOID vmStack;
	PVOID vmStackBase;

	ULONG64 originalLstar;
	PVOID newKiSystemCall64;
	BOOLEAN isVmxEnable;
};

// VMCS data fields
typedef enum _VMCS_ENCODING
{
	VIRTUAL_PROCESSOR_ID = 0x00000000,  // 16-Bit Control Field
	POSTED_INTERRUPT_NOTIFICATION = 0x00000002,
	EPTP_INDEX = 0x00000004,
	GUEST_ES_SELECTOR = 0x00000800,  // 16-Bit Guest-State Fields
	GUEST_CS_SELECTOR = 0x00000802,
	GUEST_SS_SELECTOR = 0x00000804,
	GUEST_DS_SELECTOR = 0x00000806,
	GUEST_FS_SELECTOR = 0x00000808,
	GUEST_GS_SELECTOR = 0x0000080a,
	GUEST_LDTR_SELECTOR = 0x0000080c,
	GUEST_TR_SELECTOR = 0x0000080e,
	GUEST_INTERRUPT_STATUS = 0x00000810,
	HOST_ES_SELECTOR = 0x00000c00,  // 16-Bit Host-State Fields
	HOST_CS_SELECTOR = 0x00000c02,
	HOST_SS_SELECTOR = 0x00000c04,
	HOST_DS_SELECTOR = 0x00000c06,
	HOST_FS_SELECTOR = 0x00000c08,
	HOST_GS_SELECTOR = 0x00000c0a,
	HOST_TR_SELECTOR = 0x00000c0c,
	IO_BITMAP_A = 0x00002000,  // 64-Bit Control Fields
	IO_BITMAP_A_HIGH = 0x00002001,
	IO_BITMAP_B = 0x00002002,
	IO_BITMAP_B_HIGH = 0x00002003,
	MSR_BITMAP = 0x00002004,
	MSR_BITMAP_HIGH = 0x00002005,
	VM_EXIT_MSR_STORE_ADDR = 0x00002006,
	VM_EXIT_MSR_STORE_ADDR_HIGH = 0x00002007,
	VM_EXIT_MSR_LOAD_ADDR = 0x00002008,
	VM_EXIT_MSR_LOAD_ADDR_HIGH = 0x00002009,
	VM_ENTRY_MSR_LOAD_ADDR = 0x0000200a,
	VM_ENTRY_MSR_LOAD_ADDR_HIGH = 0x0000200b,
	EXECUTIVE_VMCS_POINTER = 0x0000200c,
	EXECUTIVE_VMCS_POINTER_HIGH = 0x0000200d,
	TSC_OFFSET = 0x00002010,
	TSC_OFFSET_HIGH = 0x00002011,
	VIRTUAL_APIC_PAGE_ADDR = 0x00002012,
	VIRTUAL_APIC_PAGE_ADDR_HIGH = 0x00002013,
	APIC_ACCESS_ADDR = 0x00002014,
	APIC_ACCESS_ADDR_HIGH = 0x00002015,
	EPT_POINTER = 0x0000201a,
	EPT_POINTER_HIGH = 0x0000201b,
	EOI_EXIT_BITMAP_0 = 0x0000201c,
	EOI_EXIT_BITMAP_0_HIGH = 0x0000201d,
	EOI_EXIT_BITMAP_1 = 0x0000201e,
	EOI_EXIT_BITMAP_1_HIGH = 0x0000201f,
	EOI_EXIT_BITMAP_2 = 0x00002020,
	EOI_EXIT_BITMAP_2_HIGH = 0x00002021,
	EOI_EXIT_BITMAP_3 = 0x00002022,
	EOI_EXIT_BITMAP_3_HIGH = 0x00002023,
	EPTP_LIST_ADDRESS = 0x00002024,
	EPTP_LIST_ADDRESS_HIGH = 0x00002025,
	VMREAD_BITMAP_ADDRESS = 0x00002026,
	VMREAD_BITMAP_ADDRESS_HIGH = 0x00002027,
	VMWRITE_BITMAP_ADDRESS = 0x00002028,
	VMWRITE_BITMAP_ADDRESS_HIGH = 0x00002029,
	VIRTUALIZATION_EXCEPTION_INFO_ADDDRESS = 0x0000202a,
	VIRTUALIZATION_EXCEPTION_INFO_ADDDRESS_HIGH = 0x0000202b,
	XSS_EXITING_BITMAP = 0x0000202c,
	XSS_EXITING_BITMAP_HIGH = 0x0000202d,
	GUEST_PHYSICAL_ADDRESS = 0x00002400,  // 64-Bit Read-Only Data Field
	GUEST_PHYSICAL_ADDRESS_HIGH = 0x00002401,
	VMCS_LINK_POINTER = 0x00002800,  // 64-Bit Guest-State Fields
	VMCS_LINK_POINTER_HIGH = 0x00002801,
	GUEST_IA32_DEBUGCTL = 0x00002802,
	GUEST_IA32_DEBUGCTL_HIGH = 0x00002803,
	GUEST_IA32_PAT = 0x00002804,
	GUEST_IA32_PAT_HIGH = 0x00002805,
	GUEST_IA32_EFER = 0x00002806,
	GUEST_IA32_EFER_HIGH = 0x00002807,
	GUEST_IA32_PERF_GLOBAL_CTRL = 0x00002808,
	GUEST_IA32_PERF_GLOBAL_CTRL_HIGH = 0x00002809,
	GUEST_PDPTR0 = 0x0000280a,
	GUEST_PDPTR0_HIGH = 0x0000280b,
	GUEST_PDPTR1 = 0x0000280c,
	GUEST_PDPTR1_HIGH = 0x0000280d,
	GUEST_PDPTR2 = 0x0000280e,
	GUEST_PDPTR2_HIGH = 0x0000280f,
	GUEST_PDPTR3 = 0x00002810,
	GUEST_PDPTR3_HIGH = 0x00002811,
	HOST_IA32_PAT = 0x00002c00,  // 64-Bit Host-State Fields
	HOST_IA32_PAT_HIGH = 0x00002c01,
	HOST_IA32_EFER = 0x00002c02,
	HOST_IA32_EFER_HIGH = 0x00002c03,
	HOST_IA32_PERF_GLOBAL_CTRL = 0x00002c04,
	HOST_IA32_PERF_GLOBAL_CTRL_HIGH = 0x00002c05,
	PIN_BASED_VM_EXEC_CONTROL = 0x00004000,  // 32-Bit Control Fields
	CPU_BASED_VM_EXEC_CONTROL = 0x00004002,
	EXCEPTION_BITMAP = 0x00004004,
	PAGE_FAULT_ERROR_CODE_MASK = 0x00004006,
	PAGE_FAULT_ERROR_CODE_MATCH = 0x00004008,
	CR3_TARGET_COUNT = 0x0000400a,
	VM_EXIT_CONTROLS = 0x0000400c,
	VM_EXIT_MSR_STORE_COUNT = 0x0000400e,
	VM_EXIT_MSR_LOAD_COUNT = 0x00004010,
	VM_ENTRY_CONTROLS = 0x00004012,
	VM_ENTRY_MSR_LOAD_COUNT = 0x00004014,
	VM_ENTRY_INTR_INFO_FIELD = 0x00004016,
	VM_ENTRY_EXCEPTION_ERROR_CODE = 0x00004018,
	VM_ENTRY_INSTRUCTION_LEN = 0x0000401a,
	TPR_THRESHOLD = 0x0000401c,
	SECONDARY_VM_EXEC_CONTROL = 0x0000401e,
	PLE_GAP = 0x00004020,
	PLE_WINDOW = 0x00004022,
	VM_INSTRUCTION_ERROR = 0x00004400,  // 32-Bit Read-Only Data Fields
	VM_EXIT_REASON = 0x00004402,
	VM_EXIT_INTR_INFO = 0x00004404,
	VM_EXIT_INTR_ERROR_CODE = 0x00004406,
	IDT_VECTORING_INFO_FIELD = 0x00004408,
	IDT_VECTORING_ERROR_CODE = 0x0000440a,
	VM_EXIT_INSTRUCTION_LEN = 0x0000440c,
	VMX_INSTRUCTION_INFO = 0x0000440e,
	GUEST_ES_LIMIT = 0x00004800,  // 32-Bit Guest-State Fields
	GUEST_CS_LIMIT = 0x00004802,
	GUEST_SS_LIMIT = 0x00004804,
	GUEST_DS_LIMIT = 0x00004806,
	GUEST_FS_LIMIT = 0x00004808,
	GUEST_GS_LIMIT = 0x0000480a,
	GUEST_LDTR_LIMIT = 0x0000480c,
	GUEST_TR_LIMIT = 0x0000480e,
	GUEST_GDTR_LIMIT = 0x00004810,
	GUEST_IDTR_LIMIT = 0x00004812,
	GUEST_ES_AR_BYTES = 0x00004814,
	GUEST_CS_AR_BYTES = 0x00004816,
	GUEST_SS_AR_BYTES = 0x00004818,
	GUEST_DS_AR_BYTES = 0x0000481a,
	GUEST_FS_AR_BYTES = 0x0000481c,
	GUEST_GS_AR_BYTES = 0x0000481e,
	GUEST_LDTR_AR_BYTES = 0x00004820,
	GUEST_TR_AR_BYTES = 0x00004822,
	GUEST_INTERRUPTIBILITY_INFO = 0x00004824,
	GUEST_ACTIVITY_STATE = 0x00004826,
	GUEST_SMBASE = 0x00004828,
	GUEST_SYSENTER_CS = 0x0000482a,
	VMX_PREEMPTION_TIMER_VALUE = 0x0000482e,
	HOST_IA32_SYSENTER_CS = 0x00004c00,  // 32-Bit Host-State Field
	CR0_GUEST_HOST_MASK = 0x00006000,    // Natural-Width Control Fields
	CR4_GUEST_HOST_MASK = 0x00006002,
	CR0_READ_SHADOW = 0x00006004,
	CR4_READ_SHADOW = 0x00006006,
	CR3_TARGET_VALUE0 = 0x00006008,
	CR3_TARGET_VALUE1 = 0x0000600a,
	CR3_TARGET_VALUE2 = 0x0000600c,
	CR3_TARGET_VALUE3 = 0x0000600e,
	EXIT_QUALIFICATION = 0x00006400,  // Natural-Width Read-Only Data Fields
	IO_RCX = 0x00006402,
	IO_RSI = 0x00006404,
	IO_RDI = 0x00006406,
	IO_RIP = 0x00006408,
	GUEST_LINEAR_ADDRESS = 0x0000640a,
	GUEST_CR0 = 0x00006800,  // Natural-Width Guest-State Fields
	GUEST_CR3 = 0x00006802,
	GUEST_CR4 = 0x00006804,
	GUEST_ES_BASE = 0x00006806,
	GUEST_CS_BASE = 0x00006808,
	GUEST_SS_BASE = 0x0000680a,
	GUEST_DS_BASE = 0x0000680c,
	GUEST_FS_BASE = 0x0000680e,
	GUEST_GS_BASE = 0x00006810,
	GUEST_LDTR_BASE = 0x00006812,
	GUEST_TR_BASE = 0x00006814,
	GUEST_GDTR_BASE = 0x00006816,
	GUEST_IDTR_BASE = 0x00006818,
	GUEST_DR7 = 0x0000681a,
	GUEST_RSP = 0x0000681c,
	GUEST_RIP = 0x0000681e,
	GUEST_RFLAGS = 0x00006820,
	GUEST_PENDING_DBG_EXCEPTIONS = 0x00006822,
	GUEST_SYSENTER_ESP = 0x00006824,
	GUEST_SYSENTER_EIP = 0x00006826,
	HOST_CR0 = 0x00006c00,  // Natural-Width Host-State Fields
	HOST_CR3 = 0x00006c02,
	HOST_CR4 = 0x00006c04,
	HOST_FS_BASE = 0x00006c06,
	HOST_GS_BASE = 0x00006c08,
	HOST_TR_BASE = 0x00006c0a,
	HOST_GDTR_BASE = 0x00006c0c,
	HOST_IDTR_BASE = 0x00006c0e,
	HOST_IA32_SYSENTER_ESP = 0x00006c10,
	HOST_IA32_SYSENTER_EIP = 0x00006c12,
	HOST_RSP = 0x00006c14,
	HOST_RIP = 0x00006c16
} VMCS_ENCODING;

union SegmentAttributes
{
	USHORT UCHARs;
	struct
	{
		USHORT type : 4;              /* 0;  Bit 40-43 */
		USHORT s : 1;                 /* 4;  Bit 44 */
		USHORT dpl : 2;               /* 5;  Bit 45-46 */
		USHORT p : 1;                 /* 7;  Bit 47 */
		// gap!       
		USHORT avl : 1;               /* 8;  Bit 52 */
		USHORT l : 1;                 /* 9;  Bit 53 */
		USHORT db : 1;                /* 10; Bit 54 */
		USHORT g : 1;                 /* 11; Bit 55 */
		USHORT Gap : 4;
	} fields;
};

struct SegmentSelector
{
	USHORT sel;
	SegmentAttributes attributes;
	ULONG limit;
	ULONG64 base;
};

typedef struct
{
	USHORT limit0;
	USHORT base0;
	UCHAR  base1;
	UCHAR  attr0;
	UCHAR  limit1attr1;
	UCHAR  base2;
} SegmentDescriptor2;

#define LA_ACCESSED		0x01
#define LA_READABLE		0x02    // for code segments
#define LA_WRITABLE		0x02    // for data segments
#define LA_CONFORMING	0x04    // for code segments
#define LA_EXPANDDOWN	0x04    // for data segments
#define LA_CODE			0x08
#define LA_STANDARD		0x10
#define LA_DPL_0		0x00
#define LA_DPL_1		0x20
#define LA_DPL_2		0x40
#define LA_DPL_3		0x60
#define LA_PRESENT		0x80

#define LA_LDT64		0x02
#define LA_ATSS64		0x09
#define LA_BTSS64		0x0b
#define LA_CALLGATE64	0x0c
#define LA_INTGATE64	0x0e
#define LA_TRAPGATE64	0x0f

#define HA_AVAILABLE	0x01
#define HA_LONG			0x02
#define HA_DB			0x04
#define HA_GRANULARITY	0x08

#define P_PRESENT			0x01
#define P_WRITABLE			0x02
#define P_USERMODE			0x04
#define P_WRITETHROUGH		0x08
#define P_CACHE_DISABLED	0x10
#define P_ACCESSED			0x20
#define P_DIRTY				0x40
#define P_LARGE				0x80
#define P_GLOBAL			0x100

#define	PML4_BASE	0xFFFFF6FB7DBED000 //和windows内核的四个常量对应
#define	PDP_BASE	0xFFFFF6FB7DA00000 //#define PXE_BASE 0xFFFFF6FB7DBED000UI64
#define	PD_BASE		0xFFFFF6FB40000000 //#define PPE_BASE 0xFFFFF6FB7DA00000UI64
#define	PT_BASE		0xFFFFF68000000000 //#define PDE_BASE 0xFFFFF6FB40000000UI64
//#define PTE_BASE 0xFFFFF68000000000UI64

#pragma pack(1)
// 默认8字节对齐,会导致lgdt和lidt指令无法成功执行
struct Idtr
{
	USHORT limit;
	ULONG_PTR base;
};

struct Gdtr
{
	USHORT limit;
	ULONG_PTR base;
};
#pragma pack()

union VmxPinBasedControls
{
	ULONG32 all;
	struct
	{
		ULONG32 externalInterruptExiting : 1;    // [0]
		ULONG32 reserved1 : 2;                   // [1-2]
		ULONG32 nmiExiting : 1;                  // [3]
		ULONG32 reserved2 : 1;                   // [4]
		ULONG32 virtualNMIs : 1;                 // [5]
		ULONG32 activateVMXPreemptionTimer : 1;  // [6]
		ULONG32 processPostedInterrupts : 1;     // [7]
	} fields;
};

union VmxCpuBasedControls
{
	ULONG32 all;
	struct
	{
		ULONG32 reserved1 : 2;                 // [0-1]
		ULONG32 interruptWindowExiting : 1;    // [2]
		ULONG32 useTSCOffseting : 1;           // [3]
		ULONG32 reserved2 : 3;                 // [4-6]
		ULONG32 hLTExiting : 1;                // [7]
		ULONG32 reserved3 : 1;                 // [8]
		ULONG32 ibvkogGExiting : 1;             // [9]
		ULONG32 mwaitExiting : 1;              // [10]
		ULONG32 rdpmcExiting : 1;              // [11]
		ULONG32 rdtsCExiting : 1;              // [12]
		ULONG32 reserved4 : 2;                 // [13-14]
		ULONG32 cr3LoadExiting : 1;            // [15]
		ULONG32 cr3StoreExiting : 1;           // [16]
		ULONG32 reserved5 : 2;                 // [17-18]
		ULONG32 cr8LoadExiting : 1;            // [19]
		ULONG32 cr8StoreExiting : 1;           // [20]
		ULONG32 useTPRShadowExiting : 1;       // [21]
		ULONG32 nmiWindowExiting : 1;          // [22]
		ULONG32 movDRExiting : 1;              // [23]
		ULONG32 unconditionalIOExiting : 1;    // [24]
		ULONG32 useIOBitmaps : 1;              // [25]
		ULONG32 reserved6 : 1;                 // [26]
		ULONG32 monitorTrapFlag : 1;           // [27]
		ULONG32 useMSRBitmaps : 1;             // [28]
		ULONG32 monitorExiting : 1;            // [29]
		ULONG32 pauseExiting : 1;              // [30]
		ULONG32 activateSecondaryControl : 1;  // [31]
	} fields;
};

union VmxVmEnterControls
{
	ULONG32 all;
	struct
	{
		ULONG32 reserved1 : 2;                       // [0-1]
		ULONG32 loadDebugControls : 1;               // [2]
		ULONG32 reserved2 : 6;                       // [3-8]
		ULONG32 ia32eModeGuest : 1;                  // [9]
		ULONG32 entryToSMM : 1;                      // [10]
		ULONG32 deactivateDualMonitorTreatment : 1;  // [11]
		ULONG32 reserved3 : 1;                       // [12]
		ULONG32 loadIA32_PERF_GLOBAL_CTRL : 1;       // [13]
		ULONG32 loadIA32_PAT : 1;                    // [14]
		ULONG32 loadIA32_EFER : 1;                   // [15]
	} fields;
};

union VmxVmExitControls
{
	ULONG32 all;
	struct
	{
		ULONG32 reserved1 : 2;                    // [0-1]
		ULONG32 saveDebugControls : 1;            // [2]
		ULONG32 reserved2 : 6;                    // [3-8]
		ULONG32 hostAddressSpaceSize : 1;         // [9]
		ULONG32 reserved3 : 2;                    // [10-11]
		ULONG32 loadIA32_PERF_GLOBAL_CTRL : 1;    // [12]
		ULONG32 reserved4 : 2;                    // [13-14]
		ULONG32 acknowledgeInterruptOnExit : 1;   // [15]
		ULONG32 reserved5 : 2;                    // [16-17]
		ULONG32 saveIA32_PAT : 1;                 // [18]
		ULONG32 loadIA32_PAT : 1;                 // [19]
		ULONG32 saveIA32_EFER : 1;                // [20]
		ULONG32 loadIA32_EFER : 1;                // [21]
		ULONG32 saveVMXPreemptionTimerValue : 1;  // [22]
	} fields;
};

union VmxSecondaryCpuBasedControls
{
	ULONG32 all;
	struct
	{
		ULONG32 virtualizeAPICAccesses : 1;      // [0]
		ULONG32 enableEPT : 1;                   // [1]
		ULONG32 descriptorTableExiting : 1;      // [2]
		ULONG32 enableRDTSCP : 1;                // [3]
		ULONG32 virtualizeX2APICMode : 1;        // [4]
		ULONG32 enableVPID : 1;                  // [5]
		ULONG32 wBINVDExiting : 1;               // [6]
		ULONG32 unrestrictedGuest : 1;           // [7]
		ULONG32 apicRegisterVirtualization : 1;  // [8]
		ULONG32 virtualInterruptDelivery : 1;    // [9]
		ULONG32 pAUSELoopExiting : 1;            // [10]
		ULONG32 rdrandExiting : 1;               // [11]
		ULONG32 enableINVPCID : 1;               // [12]
		ULONG32 enableVMFunctions : 1;           // [13]
		ULONG32 vmcsShadowing : 1;               // [14]
		ULONG32 reserved1 : 1;                   // [15]
		ULONG32 rdseedExiting : 1;               // [16]
		ULONG32 reserved2 : 1;                   // [17]
		ULONG32 eptViolation : 1;                // [18]
		ULONG32 reserved3 : 1;                   // [19]
		ULONG32 enableXSAVESXSTORS : 1;          // [20]
	} fields;
};

union VmxRegmentDescriptorAccessRight
{
	unsigned int all;
	struct
	{
		unsigned type : 4;        //!< [0:3]
		unsigned system : 1;      //!< [4]
		unsigned dpl : 2;         //!< [5:6]
		unsigned present : 1;     //!< [7]
		unsigned reserved1 : 4;   //!< [8:11]
		unsigned avl : 1;         //!< [12]
		unsigned l : 1;           //!< [13] Reserved (except for CS) 64-bit mode
		unsigned db : 1;          //!< [14]
		unsigned gran : 1;        //!< [15]
		unsigned unusable : 1;    //!< [16] Segment unusable
		unsigned reserved2 : 15;  //!< [17:31]
	} fields;
};

union VmExitInformation
{
	unsigned int all;
	struct
	{
		unsigned short reason;                     //!< [0:15] VmxExitReason
		unsigned short reserved1 : 12;             //!< [16:30]
		unsigned short pending_mtf_vm_exit : 1;    //!< [28]
		unsigned short vm_exit_from_vmx_root : 1;  //!< [29]
		unsigned short reserved2 : 1;              //!< [30]
		unsigned short vm_entry_failure : 1;       //!< [31]
	} fields;
};

enum VmExitReason
{
	ExceptionNmi = 0,    // Exception or non-maskable interrupt (NMI).
	ExternalInterrupt = 1,    // External interrupt.
	TripleFault = 2,    // Triple fault.
	Init = 3,    // INIT signal.
	Sipi = 4,    // Start-up IPI (SIPI).
	IoSmi = 5,    // I/O system-management interrupt (SMI).
	OtherSmi = 6,    // Other SMI.
	PendingInterrupt = 7,    // Interrupt window exiting.
	NmiWindow = 8,    // NMI window exiting.
	TaskSwitch = 9,    // Task switch.
	Cpuid = 10,   // Guest software attempted to execute CPUID.
	GetSec = 11,   // Guest software attempted to execute GETSEC.
	Hlt = 12,   // Guest software attempted to execute HLT.
	Invd = 13,   // Guest software attempted to execute INVD.
	Invlpg = 14,   // Guest software attempted to execute INVLPG.
	Rdpmc = 15,   // Guest software attempted to execute RDPMC.
	Rdtsc = 16,   // Guest software attempted to execute RDTSC.
	Rsm = 17,   // Guest software attempted to execute RSM in SMM.
	Vmcall = 18,   // Guest software executed VMCALL.
	Vmclear = 19,   // Guest software executed VMCLEAR.
	Vmlaunch = 20,   // Guest software executed VMLAUNCH.
	Vmptrld = 21,   // Guest software executed VMPTRLD.
	Vmptrst = 22,   // Guest software executed VMPTRST.
	Vmread = 23,   // Guest software executed VMREAD.
	Vmresume = 24,   // Guest software executed VMRESUME.
	Vmrite = 25,   // Guest software executed VMWRITE.
	Vmxoff = 26,   // Guest software executed VMXOFF.
	Vmxon = 27,   // Guest software executed VMXON.
	CrAccess = 28,   // Control-register accesses.
	DrAccess = 29,   // Debug-register accesses.
	IoInstruction = 30,   // I/O instruction.
	MsrRead = 31,   // RDMSR. Guest software attempted to execute RDMSR.
	MsrWrite = 32,   // WRMSR. Guest software attempted to execute WRMSR.
	InvalidGuestState = 33,   // VM-entry failure due to invalid guest state.
	MsrLoading = 34,   // VM-entry failure due to MSR loading.
	Reserved35 = 35,   // Reserved
	MwaitInstruction = 36,   // Guest software executed MWAIT.
	Mtf = 37,   // VM-exit due to monitor trap flag.
	Reserved38 = 38,   // Reserved
	MonitorInstruction = 39,   // Guest software attempted to execute MONITOR.
	PauseInstruction = 40,   // Guest software attempted to execute PAUSE.
	MachineCheck = 41,   // VM-entry failure due to machine-check.
	Reserved42 = 42,   // Reserved
	TprBelowThreshold = 43,   // TPR below threshold. Guest software executed MOV to CR8.
	ApicAccess = 44,   // APIC access. Guest software attempted to access memory at a physical address on the APIC-access page.
	VirtualizedEio = 45,   // EOI virtualization was performed for a virtual interrupt whose vector indexed a bit set in the EOIexit bitmap
	XdtrAccess = 46,   // Guest software attempted to execute LGDT, LIDT, SGDT, or SIDT.
	TrAccess = 47,   // Guest software attempted to execute LLDT, LTR, SLDT, or STR.
	EptViolation = 48,   // An attempt to access memory with a guest-physical address was disallowed by the configuration of the EPT paging structures.
	EptMisconfig = 49,   // An attempt to access memory with a guest-physical address encountered a misconfigured EPT paging-structure entry.
	InvEpt = 50,   // Guest software attempted to execute INVEPT.
	Rdtscp = 51,   // Guest software attempted to execute RDTSCP.
	PreemptTimer = 52,   // VMX-preemption timer expired. The preemption timer counted down to zero.
	Invvpid = 53,   // Guest software attempted to execute INVVPID.
	Wbinvd = 54,   // Guest software attempted to execute WBINVD
	Xsetbv = 55,   // Guest software attempted to execute XSETBV.
	ApicWrite = 56,   // Guest completed write to virtual-APIC.
	Rdrand = 57,   // Guest software attempted to execute RDRAND.
	Invpcid = 58,   // Guest software attempted to execute INVPCID.
	Vmfunc = 59,   // Guest software attempted to execute VMFUNC.
	Reserved60 = 60,   // Reserved
	Rdseed = 61,   // Guest software attempted to executed RDSEED and exiting was enabled.
	Reserved62 = 62,   // Reserved
	Xsaves = 63,   // Guest software attempted to executed XSAVES and exiting was enabled.
	Xrstors = 64,   // Guest software attempted to executed XRSTORS and exiting was enabled.

	MaxGuestVmexit = 65
};

enum AccessType
{
	MOV_TO_CR = 0,
	MOV_FROM_CR = 1,
	CLTS,
	LMSW
};

union ExitQualification
{
	ULONG_PTR all;

	// Task Switch 
	// See:Table  27-2
	struct
	{

	}TaskSwitch;

	// Control-Registers Accesses
	// See:Table 27-3
	struct
	{
		ULONG_PTR registerNumber : 4;   //!< [0:3]
		AccessType accessType : 2;        //!< [4:5]
		ULONG_PTR lmswOperandType : 1;  //!< [6]
		ULONG_PTR reserved1 : 1;          //!< [7]
		ULONG_PTR generalRegister : 4;        //!< [8:11]
		ULONG_PTR reserved2 : 4;          //!< [12:15]
		ULONG_PTR lmswSourceData : 16;  //!< [16:31]
		ULONG_PTR reserved3 : 32;         //!< [32:63]
	} crAccess;

	// Mov Debug-Regsters
	// See:Table 27-4
	struct
	{
		ULONG_PTR registerNumber : 3;  //!< [0:2] 
		ULONG_PTR reserved1 : 1;        //!< [3]
		ULONG_PTR direction : 1;        //!< [4] (0=mov to dr,1=mov from dr)
		ULONG_PTR reserved2 : 3;        //!< [5:7]
		ULONG_PTR generalRegister : 4;      //!< [8:11]
		ULONG_PTR reserved3 : 20;       //!<
		ULONG_PTR reserved4 : 32;       //!< [12:63]
	}  drAccess;

	// I/O Instructions
	// See:Table 27-5
	struct
	{
		ULONG_PTR accessSize : 3;      //!< [0:2] (0=1byte,1=2byte,2=4byte)
		ULONG_PTR direction : 1;           //!< [3] (0=out,1=in)
		ULONG_PTR stringInstruction : 1;  //!< [4] (0=not string,1=string)
		ULONG_PTR repPrefixed : 1;        //!< [5] (0=not rep,1=rep)
		ULONG_PTR operandEncoding : 1;    //!< [6] (0=dx,1=immediate)
		ULONG_PTR reserved1 : 9;           //!< [7:15] 
		ULONG_PTR portNumber : 16;        //!< [16:31]
	} ioInst;

	// APIC-Access
	// See:Table 27-6
	struct
	{

	}apicAccess;

	// EPT Violations
	// See:Table 27-7
	struct
	{
		ULONG64 readAccess : 1;                   //!< [0]
		ULONG64 writeAccess : 1;                  //!< [1]
		ULONG64 executeAccess : 1;                //!< [2]
		ULONG64 eptReadable : 1;                  //!< [3]
		ULONG64 eptWriteable : 1;                 //!< [4]
		ULONG64 eptExecutable : 1;                //!< [5]
		ULONG64 reserved1 : 1;					 //!< [6]
		ULONG64 validGuestLinearAddress : 1;    //!< [7]
		ULONG64 causedByTranslation : 1;         //!< [8]
		ULONG64 reserved2 : 3;					//!< [9:11]
		ULONG64 nmiUnblocking : 1;                //!< [12]
		ULONG64 reserved3 : 51;					//!< [13:63]
	} eptViolation;
};

