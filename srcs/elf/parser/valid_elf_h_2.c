#include "format_elf.h"

bool	elf_valid_t_elf_h(const t_elf_h *ehdr, off_t file_size)
{
  return (elf_valid_section_size(ehdr)
          && elf_valid_section_header_string_index(ehdr)
          && elf_is_valid_elf_shoff(ehdr, &file_size)
          && elf_is_valid_elf_phoff(ehdr, &file_size));
}

bool	elf_valid_elf_header(const t_elf_h *ehdr, off_t file_size)
{
  return (elf_valid_ident(file_size, (void *)ehdr->elf32) && elf_valid_t_elf_h(ehdr, file_size));
}
