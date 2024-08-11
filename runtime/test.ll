module asm ".globl __LLVM_StackMaps"

declare void @runtime_gc_poll()
declare i8 addrspace(1)* @runtime_allocate(i64 %size)
declare void @runtime_deallocate(i8 addrspace(1)* %ptr)
declare void @runtime_inspect_ptr(i8 addrspace(1)* %ptr)

define void @gc.safepoint_poll() {
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

define void @other(i32 addrspace(1)* %ptr) gc "statepoint-example" {
entry:
    %1 = bitcast i32 addrspace(1)* %ptr to i8 addrspace(1)*
    call void @runtime_inspect_ptr(i8 addrspace(1)* %1)
    ret void
}

define i32 @test() gc "statepoint-example" {
entry:
    %1 = call i8 addrspace(1)* @runtime_allocate(i64 4)
    %2 = bitcast i8 addrspace(1)* %1 to i32 addrspace(1)*

    %3 = call i8 addrspace(1)* @runtime_allocate(i64 4)
    %4 = bitcast i8 addrspace(1)* %3 to i32 addrspace(1)*

    store i32 123, i32 addrspace(1)* %2
    store i32 456, i32 addrspace(1)* %4

    %5 = call i32 @add(i32 addrspace(1)* %2, i32 addrspace(1)* %4)

    call void @other(i32 addrspace(1)* %4)

    ret i32 %5 
}

define i32 @program_entry() gc "statepoint-example" {
entry:
    %1 = call i32 @test()
    ret i32 %1
}
