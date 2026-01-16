#include "woody.h"

int main(int argc, char **argv)
{
    off_t 	file_size;
    int   	fd;        //disposable should use a function for avoid use more space than necesary along all the program.
    void  	*file_mmap;
    t_file_type	ftype;

    if (argc == 1)
      return (BAD_NBR_PARAMS);

if (get_file (argv[1], &fd) == ERROR_STDL)
  return (ERROR_STDL);

if (get_file_size(fd, &file_size) != SUCCESS)
  {
    close (fd);
    return (FAIL_LSEEK);
  }

file_mmap = mmap (NULL, file_size, PROT_WRITE | PROT_READ,
                  MAP_PRIVATE, fd, 0);
close(fd);
if (MAP_FAILED == file_mmap)
  return (FAIL_MMAP);

ftype = get_ftype (file_mmap, &file_size);
if (FT_UNKNOW <= ftype)
  return (UNRECOGNIZED_FILE);

int (*valid[2])(void*,off_t) = {
  elf_valid_file,
  elf_valid_file//Should be here the pe_valid_file.
};
if (!valid[ftype](file_mmap, file_size))
    return (UNVALID_FTYPE);

//packer ptr fun
 int (*packer[2])(void*,off_t,const char *) = {
   elf_packer,
   elf_packer//Should be here the pe_packer.
 };
 if (packer[ftype](file_mmap, file_size, "woody"))
     return (UNVALID_PACKED);

return (SUCCESS);
}
