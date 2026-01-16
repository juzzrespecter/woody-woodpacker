global payload
payload:
	call .woody_write		; Woody Payload.
	times 14 ret			;'....WOODY....\n'
	.woody_write:
        pop    rsi		;....WOODY....\n ptr.		;[rsp]va s3 size_s2, va s2
	push 0xe		;ptr_size
	pop rdx
	push 0x1		;fd to write syscall
	pop rdi
	push 0x1		;write syscall
	pop rax
        syscall			; I shouldn't protect this in case payload fail still working as normally.
