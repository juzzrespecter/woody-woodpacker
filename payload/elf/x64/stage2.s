global _start

jmp_entry_for_stage3:
      pop r11;entrypoint interp [rsp]va, size_s2, va s2
      pop r14		;va s3 [rsp]size_s2, va s2
      test r11, r11
      cmovne r13, r11		;if r11 != 0  set r12 = r11

      .jmp_stage3:
      pop rsi			;size_s2 [rsp]  va_s2
      pop rdi			;va_s2 [rsp]
      push 0xb		;mummap
      pop rax
      push r13		;[rsp]va entrypoint that go jmp from stage3
      push 0		;[rsp] reset rdx,va entrypoint that go jmp from stage3
      push r14		;[rsp] va stage3, reset rdx, va entrypoint
      ret			; jmp -> va s3, [rsp] va entrypoint



_start:		 ;r15 aslr
      .stage2:   ;[rsp] size_s2_elf, va_s2_elf, fd_s1, size_s1, va s1
      .mmap_fd:
      pop r14			;size_s2 [rsp] va s2f, fd_s1, size_s1, va s1
      pop r13			;va_s2 [rsp] fd_s1, size_s1, va s1
      push 0x2		;MAP_PRIVATE
      pop r10			; flags
      xor r9, r9		; offset
      pop r8			;fd_s1 [rsp] size_s1, va s1
      xor edi, edi		;addr
      push 0x1000
      pop rsi			;size
      push 1
      pop rdx			;perm
      push r8			;[rsp] fd_s1, size_s1, va s1
      push 0x9
      pop rax
      syscall
      cmp rax, -4095		;Mmap error.
      jae _exit		;0x162
      .close_fd:
      pop rdi			;;fd_s1 [rsp] size_s1, va s1
      push 0x3
      pop rax
      syscall
      test rax, rax
      jnz _exit
.unmmap_s1:
      pop rsi			;size_s1 [rsp]  va_s1
      pop rdi			;va_s1 [rsp]
      push 0xb		;munmap syscall
      pop rax
      syscall
      test rax, rax
      jnz _exit
      push r13
      push r14		; [rsp] size_s2_elf, va_s2_elf
      jmp end_of_stage_2
      .return_ehdr_ptr:;[rsp] ehdr_ptr size_s2, size_s2_elf, va s2
      pop rdi		; ehdr_ptr [rsp] size_s2_elf, va_s2_elf
      push rdi	; ;[rsp] ehdr_ptr size_s2, va s2
      pop rbx 	;ehdr_ptr [rsp] size_s2, va s2

      .get_base:
	;; sub    rsp, 0x10                 ; Allocate stack space

                              ; Get program header information
	;; movzx  esi, WORD [rdi+0x38] ; esi = e_phnum; Overwrite elf_size
	;; add    rdi, QWORD [rdi+0x20] ; rdi = e_phoff
	;; lea    rdx, [rsp]          ; rdx = stack space for result min
	;; lea    rcx, [rsp+0x8]          ; rcx = stack space for result max
	;; call   calculate_elf_memory_range;0x1d7
	;; mov rdx, QWORD [rsp]		     ;rdx target_va base
      push rbx
      pop rdi			;pop  ehdr rdi ehdr/ptr ;[rsp] size_s2, va s2
      push r15		;aslr
      pop rsi			;rsi aslr_offset
      push -1
      pop rcx			;rcx fd
      call load_elf_generic	;397 ;rdi ptr, rsi aslr,[rdx target_va], rcx fd
      cmp rax, -1		;rax entrypoint
      je _exit

      push rdi		;load_elf_generic rdi == base
      pop r14			;
      push rax
      pop r13			;r13 entrypoint to jmp
     ;; 0x4f3
      call get_at_interp_x64	;[rsp] size_s2, va s2
      cmp rax, -1
      je _exit
      push rax
      pop r11			;entrypoint interp
      push rdi
      pop r12			;base interp
      ;; This should
      lea rdi, [rsp + 0x10]	;[rsp] size_s2, va s2
      call set_auxv		;0x17B
      mov r14, 0xfeffffffffffffff
      test r14, r14		;if va s3
      lea rsi, [r14 + r15]	;va s3 + aslr
      cmovne r14, rsi		;va s3 = va s3 + aslr
      push r14		;[rsp] va_s3[+ aslr], size_s2, va s2
      test r14, r14
      jne .prep_payload	;0x12a
      pop r14		;va_s3 [rsp]size_s2, va s2

      .create_stage3:
      push 0x22		;MAP_PRIVATE | MAP_ANONYMOUS
      pop r10			; flags
      xor r9, r9		; offset
      push -1
      pop r8			;fd
      xor edi, edi		;addr
      push 0x1000
      pop rsi			;size
      push 3			;RW
      pop rdx			;perm
      push 0x9
      pop rax
      push r11
      syscall
      cmp rax, -4095
      pop r11			;;entrypoint interp
      jae _exit
      push 0x4		     ; size of stage3
      pop rcx			     ; N times
      push rcx			     ; N times
      push rax		;[rsp]va s3, size s3, size_s2, va s2
      xchg    rdi, rax             ; rdi = destination mmap
      call .bytecode_stage3
      syscall			;call munmap s2
      pop rdx ;reset this register
      ret			;jmp to entrypoint Interp or not.
      .bytecode_stage3:
      pop rsi		      ;src s3, [rsp]va_s3, size_s3, size_s2, va s2
      rep movsb
      .mprotect_stage3:
      push 5			;RX
      pop rdx			;perm
      pop rdi			;va s3 [rsp]size s3, size_s2, va s2

      pop rsi			;size of bytecode_stage3 [rsp]size_s2, va s2

      push 0xa		;mprotect
      pop rax
      push rdi		;[rsp]va_s3 size_s2, va s2
      push r11		;[rsp]interp_va, va_s3, size, va s2
      syscall
      test rax, rax
      jnz _exit
      pop r11			;interp_va[rsp] va_s3, size, va s2

      .prep_payload:
      push r11	;[rsp]interp_va, va_s3, size, va s2
      push QWORD [rsp + 0x18]	;va s2
      ret

_exit:
      push 0
      pop rdi
      push 60
      pop rax
      syscall
_open:
      push 0x2
      pop rax
      syscall
      ret
_read:
      xor eax, eax		;syscall 0
      syscall
      ret
_close:
      push 0x3
      pop rax
      syscall
      ret

      ;; rdi should be rsp begin
      ;; r15 aslr
      ;; r14 ehdr ptr in the correct mmap
      ;; r13 entrypoint
      ;; r12 base interp
      ;; Modified reg rdi, rsi, r/ecx
set_auxv:
        xor ecx, ecx		; counter for detect end argv, envp.

      .reaching_auxv:
        add rdi, 8
        mov rsi, qword [rdi]
        test rsi, rsi		; register check value
        jnz .reaching_auxv
        inc ecx
        cmp ecx, 2
        jne .reaching_auxv

      .auxv_loop:
        add rdi, 8		;First case NULL, next field of auxv,rest value -> field.
        mov rsi, qword [rdi]
        test rsi, rsi
      jz .end
        add rdi, 8		;Advance to value

        cmp esi, 3          ; AT_PHDR
        je .set_phdr
        cmp esi, 5          ; AT_PHNUM
        je .set_phnum
        cmp esi, 7          ; AT_BASE
        je .set_base
        cmp esi, 9          ; AT_ENTRY
        je .set_entry
        jmp .auxv_loop

      .set_phdr:
        mov    rax,QWORD  [r14+0x20]	; r14 ehdr ptr; rax == phdr_offset
      add    rax,r14			; r14 == 0x40 + ehdr ptr -> phdr ptr.
      mov qword [rdi], rax
      jmp .auxv_loop

      .set_phnum:
      movzx  eax,WORD  [r14+0x38] ; rbx ehdr ptr
        mov qword [rdi], rax
        jmp .auxv_loop

      .set_base:
        mov qword [rdi], r12	; r12 base interp
        jmp .auxv_loop

      .set_entry:
      mov qword [rdi], r13
      jmp .auxv_loop
      .end:
      ret

; void calculate_elf_memory_range(Elf64_Phdr *phdr, unsigned int phnum, uint64_t *min_va, uint64_t *max_va)
calculate_elf_memory_range:
    xor    eax, eax                        ; rax = 0
    mov    QWORD  [rdx], 0xffffffffffffffff ; *min_va = UINT64_MAX
    mov    QWORD  [rcx], rax            ; *max_va = 0
    movsxd rax, esi                         ; rax = phnum (sign extend to 64-bit); May optimize with lea?
    imul   rax, rax, 0x38                  ; rax = phnum * sizeof(Elf64_Phdr)
    add    rdi, rax                        ; rdi = phdr + (phnum * sizeof(Elf64_Phdr))
                                            ; rdi now points to the end of the phdr array

.start_loop:
    test   esi, esi                        ; if (phnum == 0)
    je     .done                           ;   goto done
    dec    esi                             ; phnum--
    cmp    DWORD  [rdi-0x38], 0x1       ; if (phdr[-1].p_type != PT_LOAD)
    jne    .next_iteration                 ;   goto next_iteration

    ; Process PT_LOAD segment
    mov    rax, QWORD  [rdi-0x28]      ; rax = phdr[-1].p_va
    cmp    rax, QWORD  [rdx]            ; if (p_va < *min_va)
    jae    .check_max                      ;   goto check_max
    mov    QWORD  [rdx], rax            ; *min_va = p_va

.check_max:
    mov    rax, QWORD  [rdi-0x10]      ; rax = phdr[-1].p_memsz
    add    rax, QWORD  [rdi-0x28]       ; rax = p_va + p_memsz
    cmp    QWORD  [rcx], rax            ; if (*max_va < (p_va + p_memsz))
    jae    .next_iteration                 ;   goto next_iteration
    mov    QWORD  [rcx], rax            ; *max_va = p_va + p_memsz

.next_iteration:
    sub    rdi, 0x38                       ; rdi = phdr - sizeof(Elf64_Phdr) (move to previous phdr)
    jmp    .start_loop                     ; goto start_loop

.done:
    and    QWORD  [rdx], 0xfffffffffffff000 ; *min_va &= ~(PAGE_SIZE - 1) (align down to page)
    ret                                     ; return

; int load_elf_segment(void *base, void *elf_ptr, Elf64_Phdr *phdr, uint64_t min_va, int fd)
; rdi = base, rsi = elf_ptr, rdx = phdr, rcx = min_va, r8d = fd


load_elf_segment:
    ; Check if segment type is PT_LOAD (1)
    cmp    DWORD  [rdx], 0x1      ; Compare p_type with PT_LOAD (1)
    jne    .not_load_segment         ; If not PT_LOAD, exit with 0

    ; Save caller-saved registers
    push   r15
    mov    r9, rcx                  ; r9 = min_va
    push   r14
    push   r13
    push   r12
    push   rbp
    push   rbx
    sub    rsp, 0x18                ; Allocate stack space

    ; Extract segment information
    mov    ebp, DWORD  [rdx+0x4]   ; ebp = p_flags
    mov    r13, QWORD  [rdx+0x8]  ; r13 = p_offset
    mov    r12, QWORD  [rdx+0x10] ; r12 = p_va
    mov    eax, ebp                 ; eax = p_flags
    mov    r15, r13                 ; r15 = p_offset (copy)

    ; Align va and offset to page boundary
    and    eax, 0x2                 ; Test PF_W flag (bit 1)
    and    r15, 0xfffffffffffff000   ; Align p_offset to page boundary
    and    r12, 0xfffffffffffff000   ; Align p_va to page boundary
    cmp    eax, 0x1                 ; Compare PF_W flag with 1
    sbb    eax, eax                 ; Set eax to 0xffffffff if PF_W is set, 0 otherwise
    add    r13, QWORD  [rdx+0x20] ; r13 = p_offset + p_filesz
    and    eax, 0xfffffffe          ; Mask eax (0xfffffffe if PF_W, 0 otherwise)
    add    eax, 0x3                 ; eax = (1{R--}|| 3{RW-})

    ; Check if fd == -1
    cmp    r8d, 0xffffffff           ; Check if fd == -1
    je     .fd_is_negative_one       ; Jump if fd == -1

    ; fd != -1 case
    and    ebp, 0x1                 ; Test PF_X flag (bit 0)
    mov    r10d, 0x12                ; ecx = 0x12 (MAP_PRIVATE)
    shl    ebp, 0x2                 ; Shift PF_X flag left by 2 (becomes PROT_EXEC if set)
    or     ebp, eax                 ; Combine flags
    jmp    .setup_mmap_params        ; Jump to setup mmap parameters

.fd_is_negative_one:
    ; fd == -1 case
    mov r10, QWORD  [rdx+0x20]
      mov rbp, QWORD  [rdx+0x28]
      sub rbp, r10
      add r13, rbp
    mov    r10, 0x32                ; ecx = 0x32 (MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS)
    mov    ebp, 0x3                 ; ebp = 0x3 (PROT_READ | PROT_WRITE)

.setup_mmap_params:
    ; Setup mmap parameters
    mov    rax, r12                 ; rax = aligned p_va
    sub    r13, r15                 ; r13 = (p_offset + p_filesz) - aligned p_offset
    mov    QWORD  [rsp], rsi     ; Save elf_ptr on stack
    mov    rbx, rdx                 ; rbx = phdr
    sub    rax, r9                  ; rax = aligned p_va - min_va
    lea    r14, [rdi+rax*1]         ; r14 = base + (aligned p_va - min_va)
    mov    edx, ebp                 ; edx = combined flags/protection
    mov    r9, r15                  ; r9 = aligned p_offset
    mov    rsi, r13                 ; rsi = size
    mov    DWORD  [rsp+0xc], r8d ; Save fd on stack
    mov    rdi, r14                 ; rdi = mapping address
      push 0x9
      pop rax
      syscall
    mov    rdx, rax                 ; rdx = result of mmap
    mov    eax, 0xffffffff          ; Set eax to -1
    cmp rdx, -4095
    jae     _exit


    ; Check fd value
    cmp    DWORD  [rsp+0xc], 0xffffffff ; Check if fd == -1
    jne    .fd_not_negative_one     ; Jump if fd != -1

    ; fd == -1: Copy data from ELF buffer to mapped memory
    mov    rsi, QWORD  [rsp]     ; rsi = elf_ptr
    mov    rcx, QWORD  [rbx+0x20] ; rcx = p_filesz
    add    rcx, QWORD  [rbx+0x8]  ; rcx = p_offset + p_filesz
    mov    rdi, rdx                 ; rdi = mapped address
    sub    rcx, r15                 ; rcx = (p_offset + p_filesz) - aligned p_offset
    add    rsi, r15                 ; rsi = elf_ptr + aligned p_offset
    rep movsb                      ; Copy data
    mov    rsi, r13                 ; rsi = size
    mov    rdi, r14                 ; rdi = base address
    mov    edx, DWORD  [rbx+0x4] ; edx = p_flags
    mov    eax, edx                 ; eax = p_flags
    and    eax, 0x2                 ; Test PF_W flag
    cmp    eax, 0x1                 ; Compare with 1
    sbb    eax, eax                 ; Set eax based on comparison
    and    edx, 0x1                 ; Test PF_X flag
    and    eax, 0xfffffffe          ; Mask eax
    shl    edx, 0x2                 ; Shift PF_X flag left by 2
    add    eax, 0x3                 ; Add 3 to eax
    or     edx, eax                 ; Combine flags
      push 0xa		    ;mprotect
      pop rax
      syscall
    ;; sar    eax, 0x1f                ; Arithmetic shift right by 31
    jmp    .end                     ; Jump to end

.fd_not_negative_one:
    ; fd != -1: Check if p_memsz > p_filesz

    mov    rax, QWORD  [rbx+0x28] ; rax = p_memsz
    mov    rcx, QWORD  [rbx+0x8] ; rcx = p_offset
      sub rcx, r15		 ;p_offset - page_offset
    lea rsi, [rax + rcx]	    ;rsi = p_memsz + p_offset - page_offset
      push r13
      pop rcx			;rcx = (p_filesz + p_offset - page_offset)
    cmp    rcx, rsi                 ; Compare (p_offset + p_filesz - aligned_p_offset) with (p_offset - page_offset p_memsz)
    js     .handle_bss              ; Jump if p_filesz is less than calculated size
    xor    eax, eax                 ; Return 0 (success)
    jmp    .end                     ; Jump to end

.handle_bss:
    ; Handle .bss section
      lea rdi, [rdx + rcx]
      push rcx
      pop rax
      add rcx, 0xfff
      and    rcx, 0xfffffffffffff000   ; Align bss_start
      push rcx
      sub rcx, rax
      xor eax, eax
      rep stosb
      pop rcx
      sub rsi, rcx	;rsi - rcx = n rsi > rcx
      js .end		;rcx >= rsi no mmap
      lea rdi, [rcx + rdx]
    xor    r9d, r9d                 ; Clear r9d offset
    mov    r8d, 0xffffffff          ; Set r8d to -1 fd
    mov    edx, ebp                 ; edx = original flags
    mov    r10, 0x32                ; Set ecx to 0x32 FIXED, PRIVATE, ANONYMOUS
      push 0x9
      pop rax
      syscall

.end:
    add    rsp, 0x18                ; Deallocate stack space
    pop    rbx                      ; Restore rbx
    pop    rbp                      ; Restore rbp
    pop    r12                      ; Restore r12
    pop    r13                      ; Restore r13
    pop    r14                      ; Restore r14
    pop    r15                      ; Restore r15

    cmp rax, -4095
    setae  al                      ; Set al if rax == 0 || setz
    movzx  eax, al                 ; Zero extend al to eax
    neg    eax                      ; Negate eax
    ret                            ; Return

.not_load_segment:
    xor    eax, eax                 ; Return 0 (not a load segment)
    ret                            ; Return

; uint64_t load_elf_generic(void *ptr, uint64_t aslr_offset, uint64_t target_va, int fd)
; rdi = ptr, rsi = aslr_offset, rdx = target_va, ecx = fd

load_elf_generic:
    ; Save callee-saved registers
    push   r15
    mov    r10, rdx              ; r10 = target_va
    mov    r15d, ecx             ; r15d = fd
    push   r14
    mov    r14, rsi              ; r14 = aslr_offset
    push   r13
    push   r12
    push   rbp
    mov    rbp, rdi              ; rbp = ptr; push pop
    push   rbx
    sub    rsp, 0x28             ; Allocate stack space

    ; Load ELF program header pointer and count
    mov    r12, QWORD  [rdi+0x20]  ; r12 = e_phoff
    movzx  esi, WORD  [rdi+0x38]   ; esi = e_phnum
    lea    rcx, [rsp+0x18]       ; rcx = pointer to max_va result
    lea    rdx, [rsp+0x10]       ; rdx = pointer to min_va result
    add    r12, rdi              ; r12 = ptr + e_phoff (actual program header address)
    mov    rdi, r12              ; rdi = program header address
    call   calculate_elf_memory_range  ; Call helper function

    ; Get results from stack
    mov    r13, QWORD  [rsp+0x10]  ; r13 = min_va
    mov    r10, r13		   ; r10 target_va = min_va
    mov    rsi, QWORD  [rsp+0x18] ; rsi = max_va
    mov    ecx, 0x22             ; ecx = MAP_PRIVATE | MAP_ANONYMOUS ; push pop

    ; Calculate total size and check target_va
    sub    rsi, r13              ; rsi = max_va - min_va (total_size)
    cmp   r15d, -1              ; Test target_va
    jne     .use_default_flags    ; If zero, use default flags

    ; Add ASLR offset to target address and set MAP_FIXED
    add    r10, r14              ; r10 = target_va + aslr_offset
    mov    ecx, 0x32             ; ecx = MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS push pop

.use_default_flags:
    ; Setup mmap parameters
    xor    r9d, r9d              ; r9d = 0 (file descriptor for MAP_ANONYMOUS)
    mov    r8d, 0xffffffff        ; r8d = -1 (MAP_ANONYMOUS flag)
    xor    edx, edx              ; edx = 0 (offset)
    mov    rdi, r10              ; rdi = target address
    mov    r10, rcx
      push 0x9
      pop rax
      syscall

    ; Check for mmap error
    mov    rbx, rax              ; rbx = result of mmap
    xor    r9d, r9d              ; r9d = 0 (counter)
    cmp    rax, -0xfff; Check for mmap error
    jb    .check_phdr_count     ; If no error, continue

.mmap_error:
      jmp    _exit

.load_segment_loop:
    ; Prepare arguments for load_elf_segment call
    mov    r8d, r15d             ; r8d = fd
    mov    rcx, r13              ; rcx = min_va
    mov    rdx, r12              ; rdx = current program header
    mov    rsi, rbp              ; rsi = elf_ptr (ptr)
    mov    rdi, rbx              ; rdi = mapping address
    mov    DWORD  [rsp+0xc], r9d  ; Save counter on stack
    call   load_elf_segment      ; Call load_elf_segment

    inc    eax                   ; Increment result
    je     _exit

    ; Move to next program header and increment counter
    mov    r9d, DWORD  [rsp+0xc]  ; Load counter
    add    r12, 0x38             ; Advance to next program header (56 bytes)
    inc    r9d                   ; Increment counter

.check_phdr_count:
    movzx  eax, WORD  [rbp+0x38]  ; eax = e_phnum
    cmp    eax, r9d              ; Compare with counter
    jg     .load_segment_loop    ; If more headers, continue loop

    ; Check if we need to apply ASLR to dynamic sections
    inc    r15d                  ; Increment r15d (fd)
    jne    .skip_write_dyn       ; If fd != -1, skip write_dyn

    ; Apply ASLR to dynamic sections
    mov    rsi, r14              ; rsi = aslr_offset
    mov    rdi, rbx              ; rdi = mapping address; push pop 1 B -
    call   write_dyn             ; Call write_dyn to adjust dynamic addresses

.skip_write_dyn:
    ; Calculate entrypoint
	push  rbx
	pop rax			;rax = mapping va
	push  rbx
	pop rdi			; rdi = mapping va
    sub    rax, r13              ; rax = mapping_address - min_va
    add    rax, QWORD  [rbp+0x18]  ; rax = adjusted + entry point

.cleanup_and_return:
    add    rsp, 0x28             ; Deallocate stack space
    pop    rbx                   ; Restore rbx
    pop    rbp                   ; Restore rbp
    pop    r12                   ; Restore r12
    pop    r13                   ; Restore r13
    pop    r14                   ; Restore r14
    pop    r15                   ; Restore r15
    ret                         ; Return

; uint64_t write_dyn(void *ptr, uint64_t aslr_offset)
; rdi = ptr mmap, rsi = aslr_offset
write_dyn:
    ; Check if aslr_offset is zero
    test   rsi, rsi              ; Test aslr_offset
    je     .end_success          ; If zero, return success (rax=0 from xor)

    ; Load ELF header and program header information
    mov    rdx, QWORD  [rdi+0x20]  ; rdx = e_phoff (program header offset)
    movzx  ecx, WORD  [rdi+0x38]   ; ecx = e_phnum (number of program headers)
    xor    eax, eax              ; eax = 0 (counter)
    add    rdx, rdi              ; rdx = ptr + e_phoff (actual program header address)

.loop_phdr:
    ; Check if we've processed all program headers
    cmp    eax, ecx              ; Compare counter with phnum
    jge    .end_success          ; If >=, return success

    ; Check current program header type
    cmp    DWORD  [rdx], 0x2  ; Compare p_type with PT_DYNAMIC (2)
    je     .found_dynamic        ; If PT_DYNAMIC, process it

    ; Move to next program header
    inc    eax                   ; Increment counter
    add    rdx, 0x38             ; Advance 56 bytes to next phdr (size of Elf64_Phdr)
    jmp    .loop_phdr            ; Continue loop

.found_dynamic:
    ; Process PT_DYNAMIC segment
    mov    rcx, QWORD  [rdx+0x20]  ; rcx = p_memsz (size of dynamic section)
    cdqe                         ; Sign extend eax to rax (for shifting)
    mov    r8d, 0x68230f8        ; r8d = bitmask for address-type dynamic tags
    shl    rax, 0x4              ; rax = counter * 16 (for calculating final address)
    add    rax, QWORD  [rdx+0x8]  ; rax = p_va + (counter * 16)
    xor    edx, edx              ; edx = 0 (inner loop counter)
    add    rdi, rax              ; rdi = ptr + p_va + (counter * 16) (dynamic section start)
    shr    rcx, 0x4              ; rcx = p_memsz / 16 (number of dynamic entries)

.inner_loop:
    ; Check if inner loop is done
    cmp    edx, ecx              ; Compare inner counter with entry count
    je     .end_success          ; If done, return success

    ; Load current dynamic entry
    mov    rax, QWORD  [rdi]  ; rax = d_tag (dynamic tag)

    ; Check if it's a small tag that needs address adjustment
    cmp    rax, 0x1a             ; Compare with DT_JMPREL (26)
    jg     .check_large_tags     ; If > 26, check large tags
    cmp    rax, 0x2              ; Compare with DT_NEEDED (2)
    jle    .next_inner           ; If <= 2, skip (not an address)
    bt     r8, rax               ; Check if bit corresponding to tag is set
    jb     .adjust_address       ; If bit is set, adjust address
    jmp    .next_inner           ; Otherwise, continue to next entry

.check_large_tags:
    ; Check specific large tags that need address adjustment
    cmp    rax, 0x6ffffff0       ; Compare with DT_LOPROC special tag
    je     .adjust_address       ; If equal, adjust address
    cmp    rax, 0x6ffffffe       ; Compare with DT_VERNEED special tag
    je     .adjust_address       ; If equal, adjust address
    cmp    rax, 0x6ffffef5       ; Compare with DT_ADDRRNGHI special tag
    jne    .next_inner           ; If not equal, continue to next entry

.adjust_address:
    ; Add aslr_offset to dynamic entry value
    add    QWORD  [rdi+0x8], rsi  ; d_val += aslr_offset

.next_inner:
    ; Continue inner loop
    inc    edx                   ; Increment inner counter
    jmp    .inner_loop           ; Continue inner loop

.end_success:
    xor    eax, eax              ; Return 0 (success)
    ret                          ; Return


get_at_interp_x64:
      push   rbp
      xor    edx, edx                ; ndx_ph = 0 (initialize program header index)
      push   rbx
      sub    rsp, 0x408              ; Allocate 1032 bytes on stack for buffer (1024 + 8)
      mov    rax, QWORD [rdi + 0x20] ; rax = e_phoff (program header offset)
      movzx  ecx, WORD [rdi + 0x38]  ; ecx = e_phnum (program header number)
      add    rax, rdi                ; rax = ptr + e_phoff (pointer to first phdr)
      .loop_start:
      cmp    rdx, rcx                ; Compare ndx_ph with e_phnum
      jb     .find_interp_segment    ; .purpose: Check if current phdr is PT_INTERP
      xor    ebx, ebx                ; ebx = 0 (no interpreter found)
      xor    edi, edi                ; edi = 0 (interpreter base)
      jmp     .cleanup_and_return     ; .purpose: Exit if no PT_INTERP found

      .find_interp_segment:
      imul   rsi, rdx, 0x38          ; rsi = ndx_ph * sizeof(Elf64_Phdr) (offset to current phdr)
      cmp    DWORD [rax + rsi*1], 0x3 ; Compare p_type with PT_INTERP (3)
      je     .interp_found           ; .purpose: Found PT_INTERP segment
      inc    rdx                     ; ndx_ph++ (increment to check next phdr)
      jmp    .loop_start             ; .purpose: Continue searching for PT_INTERP

      .interp_found:
      imul   rdx, rdx, 0x38          ; rdx = ndx_ph * sizeof(Elf64_Phdr) (offset to found phdr)
      xor    esi, esi                ; esi = 0 (clear for open() flags)
      add    rdi, QWORD [rax + rdx*1 + 0x8] ; rdi = ptr + phdr[ndx_ph].p_offset (interp string address)
      xor    eax, eax                ; eax = 0 (clear for open() system call)
      call   _open              ; .purpose: Call open() to open interpreter file
      mov    ebp, eax                ; interp_fd = result of open()
      test   eax, eax
      js     .error_cleanup          ; .purpose: Handle error if open() failed
      mov    edx, 0x400              ; edx = 1024 (buffer size for read)
      mov    rsi, rsp                ; rsi = &interp_elfh[0] (buffer on stack)
      mov    edi, eax                ; edi = interp_fd (file descriptor)
      call   _read              ; .purpose: Call read() to read interpreter ELF header
      cmp    rax, 0x3ff              ; Compare bytes read with 1023 (check if < 1024)
      jg     .process_interpreter    ; .purpose: Process if we read at least 1024 bytes
      mov    edi, ebp                ; edi = interp_fd
      call   _close             ; .purpose: Close file on read error
      jmp    .error_cleanup          ; .purpose: Return error

      .process_interpreter:
      mov    rdi, rsp                ; rdi = &interp_elfh[0] (buffer address)
      mov    ecx, ebp                ; ecx = interp_fd
;      xor    edx, edx                ; edx = 0 (target_va = 0)
      xor    esi, esi                ; esi = 0 (aslr_offset = 0)
      call   load_elf_generic  ; .purpose: Call load_elf_generic for interpreter
      push rdi		     ;Base of elf.
      mov    edi, ebp                ; edi = interp_fd
      mov    rbx, rax                ; rbx = result from load_elf_generic
      call   _close
      pop rdi			; Base of elf
      cmp  rbx, -1
      jne .cleanup_and_return

      .error_cleanup:
      mov    rbx, 0xffffffffffffffff ; rbx = -1 (error return value)

      .cleanup_and_return:
      add    rsp, 0x408              ; Deallocate 1032 bytes from stack
      mov    rax, rbx                ; rax = return value
      pop    rbx
      pop    rbp
      ret

end_of_stage_2:
        call _start.return_ehdr_ptr
