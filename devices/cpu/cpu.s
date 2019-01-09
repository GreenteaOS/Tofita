.global keyboardHandler
.global mouseHandler

.global fallback_handler0
.global fallback_handler1
.global fallback_handler2
.global fallback_handler3
.global fallback_handler4
.global fallback_handler5
.global fallback_handler6
.global fallback_handler7
.global fallback_handler8
.global fallback_handler9
.global fallback_handler10
.global fallback_handler11
.global fallback_handler12
.global fallback_handler13
.global fallback_handler14
.global fallback_handler15

# MACRO

.macro SAVE_REGS
	push    %gs
	push    %fs
	push    %rbp
	push    %r15
	push    %r14
	push    %r13
	push    %r12
	push    %r11
	push    %r10
	push    %r9
	push    %r8
	push    %rdi
	push    %rsi
	push    %rdx
	push    %rcx
	push    %rbx
	push    %rax
	xor     %rax, %rax
	mov     %ax, %ds
	push    %rax
	mov     %ax, %es
	push    %rax
.endm

.macro RESTORE_REGS
	pop     %rax
	mov     %es, %ax
	pop     %rax
	mov     %ds, %ax
	pop     %rax
	pop     %rbx
	pop     %rcx
	pop     %rdx
	pop     %rsi
	pop     %rdi
	pop     %r8
	pop     %r9
	pop     %r10
	pop     %r11
	pop     %r12
	pop     %r13
	pop     %r14
	pop     %r15
	pop     %rbp
	pop     %fs
	pop     %gs
.endm

# MACRO END

keyboardHandler:
	SAVE_REGS
	call    handleKeyboard
	RESTORE_REGS
	iretq

mouseHandler:
	SAVE_REGS
	call    handleMouse
	RESTORE_REGS
	iretq

.macro FALLBACK n
	fallback_handler\n:
		SAVE_REGS
		call    handle_fallback\n
		RESTORE_REGS
		iretq
.endm

FALLBACK 0
FALLBACK 1
FALLBACK 2
FALLBACK 3
FALLBACK 4
FALLBACK 5
FALLBACK 6
FALLBACK 7
FALLBACK 8
FALLBACK 9
FALLBACK 10
FALLBACK 11
FALLBACK 12
FALLBACK 13
FALLBACK 14
FALLBACK 15

# fallback_handler0:
# 	call    handle_fallback0
# 	iretq
#
# fallback_handler1:
# 	call    handle_fallback1
# 	iretq
#
# fallback_handler2:
# 	call    handle_fallback2
# 	iretq
#
# fallback_handler3:
# 	call    handle_fallback3
# 	iretq
#
# fallback_handler4:
# 	call    handle_fallback4
# 	iretq
#
# fallback_handler5:
# 	call    handle_fallback5
# 	iretq
#
# fallback_handler6:
# 	call    handle_fallback6
# 	iretq
#
# fallback_handler7:
# 	call    handle_fallback7
# 	iretq
#
# fallback_handler8:
# 	call    handle_fallback8
# 	iretq
#
# fallback_handler9:
# 	call    handle_fallback9
# 	iretq
#
# fallback_handler10:
# 	call    handle_fallback10
# 	iretq
#
# fallback_handler11:
# 	call    handle_fallback11
# 	iretq
#
# fallback_handler12:
# 	call    handle_fallback12
# 	iretq
#
# fallback_handler13:
# 	call    handle_fallback13
# 	iretq
#
# fallback_handler14:
# 	call    handle_fallback14
# 	iretq
#
# fallback_handler15:
# 	call    handle_fallback15
# 	iretq
#

# extern g_gdtr
#
# global gdt_write
# gdt_write:
# 	lgdt [rel g_gdtr]
# 	mov %ax, %si ; second arg is data segment
# 	mov %ds, %ax
# 	mov %es, %ax
# 	mov %fs, %ax
# 	mov %gs, %ax
# 	mov %ss, %ax
# 	push qword %rdi ; first arg is code segment
# 	lea %rax, [rel .next]
# 	push %rax
# 	o64 retf
# .next:
# 	ltr %dx
# 	ret
