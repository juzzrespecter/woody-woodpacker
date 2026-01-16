#include <elf.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define PAGE_SIZE 0x1000


// Generic function to calculate memory range for ELF segments
void calculate_elf_memory_range(Elf64_Phdr *phdr, int phnum, uint64_t *min_va, uint64_t *max_va)
{
    *min_va = -1;
    *max_va = 0;

    while (phnum--) {
        if (phdr[phnum].p_type == PT_LOAD) {
            if (phdr[phnum].p_va < *min_va)
                *min_va = phdr[phnum].p_va;
            uint64_t seg_end = phdr[phnum].p_va + phdr[phnum].p_memsz;
            if (seg_end > *max_va)
                *max_va = seg_end;
        }
    }

    // Align to page boundaries
    *min_va &= ~(PAGE_SIZE - 1);
    /* *max_va = (*max_va + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1); */
}

// Generic function to load a single ELF segment
// phdr = rdx
int load_elf_segment(void *elf_ptr, Elf64_Phdr *phdr, uint64_t min_va, int fd)
{
    if (phdr->p_type != PT_LOAD)
        return 0;
    char *base = min_va;
    // Align offset and size to page boundaries
    uint64_t page_offset = phdr->p_offset & ~(PAGE_SIZE - 1);
    uint64_t page_va = phdr->p_va & ~(PAGE_SIZE - 1);

    // Map at the correct address with MAP_FIXED
    int prot = PROT_READ;
    int flags = MAP_FIXED | MAP_PRIVATE;
    if (phdr->p_flags & PF_W)
      prot |= PROT_WRITE;
    if (phdr->p_flags & PF_X)
      prot |= PROT_EXEC;

    uint64_t map_end = phdr->p_offset + phdr->p_filesz;
    if (fd == -1)
      {
	if (phdr->p_memsz > phdr->p_filesz)
	  map_end += phdr->p_memsz - phdr->p_filesz;
	flags |= MAP_ANONYMOUS;
	prot = PROT_READ | PROT_WRITE;
      }

    size_t map_size = map_end - page_offset;
    void *map_addr = (char *)base + (page_va - min_va);
    void *result = mmap(map_addr, map_size, prot,
			flags, fd, page_offset);
    if (result <= -0xfff)/* == MAP_FAILED) */
      return (-1);
    if (fd == -1) {
        // Copy data from ELF buffer to mapped memory
        memcpy((char *)result, (char *)elf_ptr + page_offset, phdr->p_offset - page_offset + phdr->p_filesz);
	prot = PROT_READ;
	if (phdr->p_flags & PF_W) prot |= PROT_WRITE;
	if (phdr->p_flags & PF_X) prot |= PROT_EXEC;
	if (mprotect(map_addr, map_size, prot) < 0)
	  return (-1);
	return (0);
    }
    if (phdr->p_memsz > phdr->p_filesz)
      {

	// La dirección virtual donde comienza .bss dentro del segmento
	uint64_t va_bss_start = phdr->p_va - page_va + phdr->p_filesz;
	uint64_t va_bss_end = va_bss_start + (phdr->p_filesz - p_memsz);
	uint64_t va_bss_start_align = ((uint64_t)(offset_bss_start_align + PAGE_SIZE - 1)) & ~(PAGE_SIZE -1);
	if (va_bss_end > va_bss_start_align)
	  {
	    uint64_t bss_map_size = va_bss_end - va_bss_start_align;
	    void * va_bss_start_in_segment_align = result + va_bss_start_align;
	    void *bss_result = mmap(va_bss_start_in_segment_align, bss_map_size, prot,
				MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	    if (bss_result <= -0xfff)
	      exit (0);
	  }
      }
    return 0;
}

int write_dyn(void *ptr, uint64_t aslr);



// Generic ELF unpacking function that works for both main ELF and interpreter
uint64_t load_elf_generic(void *ptr, uint64_t aslr_offset, int fd)
{
    Elf64_Ehdr *elf = (Elf64_Ehdr *)ptr;
    Elf64_Phdr *elf_ph = (Elf64_Phdr *)((char *)ptr + elf->e_phoff);

    uint64_t min_va, max_va;

    // Calculate memory range needed
    calculate_elf_memory_range(elf_ph, elf->e_phnum, &min_va, &max_va);
    uint64_t total_size = max_va - min_va;
    uint64_t map_flags = MAP_PRIVATE | MAP_ANONYMOUS;

    // Reserve entire address space with PROT_NONE
    void *target_va = 0;
    void *base_addr;
    if (fd == -1)
      {
	target_va = min_va + aslr_offset;
	map_flags |= MAP_FIXED;
      }
    base_addr = mmap(target_va, total_size,
                        PROT_NONE, map_flags, -1, 0);

    if (base_addr <= -0xfff)
      exit(0);

    // Load each segment
    for (int i = 0; i < elf->e_phnum; i++) {
        if (load_elf_segment(base_addr, ptr, &elf_ph[i], min_va, fd) < 0)
	  exit(0);
    }
    if (fd == -1)
	write_dyn(base_addr, aslr_offset);

    uint64_t entrypoint = (uint64_t)base_addr + (elf->e_entry - min_va);
    return entrypoint;
}

int write_dyn(void *ptr, uint64_t aslr)
{
  if (!aslr)
    return (0);
  Elf64_Ehdr *ehdr = (Elf64_Ehdr *)ptr;
  Elf64_Phdr *phdrs = (Elf64_Phdr *)((char *)ptr + ehdr->e_phoff);
  Elf64_Phdr *dyn_phdr = NULL;
  int i = 0;
  while (i < ehdr->e_phnum) {
    if (phdrs[i].p_type == PT_DYNAMIC) {
      dyn_phdr = &phdrs[i];
      break;
    }
    i++;
  }
  if (!dyn_phdr)
    return (0); // No se encontró el segmento dinámico

  // Obtener la sección dinámica
  Elf64_Dyn *dynamic_section = (Elf64_Dyn *)((char *)ptr + dyn_phdr->p_offset);
  uint64_t entry_count = dyn_phdr->p_filesz / sizeof(Elf64_Dyn);
  int ndx_dyn = 0;
  while (ndx_dyn < entry_count)
    {
      Elf64_Dyn *dyn = &dynamic_section[i];
      uint64_t d_tag = dyn->d_tag;

      // Check if this entry contains a virtual address that needs updating
      switch(d_tag) {
      case DT_INIT:
      case DT_FINI:
      case DT_INIT_ARRAY:
      case DT_FINI_ARRAY:
      case DT_HASH:
      case DT_GNU_HASH:
      case DT_STRTAB:
      case DT_SYMTAB:
      case DT_PLTGOT:
      case DT_JMPREL:
      case DT_RELA:
      case DT_VERNEED:
      case DT_REL:
      case DT_VERSYM:
	dyn->d_un.d_val += aslr;
      default:
	ndx_dyn++;
      }
    }
  return (0);
}
// r/eax -1 error, 0 no interp, != 0 && != -1 entrypoint to jmp.
// rdi base.
uint64_t get_at_interp_x64(void *ptr)
{
    Elf64_Ehdr *elf = (Elf64_Ehdr *)ptr;
    Elf64_Phdr *elf_ph = (Elf64_Phdr *)((char *)ptr + elf->e_phoff);
    uint64_t ndx_ph = 0;
    char *interp_str;
    int interp_fd;
    char interp_elfh[1024];

    // Find PT_INTERP segment
    while (ndx_ph < elf->e_phnum && elf_ph[ndx_ph].p_type != PT_INTERP)
        ndx_ph++;

    if (ndx_ph == elf->e_phnum)
        return 0;  // No interpreter found

    interp_str = (char *)ptr + elf_ph[ndx_ph].p_offset;
    interp_fd = open(interp_str, O_RDONLY);
    if (interp_fd < 0)
      exit(0);

    if (read(interp_fd, &interp_elfh[0], 1024) < 1024)
      exit(0);

    // Use the generic unpacker for the interpreter
    uint64_t result = load_elf_generic(&interp_elfh[0], 0, 0, interp_fd);

    close(interp_fd);
    return result;
}

size_t lz77_decompress_asm(const unsigned char *in, size_t in_size, unsigned char *out) {
  size_t orig_size;
  size_t pos = 0;
  uint16_t dl;
  size_t len, dist;
  const unsigned char *p = in + sizeof(orig_size);

  // Validate input
  if (!in || in_size < sizeof(orig_size) || !out)
    goto error;

  // Read original size
  orig_size = *((size_t *) in);
  if (!orig_size)
    goto error;

  // Decompress loop
  while ((p + 3 <= in + in_size) && pos < orig_size) {
    // Read distance/length as little-endian
    dl = p[0] | (p[1] << 8);
    dist = (dl >> 4) & 0xFFF;
    len = (dl & 0xF);
    if (len) len += 2;

    // Copy match data
    if (len && dist && dist <= pos) {
      if (len > orig_size - pos)
	len = orig_size - pos;

      unsigned char *dst = out + pos;
      unsigned char *src = out + (pos - dist);

      while (len--)
	*dst++ = *src++;
      pos += (dst - (out + pos));  // Update pos
    }
    // Copy literal byte
    if (pos < orig_size)
      out[pos++] = p[2];
    p += 3;
  }
  // Check if we decompressed all data
  if (pos != orig_size)
    goto error;
  return 0;

 error:
  return -1;
}
