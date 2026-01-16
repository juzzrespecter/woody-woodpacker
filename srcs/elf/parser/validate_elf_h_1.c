#include "format_elf.h"

bool	elf_valid_section_size(const t_elf_h *ehdr)
{
  return (elf_get_elfh_shentsize(ehdr) == elf_get_section_header_struct_size(ehdr));
}

bool	elf_valid_section_header_string_index(const t_elf_h *ehdr)
{
  const uint16_t	shnum = elf_get_elfh_shnum(ehdr);

  return (shnum && shnum > elf_get_elfh_shstrndx(ehdr));
}

bool	elf_valid_elfh_class(const char *e_indent)
{
  const unsigned char	class = elf_get_elfh_e_ident_class((void *)e_indent);

  return (class == ELFCLASS64 || class == ELFCLASS32);
}

bool	elf_valid_ident(off_t file_size, void *elf_mmap)
{
  return (elf_is_file_size_enought_for_elf_header(file_size, elf_mmap)
          && elf_valid_elf_magic_number(elf_mmap)
          && elf_valid_elfh_class(elf_mmap) && elf_is_valid_elf_data_format(elf_mmap)
          && elf_is_valid_elf_version(elf_mmap));
}
