	.section	__TEXT,__text,regular,pure_instructions
	.build_version macos, 12, 0
	.intel_syntax noprefix
                                        ## Start of file scope inline assembly
	.globl	__LLVM_StackMaps

                                        ## End of file scope inline assembly
	.globl	_gc.safepoint_poll              ## -- Begin function gc.safepoint_poll
	.p2align	4, 0x90
_gc.safepoint_poll:                     ## @gc.safepoint_poll
	.cfi_startproc
## %bb.0:
	push	rax
	.cfi_def_cfa_offset 16
	call	_runtime_gc_poll
	pop	rax
	ret
	.cfi_endproc
                                        ## -- End function
	.globl	_print                          ## -- Begin function print
	.p2align	4, 0x90
_print:                                 ## @print
	.cfi_startproc
## %bb.0:                               ## %entry
	sub	rsp, 24
	.cfi_def_cfa_offset 32
	mov	rax, qword ptr [rdi]
	mov	rcx, qword ptr [rdi + 8]
	mov	qword ptr [rsp + 8], rcx
	mov	qword ptr [rsp + 16], rax
	call	_runtime_gc_poll
Ltmp0:
	mov	rdi, qword ptr [rsp + 16]
	call	_runtime_inspect_ptr
Ltmp1:
	mov	rdi, qword ptr [rsp + 8]
	call	_runtime_inspect_ptr
Ltmp2:
	add	rsp, 24
	ret
	.cfi_endproc
                                        ## -- End function
	.globl	_do_nothing                     ## -- Begin function do_nothing
	.p2align	4, 0x90
_do_nothing:                            ## @do_nothing
	.cfi_startproc
## %bb.0:                               ## %entry
	push	rax
	.cfi_def_cfa_offset 16
	call	_runtime_gc_poll
Ltmp3:
	pop	rax
	ret
	.cfi_endproc
                                        ## -- End function
	.globl	_test                           ## -- Begin function test
	.p2align	4, 0x90
_test:                                  ## @test
	.cfi_startproc
## %bb.0:                               ## %entry
	sub	rsp, 24
	.cfi_def_cfa_offset 32
	call	_runtime_gc_poll
Ltmp4:
	mov	edi, 4
	call	_runtime_allocate
Ltmp5:
	mov	dword ptr [rax], 1
	mov	qword ptr [rsp + 8], rax
	mov	edi, 4
	call	_runtime_allocate
Ltmp6:
	mov	dword ptr [rax], 2
	mov	qword ptr [rsp + 16], rax
	call	_do_nothing
Ltmp7:
	lea	rdi, [rsp + 8]
	call	_print
Ltmp8:
	xor	eax, eax
	add	rsp, 24
	ret
	.cfi_endproc
                                        ## -- End function
	.globl	_program_entry                  ## -- Begin function program_entry
	.p2align	4, 0x90
_program_entry:                         ## @program_entry
	.cfi_startproc
## %bb.0:                               ## %entry
	push	rax
	.cfi_def_cfa_offset 16
	call	_runtime_gc_poll
Ltmp9:
	call	_test
Ltmp10:
	pop	rcx
	ret
	.cfi_endproc
                                        ## -- End function
	.section	__LLVM_STACKMAPS,__llvm_stackmaps
__LLVM_StackMaps:
	.byte	3
	.byte	0
	.short	0
	.long	4
	.long	0
	.long	11
	.quad	_print
	.quad	24
	.quad	3
	.quad	_do_nothing
	.quad	8
	.quad	1
	.quad	_test
	.quad	24
	.quad	5
	.quad	_program_entry
	.quad	8
	.quad	2
	.quad	2882400000
	.long	Ltmp0-_print
	.short	0
	.short	7
	.byte	4
	.byte	0
	.short	8
	.short	0
	.short	0
	.long	0
	.byte	4
	.byte	0
	.short	8
	.short	0
	.short	0
	.long	0
	.byte	4
	.byte	0
	.short	8
	.short	0
	.short	0
	.long	0
	.byte	3
	.byte	0
	.short	8
	.short	7
	.short	0
	.long	16
	.byte	3
	.byte	0
	.short	8
	.short	7
	.short	0
	.long	16
	.byte	3
	.byte	0
	.short	8
	.short	7
	.short	0
	.long	8
	.byte	3
	.byte	0
	.short	8
	.short	7
	.short	0
	.long	8
	.p2align	3, 0x0
	.short	0
	.short	0
	.p2align	3, 0x0
	.quad	2882400000
	.long	Ltmp1-_print
	.short	0
	.short	7
	.byte	4
	.byte	0
	.short	8
	.short	0
	.short	0
	.long	0
	.byte	4
	.byte	0
	.short	8
	.short	0
	.short	0
	.long	0
	.byte	4
	.byte	0
	.short	8
	.short	0
	.short	0
	.long	0
	.byte	3
	.byte	0
	.short	8
	.short	7
	.short	0
	.long	16
	.byte	3
	.byte	0
	.short	8
	.short	7
	.short	0
	.long	16
	.byte	3
	.byte	0
	.short	8
	.short	7
	.short	0
	.long	8
	.byte	3
	.byte	0
	.short	8
	.short	7
	.short	0
	.long	8
	.p2align	3, 0x0
	.short	0
	.short	0
	.p2align	3, 0x0
	.quad	2882400000
	.long	Ltmp2-_print
	.short	0
	.short	5
	.byte	4
	.byte	0
	.short	8
	.short	0
	.short	0
	.long	0
	.byte	4
	.byte	0
	.short	8
	.short	0
	.short	0
	.long	0
	.byte	4
	.byte	0
	.short	8
	.short	0
	.short	0
	.long	0
	.byte	3
	.byte	0
	.short	8
	.short	7
	.short	0
	.long	8
	.byte	3
	.byte	0
	.short	8
	.short	7
	.short	0
	.long	8
	.p2align	3, 0x0
	.short	0
	.short	0
	.p2align	3, 0x0
	.quad	2882400000
	.long	Ltmp3-_do_nothing
	.short	0
	.short	3
	.byte	4
	.byte	0
	.short	8
	.short	0
	.short	0
	.long	0
	.byte	4
	.byte	0
	.short	8
	.short	0
	.short	0
	.long	0
	.byte	4
	.byte	0
	.short	8
	.short	0
	.short	0
	.long	0
	.p2align	3, 0x0
	.short	0
	.short	0
	.p2align	3, 0x0
	.quad	2882400000
	.long	Ltmp4-_test
	.short	0
	.short	3
	.byte	4
	.byte	0
	.short	8
	.short	0
	.short	0
	.long	0
	.byte	4
	.byte	0
	.short	8
	.short	0
	.short	0
	.long	0
	.byte	4
	.byte	0
	.short	8
	.short	0
	.short	0
	.long	0
	.p2align	3, 0x0
	.short	0
	.short	0
	.p2align	3, 0x0
	.quad	2882400000
	.long	Ltmp5-_test
	.short	0
	.short	3
	.byte	4
	.byte	0
	.short	8
	.short	0
	.short	0
	.long	0
	.byte	4
	.byte	0
	.short	8
	.short	0
	.short	0
	.long	0
	.byte	4
	.byte	0
	.short	8
	.short	0
	.short	0
	.long	0
	.p2align	3, 0x0
	.short	0
	.short	0
	.p2align	3, 0x0
	.quad	2882400000
	.long	Ltmp6-_test
	.short	0
	.short	3
	.byte	4
	.byte	0
	.short	8
	.short	0
	.short	0
	.long	0
	.byte	4
	.byte	0
	.short	8
	.short	0
	.short	0
	.long	0
	.byte	4
	.byte	0
	.short	8
	.short	0
	.short	0
	.long	0
	.p2align	3, 0x0
	.short	0
	.short	0
	.p2align	3, 0x0
	.quad	2882400000
	.long	Ltmp7-_test
	.short	0
	.short	3
	.byte	4
	.byte	0
	.short	8
	.short	0
	.short	0
	.long	0
	.byte	4
	.byte	0
	.short	8
	.short	0
	.short	0
	.long	0
	.byte	4
	.byte	0
	.short	8
	.short	0
	.short	0
	.long	0
	.p2align	3, 0x0
	.short	0
	.short	0
	.p2align	3, 0x0
	.quad	2882400000
	.long	Ltmp8-_test
	.short	0
	.short	3
	.byte	4
	.byte	0
	.short	8
	.short	0
	.short	0
	.long	0
	.byte	4
	.byte	0
	.short	8
	.short	0
	.short	0
	.long	0
	.byte	4
	.byte	0
	.short	8
	.short	0
	.short	0
	.long	0
	.p2align	3, 0x0
	.short	0
	.short	0
	.p2align	3, 0x0
	.quad	2882400000
	.long	Ltmp9-_program_entry
	.short	0
	.short	3
	.byte	4
	.byte	0
	.short	8
	.short	0
	.short	0
	.long	0
	.byte	4
	.byte	0
	.short	8
	.short	0
	.short	0
	.long	0
	.byte	4
	.byte	0
	.short	8
	.short	0
	.short	0
	.long	0
	.p2align	3, 0x0
	.short	0
	.short	0
	.p2align	3, 0x0
	.quad	2882400000
	.long	Ltmp10-_program_entry
	.short	0
	.short	3
	.byte	4
	.byte	0
	.short	8
	.short	0
	.short	0
	.long	0
	.byte	4
	.byte	0
	.short	8
	.short	0
	.short	0
	.long	0
	.byte	4
	.byte	0
	.short	8
	.short	0
	.short	0
	.long	0
	.p2align	3, 0x0
	.short	0
	.short	0
	.p2align	3, 0x0

.subsections_via_symbols
