#include "format_elf.h"

  unsigned char	elf_get_elfh_e_ident_class(const unsigned char *e_ident)
  {
          return (e_ident[EI_CLASS]);
  }

  unsigned char	elf_get_elfh_e_ident_data(const unsigned char *e_ident)
  {
          return (e_ident[EI_DATA]);
  }

  unsigned char	elf_get_elfh_e_ident_version(const unsigned char *e_ident)
  {
          return (e_ident[EI_VERSION]);
  }

  unsigned char	elf_get_elfh_e_ident_osabi(const unsigned char *e_ident)
  {
          return (e_ident[EI_OSABI]);
  }

  unsigned char	elf_get_elfh_e_ident_abiversion(const unsigned char *e_ident)
  {
          return (e_ident[EI_ABIVERSION]);
  }
