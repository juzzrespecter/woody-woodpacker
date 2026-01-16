#include "format_elf.h"

bool elf_is_file_size_enought_for_elf_header(off_t file_size, void *elf_mmap)
{
  if (file_size < EI_NIDENT)
    return (false);
  return (file_size
          >= elf_get_struct_size_elf_header(elf_mmap));
}

bool	elf_is_valid_elf_data_format(char *elf_mmap)
{
  const char	data = elf_mmap[EI_DATA];

  return (ELFDATA2LSB == data|| ELFDATA2MSB == data);
}

bool	elf_is_valid_elf_version(char *elf_mmap)
{
  return (EV_CURRENT == elf_mmap[EI_VERSION]);
}

bool	elf_is_executable_elf_file(void *elf_mmap)
{
  const t_elf_h ehdr = (t_elf_h){.elf32 = elf_mmap};
  const uint16_t type = elf_get_elfh_type(&ehdr);

  return ((ET_EXEC == type || ET_DYN == type) && elf_get_elfh_phnum(&ehdr) && elf_get_elfh_entry(&ehdr));
}
