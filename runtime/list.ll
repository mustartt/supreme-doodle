declare void @runtime_gc_poll()
declare ptr addrspace(1) @runtime_allocate(i64)
declare void @runtime_deallocate(ptr addrspace(1))
declare void @runtime_inspect_ptr(ptr addrspace(1))

define private void @gc.safepoint_poll() {
    call void @runtime_gc_poll()
    ret void
}

%ListNode = type {
    i32, 
    %ListNode addrspace(1)*
}

define void @tmp() {
entry:
    %node = alloca %ListNode, align 8
    ret void
}


