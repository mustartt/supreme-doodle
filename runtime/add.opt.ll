; ModuleID = 'add.ll'
source_filename = "add.ll"

declare void @runtime_gc_poll()

declare ptr addrspace(1) @runtime_allocate(i64)

declare void @runtime_deallocate(ptr addrspace(1))

declare void @runtime_inspect_ptr(ptr addrspace(1))

define private void @gc.safepoint_poll() {
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

declare token @llvm.experimental.gc.statepoint.p0(i64 immarg, i32 immarg, ptr, i32 immarg, i32 immarg, ...)
