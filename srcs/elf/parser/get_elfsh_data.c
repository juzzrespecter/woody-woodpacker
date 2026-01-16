#include "format_elf.h"

  uint64_t	elf_get_section_header_struct_size(const t_elf_h *ehdr)
  {
          const unsigned char	class = elf_get_elfh_e_ident_class(ehdr->elf32->e_ident);

          if (class == ELFCLASS32)
                  return (sizeof (Elf32_Shdr));
          return (sizeof (Elf64_Shdr));
  }

bool	elf_is_special_section_index(uint16_t index)
{
  static const uint16_t	special_section_index[] = {SHN_COMMON, SHN_ABS,
                                                   SHN_XINDEX};
  uint16_t						i;

  if (SHN_LOPROC <= index && index >= SHN_HIOS)
    return (true);
  i = 0;
  while (i < (sizeof (special_section_index) / sizeof (*special_section_index))  && index != special_section_index[i])
    i++;
  return (i != (sizeof (special_section_index) / sizeof (*special_section_index)));
}

  t_elf_sh	elf_get_section_header(const t_elf_h *ehdr, uint16_t index)
  {
          const uint64_t			sh_offset = elf_get_elfh_shoff(ehdr);
          const uint64_t			sh_size = elf_get_section_header_struct_size(ehdr);
          const void				*base = (void*)ehdr->elf32;
          t_elf_sh				shdr;

          if (!sh_offset || index >= elf_get_elfh_shnum(ehdr)
                  || elf_is_special_section_index(index))
          {
                  shdr.elf32 = NULL;
                  return (shdr);
          }
          shdr.elf64 = (void *) (base) + sh_offset + index * sh_size;
          return (shdr);
  }

  uint64_t	elf_get_section_header_offset(const t_elf_sh *shdr, unsigned char class)
  {
          if (!shdr->elf32)
          {
//                  ft_printf("section header Invalid\n");
                  return (0);
          }
          if (class == ELFCLASS32)
                  return ((uint64_t)shdr->elf32->sh_offset);
          return (shdr->elf64->sh_offset);
  }

  bool	valid_null_end_string_table(const t_elf_sh *shstrtab,
                          unsigned char class, char *ptr)
  {
          const uint64_t	size = elf_get_section_header_size(shstrtab, class);

          if (!ptr || !size)
                  return (false);
          return (!ptr[size - 1]);
  }
