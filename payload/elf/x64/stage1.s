global _start

_start:
.stage1:
.aslr_offset:
  	call   $+5
	pop rax
  	mov rsi, 0xfefffffffffff000	;
	push rax
	pop r15
    	sub r15, rsi		; r15 aslr
.va_stage1:
	and rax, 0xfffffffffffff000
	push rax			;[rsp] = (va) s1
	mov rsi,  0xfefffffffffff000	; (len) s1
	push rsi			;[rsp] = (len, va) s1
	call .open
.proc_self_exec:
	times 15 ret ; "/proc/self/exe"
.open:
	xor esi, esi
	pop rdi
	push 0x2
	pop rax
	syscall
	push rax		;[rsp] (fd, len, va) s1
	test rax, rax
	js _exit
.mmap_stage_2:
  	xor    r9d,r9d		;offset
  	push -1			;
  	pop    r8		;fd
  	push 0x22		;anonymous | private
  	pop r10			; MAP
  	push 0x3		; RW
  	pop rdx			; prot
  	xor    edi,edi 		;Va
	mov rsi, 0xfefffffffffff000	; (len) s2
	push rsi		;[rsp] (len) s2, (fd, len, va) s1
	push 0x9
	pop rax
	syscall
  	cmp rax, -4095		;Mmap error.
  	jae _exit
  	push rax
	pop r13		;r13 va stage 2, r15 aslr_offset.
	push 0x9
	pop rax
	syscall
  	cmp rax, -4095		;Mmap error.
  	jae _exit
  	push rax
	pop r14		;r14 decrypt mmap, r13 va stage 2, r15 aslr_offset.

	jmp end_of_stage1
.return_crypt_compress_addr:
	pop rsi		;src;;[rsp](len) s2, (fd, len, va) s1
.set_aeskey:
  	call .pass_aeskey
  	times 16 nop;[Placeholder aeskey]
.pass_aeskey:
  	pop rax
         ;movdqu unaligned memory if movdqa should be aligned.
  	movdqu xmm0, [rax]
.call_decrypt:
	pop rdx			;(len) s2 [rsp](fd, len, va) s1

	push rdx	       ;[rsp](len) s2, (fd, len, va) s1
	call decrypt; rsi in, rdx in_len, rdi out, xmm0 key
	test rax, rax
	jz _exit
	push r13
	pop rdx		; mmap dest
	push rax	; va of decrypted_s2_elf
	pop rdi		; src
	mov rsi, 0xfefffffffffff000
	call lz77_decompress
	test rax, rax
	jnz _exit
.munmap_decrypt_mmap:
	pop rsi			;(len) s2 [rsp] (fd, len, va) s1

	xchg rdi, r14
	push 0xb
	pop rax
	syscall
.mprotect_stage_2:
	push r13
	pop rdi			;va
	push 0x5
	pop rdx			;prot R-X
	;; pop rsi			;(len) s2 [rsp] (fd, len, va) s1
	push r13		;[rsp] (va) s2,(fd, len, va) s1
	push rsi		;[rsp] (len, va) s2, (fd, len, va) s1
	push 0xA
	pop rax			;syscall mprotect 10
  	syscall
	test rax, rax
  	jnz _exit
.jmp_stage_2:
	;; pop rsi push rsi
	mov rax, 0xfefffffffeffffff
	add rax, r13
	push rax
	;; push r13;[rsp] (entry ,len,va) s2, (fd, len, va) s1
	ret;		 ;(pop rip va s2[rsp] (len, va) s2, (fd, len, va) s1

_exit:
	xor edi, edi
    push 60
    pop rax
    syscall
lz77_decompress:
    ; Function entry - validate input parameters
    ; rdi = in, rsi = in_size, rdx = out
	cmp rsi,byte +0x7
	mov r9,rdx
	setna al
	test rdx,rdx
	setz dl
	or al,dl
	jnz .end
	test rdi,rdi
	jz .end
	mov rcx,rsi
.init_loop:
	mov rsi,[rdi]
	test rsi,rsi
	push byte +0x0
	pop rax
	jz .end
	lea r8,[rdi+0x8]
	add rdi,rcx
.advance_token:
	add r8,byte +0x3
	cmp r8,rdi
	jna .get_data_token	;Could shrink Bytecode len.
	jmp .end

.get_data_token:
	cmp rax,rsi
	jnc .end
	mov cx,[r8-0x3]
	mov edx,ecx
	and dx,byte +0xf
	jnz .check_len_distance
.literal:
	mov dl,[r8-0x1]
	mov [r9+rax],dl
	inc rax
	jmp .advance_token
.check_len_distance:
	shr cx,byte 0x4
	movzx r11d,cx
	jz .literal
	cmp rax,r11
	jc .literal
.match:
	push rbx
	add edx,byte +0x2
	mov rcx,rsi
	lea rbx,[r9+rax]
	mov r10,rax
	sub rcx,rax
	movsx rdx,dx
	cmp rcx,rdx
	cmovna rdx,rcx
	sub r10,r11
	xor ecx,ecx
	add r10,r9
.match_write:
	mov r11b,[r10+rcx]
	mov [rbx+rcx],r11b
	inc rcx
	cmp rdx,rcx
	jnz .match_write
.restore_match:
	pop rbx
	add rax,rdx
	cmp rax,rsi
	jnc .advance_token
	mov dl,[r8-0x1]
	mov [r9+rax],dl
	inc rax
	jmp .advance_token
.end:
	cmp rax,rsi
	setnz al
	movzx eax,al
	neg rax
	ret

;;TODO @danrodri implement the logic.
decrypt:
	push rsi
	pop rax
	ret

end_of_stage1:
	call _start.return_crypt_compress_addr
