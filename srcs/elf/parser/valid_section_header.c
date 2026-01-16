#include "format_elf.h"

  bool	elf_valid_elf_section_header_symtab_shndx(unsigned char class, const uint64_t symtab_index, const t_elf_h *ehdr)
  {
          const t_elf_sh	shdr = elf_get_section_header_by_type(ehdr, SHT_SYMTAB_SHNDX);

          if (!shdr.elf32)
                  return (false);
          return (elf_get_section_header_link(&shdr, class) == symtab_index);
  }

  bool	elf_valid_elf_section_headers(unsigned char elf_class, const t_elf_h *ehdr)
  {
          const uint64_t	eh_shnum = elf_get_real_shnum(ehdr, elf_class);
          const uint64_t	real_shstrndx = elf_get_real_shstrndx(ehdr, elf_class);
          uint64_t		id;
          uint64_t		symtab_shndx[2];

          if (!eh_shnum || !real_shstrndx)
                  return (false);
          id = 0;
          bzero(symtab_shndx, sizeof (symtab_shndx));
          while (id < eh_shnum && elf_valid_elf_section_header_handler(elf_class, &symtab_shndx[0], ehdr, id))
                  id++;
          if (id < eh_shnum)
                  return (false);
          if (real_shstrndx >= eh_shnum
                  || elf_get_section_header_type_by_index(ehdr, real_shstrndx)
                  != SHT_STRTAB)
                  return (false);
          return (!symtab_shndx[0]
                  || elf_valid_elf_section_header_symtab_shndx(elf_class, symtab_shndx[1], ehdr));
  }
