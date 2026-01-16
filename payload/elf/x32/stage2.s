global _start:
      ;; Cheatsheet mmap
      ;; ebx addr
      ;; ecx size
      ;; edx Perm
      ;; esi flags
      ;; edi fd
      ;; ebp offset/0x1000
      ;; eax 192
 ;[esp] (fd, len, va) s1, aslr, (len, va) s2 Fine
jmp_pad_s3:
      pop edx 		;Entry interp;[esp] (e) elf, (va) s3, (len, va) s2;
      pop esi			;Entry elf; [esp] (va) s3, (len, va) s2;
      pop edi			;(va) s3 [esp] (len, va) s2
      ;; munmap
         pop ecx 		;(len)
         pop ebx			;(va)
         push 91			;munmap
      pop eax
      test edx, edx		;if interp
      cmovnz esi, edx		; modify entrypoint
      push esi	; entrypoint (interp || elf)
      push 0		; reset edx
      push edi	; va s3
      ret

 _start:				;mmap fd
      xor ebx, ebx
      mov ecx, 0x1000
      push 1
      pop edx
      push 0x2
      pop esi
      pop edi
      push edi		;Save fd ;[esp] (fd, len, va) s1, aslr, (len, va) s2
      xor ebp, ebp
      push 192
      pop eax
      int 0x80
      cmp eax, -4095
      jae _exit
      pop ebx		;Restore fd close fd ;[esp] (len, va) s1, aslr, (len, va) s2

      push 0x6	;Close Sys nbr
      pop eax
      int 0x80
      test eax, eax
      jnz _exit
 ;[esp] (len, va) s1, aslr, (len, va) s2
      pop ecx			;(len) [esp] (va) s1, aslr, (len, va) s2
      pop ebx			;(va) s1, [esp] aslr, (len, va) s2
      push 91			;Munmap
      pop eax
      int 0x80
      test eax, eax
      jnz _exit

      jmp end_s2
 return_ehdr_ptr: 		;[X] Fine Stack
      pop edi			;[esp] aslr, (len, va) s2 -> Fine
      push edi		; [esp] ehdr, aslr, (len, va)s2 -> Fine
      .get_base:		;[X]
      mov    edx, DWORD  [edi+0x1c] ; eax = e_phoff
      add edx, edi      ; PHDR
      pop eax	       ;ehdr  [esp] aslr, (len, va)s2.
      pop ebx	       ;aslr [esp] (len, va) s2;
      push ebx       ;[esp] aslr, (len, va) s2;
      push -1	       ;[esp] (fd) args, aslr, (len, va) s2;

      push ebx       ;[esp](aslr, fd) args, aslr, (len, va) s2; .
      push eax       ;[esp](ehdr, aslr, fd) args, aslr, (len, va) s2;
      ;; load elf (ehdr,aslr, fd)
      call load_elf; Fine esp
      add esp, 0xc
 ;[esp] aslr, (len, va) s2
      push edi
 ;[esp] va elf, aslr, (len, va)s2
      push eax
 ;[esp] (entry, va) elf, aslr, (len, va)s2
      push edi
 ;[esp] (va) args, (entry, va) elf, aslr, (len, va)s2
      ;; get_at_interp_x32(ehdr)
      call get_at_interp_x32
      cmp eax, -1
      je _exit
      add esp, 0x4
      push edi
 ;[esp] (va) interp ,(entry, va) elf, aslr, (len, va)s2
      push eax
 ;[esp] (entry, va) interp ,(entry, va) elf, aslr, (len, va)s2
      lea edi, [esp + 28]
 ;; set_auxv([ignored]entry interp, va interp, entry elf, va elf, aslr); I break stack and set_auxv segfault.
      call set_auxv		; Working fine.
      pop ebx			;entry interp [esp] (va) interp ,(entry, va) elf, aslr, (len, va)s2;
      pop ecx			;Discard (va) interp [esp] (entry, va) elf, aslr, (len, va)s2;
      pop ecx			;elf entry [esp] (va) elf, aslr, (len, va) s2 ;
      pop edx			;elf va Discard (va) elf [esp] aslr, (len, va) s2 ;
      pop edx			;aslr [esp] (len, va) s2 ;
 ;; [esp] (len, va) s2
      mov esi, 0xfeffffff	;va s3 Modified in C
      test esi, esi
      lea edi, [esi + edx]	;
      cmovne esi, edi
      push esi;[esp] va s3, (len, va) s2
      push ecx;[esp] (elf) entry, ...
      push ebx;[esp] (interp, elf) entry;
      test esi, esi
      jnz .payload
      ;; Create s3
      ;; [esp] (interp, elf) entry, v s3, l,v s2
      .mmap_s3:
      xor ebx, ebx		;va
      push 0x4
      pop ecx		;len
      push 3
      pop edx			;RW perm
      push 0x22
      pop esi			;MAP_PRIVATE|ANONYMOUS
      push -1
      pop edi			;fd
      xor ebp, ebp		;offset
      push 192
      pop eax
      int 0x80
      cmp eax, -4095
      jae _exit
      .write_s3:
      mov edi, eax		;dst
      mov ebp, edi		;va
      call .end_s3
      int 0x80
      pop edx
      ret
      .end_s3:
      pop esi			;src
      push 0x4
      pop ecx
      rep movsb
      .mprotect_s3:
      push 0x4
      pop ecx			;len
      push 0x5
      pop edx			;R-X
      push 125		;mprotect
      pop eax
      int 0x80
      test eax, eax
      jnz _exit
      mov DWORD [esp + 0x4], ebp ;s3 = va
      .payload:
      push DWORD [esp + 0x10] ;s2 va
      ;; add eax, 0x31
      ret
      .jmp_s3:
      pop edx 		;Entry interp;[esp] (e) elf, (va) s3, (len, va) s2;
      pop esi			;Entry elf; [esp] (va) s3, (len, va) s2;
      pop edi			;(va) s3 [esp] (len, va) s2
      ;; munmap
         pop ecx 		;(len)
         pop ebx			;(va)
         push 91			;munmap
      pop eax
      ;; int 0x80	I was choping the va that i was executing Segfault error deserved.
      ;; test eax, eax
      ;; jnz _exit
      test edx, edx		;if interp
      cmovnz esi, edx		; modify entrypoint
      push esi	; entrypoint (interp || elf)
      push 0		; reset edx
      push edi	; va s3
      ret
              ; jmp s3 int 0x80, reset edx, jmp entry


      ;; edi should be rsp begin
 ;[esp] (entry, va) interp ,(entry, va) elf, aslr, (len, va)s2

      ;; r15 aslr -> [esp + 0x4 * 5]
      ;; r14 ehdr ptr in the correct mmap -> [esp + 0x4 * 4]
      ;; r13 entrypoint [esp + 0x4 * 3]
      ;; r12 base interp [esp + 0x4 * 2]
      ;; r11 entry interp [sep + 0x4 ]
      ;; Modified reg rdi, rsi, r/ecx
 set_auxv:			;[esp](va) ret, ((entry, va) interp ,(entry, va) elf, aslr)) args, (len, va) s2
         xor ecx, ecx		; counter for detect end argv, envp.

      .reaching_auxv:
         add edi, 4
         mov esi, dword [edi]
         test esi, esi		; register check value
         jnz .reaching_auxv
         inc ecx
         cmp ecx, 2
         jne .reaching_auxv

      .auxv_loop:
         add edi, 4		;First case NULL, next field of auxv,rest value -> field.
         mov esi, dword [edi]
         test esi, esi
      jz .end
         add edi, 4		;Advance to value

         cmp esi, 3          ; AT_PHDR
         je .set_phdr
         cmp esi, 5          ; AT_PHNUM
         je .set_phnum
         cmp esi, 7          ; AT_BASE
         je .set_base
         cmp esi, 9          ; AT_ENTRY
         jne .auxv_loop

      .set_entry:
         mov eax, dword [esp + 0x4 * 3]
         mov dword [edi], eax 	;elf_entry
      jmp .auxv_loop

      .set_phdr:
      mov eax, [esp + 0x4 * 4]     ; ehdr
         add    eax,DWORD  [eax+0x1c]	; e_phoff
      mov dword [edi], eax
      jmp .auxv_loop

      .set_phnum:
      mov eax, [esp + 0x4 * 4]     ; ehdr
      movzx  eax,WORD  [eax+0x2c] ;
         mov dword [edi], eax
         jmp .auxv_loop

      .set_base:
         mov eax, dword [esp + 0x4 * 2]
         mov dword [edi], eax 	;base_interp
         jmp .auxv_loop

 .end:
      ret
 ; int load_elf_segment(void *base, void *elf_ptr, Elf32_Phdr *phdr, uint32_t min_va, int fd)
 ; Parameters:
 ;   [ebp+0x8] = base (base address where ELF should be loaded)
 ;   [ebp+0xc] = elf_ptr (pointer to ELF file in memory)
 ;   [ebp+0x10] = phdr (pointer to current program header)
 ;   [ebp+0x14] = min_va (minimum virtual address of all segments)
 ;   [ebp+0x18] = fd (file descriptor, -1 for buffer)

 load_elf_segment:
     push   ebp
     mov    ebp, esp
     push   ebx
     sub    esp, 0x54

     mov    eax, DWORD  [ebp+0x10]
      push eax
     mov    eax, DWORD  [eax+0x4]
     and    eax, 0xfffff000
      .set_p_off:
     mov    DWORD  [ebp-0x18], eax
      pop eax
      push eax
     mov    eax, DWORD  [eax+0x4]
     sub    eax, DWORD  [ebp-0x18]
     mov    DWORD  [ebp-0x1c], eax
      pop eax
      push eax
     mov    eax, DWORD  [eax+0x8]
     and    eax, 0xfffff000
     mov    DWORD  [ebp-0x20], eax
     mov    eax, DWORD  [ebp-0x20]
     sub    eax, DWORD  [ebp+0x14]
     mov    DWORD  [ebp-0x24], eax
     mov    eax, DWORD  [ebp+0x10]
     mov    edx, DWORD  [eax+0x14]
     mov    eax, DWORD  [ebp+0x10]
     mov    ecx, DWORD  [eax+0x10]
     mov    eax, edx
     sub    eax, ecx
     mov    DWORD  [ebp-0x28], eax
      pop eax
      push eax
     mov    edx, DWORD  [eax+0x10]
     mov    eax, DWORD  [ebp-0x1c]
     add    eax, edx
     mov    DWORD  [ebp-0x2c], eax
      pop eax
      push eax
     mov    eax, DWORD  [eax]
     cmp    eax, 0x1
     jne     .exit_success

 .is_pt_load:
     mov    DWORD  [ebp-0x10], 0x1
     mov    eax, DWORD  [ebp+0x10]
     mov    eax, DWORD  [eax+0x18]
     and    eax, 0x2
     test   eax, eax
     je     .check_pf_x_flag
     or     DWORD  [ebp-0x10], 0x2

 .check_pf_x_flag:
      pop eax
      push eax
     mov    eax, DWORD  [eax+0x18]
     and    eax, 0x1
     test   eax, eax
     je     .setup_protection
     or     DWORD  [ebp-0x10], 0x4 ;prot

 .setup_protection:
      pop eax
      push eax
     mov    DWORD  [ebp-0x30], eax
     mov    eax, DWORD  [ebp+0x8]
     mov    eax, DWORD  [eax]
     test   eax, eax
     je     .use_map_private
      push 0x12
      pop eax
     jmp    .set_flags
 .use_map_private:
      push 0x02
      pop eax
 .set_flags:
     mov    DWORD  [ebp-0x14], eax
     mov    eax, DWORD  [ebp-0x2c]
     mov    DWORD  [ebp-0xc], eax
     cmp    DWORD  [ebp+0x18], 0xffffffff
     jne    .calculate_map_params
     mov    eax, DWORD  [ebp-0x28]
     add    DWORD  [ebp-0xc], eax
     or     DWORD  [ebp-0x14], 0x20
     mov    DWORD  [ebp-0x10], 0x3

 .calculate_map_params:
     mov    eax, DWORD  [ebp+0x8]
     mov    eax, DWORD  [eax]
     add    eax, DWORD  [ebp-0x24]
     mov    DWORD  [ebp-0x34], eax
      ;; Cheatsheet mmap
      ;; ebx addr
      ;; ecx size
      ;; edx Perm
      ;; esi flags
      ;; edi fd
      ;; ebp offset/0x1000
      ;; eax 192

     mov esi,   DWORD  [ebp-0x14]
     mov edx,   DWORD  [ebp-0x10]
      mov edi, DWORD [ebp+0x18]
     mov ecx,   DWORD  [ebp-0xc]
      mov ebx, eax
      push ebp
     mov    ebp, DWORD  [ebp-0x18]
      shr ebp, 12
      push 192
      pop eax
      int 0x80
      pop ebp
     mov    DWORD  [ebp-0x38], eax
     cmp    eax, 0xfffff000	;if fd != -1 Fine
     ja     _exit
     mov    eax, DWORD  [ebp+0x8]
     mov    eax, DWORD  [eax]
     test   eax, eax
     je     .check_fd_value
     mov    eax, DWORD  [ebp-0x38]
     cmp    eax, ebx		;
     jne    _exit

 .check_fd_value:
     cmp    DWORD  [ebp+0x18], 0xffffffff
     jne    .handle_bss
     mov    esi, DWORD  [ebp+0xc]
     add    esi, DWORD  [ebp-0x18]
     mov edx,   DWORD  [ebp-0x2c]
     push   eax
     mov edi,   DWORD  [ebp-0x38]
     rep movsb
     mov    DWORD  [ebp-0x10], 0x1
     mov    eax, DWORD  [ebp+0x10]
     mov    eax, DWORD  [eax+0x18]
     and    eax, 0x2
     test   eax, eax
     je     .check_pf_x_flag_v2
     or     DWORD  [ebp-0x10], 0x2

 .check_pf_x_flag_v2:
      mov eax, DWORD [ebp+0x10]
     mov    eax, DWORD  [eax+0x18]
     and    eax, 0x1
     test   eax, eax
     je     .setup_mprotect
     or     DWORD  [ebp-0x10], 0x4 ;prot
 .setup_mprotect:
     mov edx ,  DWORD  [ebp-0x10]
     mov ecx ,  DWORD  [ebp-0xc]
     mov ebx ,  DWORD  [ebp-0x38]
      push 125
      pop eax
      int 0x80
     test   eax, eax
     je     .update_base_pointer
     jmp    _exit

 .handle_bss:
     cmp    DWORD  [ebp-0x28], 0x0
     jle    .update_base_pointer
     mov    ecx, DWORD  [ebp-0x2c]
     add    ecx, 0xfff
     and    ecx, 0xfffff000
     mov    DWORD  [ebp-0x3c], ecx
     sub    ecx, DWORD  [ebp-0x2c]
     mov    edi, DWORD  [ebp-0x38]
     add    edi, DWORD  [ebp-0x2c]
      xor eax, eax
      rep stosb
     mov    ecx, DWORD  [ebp-0x28] ;bss_len
     add    ecx, DWORD  [ebp-0x2c] ;len_fs
     mov    DWORD  [ebp-0x40], ecx ;len_bss_end
     ;; mov    eax, DWORD  [ebp-0x40]
     sub    ecx, DWORD  [ebp-0x3c] ;len_bss_end - len_bss_start_align;
     jle     .update_base_pointer
     mov    ebx, DWORD  [ebp-0x38]
     add    ebx, DWORD  [ebp-0x3c]
     mov edi,   0xffffffff
      push 0x32
      pop esi
     mov edx,   DWORD  [ebp-0x30]
     mov ebx,   DWORD  [ebp-0x48]
      push ebp
      xor ebp, ebp
      push 192
      pop eax
      int 0x80
      pop ebp
     cmp    eax, 0xfffff000
     ja    _exit
     cmp    eax, DWORD  [ebp-0x48]
     jne    _exit

 .update_base_pointer:
     mov    eax, DWORD  [ebp+0x8]
     mov    eax, DWORD  [eax]
     test   eax, eax
     jne    .exit_success
     mov    eax, DWORD  [ebp+0x8]
     mov    edx, DWORD  [ebp-0x38]
     mov    DWORD  [eax], edx

 .exit_success:
 .exit_error:
 .exit_function:
     mov    ebx, DWORD  [ebp-0x4]
     leave
     ret


 ; int write_dyn(void *ptr, uint32_t aslr)
 ; Parameters:
 ;   [ebp+0x8] = ptr (pointer to ELF file in memory)
 ;   [ebp+0xc] = aslr (ASLR offset to add to addresses)
      ;; done
 write_dyn:
     push   ebp
     mov    ebp, esp
     push   edi                 ; Save callee-saved registers
     push   esi
     mov    eax, DWORD  [ebp+0x8] ; eax = ptr
     push   ebx
     ; Check if aslr is zero
     cmp    DWORD  [ebp+0xc], 0x0 ; Compare aslr with 0
     je     .end_write_dyn      ; If zero, return 0

     ; Extract ELF header information
     mov    ecx, DWORD  [eax+0x1c] ; ecx = e_phoff
     movzx  edi, WORD  [eax+0x2c]  ; edi = e_phnum
     xor    edx, edx            ; edx = 0 (counter)
     add    ecx, eax            ; ecx = ptr + e_phoff (actual program header address)

 .loop_phdr:
     cmp    edx, edi            ; Compare counter with phnum
     jge    .end_write_dyn      ; If >=, return 0

     ; Check if current header is PT_DYNAMIC
     cmp    DWORD  [ecx], 0x2 ; Compare p_type with PT_DYNAMIC (2)
     je     .found_dynamic      ; If PT_DYNAMIC, process it

     ; Move to next program header
     inc    edx                 ; Increment counter
     add    ecx, 0x20           ; Advance 32 bytes to next phdr (size of Elf32_Phdr)
     jmp    .loop_phdr          ; Continue loop

 .found_dynamic:
     ; Process PT_DYNAMIC segment
     lea    edx, [eax+edx*8]    ; edx = ptr + counter * 8
     add    edx, DWORD  [ecx+0x4] ; edx = ptr + counter * 8 + p_offset
     mov    ebx, DWORD  [ecx+0x10] ; ebx = p_filesz
     xor    ecx, ecx            ; ecx = 0 (inner loop counter)
     shr    ebx, 0x3            ; ebx = p_filesz / 8 (number of dynamic entries)

 .inner_loop:
     cmp    ecx, ebx            ; Compare inner counter with entry count
     jge    .end_write_dyn      ; If >=, return 0

     ; Load current dynamic entry
     mov    eax, DWORD  [edx] ; eax = d_tag
     cmp    eax, 0x1a           ; Compare with DT_JMPREL (26)
     jg     .check_large_tags   ; If > 26, check large tags
     cmp    eax, 0x2            ; Compare with DT_NEEDED (2)
     jle    .next_inner         ; If <= 2, skip (not an address)
     sub    eax, 0x3            ; Subtract 3
     cmp    eax, 0x17           ; Compare with 0x17
     ja     .next_inner         ; If > 0x17, skip
     jmp    .adjust_address     ;

 .check_large_tags:
     ; Check specific large tags that need address adjustment
     cmp    eax, 0x6ffffff0     ; Compare with DT_LOPROC special tag
     je     .adjust_address     ; If equal, adjust address
     cmp    eax, 0x6ffffffe     ; Compare with DT_VERNEED special tag
     je     .adjust_address     ; If equal, adjust address
     cmp    eax, 0x6ffffef5      ; Compare with DT_ADDRRNGHI special tag
     jne    .next_inner         ; If not equal, continue to next entry

 .adjust_address:
     ; Add aslr_offset to dynamic entry value
     mov    eax, DWORD  [ebp+0xc] ; eax = aslr
     add    DWORD  [edx+0x4], eax ; d_val += aslr

 .next_inner:
     inc    ecx                 ; Increment inner counter
     jmp    .inner_loop         ; Continue inner loop

 .end_write_dyn:
     pop    ebx                 ; Restore callee-saved registers
     xor    eax, eax            ; Return 0 (success)
     pop    esi
     pop    edi
     pop    ebp                 ; Restore stack frame
     ret                       ; Return

 ; void* load_elf(void *ptr, uint32_t aslr_offset, int fd)
 ;   [ebp+0x8] = ptr ehdr
 ;   [ebp+0xc] = aslr
 ;   [ebp+0x10] = fd
 ; Function: load_elf
 ; Purpose: Loads an ELF file into memory, handling segments, mapping memory, and dynamic linking.
 load_elf:
 ; Prologue
 push   ebp
 mov    ebp, esp
 sub    esp, 0x38

 ; Save the pointer to the ELF header (first argument)
 mov    eax, DWORD  [ebp+0x8]
 mov    DWORD  [ebp-0x20], eax

 ; Get the offset of the program header table from the ELF header
 mov    eax, DWORD  [ebp-0x20]
 mov    edx, DWORD  [eax+0x1c] ; e_phoff
 mov    eax, DWORD  [ebp+0x8]
 add    eax, edx                  ; Address of program header table
 mov    DWORD  [ebp-0x24], eax ; Store it

 ; Get the number of program headers (e_phnum) and store it
 mov    eax, DWORD  [ebp-0x20]
 movzx  eax, WORD  [eax+0x2c]  ; e_phnum
 movzx  eax, ax                   ; Zero extend to 32 bits
 mov    DWORD  [ebp-0x14], eax ; Store number of headers

 ; Initialize loop control variables
 mov    DWORD  [ebp-0xc], 0xffffffff ; Initialize min virtual address
 mov    DWORD  [ebp-0x10], 0x0      ; Initialize max virtual address

 ; Check if there are no program headers
 cmp    DWORD  [ebp-0x14], 0x0
 je     _exit

 ; Loop body: Process program headers to find min/max virtual addresses
 program_header_loop_body:
     ; Get the current program header index
     mov    eax, DWORD  [ebp-0x14]
     shl    eax, 0x5          ; Multiply by 32
     add    eax, DWORD  [ebp-0x24]
     ; Get the program header type (p_type)
     mov    eax, DWORD  [eax]
     ; Check if it's a LOAD segment (PT_LOAD = 1)
     cmp    eax, 0x1
     jne    .program_header_loop_next

     ; Get the virtual address of the segment (p_va)
     mov    eax, DWORD  [ebp-0x14]
     shl    eax, 0x5          ; Multiply by 32
     add    eax, DWORD  [ebp-0x24]
     mov    edx, DWORD  [eax+0x8] ; p_va

     ; Update mmin virtual address if needed
     cmp    edx, DWORD  [ebp-0xc]
     jae    .no_update_min_addr
     mov    DWORD  [ebp-0xc], edx
 .no_update_min_addr:
      add edx, DWORD [eax+0x14] ; p_va + p_memsz
     cmp    edx, DWORD  [ebp-0x10]
      jle .program_header_loop_next
      mov    DWORD  [ebp-0x10], edx

 .program_header_loop_next:
     ; Get the current program header index
     mov    eax, DWORD  [ebp-0x14]
     ; Decrement index
      dec eax
     mov    DWORD  [ebp-0x14], eax

     ; Test if we've processed all headers
     test   eax, eax
     jns    program_header_loop_body

 ; Ensure min virtual address is page-aligned
 and    DWORD  [ebp-0xc], 0xfffff000
 mov eax, DWORD  [ebp-0xc]  ; Min virtual address
 mov DWORD [ebp-0x34], eax  ; vmap = min

 cmp DWORD [ebp+0x10], -1
 je skip_alignment_adjustment.skip_mmap

 ; Calculate the size of the memory region needed
 mov    eax, DWORD  [ebp-0x10]
 sub    eax, DWORD  [ebp-0xc]
 mov    DWORD  [ebp-0x30], eax ; Store size

 ; Initialize loop counter for segment loading
 mov    DWORD  [ebp-0x2c], 0x0

 ; Set up mmap flags (MAP_PRIVATE | MAP_ANONYMOUS)
 mov    DWORD  [ebp-0x18], 0x22

 ; Initialize file offset
 mov    DWORD  [ebp-0x34], 0x0

 ; Check if the segment alignment flag is set (indicates special handling needed)
 cmp    DWORD  [ebp+0x10], 0xffffffff
 jne    skip_alignment_adjustment   ; If not -1, no adjustment needed

 ; Adjust base address by adding the file offset to the segment's virtual address
 mov    edx, DWORD  [ebp+0xc]  ; File offset (second arg)
 mov    eax, DWORD  [ebp-0xc]  ; Min virtual address
 add    eax, edx                  ; Add to max virtual address
 mov    DWORD  [ebp-0x34], eax ; Store adjusted base address

 ; Set additional mmap flag (MAP_FIXED)
 or     DWORD  [ebp-0x18], 0x10

 skip_alignment_adjustment:

 ; Call mmap to allocate memory
      push -1
      pop edi		   ;fd
 mov esi,   DWORD [ebp-0x18]      ; Protection flags (MAP_PRIVATE | MAP_ANONYMOUS)
 xor   edx, edx                       ; PROT_NONE
 mov    ecx, DWORD  [ebp-0x30] ; Size of region
 mov    ebx, DWORD  [ebp-0x34] ; Base address
 push ebp
      xor ebp, ebp
      mov eax, 192
 int 0x80; mmap system call
 pop ebp

 ; Store returned address from mmap
      cmp DWORD [ebp-0x34], 0
      je .save_mmap
 cmp    DWORD  [ebp-0x34], eax
      jne _exit
      .save_mmap:
 mov DWORD [ebp-0x34], eax
 .skip_mmap:
 mov    DWORD  [ebp-0x1c], 0x0 ; Reset segment counter

 ; Segment loading loop (load each segment into memory)
 segment_loading_loop_body:
     ; Get the current segment index
     mov    eax, DWORD  [ebp-0x1c]
     ; Calculate offset within program header table (index * sizeof(Elf32_Phdr))
     shl    eax, 0x5          ; Multiply by 32 (size of Elf32_Phdr)
     mov    edx, eax
     ; Get the address of the program header table
     mov    eax, DWORD  [ebp-0x24]
     ; Add the offset to get the address of the current program header
     add    eax, edx

     ; Prepare arguments for load_elf_segment function
     sub    esp, 0xc          ; Align stack
     push   DWORD  [ebp+0x10] ; fd
     push   DWORD  [ebp-0xc] ; min_va
     push   eax               ; phdr
     push   DWORD  [ebp+0x8] ; Pointer to ELF header
     lea    eax, [ebp-0x34]   ; Address of mmap result
     push   eax               ; Pass reference to mmap result
     call   load_elf_segment ; Load the segment
     add    esp, 0x20         ; Clean up stack

     ; Increment the segment counter
     add    DWORD  [ebp-0x1c], 0x1

 ; Loop condition check for segment loading
 segment_loading_loop_condition:
     ; Get the number of program headers again
     mov    eax, DWORD  [ebp-0x20]
     movzx  eax, WORD  [eax+0x2c]  ; e_phnum
     movzx  eax, ax                   ; Zero extend to 32 bits

     ; Compare with segment counter
     cmp    DWORD  [ebp-0x1c], eax
     jl     segment_loading_loop_body ; If counter < num_headers, continue loop

 ; Check if the segment alignment flag is set again (after loop)
 cmp    DWORD  [ebp+0x10], 0xffffffff
 jne    skip_dynamic_linking      ; If not -1, skip dynamic linking

 ; Perform dynamic linking if needed
 mov    eax, DWORD  [ebp-0x34] ; mmap result (base address)
 sub    esp, 0x8                  ; Align stack
 push   DWORD  [ebp+0xc]       ; File offset
 push   eax                       ; Base address
 call   write_dyn
 add    esp, 0x10                 ; Clean up stack

 skip_dynamic_linking:

 ; Calculate final address of the end of the loaded image
 mov    eax, DWORD  [ebp-0x20]
 mov    eax, DWORD  [eax+0x18] ; e_entry
 sub    eax, DWORD  [ebp-0xc]  ; Subtract min va
 mov    edi, DWORD  [ebp-0x34] ; mmap result (base address)
 add    eax, edi                  ; Add base address to get final address
     leave
     ret

 ; uint32_t get_at_interp_x32(void *ptr)
 ; rdi = ptr
 get_at_interp_x32:
     push   ebp                 ; Save caller's base pointer
     xor    eax, eax            ; eax = 0 (counter)
     xor edi, edi	       ; for case of error.
     mov    ebp, esp            ; Set up new stack frame
     push   esi                 ; Save callee-saved registers
     push   ebx
     sub    esp, 0x400          ; Allocate large stack space

     ; Load ELF header and program header information
     mov    ebx, DWORD  [ebp+0x8] ; ebx = ptr
     mov    ecx, DWORD  [ebx+0x1c] ; ecx = e_phoff
     movzx  esi, WORD  [ebx+0x2c] ; esi = e_phnum
     add    ecx, ebx            ; ecx = ptr + e_phoff (actual program header address)

 .loop_phdr:
     cmp    esi, eax            ; Compare phnum with counter
     jbe    .not_found          ; If <=, return 0 (not found)

     ; Calculate current program header address
     mov    edx, eax            ; edx = counter
     shl    edx, 0x5            ; edx = counter * 32
     cmp    DWORD  [ecx+edx*1], 0x3 ; Compare p_type with PT_INTERP (3)
     je     .found_interp       ; If PT_INTERP, process it

     ; Move to next program header
     inc    eax                 ; Increment counter
     jmp    .loop_phdr          ; Continue loop

 .not_found:
     mov    edx, 0x0            ; edx = 0 (return value)
     je     .end_interp        ; Jump to end if zero flag is set

 .found_interp:
     ; Process PT_INTERP segment
     shl    eax, 0x5            ; eax = counter * 32
     add    ebx, DWORD  [ecx+eax*1+0x4] ; ebx = ptr + phdr[counter].p_offset
     xor    ecx, ecx            ; ecx = 0 (flags for open)
     mov    eax, 0x5            ; eax = SYS_open
     int    0x80                ; Call open syscall
     mov    esi, eax            ; esi = file descriptor
     test   eax, eax            ; Test file descriptor
     js    _exit

 .read_interp:
     lea    ecx, [ebp-0x408]    ; ecx = buffer on stack
     mov    eax, 0x3            ; eax = SYS_read
     mov    edx, 0x400          ; edx = buffer size (1024 bytes)
     mov    ebx, esi            ; ebx = file descriptor
     int    0x80                ; Call read syscall
     cmp    eax, 0x3ff          ; Compare bytes read with 1023
     jle     _exit        ; If < 1023, load interpreter

 .load_interp:
     push   esi                 ; Push file descriptor
     mov    ebx, esi            ; ebx = file descriptor
     push   0x0                 ; Push parameter
     push   ecx                 ; Push interpreter buffer
     call   load_elf    		; Call load_elf
     mov    edx, eax            ; edx = result of load_elf_generic
     mov    eax, 0x6            ; eax = SYS_close
     int    0x80                ; Call close syscall
      test eax, eax
      jnz _exit
     ;; add    esp, 0xc           ; Deallocate stack space

 .end_interp:
     lea    esp, [ebp-0x8]      ; Deallocate local variables
     mov    eax, edx            ; eax = return value
     pop    ebx                 ; Restore callee-saved registers
     pop    esi
     pop    ebp                 ; Restore stack frame
     ret                       ; Return


 _exit:
      xor ebx, ebx
      push 1
      pop eax
      int 0x80

 end_s2:
      call return_ehdr_ptr
