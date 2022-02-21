EXTERN launchVmx : PROC
EXTERN vmExitEntryPoint : PROC


.data
vmmEntryRcx dq 0
vmmEntryRdx dq 0


PUSHAQ MACRO
push    rax
push    rcx
push    rdx
push    rbx
push - 1; dummy for rsp
push    rbp
push    rsi
push    rdi
push    r8
push    r9
push    r10
push    r11
push    r12
push    r13
push    r14
push    r15
ENDM

; Loads all general purpose registers from the stack
POPAQ MACRO
pop     r15
pop     r14
pop     r13
pop     r12
pop     r11
pop     r10
pop     r9
pop     r8
pop     rdi
pop     rsi
pop     rbp
add     rsp, 8; dummy for rsp
pop     rbx
pop     rdx
pop     rcx
pop     rax
ENDM

.data

.code


; ����Vmx
__vmlaunch PROC
	pushfq ;����flags
	PUSHAQ ;����ͨ�üĴ���
	mov rax, rcx			; rcx=VmxManager::launchVmx, rdx=vcpu
	mov rcx, rdx			; arg0:vcpu
	mov rdx, rsp			; arg1:guest rsp
	mov r8,	VmLaunchToGuest	; arg2:guest rip:VmLaunchToGuest �ָ���guest����ִ�е�rip
	sub rsp, 200h			; ����ջ������һЩ�ֲ����������㹻��ջ�ռ�
	call rax				; ����VmxManager::launchVmx
	; int 3
	add rsp, 200h			; �����е�������˵��vmlaunchʧ����,�����ֶ��ָ�ջ�ռ�
	POPAQ
	popfq
	mov rax, 0C0000001h		; STATUS_UNSUCCESSFUL == 0xC0000001
	ret
VmLaunchToGuest :
	POPAQ ;���ڻָ���guest��,vmm���ȡ���ָ�GUEST_RSP,�����������ֶ�ƽ������������ջ��
	popfq
	xor rax, rax			; STATUS_SUCESS == 0
	ret
__vmlaunch ENDP


; Vmm��ڵ�
__vmm_entry_point PROC
	; int 3
	;mov vmmEntryRcx, rcx
	;mov vmmEntryRdx, rdx
	PUSHAQ					;��ͨ�üĴ���ѹջ(rsp, rip, rflags�ȼĴ����ᱻ������guest state area��)
	pushfq					;����rflags
	;call currentCpuContext
	;mov rcx, rax
	mov rcx, rsp			; ��rsp��������, ��������һ��ѹ��ջ�ļĴ���
	sub rsp, 200h
	call vmExitEntryPoint
	add rsp, 200h
	test rax, rax
	jz ExitVmx
	popfq
	POPAQ
	sti
	vmresume
	jmp ErrorHandler		 ;vmresume�������guestRip, ��������²���ִ������ָ��

ExitVmx:
    
	popfq
	POPAQ
	vmxoff						;ִ�����rax=rflags,rdx=ԭ����ջ,rcx=����vmexit����һ��ָ���ַ
	jz ErrorHandler             ; if VmfailInvalid(ZF) jmp  see 30.2 Operation sections for the VMX instructions
    jc ErrorHandler             ; if VmfailValid(CF) jmp
	                            ; handleVmcall rcx=return address, rdx=guest rsp, r8=guest rflags
	push r8                     
	popfq						; r8=rflags
	mov rsp, rdx				; �ָ�ִ��vmcallǰ��rsp
	push rcx 					; rcxΪreturn address, Ϊvmcall����һ��ָ��ĵ�ַ
	ret
ErrorHandler:
	int 3
__vmm_entry_point ENDP

;__vmxoff PROC
;popfq
;POPAQ
;vmxoff
;push[rdx]
;popfq; rflags
;mov rsp, [rdx + 8]; rsp
;jmp qword ptr[rdx + 10h]; rip
;__vmxoff ENDP


__readcs PROC
mov		rax, cs
ret
__readcs ENDP

__readds PROC
mov		rax, ds
ret
__readds ENDP

__reades PROC
mov		rax, es
ret
__reades ENDP

__readss PROC
mov		rax, ss
ret
__readss ENDP

__readfs PROC
mov		rax, fs
ret
__readfs ENDP

__readgs PROC
mov		rax, gs
ret
__readgs ENDP

__readldtr PROC
sldt	rax
ret
__readldtr ENDP

__readtr PROC
str	rax
ret
__readtr ENDP

__getidtbase PROC
LOCAL	idtr[10]:BYTE
sidt	idtr
mov		rax, qword PTR idtr[2]
ret
__getidtbase ENDP

__getidtlimit PROC

LOCAL	idtr[10]:BYTE
xor rax, rax
sidt	idtr
mov		ax, WORD PTR idtr[0]
ret
__getidtlimit ENDP

__getgdtbase PROC
LOCAL	gdtr[10]:BYTE

sgdt	gdtr
mov		rax, qword PTR gdtr[2]
ret
__getgdtbase ENDP

__getgdtlimit PROC
LOCAL	gdtr[10]:BYTE
xor rax, rax
sgdt	gdtr
mov		ax, WORD PTR gdtr[0]
ret
__getgdtlimit ENDP

__load_access_rights_byte PROC
lar rax, rcx
ret
__load_access_rights_byte ENDP

__invd PROC
invd
ret
__invd ENDP


__vmcall PROC
vmcall
ret
__vmcall ENDP

__lgdt PROC
lgdt fword ptr[rcx]
ret
__lgdt ENDP



__svreg PROC
push rax
pushfq
pop rax
mov[rcx], rax; rflags
pop rax

mov[rcx + 8h], r15
mov[rcx + 10h], r14
mov[rcx + 18h], r13
mov[rcx + 20h], r12
mov[rcx + 28h], r11
mov[rcx + 30h], r10
mov[rcx + 38h], r9
mov[rcx + 40h], r8
mov[rcx + 48h], rdi
mov[rcx + 50h], rsi
mov[rcx + 58h], rbp
mov[rcx + 60h], rsp
mov[rcx + 68h], rbx
mov[rcx + 70h], rdx
mov[rcx + 78h], rcx
mov[rcx + 80h], rax
ret
__svreg ENDP

__ldreg PROC
push rax
mov rax, [rcx]; rflags
push rax
popfq
pop rax
mov r15, [rcx + 8h]
mov r14, [rcx + 10h]
mov r13, [rcx + 18h]
mov r12, [rcx + 20h]
mov r11, [rcx + 28h]
mov r10, [rcx + 30h]
mov r9, [rcx + 38h]
mov r8, [rcx + 40h]
mov rdi, [rcx + 48h]
mov rsi, [rcx + 50h]
mov rbp, [rcx + 58h]
mov rsp, [rcx + 60h]
mov rbx, [rcx + 68h]
mov rdx, [rcx + 70h]
mov rcx, [rcx + 78h]
mov rax, [rcx + 80h]
ret
__ldreg ENDP


UtilEnterCriticalSection PROC
WaitLab :
mov eax, 1
lock xadd[rcx], eax
cmp eax, 0
jz EndLab
lock dec dword ptr[rcx]
jmp WaitLab
EndLab :
ret
UtilEnterCriticalSection ENDP

UtilDeleteCriticalSection PROC
lock dec dword ptr[rcx]
UtilDeleteCriticalSection ENDP

END