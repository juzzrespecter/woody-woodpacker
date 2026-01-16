#include "format_elf.h"

  const char	*elf_get_section_string_table(const t_elf_h *ehdr)
  {
          const uint16_t	sh_str_index = elf_get_elfh_shstrndx(ehdr);
          const t_elf_sh	shstrtab_shdr = elf_get_section_header(ehdr, sh_str_index);
          const uint64_t	shstr_tab_offset = elf_get_section_header_offset(&shstrtab_shdr,
                          elf_get_elfh_e_ident_class(ehdr->elf32->e_ident));
          char			*ptr;

          if (!shstr_tab_offset && !shstrtab_shdr.elf32)
                  return (NULL);
          ptr = (char *)ehdr->elf32 + shstr_tab_offset;
          if (!valid_null_end_string_table(&shstrtab_shdr,
                          elf_get_elfh_e_ident_class((void *)ehdr->elf32), ptr))
                  return (NULL);
          return (ptr);
  }

  uint32_t	elf_get_section_header_name(const t_elf_sh *shdr, unsigned char class)
  {
          if (!shdr || !shdr->elf32)
                  return (0);
          if (class == ELFCLASS32)
                  return (shdr->elf32->sh_name);
          if (class == ELFCLASS64)
                  return (shdr->elf64->sh_name);
          return (0);
  }

  const char	*elf_get_section_name(const char *str_tab, uint64_t index_str_tab)
  {
          if (!str_tab)
                  return (NULL);
          return (str_tab + index_str_tab);
  }
