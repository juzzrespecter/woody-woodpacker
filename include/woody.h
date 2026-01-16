#ifndef WOODY_H
# define WOODY_H
# include <sys/mman.h> //mmap, munmap
# include <unistd.h> // lseek
# include <fcntl.h> // open, close
# include <stdio.h> // perror
# include "common.h" // t_type
# include "format_elf.h" // elf
# include "compress.h" // lz77
# include "libft.h" // lists

typedef enum e_return_errors
{
  SUCCESS = 0,
  BAD_NBR_PARAMS = 1,
  UNRECOGNIZED_FILE = 2,
  UNVALID_FTYPE = 3,
  UNVALID_FILE = 4,
  UNVALID_PACKED = 5,
  ERROR_STDL = -1,
  FAIL_LSEEK = -2,
  FAIL_MMAP = -3,
}	t_return_errors;

# define ELF_MAGIC ELFMAG
/* # define MACH_O_32b_MAGIC "\xfe\xed\xfa\xce" */
/* # define MACH_O_64b_MAGIC "\xfe\xed\xfa\xcf" */
/* # define MACH_O_MAGIC_SIZE 4 */
# define DOS_SIGN "MZ" //  0x5A4D
# define DOS_SIGN_SIZE 2 //
# define NT_HEADER_OFFSET 0x3C
# define PE_SIGN "PE\x0\x0" // 0x50450000 is the signature on NT_HEADER
# define PE_SIGN_SIZE 4
# define ELF_MAGIC_SIZE SELFMAG      // Below this line end the ignored defines



  typedef  union u_elf_addr
  {
    Elf32_Addr b32;
    Elf64_Addr b64;
  }	t_elf_addr;

  typedef struct s_list
  {
  	void			*content;
  	struct s_list	*next;
  }	t_list;

  typedef enum e_error {
    ERR_SUCCESS = 0,
    ERR_INVALID_ELF = 1,
    ERR_PREPARE_INFO = 2,
    ERR_WRITE_HEADER = 3,
    ERR_WRITE_PHDR = 4,
    ERR_WRITE_PAYLOAD = 5,
    ERR_LOAD_STAGE = 6,
    ERR_ALLOC = 7,
    ERR_COMPRESS = 8,
    ERR_ENCRYPT = 9,
    ERR_INVAL_PH_NDX = 10,
    ERR_INJECT_S3 = 11
  } t_error;

// signature_check
t_file_type get_ftype(const char *file_content, const off_t *file_size);

# endif
