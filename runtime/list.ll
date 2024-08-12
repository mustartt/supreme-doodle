declare void @runtime_gc_poll()
declare i8 addrspace(1)* @runtime_allocate(i64 %size)
declare void @runtime_deallocate(i8 addrspace(1)* %ptr)
declare void @runtime_inspect_ptr(i8 addrspace(1)* %ptr)

define private void @gc.safepoint_poll() {
    call void @runtime_gc_poll()
    ret void
}

%ListNode = type {
    i32, 
    %ListNode addrspace(1)*
}


