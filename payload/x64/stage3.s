global _start

_start:
  	syscall			;call munmap s2
  	pop rdx ;reset this register
  	ret			;jmp to entrypoint Interp or not.
