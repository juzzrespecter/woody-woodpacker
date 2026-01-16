payload:
        call .woody_write	;;
	times 14 ret
        .woody_write:
	push 1
	pop ebx			;fd
	pop ecx			;buffer
	push 0xe		;len
        pop    edx
	push 0x4		;write sysnbr
	pop eax
	int 0x80
