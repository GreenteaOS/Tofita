.section .head1
call kernelMain
hlt

.section .stack1
.align 16
stack_bottom:
.skip 0x10000 # 64 KiB
stack_top:
