#include "format_elf.h"

  bool	elf_is_valid_elf_shoff(const t_elf_h *ehdr, off_t *elf_size)
  {
    const uint64_t shoff = elf_get_elfh_shoff(ehdr);
    const uint64_t elfhsize = (uint64_t)elf_get_struct_size_elf_header((char *)ehdr->elf32);
   const uint64_t shsize = elf_get_section_header_struct_size (ehdr);
   const uint64_t shnum = elf_get_elfh_shnum(ehdr);

    return (shoff < (uint64_t )*elf_size
            && shoff >= elfhsize  &&
            shoff + shnum * shsize <= (uint64_t)*elf_size);
  }

  bool	elf_is_valid_elf_phoff(const t_elf_h *ehdr, off_t *elf_size)
  {
    return (!elf_get_elfh_phnum(ehdr)
            || (elf_get_elfh_phoff(ehdr) >= (uint64_t)elf_get_struct_size_elf_header((char *)ehdr->elf32)
            && elf_get_elfh_phoff(ehdr) < (uint64_t)*elf_size
                && (elf_get_elfh_phoff(ehdr) + elf_get_elfh_phnum(ehdr) * elf_get_elfh_phentsize(ehdr)) <= (uint64_t)*elf_size));
  }
