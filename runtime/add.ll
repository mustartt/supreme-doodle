
declare void @runtime_gc_poll()
declare i8 addrspace(1)* @runtime_allocate(i64 %size)
declare void @runtime_deallocate(i8 addrspace(1)* %ptr)
declare void @runtime_inspect_ptr(i8 addrspace(1)* %ptr)

define private void @gc.safepoint_poll() {
    call void @runtime_gc_poll()
    ret void
}

define i32 @add(i32 addrspace(1)* %a, i32 addrspace(1)* %b) gc "statepoint-example" {
entry:
    %1 = load i32, i32 addrspace(1)* %a
    %2 = load i32, i32 addrspace(1)* %b
    %3 = add i32 %1, %2
    ret i32 %3
}

