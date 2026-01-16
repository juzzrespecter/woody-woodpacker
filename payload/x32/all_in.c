#include <elf.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PAGE_SIZE 0x1000

// Funciones específicas para syscalls comunes
static inline void *sys_mprotect(void *addr, size_t length, int prot) {
    // En x86, mmap usa el syscall 91 (mmap2 sería 192)
  long result;
   __asm__ volatile (
	"int $0x80\n\t"
	: "=a" (result)
        : "a" (192), "b" (addr), "c" (length), "d" (prot)
        : "memory");
    return (void *)result;
}

static inline void *sys_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
  long result;
   __asm__ volatile (
	"pushl %%ebp\n\t"
	"movl %7, %%ebp\n\t"
	"int $0x80\n\t"
	"popl %%ebp\n\t"
	: "=a" (result)
        : "a" (192), "b" (addr), "c" (length), "d" (prot), "S" (flags), "D" (fd), "m"(offset)
        : "memory"

		     );
    return (void *)result;
}

int load_elf_segment(void **base, void *elf_ptr, Elf32_Phdr *phdr, unsigned int min_va, int fd)
{
    const unsigned int page_offset = phdr->p_offset & ~(PAGE_SIZE - 1);
    const unsigned int offset_dif = phdr->p_offset - page_offset;
    const unsigned int page_va = phdr->p_va & ~(PAGE_SIZE - 1);
    const unsigned int va_dif = page_va - min_va;
    const int bss_len = phdr->p_memsz - phdr->p_filesz;
    unsigned int map_len;
    int prot;
    int prot_bk;
    unsigned int len_fs = offset_dif + phdr->p_filesz;
    void *result;

    if (phdr->p_type != PT_LOAD)
        return 0;
    prot = PROT_READ;
    if (phdr->p_flags & PF_W)
      prot |= PROT_WRITE;
    if (phdr->p_flags & PF_X)
      prot |= PROT_EXEC;
    prot_bk = prot;
    int flags = MAP_PRIVATE | MAP_FIXED;
    map_len = len_fs;
    if (fd == -1)
      {
	map_len += bss_len;
	flags |= MAP_ANONYMOUS;
	prot = PROT_READ | PROT_WRITE;
      }
    void *map_addr = (void *)(*base + va_dif);
    result = sys_mmap(map_addr, map_len, prot, flags, fd, page_offset >> 12);
    if (result >= (void *)(-4095))
      goto error;
    if (fd == -1) {
        // Copy data from ELF buffer to mapped memory
        memcpy((char *)result, (char *)elf_ptr + page_offset, len_fs);
	if (sys_mprotect(result, map_len, prot_bk) != 0)
	  goto error;
    }
    else if (bss_len > 0)
      {
	unsigned int len_bss_start_align = ((unsigned int)(len_fs + PAGE_SIZE - 1)) & ~(PAGE_SIZE -1);
	memset(result + len_fs, 0, len_bss_start_align - len_fs);
	unsigned int len_bss_end = len_fs + bss_len;
	unsigned int bss_map_len = len_bss_end - len_bss_start_align;
	if (bss_map_len > 0)
	  {
	    void *va_bss = result + len_bss_start_align;
	    void *mmap_bss = sys_mmap(va_bss, bss_map_len, prot_bk,
					MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	    if (mmap_bss <= (void *)-0xfff || va_bss != mmap_bss)
	      goto error;
	  }
      }
    return (0);
 error:
    return (-1);
}

unsigned int write_dyn(void *ptr, unsigned int aslr)
{
  return (aslr & (unsigned int)ptr);
}


// Generic ELF unpacking function that works for both main ELF and interpreter
static inline unsigned int load_elf(void *ptr, unsigned int aslr_offset, int fd)
{
    void	*mmap_va;
    Elf32_Ehdr *elf = (Elf32_Ehdr *)ptr;
    Elf32_Phdr *phdr = (Elf32_Phdr *)((char *)ptr + elf->e_phoff);
    uint32_t phnum = elf->e_phnum;
    uint32_t min_va;
    uint32_t max_va;

    min_va = -1;
    max_va = 0;
    if (!phnum)
      goto exit;
    while (phnum--) {
      if (phdr[phnum].p_type == PT_LOAD) {
	if (phdr[phnum].p_va < min_va)
	  min_va = phdr[phnum].p_va;
	if (phdr[phnum].p_va > max_va)
	  max_va = phdr[phnum].p_va + phdr[phnum].p_memsz;
      }
    }
    min_va &= ~(PAGE_SIZE - 1);
    if (fd != -1)// Interp
      {
	uint64_t total_size = (max_va + (PAGE_SIZE - 1) - min_va) & ~(PAGE_SIZE -1);

	int flags = MAP_PRIVATE | MAP_ANONYMOUS;
	mmap_va = NULL;

	mmap_va = sys_mmap(mmap_va, total_size,
			   PROT_NONE, flags, -1, 0);
	if ((uint32_t)mmap_va <= -0xfff)
	  goto exit;
      }
    else
      mmap_va = min_va + aslr_offset;
    for (int i = 0; i < elf->e_phnum; i++)
      load_elf_segment(&mmap_va, ptr, &phdr[i], (unsigned int)min_va, fd);
    if (fd == -1)
	write_dyn(mmap_va, aslr_offset);
    return ((unsigned int)mmap_va + (elf->e_entry - (unsigned int)min_va));
 exit:
    return (-1);
}



// r/eax -1 error, 0 no interp, != 0 && != -1 entrypoint to jmp.
// rdi base.

int _start(void)
{
  load_elf((void *)0,0, -1);
  return (0);
}
