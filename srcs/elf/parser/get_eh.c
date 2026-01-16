#include "format_elf.h"

uint16_t	elf_get_elfh_type(const t_elf_h *ehdr)
{
  const unsigned char	class = elf_get_elfh_e_ident_class(ehdr->elf32->e_ident);

  if (class == ELFCLASS32)
    return (ehdr->elf32->e_type);
  return (ehdr->elf64->e_type);
}

uint16_t	elf_get_elfh_machine(const t_elf_h *ehdr)
{
  const unsigned char	class = elf_get_elfh_e_ident_class(ehdr->elf32->e_ident);

  if (class == ELFCLASS32)
    return (ehdr->elf32->e_machine);
  return (ehdr->elf64->e_machine);
}

uint32_t	elf_get_elfh_version(const t_elf_h *ehdr)
{
  const unsigned char	class = elf_get_elfh_e_ident_class(ehdr->elf32->e_ident);

  if (class == ELFCLASS32)
    return (ehdr->elf32->e_version);
  return (ehdr->elf64->e_version);
}

uint32_t	elf_get_elfh_flags(const t_elf_h *ehdr)
{
  const unsigned char	class = elf_get_elfh_e_ident_class(ehdr->elf32->e_ident);

  if (class == ELFCLASS32)
    return (ehdr->elf32->e_flags);
  return (ehdr->elf64->e_flags);
}

uint16_t	elf_get_elfh_shstrndx(const t_elf_h *ehdr)
{
  const unsigned char	class = elf_get_elfh_e_ident_class(ehdr->elf32->e_ident);

  if (class == ELFCLASS32)
    return (ehdr->elf32->e_shstrndx);
  return (ehdr->elf64->e_shstrndx);
}
