#include "woody.h"

t_file_type get_ftype(const char *file_content, const off_t *file_size)
{

  if (ELF_MAGIC_SIZE < *file_size && !ft_memcmp(file_content, ELFMAG, SELFMAG))
    return (FT_ELF);
  if (DOS_SIGN_SIZE < *file_size && !ft_memcmp(file_content, DOS_SIGN, DOS_SIGN_SIZE))
     return (FT_PE);
  return (FT_UNKNOW);
}
