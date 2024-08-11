module asm ".globl __LLVM_StackMaps"

declare void @runtime_gc_poll()
declare i8 addrspace(1)* @runtime_allocate(i64 %size)
declare void @runtime_deallocate(i8 addrspace(1)* %ptr)
declare void @runtime_inspect_ptr(i8 addrspace(1)* %ptr)

define void @gc.safepoint_poll() {
    call void @runtime_gc_poll()
    ret void
}

%struct = type { i32 addrspace(1)*, i32 addrspace(1)* }

define void @print(%struct* %p) gc "statepoint-example" {
entry:
    %1 = getelementptr %struct, %struct* %p, i32 0, i32 0
    %2 = load i32 addrspace(1)*, i32 addrspace(1)** %1
    %3 = bitcast i32 addrspace(1)* %2 to i8 addrspace(1)*

    %4 = getelementptr %struct, %struct* %p, i32 0, i32 1
    %5 = load i32 addrspace(1)*, i32 addrspace(1)** %4
    %6 = bitcast i32 addrspace(1)* %5 to i8 addrspace(1)*

    call void @runtime_inspect_ptr(i8 addrspace(1)* %3)
    call void @runtime_inspect_ptr(i8 addrspace(1)* %6)

    ret void
}

define void @do_nothing() gc "statepoint-example" {
entry:
    ret void
}

define i32 @test() gc "statepoint-example" {
entry:
    %1 = alloca %struct

    %2 = call i8 addrspace(1)* @runtime_allocate(i64 4)
    %3 = bitcast i8 addrspace(1)* %2 to i32 addrspace(1)*
    store i32 1, i32 addrspace(1)* %3
    %4 = getelementptr %struct, %struct* %1, i32 0, i32 0
    store i32 addrspace(1)* %3, i32 addrspace(1)** %4

    %5 = call i8 addrspace(1)* @runtime_allocate(i64 4)
    %6 = bitcast i8 addrspace(1)* %5 to i32 addrspace(1)*
    store i32 2, i32 addrspace(1)* %6
    %7 = getelementptr %struct, %struct* %1, i32 0, i32 1
    store i32 addrspace(1)* %6, i32 addrspace(1)** %7

    call void @do_nothing()

    call void @print(%struct* %1)

    ret i32 0
}

define i32 @program_entry() gc "statepoint-example" {
entry:
    %1 = call i32 @test()
    ret i32 %1
}

