
.global irq0_stub
.global irq1_stub
.global first_task_switch

irq0_stub:
    pusha
    mov %esp, %eax
    push %eax
    call timer_interrupt
    add $4, %esp
    mov %eax, %esp
    popa
    iret

irq1_stub:
    pusha
    mov %esp, %eax
    push %eax
    call keyboard_interrupt
    add $4, %esp
    mov %eax, %esp
    popa
    iret

first_task_switch:
    mov 4(%esp), %esp
    sti
    popa
    iret

