#include "format_elf.h"

bool	elf_valid_elf_magic_number(void *elf_mmap)
{
  return (!ft_memcmp(elf_mmap, ELFMAG, (size_t)SELFMAG));
}

bool elf_valid_elf_file(void *elf_mmap, off_t elf_size)
{
  const t_elf_h ehdr = (t_elf_h){.elf32 = elf_mmap};
  const unsigned char class = elf_get_elfh_e_ident_class(elf_mmap);

  if (elf_valid_elf_header(&ehdr, elf_size)
      && elf_valid_elf_section_headers(class, &ehdr)
      && elf_valid_elf_phs(elf_size, &ehdr))
    return (true);
  return (false);
}

int elf_valid_file(void *elf_mmap, off_t elf_size)
{
  return (elf_valid_elf_file(elf_mmap, elf_size)
 	    &&  elf_is_executable_elf_file(elf_mmap));
/*        &&  elf_is_executable_elf_file(elf_mmap);*/
/* 	   && (elf_get_elfh_e_ident_class(elf_mmap) == ELFCLASS64 || is_valid_cpu_x86())));*/
}
