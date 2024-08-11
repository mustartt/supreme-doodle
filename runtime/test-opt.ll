; ModuleID = 'test.ll'
source_filename = "test.ll"

module asm ".globl __LLVM_StackMaps"

declare void @runtime_gc_poll()

declare ptr addrspace(1) @runtime_allocate(i64)

declare void @runtime_deallocate(ptr addrspace(1))

declare void @runtime_inspect_ptr(ptr addrspace(1))

define void @gc.safepoint_poll() {
  call void @runtime_gc_poll()
  ret void
}

define i32 @add(ptr addrspace(1) %a, ptr addrspace(1) %b) gc "statepoint-example" {
entry:
  %0 = load i32, ptr addrspace(1) %a, align 4
  %1 = load i32, ptr addrspace(1) %b, align 4
  %2 = add i32 %0, %1
  %statepoint_token = call token (i64, i32, ptr, i32, i32, ...) @llvm.experimental.gc.statepoint.p0(i64 2882400000, i32 0, ptr elementtype(void ()) @runtime_gc_poll, i32 0, i32 0, i32 0, i32 0)
  ret i32 %2
}

define void @other(ptr addrspace(1) %ptr) gc "statepoint-example" {
entry:
  %0 = bitcast ptr addrspace(1) %ptr to ptr addrspace(1)
  %statepoint_token = call token (i64, i32, ptr, i32, i32, ...) @llvm.experimental.gc.statepoint.p0(i64 2882400000, i32 0, ptr elementtype(void ()) @runtime_gc_poll, i32 0, i32 0, i32 0, i32 0) [ "gc-live"(ptr addrspace(1) %ptr) ]
  %ptr.relocated = call coldcc ptr addrspace(1) @llvm.experimental.gc.relocate.p1(token %statepoint_token, i32 0, i32 0) ; (%ptr, %ptr)
  %.remat = bitcast ptr addrspace(1) %ptr.relocated to ptr addrspace(1)
  %statepoint_token2 = call token (i64, i32, ptr, i32, i32, ...) @llvm.experimental.gc.statepoint.p0(i64 2882400000, i32 0, ptr elementtype(void (ptr addrspace(1))) @runtime_inspect_ptr, i32 1, i32 0, ptr addrspace(1) %.remat, i32 0, i32 0) [ "gc-live"(ptr addrspace(1) %ptr.relocated) ]
  %ptr.relocated3 = call coldcc ptr addrspace(1) @llvm.experimental.gc.relocate.p1(token %statepoint_token2, i32 0, i32 0) ; (%ptr.relocated, %ptr.relocated)
  %.remat1 = bitcast ptr addrspace(1) %ptr.relocated3 to ptr addrspace(1)
  ret void
}

define i32 @test() gc "statepoint-example" {
entry:
  %statepoint_token = call token (i64, i32, ptr, i32, i32, ...) @llvm.experimental.gc.statepoint.p0(i64 2882400000, i32 0, ptr elementtype(void ()) @runtime_gc_poll, i32 0, i32 0, i32 0, i32 0)
  %statepoint_token4 = call token (i64, i32, ptr, i32, i32, ...) @llvm.experimental.gc.statepoint.p0(i64 2882400000, i32 0, ptr elementtype(ptr addrspace(1) (i64)) @runtime_allocate, i32 1, i32 0, i64 4, i32 0, i32 0)
  %0 = call ptr addrspace(1) @llvm.experimental.gc.result.p1(token %statepoint_token4)
  %1 = bitcast ptr addrspace(1) %0 to ptr addrspace(1)
  %statepoint_token5 = call token (i64, i32, ptr, i32, i32, ...) @llvm.experimental.gc.statepoint.p0(i64 2882400000, i32 0, ptr elementtype(ptr addrspace(1) (i64)) @runtime_allocate, i32 1, i32 0, i64 4, i32 0, i32 0) [ "gc-live"(ptr addrspace(1) %0) ]
  %2 = call ptr addrspace(1) @llvm.experimental.gc.result.p1(token %statepoint_token5)
  %3 = call coldcc ptr addrspace(1) @llvm.experimental.gc.relocate.p1(token %statepoint_token5, i32 0, i32 0) ; (%0, %0)
  %4 = bitcast ptr addrspace(1) %2 to ptr addrspace(1)
  %.remat1 = bitcast ptr addrspace(1) %3 to ptr addrspace(1)
  store i32 123, ptr addrspace(1) %.remat1, align 4
  store i32 456, ptr addrspace(1) %4, align 4
  %.remat = bitcast ptr addrspace(1) %3 to ptr addrspace(1)
  %statepoint_token6 = call token (i64, i32, ptr, i32, i32, ...) @llvm.experimental.gc.statepoint.p0(i64 2882400000, i32 0, ptr elementtype(i32 (ptr addrspace(1), ptr addrspace(1))) @add, i32 2, i32 0, ptr addrspace(1) %.remat, ptr addrspace(1) %4, i32 0, i32 0) [ "gc-live"(ptr addrspace(1) %2, ptr addrspace(1) %3) ]
  %5 = call i32 @llvm.experimental.gc.result.i32(token %statepoint_token6)
  %6 = call coldcc ptr addrspace(1) @llvm.experimental.gc.relocate.p1(token %statepoint_token6, i32 0, i32 0) ; (%2, %2)
  %7 = call coldcc ptr addrspace(1) @llvm.experimental.gc.relocate.p1(token %statepoint_token6, i32 1, i32 1) ; (%3, %3)
  %.remat2 = bitcast ptr addrspace(1) %6 to ptr addrspace(1)
  %statepoint_token7 = call token (i64, i32, ptr, i32, i32, ...) @llvm.experimental.gc.statepoint.p0(i64 2882400000, i32 0, ptr elementtype(void (ptr addrspace(1))) @other, i32 1, i32 0, ptr addrspace(1) %.remat2, i32 0, i32 0) [ "gc-live"(ptr addrspace(1) %6) ]
  %8 = call coldcc ptr addrspace(1) @llvm.experimental.gc.relocate.p1(token %statepoint_token7, i32 0, i32 0) ; (%6, %6)
  %.remat3 = bitcast ptr addrspace(1) %8 to ptr addrspace(1)
  ret i32 %5
}

define i32 @program_entry() gc "statepoint-example" {
entry:
  %statepoint_token = call token (i64, i32, ptr, i32, i32, ...) @llvm.experimental.gc.statepoint.p0(i64 2882400000, i32 0, ptr elementtype(void ()) @runtime_gc_poll, i32 0, i32 0, i32 0, i32 0)
  %statepoint_token1 = call token (i64, i32, ptr, i32, i32, ...) @llvm.experimental.gc.statepoint.p0(i64 2882400000, i32 0, ptr elementtype(i32 ()) @test, i32 0, i32 0, i32 0, i32 0)
  %0 = call i32 @llvm.experimental.gc.result.i32(token %statepoint_token1)
  ret i32 %0
}

declare token @llvm.experimental.gc.statepoint.p0(i64 immarg, i32 immarg, ptr, i32 immarg, i32 immarg, ...)

declare void @__tmp_use(...)

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(none)
declare ptr addrspace(1) @llvm.experimental.gc.relocate.p1(token, i32 immarg, i32 immarg) #0

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(none)
declare ptr addrspace(1) @llvm.experimental.gc.result.p1(token) #0

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(none)
declare i32 @llvm.experimental.gc.result.i32(token) #0

attributes #0 = { nocallback nofree nosync nounwind willreturn memory(none) }
