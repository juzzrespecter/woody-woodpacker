#include "format_elf.h"

  static bool	elf_valid_elf_ph(unsigned char elf_class, off_t elf_size, const t_elf_h *ehdr,
                          uint16_t index)
  {
          const t_elf_ph	phdr = elf_get_program_header(ehdr, index);
          const uint16_t	size = elf_get_struct_size_elf_ph(elf_class);
          unsigned char	p_align;

          if (!phdr.elf32 || !ehdr->elf32
                  || (void *)phdr.elf32 + size >= (void *)ehdr->elf32 + elf_size)
                  return (false);
          p_align = elf_get_program_header_align(&phdr, elf_class);
          if (p_align != (p_align & -p_align))
                  return (false);
          if ((off_t)(elf_get_program_header_offset(&phdr, elf_class)
                  + elf_get_program_header_filesz(&phdr, elf_class)) >= elf_size)
                  return (false);
          return (true);
  }

  bool	elf_valid_elf_phs(off_t elf_size, const t_elf_h *ehdr)
  {
          const uint64_t		real_phnum	= elf_get_real_phnum(ehdr);
          const unsigned char	class		= elf_get_elfh_e_ident_class((void *)ehdr->elf32);
          uint64_t			index;

          if (!real_phnum)
                  return (true);
          if (elf_get_struct_size_elf_ph(class) != elf_get_elfh_phentsize(ehdr))
                  return (false);
          if (signed_overflow_limit (real_phnum, elf_get_struct_size_elf_ph(class), elf_size))
                  return (false);
          index = 0;
          while (index < real_phnum
                  && elf_valid_elf_ph(class, elf_size, ehdr, index))
                  index++;
          return (index >= real_phnum);
  }
