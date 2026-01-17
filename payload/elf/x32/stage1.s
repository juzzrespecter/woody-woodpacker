global _start

_start:
.mmap_stage_2:
       	xor    ebp, ebp 	;offset
       	push -1			;
       	pop    edi		;fd
      	push 0x22
      	pop esi
      	push 0x3		;RW-
      	pop edx
      	mov ecx, 0xfeffffff	; (len) s2
      	push ecx		;[esp] (len) s2
       	xor    ebx,ebx 		;Va
      	mov eax, 192
      	int 0x80
       	cmp eax, -4095		;Mmap error.
       	jae _exit
      	pop ecx			; (len) s2 [esp]
      	push ecx		; [esp] (len) s2
      	push eax		; [esp] (va, len) s2
.decrypt_mmap:
       	xor    ebp, ebp 	;offset
       	push -1			;
       	pop    edi		;fd
      	push 0x22
      	pop esi
      	push 0x3		;RW-
      	pop edx
       	xor    ebx,ebx 		;Va
      	mov eax, 192
      	int 0x80
       	cmp eax, -4095		;Mmap error.
       	jae _exit
	xchg edi, eax		;Save decrypt buffer in edi
	pop eax
	pop ecx
	push eax		;[esp] (va)s2
	push ecx		;[esp] (len, va) s2
      	jmp end_of_stage1
.return_crypt_compress_addr:
      	pop esi		; -> input
	pop edx		;(len) s2 [esp] (va) s2
	push edx	;[esp] (len, va) s2
      	push eax	;[esp] (va, len, va) s2
      	push esi	;-> in for simplify let's pass all an then he choose.
      	push edx	;-> in_len
      	push edi	;-> out
      	push edx	;-> out_len
.set_aes_key:
	call  .param_aes_key
	times 16 nop
.param_aes_key:
	pop eax
	movdqu xmm0, [eax]
.call_decrypt:
      	call decrypt	; The munmap fail cause expect return out but the mock return in.
      	add esp, 0x10
      	test eax, eax
      	jz _exit
      	pop edx		; (va) s2 ; [esp] (len, va) s2
      	push edx	;[esp] (va, len, va) s2.
      	push 0xfeffffff		; ;[esp] (len) c_s2, (va) s2,  (len, va) s2.
      	push eax 	;[esp] (va, len) c_s2, (va, len, va) s2.
      	push edx	;[esp] ((dst) s2, (va, len) c_s2) func params, (va, len, va) s2.
      	call lz77_decompress	;Previous 3 B -> New 8 B
	test eax, eax
      	jnz _exit		;Fine
.munmap_decrypt: 	;[esp] ((dst) s2, (va, len) c_s2) func params, (va, len, va) s2.
	pop ebx			;(va) s2,[esp] (va, len) c_s2) func params, (va, len, va) s2.
	pop ebx			;(va) c_s2[esp] ((len) c_s2) func params, (va, len, va) s2.
        pop ecx 		;(len) c_s2[esp] (va, len, va) s2.
        push 91			;munmap[esp] (va, len, va) s2.
	pop eax			;[esp] (va, len, va) s2.
	int 0x80		;[esp] (va, len, va) s2.
.set_var_stage_2:		;[esp] (va, len, va) s2.
	pop ebx			;(va) s2 [esp] (len, va) s2.
.mprotect_stage_2:
      	push 0x5		;Flags R-X
      	pop edx			;[esp] (len, va) s2
      	pop ecx			;(len) s2 [esp] (va) s2
      	push ecx		;[rsp]  (len, va) s2
      	push 125
      	pop eax
      	int 0x80
      	test eax, eax
        jnz _exit
.init_open_s1:
      	call .aslr
      ._proc_self_exe:
      	times 15 ret
.aslr:				;Save ebx
      	pop ecx
      	mov eax, ecx
      	mov esi, 0xfeffffff	;expected va
      	sub eax, esi
      	push eax		;;[rsp]  aslr, (len, va) s2
.save_va2:
      	mov edx, ebx		;edx = va s2
.va_s1:
      	mov eax, ecx
      	and eax, 0xfffff000	;s1 va
      	push eax		;[rsp] (va)s1, aslr, (len, va) s2
.len_s1:
      	push 0xfeffffff		;[rsp] (len, va)s1, aslr, (len, va) s2
.open:
      	mov ebx, ecx
      	xor ecx, ecx		;RD_ONLY
      	push 5
      	pop eax
      	int 0x80
      	test eax, eax
      	js _exit
      	push eax		;[rsp] (fd, len, va) s1, aslr, (len, va) s2
.jmp_stage_2:
      	mov eax, 0xfeffffff		;Modify in C.
      	add edx, eax
      	push edx ;; push (va) s2
      	ret;		 ;(pop rip [rsp] -> va_s2_elf)[rsp] (fd, len, va) s1, aslr, (len, va) s2

_exit:
      	xor ebx, ebx		;
      	push 1
      	pop eax
      	int 0x80

      ; lz77_decompress - Decompresses data using a simple LZ77 variant
      ; Parameters:
      ;   [ebp+0x8] = input buffer (compressed data)
      ;   [ebp+0xc] = input size
      ;   [ebp+0x10] = output buffer (decompressed data)
      ; Returns:
      ;   eax = 0 on success, -1 on error
      ; Notes:
      ;   - Compressed format: [token][literal_byte] repeated
      ;   - Token: 16-bit value [distance:12][length:4]
      ;   - If length == 0, copy literal_byte directly
      ;   - If length > 0, copy length+2 bytes from (current_pos - distance)
      ;   - Each iteration processes 3 bytes: 2-byte token + 1-byte literal

      lz77_decompress:
          ; Function prologue - set up stack frame and save registers
          mov    ebp, esp              ; Set up new stack frame
          push   edi                 ; Save callee-saved registers
          push   esi
          push   ebx
          sub    esp, 0x1c           ; Allocate local variables (0x1c = 28 bytes)

          ; Load and validate parameters
          mov    edi, DWORD  [ebp+0xc]  ; edi = input_size
          mov    esi, DWORD  [ebp+0x8]  ; esi = input_buffer

          cmp    edi, 0x3            ; Compare input_size with 3
          jna  _exit
          ; Validate input buffer pointer
          test   esi, esi            ; Test if input_buffer is NULL
          jz     _exit; Jump if input_buffer is NULL
          cmp    DWORD  [ebp+0x10], 0x0 ; Check if output_buffer is NULL
          je    _exit; Jump if validation failed (input_size <= 3 or output_buffer == NULL)


          ; Load original (expected) decompressed size from input buffer
          mov    eax, DWORD  [esi] ; eax = *(uint32_t *)input_buffer (original size)
          mov    DWORD  [ebp-0x1c], eax ; Save original_size to local variable
          test   eax, eax            ; Test if original_size is 0
          jz     _exit; Jump if original_size is 0

          ; Initialize loop variables
          lea    eax, [esi+0x7]      ; eax = input_buffer + 7 (skip 4-byte size + 3-byte first token)
          mov    DWORD  [ebp-0x20], eax ; Save current_input_ptr to local variable
          lea    ecx, [esi+edi*1]    ; ecx = input_buffer + input_size (end of input)
          mov    DWORD  [ebp-0x24], ecx ; Save end_of_input_ptr to local variable

          ; Check if we have enough data to process
          cmp    eax, ecx            ; Compare current_input_ptr with end_of_input_ptr
          mov    ecx, eax            ; ecx = current_input_ptr (prepare for loop)
          ja     .check_completion   ; Jump if current_input_ptr > end_of_input_ptr (no data to process)

          ; Main decompression loop
      .loop_init:
          xor    eax, eax            ; eax = 0 (output_position counter)
          cmp    eax, DWORD  [ebp-0x1c] ; Compare output_position with original_size
          jnc    .check_completion   ; Jump if output_position >= original_size (done)

      .loop_start:
          ; Process LZ77 token
          mov    ecx, DWORD  [ebp-0x20] ; ecx = current_input_ptr
          mov    di, WORD  [ecx-0x3] ; di = *(uint16_t *)(current_input_ptr - 3) (token)
          mov    edx, edi            ; edx = di (copy token)
          and    dx, 0xf             ; dx = di & 0xf (extract length bits 0-3)
          jz     .copy_literal       ; Jump if length is 0 (copy literal byte only)

          ; Process match (length > 0)
          shr    di, 0x4             ; di = di >> 4 (shift to get distance bits 4-15)
          movzx  edi, di             ; edi = di (zero extend to 32 bits)
          test   edi, edi            ; Test if distance is 0
          jz     .copy_literal       ; Jump if distance is 0 (copy literal byte only)
          cmp    edi, eax            ; Compare distance with output_position
          ja     .copy_literal       ; Jump if distance > output_position (invalid, copy literal)

      .copy_match:
          ; Copy match data from previously decompressed output
          mov    esi, eax            ; esi = output_position (destination index)
          add    edx, 0x2            ; edx = length + 2 (minimum match length is 3)
          mov    ecx, DWORD  [ebp+0x10] ; ecx = output_buffer
          sub    esi, edi            ; esi = output_position - distance (source index)
          mov    edi, DWORD  [ebp-0x1c] ; edi = original_size
          movzx  edx, dx             ; edx = length (zero extend to 32 bits)
          add    esi, DWORD  [ebp+0x10] ; esi = output_buffer + (output_position - distance) (source address)
          add    ecx, eax            ; ecx = output_buffer + output_position (destination address)
          sub    edi, eax            ; edi = original_size - output_position (remaining space)
          cmp    edi, edx            ; Compare remaining space with length to copy
          cmovbe edx, edi             ; If length > remaining space, use remaining space
          mov    edi, ecx            ; edi = destination address
          add    edx, ecx            ; edx = destination address + adjusted length (end pointer)
      .copy_match_loop:
          cmp    edx, edi            ; Compare end pointer with start pointer
          je     .copy_literal_after_match ; Jump if equal (length 0, copy literal)
          movsb
          jmp    .copy_match_loop    ; Jump if not equal (continue copying)

      .copy_literal_after_match:
          ; Update output position after copying match
          sub    edx, ecx            ; edx = adjusted length
          add    eax, edx            ; eax = output_position + adjusted length
          cmp    DWORD  [ebp-0x1c], eax ; Compare original_size with output_position
          jbe    .advance_input      ; Jump if original_size <= output_position (done with this iteration)

      .copy_literal:
          ; Copy literal byte from input to output
          mov    ecx, DWORD  [ebp-0x20] ; ecx = current_input_ptr
          mov    edi, DWORD  [ebp+0x10] ; edx = output_buffer
          mov    dl, BYTE  [ecx-0x1] ; dl = *(current_input_ptr - 1) (literal byte)
          mov    BYTE  [edi+eax*1], dl ; *(output_buffer + output_position) = literal byte
          inc    eax                 ; Increment output_position

      .advance_input:
          ; Advance input pointer by 3 bytes (token size + literal byte)
          add    DWORD  [ebp-0x20], 0x3 ; current_input_ptr += 3
          mov    ecx, DWORD  [ebp-0x20] ; ecx = current_input_ptr
          cmp    ecx, DWORD  [ebp-0x24] ; Compare current_input_ptr with end_of_input_ptr
          jbe    .loop_start; Jump if current_input_ptr <= end_of_input_ptr (continue loop)

      .check_completion:
          ; Check if decompression completed successfully
          cmp    eax, DWORD  [ebp-0x1c] ; Compare final output_position with original_size
          jne   _exit
          xor    eax, eax            ; Return 0 (success)
          ; Function epilogue - restore stack and registers
          lea    esp, [ebp-0x1c]      ; Deallocate local variables (esp = ebp - 4)
          pop    ebx                 ; Restore callee-saved registers
          pop    esi
          pop    edi
          mov    esp, ebp            ; Restore stack pointer
          ret                       ; Return

    ;;TODO @danrodri implement the logic.
      decrypt:
      	mov eax, esi
      	ret

      end_of_stage1:
      	call _start.return_crypt_compress_addr
