#include "format_elf.h"

uint16_t	elf_get_elfh_ehsize(const t_elf_h *ehdr)
{
  const unsigned char	class = elf_get_elfh_e_ident_class(ehdr->elf32->e_ident);

  if (class == ELFCLASS32)
    return (ehdr->elf32->e_ehsize);
  return (ehdr->elf64->e_ehsize);
}

uint16_t	elf_get_elfh_phentsize(const t_elf_h *ehdr)
{
  const unsigned char	class = elf_get_elfh_e_ident_class(ehdr->elf32->e_ident);

  if (class == ELFCLASS32)
    return (ehdr->elf32->e_phentsize);
  return (ehdr->elf64->e_phentsize);
}

uint16_t	elf_get_elfh_phnum(const t_elf_h *ehdr)
{
  const unsigned char	class = elf_get_elfh_e_ident_class(ehdr->elf32->e_ident);

  if (class == ELFCLASS32)
    return (ehdr->elf32->e_phnum);
  return (ehdr->elf64->e_phnum);
}

uint16_t	elf_get_elfh_shentsize(const t_elf_h *ehdr)
{
  const unsigned char	class = elf_get_elfh_e_ident_class(ehdr->elf32->e_ident);

  if (class == ELFCLASS32)
    return (ehdr->elf32->e_shentsize);
  return (ehdr->elf64->e_shentsize);
}

uint16_t	elf_get_elfh_shnum(const t_elf_h *ehdr)
{
  const unsigned char	class = elf_get_elfh_e_ident_class(ehdr->elf32->e_ident);

  if (class == ELFCLASS32)
    return (ehdr->elf32->e_shnum);
  return (ehdr->elf64->e_shnum);
}
