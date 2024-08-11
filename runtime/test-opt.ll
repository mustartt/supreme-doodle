; ModuleID = 'test.ll'
source_filename = "test.ll"

module asm ".globl __LLVM_StackMaps"

%struct = type { ptr addrspace(1), ptr addrspace(1) }

declare void @runtime_gc_poll()

declare ptr addrspace(1) @runtime_allocate(i64)

declare void @runtime_deallocate(ptr addrspace(1))

declare void @runtime_inspect_ptr(ptr addrspace(1))

define void @gc.safepoint_poll() {
  call void @runtime_gc_poll()
  ret void
}

define void @print(ptr %p) gc "statepoint-example" {
entry:
  %0 = getelementptr %struct, ptr %p, i32 0, i32 0
  %1 = load ptr addrspace(1), ptr %0, align 8
  %2 = bitcast ptr addrspace(1) %1 to ptr addrspace(1)
  %3 = getelementptr %struct, ptr %p, i32 0, i32 1
  %4 = load ptr addrspace(1), ptr %3, align 8
  %5 = bitcast ptr addrspace(1) %4 to ptr addrspace(1)
  %statepoint_token = call token (i64, i32, ptr, i32, i32, ...) @llvm.experimental.gc.statepoint.p0(i64 2882400000, i32 0, ptr elementtype(void ()) @runtime_gc_poll, i32 0, i32 0, i32 0, i32 0) [ "gc-live"(ptr addrspace(1) %4, ptr addrspace(1) %1) ]
  %6 = call coldcc ptr addrspace(1) @llvm.experimental.gc.relocate.p1(token %statepoint_token, i32 0, i32 0) ; (%4, %4)
  %7 = call coldcc ptr addrspace(1) @llvm.experimental.gc.relocate.p1(token %statepoint_token, i32 1, i32 1) ; (%1, %1)
  %.remat1 = bitcast ptr addrspace(1) %7 to ptr addrspace(1)
  %.remat = bitcast ptr addrspace(1) %6 to ptr addrspace(1)
  %statepoint_token5 = call token (i64, i32, ptr, i32, i32, ...) @llvm.experimental.gc.statepoint.p0(i64 2882400000, i32 0, ptr elementtype(void (ptr addrspace(1))) @runtime_inspect_ptr, i32 1, i32 0, ptr addrspace(1) %.remat1, i32 0, i32 0) [ "gc-live"(ptr addrspace(1) %6, ptr addrspace(1) %7) ]
  %8 = call coldcc ptr addrspace(1) @llvm.experimental.gc.relocate.p1(token %statepoint_token5, i32 0, i32 0) ; (%6, %6)
  %9 = call coldcc ptr addrspace(1) @llvm.experimental.gc.relocate.p1(token %statepoint_token5, i32 1, i32 1) ; (%7, %7)
  %.remat3 = bitcast ptr addrspace(1) %9 to ptr addrspace(1)
  %.remat2 = bitcast ptr addrspace(1) %8 to ptr addrspace(1)
  %statepoint_token6 = call token (i64, i32, ptr, i32, i32, ...) @llvm.experimental.gc.statepoint.p0(i64 2882400000, i32 0, ptr elementtype(void (ptr addrspace(1))) @runtime_inspect_ptr, i32 1, i32 0, ptr addrspace(1) %.remat2, i32 0, i32 0) [ "gc-live"(ptr addrspace(1) %8) ]
  %10 = call coldcc ptr addrspace(1) @llvm.experimental.gc.relocate.p1(token %statepoint_token6, i32 0, i32 0) ; (%8, %8)
  %.remat4 = bitcast ptr addrspace(1) %10 to ptr addrspace(1)
  ret void
}

define void @do_nothing() gc "statepoint-example" {
entry:
  %statepoint_token = call token (i64, i32, ptr, i32, i32, ...) @llvm.experimental.gc.statepoint.p0(i64 2882400000, i32 0, ptr elementtype(void ()) @runtime_gc_poll, i32 0, i32 0, i32 0, i32 0)
  ret void
}

define i32 @test() gc "statepoint-example" {
entry:
  %0 = alloca %struct, align 8
  %statepoint_token = call token (i64, i32, ptr, i32, i32, ...) @llvm.experimental.gc.statepoint.p0(i64 2882400000, i32 0, ptr elementtype(void ()) @runtime_gc_poll, i32 0, i32 0, i32 0, i32 0)
  %statepoint_token1 = call token (i64, i32, ptr, i32, i32, ...) @llvm.experimental.gc.statepoint.p0(i64 2882400000, i32 0, ptr elementtype(ptr addrspace(1) (i64)) @runtime_allocate, i32 1, i32 0, i64 4, i32 0, i32 0)
  %1 = call ptr addrspace(1) @llvm.experimental.gc.result.p1(token %statepoint_token1)
  %2 = bitcast ptr addrspace(1) %1 to ptr addrspace(1)
  store i32 1, ptr addrspace(1) %2, align 4
  %3 = getelementptr %struct, ptr %0, i32 0, i32 0
  store ptr addrspace(1) %2, ptr %3, align 8
  %statepoint_token2 = call token (i64, i32, ptr, i32, i32, ...) @llvm.experimental.gc.statepoint.p0(i64 2882400000, i32 0, ptr elementtype(ptr addrspace(1) (i64)) @runtime_allocate, i32 1, i32 0, i64 4, i32 0, i32 0)
  %4 = call ptr addrspace(1) @llvm.experimental.gc.result.p1(token %statepoint_token2)
  %5 = bitcast ptr addrspace(1) %4 to ptr addrspace(1)
  store i32 2, ptr addrspace(1) %5, align 4
  %6 = getelementptr %struct, ptr %0, i32 0, i32 1
  store ptr addrspace(1) %5, ptr %6, align 8
  %statepoint_token3 = call token (i64, i32, ptr, i32, i32, ...) @llvm.experimental.gc.statepoint.p0(i64 2882400000, i32 0, ptr elementtype(void ()) @do_nothing, i32 0, i32 0, i32 0, i32 0)
  %statepoint_token4 = call token (i64, i32, ptr, i32, i32, ...) @llvm.experimental.gc.statepoint.p0(i64 2882400000, i32 0, ptr elementtype(void (ptr)) @print, i32 1, i32 0, ptr %0, i32 0, i32 0)
  ret i32 0
}

define i32 @program_entry() gc "statepoint-example" {
entry:
  %statepoint_token = call token (i64, i32, ptr, i32, i32, ...) @llvm.experimental.gc.statepoint.p0(i64 2882400000, i32 0, ptr elementtype(void ()) @runtime_gc_poll, i32 0, i32 0, i32 0, i32 0)
  %statepoint_token1 = call token (i64, i32, ptr, i32, i32, ...) @llvm.experimental.gc.statepoint.p0(i64 2882400000, i32 0, ptr elementtype(i32 ()) @test, i32 0, i32 0, i32 0, i32 0)
  %0 = call i32 @llvm.experimental.gc.result.i32(token %statepoint_token1)
  ret i32 %0
}

declare void @__tmp_use(...)

declare token @llvm.experimental.gc.statepoint.p0(i64 immarg, i32 immarg, ptr, i32 immarg, i32 immarg, ...)

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(none)
declare ptr addrspace(1) @llvm.experimental.gc.relocate.p1(token, i32 immarg, i32 immarg) #0

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(none)
declare ptr addrspace(1) @llvm.experimental.gc.result.p1(token) #0

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(none)
declare i32 @llvm.experimental.gc.result.i32(token) #0

attributes #0 = { nocallback nofree nosync nounwind willreturn memory(none) }
