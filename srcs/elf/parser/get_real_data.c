#include "format_elf.h"

uint64_t	elf_get_real_phnum(const t_elf_h *ehdr)
{
  const t_elf_sh	shdr = elf_get_section_header(ehdr, 0);
  uint64_t		phnum;

  phnum = elf_get_elfh_phnum(ehdr);
  if (PN_XNUM == phnum)
    phnum = elf_get_section_header_info(&shdr,
                                    elf_get_elfh_e_ident_class((void *)ehdr->elf32));
  return (phnum);
}

uint64_t	elf_get_real_shstrndx(const t_elf_h *ehdr, unsigned char class)
{
  const t_elf_sh	shdr = elf_get_section_header(ehdr, 0);
  uint64_t		shstrndx;

  shstrndx = elf_get_elfh_shstrndx(ehdr);
  if (shstrndx == SHN_XINDEX)
    shstrndx = elf_get_section_header_link(&shdr, class);
  return (shstrndx);
}

uint64_t	elf_get_real_shnum(const t_elf_h *ehdr, unsigned char class)
{
  const t_elf_sh	shdr = elf_get_section_header(ehdr, 0);
  uint64_t		shnum;

  shnum = elf_get_elfh_shnum(ehdr);
  if (shnum == SHN_UNDEF)
    shnum = elf_get_section_header_size(&shdr, class);
  return (shnum);
}
