%ListNode = type {
    i32,
    %ListNode addrspace(1)*
}

%NestType = type {
    i32, 
    %ListNode
}

define void @tmp() {
entry:
    %node = alloca %ListNode, align 8
    %other = alloca %NestType, align 8
    ret void
}

