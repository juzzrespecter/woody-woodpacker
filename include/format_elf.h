#ifndef FORMAT_ELF_H
# define FORMAT_ELF_H
# ifndef DEBUG_LVL
#  define DEBUG_LVL 1
# endif
# include <elf.h>    // Typedef ELFNN_*, ST_BIND, ...
# include <stdint.h> // uint64_t
# include <stdbool.h> // bool
# include <unistd.h> // off_t
# include <string.h> // memcmp TODO Delete.
# include "common.h" // overflows, enums registers
# include "libft.h" // t_list ft_printf ...

typedef union u_elf_h
{
  Elf32_Ehdr	*elf32;
  Elf64_Ehdr	*elf64;
}	t_elf_h;

typedef union u_elf_ph
{
  Elf32_Phdr	*elf32;
  Elf64_Phdr	*elf64;
}	t_elf_ph;

typedef union u_elf_sh
{
  Elf32_Shdr	*elf32;
  Elf64_Shdr	*elf64;
}	t_elf_sh;

typedef union u_elf_sym
{
  Elf32_Sym	*elf32;
  Elf64_Sym	*elf64;
}	t_elf_sym;

typedef union u_elf_dyn
{
  Elf32_Dyn	*elf32;
  Elf64_Dyn	*elf64;
}	t_elf_dyn;
typedef struct	s_stage2_result {
	t_buffer	buffer;           // Stage2 buffer
	size_t		payload_raw_size; // Size of raw payload used in concat
}	t_stage2_result;

typedef struct s_stage1_params {
  	size_t			stage2_total_size;
  	t_buffer		encrypted;
  	size_t			compressed_hdr_size;
  	size_t			payload_raw_size;
  	t_aes128_key	key;
}	t_stage1_params;

typedef struct	s_compress_encrypt {
  	t_encrypt encrypt;
  	size_t head_compress_len;
}	t_compress_encrypt;

typedef struct	s_payload_prereq {
	t_buffer        stage1;
	t_buffer        encrypted_payload;
	t_stage1_params stage1_params;
}	t_payload_prereq;

typedef struct	s_phdr_layout {
	uint64_t offset;
	uint64_t vaddr;
	uint64_t paddr;
	uint64_t filesz;
	uint64_t memsz;
	uint64_t align;
	uint64_t type;
	uint64_t flags;
}	t_phdr_layout;

typedef struct	s_elf_write_ctx {
  	t_elf_h		   *ehdr;
  	int				fd;
  	unsigned char	class;
  	uint64_t		phnum;
  	uint64_t		new_phdr_vaddr;
  	uint64_t		payload_size;
}	t_elf_write_ctx;

typedef struct s_s3_find_params {
	const t_elf_h   *ehdr;
	unsigned char    class;
	uint64_t         phnum;
	size_t           gap_size;
} t_s3_find_params;

typedef struct s_s3_find_result {
	bool      found;
	uint64_t  vaddr;
	uint64_t  offset;
	t_elf_ph  phdr;
} t_s3_find_result;

typedef struct	s_stage1_ctx {
	t_arch    arch;
	size_t    value_size;
	uint64_t  phnum;
	uint32_t  prefix_size;
	uint64_t  s2_entry_offset;
}	t_stage1_ctx;

typedef enum e_stage1_patch_idx {
	STAGE1_PATCH_EXPECTED_VADDR = 0,   // Highest loadable VA + prefix + ASLR adjustment
	STAGE1_PATCH_S1_SIZE,             // Stage1 length + prefix + encrypted len
	STAGE1_PATCH_STAGE2_SIZE,         // Stage2 total size
	STAGE1_PATCH_COMPRESSED_HDR_SIZE, // Compressed header size
	STAGE1_PATCH_S2_ENTRY_OFFSET,     // Stage2 entry offset
	STAGE1_PATCH_AES_KEY,             // AES key overwrite
	STAGE1_PATCH_COUNT                // Total number of valid patches
} t_stage1_patch_idx;

typedef struct	s_stage1_patch_values {
	size_t expected_va;
	size_t s1_len;
}	t_stage1_patch_values;

# include "elf_setters_phdr.h" // phdr setters
# include "elf_setters_ehdr.h" // ehdr setters

//get_eh
uint16_t		elf_get_elfh_type(const t_elf_h *hdr);
uint16_t		elf_get_elfh_machine(const t_elf_h *ehdr);// Unused
uint32_t		elf_get_elfh_version(const t_elf_h *ehdr);// Unused
uint32_t		elf_get_elfh_flags(const t_elf_h *ehdr); // Unused
uint16_t		elf_get_elfh_shstrndx(const t_elf_h *ehdr);

//get_eh_1
uint16_t		elf_get_elfh_ehsize(const t_elf_h *ehdr);
uint16_t		elf_get_elfh_phentsize(const t_elf_h *ehdr);
uint16_t		elf_get_elfh_phnum(const t_elf_h *ehdr);
uint16_t		elf_get_elfh_shentsize(const t_elf_h *ehdr);
uint16_t		elf_get_elfh_shnum(const t_elf_h *ehdr);

// get_eh_2
uint64_t		elf_get_elfh_entry(const t_elf_h *ehdr);
uint64_t		elf_get_elfh_phoff(const t_elf_h *ehdr);
uint64_t		elf_get_elfh_shoff(const t_elf_h *ehdr);
int				elf_get_struct_size_elf_header(char *elf_mmap);

//get_real_data
uint64_t	elf_get_real_shnum(const t_elf_h *ehdr, unsigned char class);
uint64_t	elf_get_real_shstrndx(const t_elf_h *ehdr, unsigned char class);
uint64_t	elf_get_real_phnum(const t_elf_h *ehdr);

// valid_elf_h_attrib

bool	elf_is_valid_elf_shoff(const t_elf_h *ehdr, off_t *elf_size);
bool	elf_is_valid_elf_phoff(const t_elf_h *ehdr, off_t *elf_size);

// validate_elf_h
bool	elf_is_file_size_enought_for_elf_header(off_t file_size, void *elf_mmap);
bool	elf_is_valid_elf_data_format(char *elf_mmap);
bool	elf_is_valid_elf_version(char *elf_mmap);
bool	elf_is_executable_elf_file(void *elf_mmap);

// valid_elf_h_1
bool	elf_valid_section_size(const t_elf_h *ehdr);
bool	elf_valid_ident(off_t file_size, void *elf_mmap);
bool	elf_valid_elfh_class(const char *e_indent);
bool	elf_valid_section_header_string_index(const t_elf_h *ehdr);

// elf_valid_t_elf_h_2
bool	elf_valid_elf_header(const t_elf_h *ehdr, off_t file_size);
bool	elf_valid_t_elf_h(const t_elf_h *ehdr, off_t file_size);

// elf_valid_elf_ph
bool	elf_valid_elf_phs(off_t elf_size, const t_elf_h *ehdr);

// valid_program_header_2
  uint64_t	elf_get_struct_size_elf_ph(unsigned char class);
  t_elf_ph	elf_get_program_header(const t_elf_h *ehdr, uint16_t index);
  uint32_t	elf_get_program_header_offset(const t_elf_ph *phdr,
                          unsigned char class);
  uint64_t	elf_get_program_header_filesz(const t_elf_ph *phdr,
                          unsigned char class);
uint64_t	elf_get_program_header_memsz(const t_elf_ph *phdr,
					      unsigned char class);
uint64_t elf_get_program_header_align(const t_elf_ph *phdr,
                          unsigned char class);

uint64_t	elf_get_program_header_vaddr(const t_elf_ph *phdr,
                        unsigned char class);
uint64_t	elf_get_program_header_paddr(const t_elf_ph *phdr,
                        unsigned char class);
uint64_t elf_get_program_header_type(t_elf_ph *phdr, unsigned char class);
uint64_t elf_get_program_header_flags(t_elf_ph *phdr, unsigned char class);

// elf_get_elfh_e_ident
  unsigned char	elf_get_elfh_e_ident_data(const unsigned char *e_ident);
  unsigned char	elf_get_elfh_e_ident_class(const unsigned char *e_ident);
  unsigned char	elf_get_elfh_e_ident_version(const unsigned char *e_ident);
  unsigned char	elf_get_elfh_e_ident_osabi(const unsigned char *e_ident);
  unsigned char	elf_get_elfh_e_ident_abiversion(const unsigned char *e_ident);

// elf_valid_elfh

bool	elf_valid_elf_magic_number(void *elf_mmap);
bool	elf_valid_elf_file(void *elf_mmap, off_t elf_size);
int	elf_valid_file(void *elf_mmap, off_t elf_size);

// get_elfsh_data
uint64_t	elf_get_section_header_struct_size(const t_elf_h *ehdr);
bool	elf_is_special_section_index(uint16_t index);
t_elf_sh	elf_get_section_header(const t_elf_h *ehdr, uint16_t index);
uint64_t	elf_get_section_header_offset(const t_elf_sh *shdr, unsigned char class);
bool	valid_null_end_string_table(const t_elf_sh *shstrtab,
                                    unsigned char class, char *ptr);

// get_elfsh_data_1
uint32_t	elf_get_section_header_type(const t_elf_sh *shdr, unsigned char class);
uint32_t	elf_get_section_header_flags(const t_elf_sh *shdr, unsigned char class);
uint64_t	elf_get_section_header_size(const t_elf_sh *shdr, unsigned char class);
uint32_t	elf_get_section_header_link(const t_elf_sh *shdr, unsigned char class);
uint32_t	elf_get_section_header_info(const t_elf_sh *shdr, unsigned char class);

// get_elfsh_data_2

uint64_t	elf_get_section_header_addralign(const t_elf_sh *shdr,
                    unsigned char class);
uint32_t	elf_get_section_header_entsize(const t_elf_sh *shdr,
                    unsigned char class);
uint32_t	elf_get_section_header_addr(const t_elf_sh *shdr,
                    unsigned char class);

// get_elfsh_data_3

const char	*elf_get_section_string_table(const t_elf_h *ehdr);
uint32_t	elf_get_section_header_name(const t_elf_sh *shdr, unsigned char class);
const char	*elf_get_section_name(const char *str_tab, uint64_t index_str_tab);

// elf_valid_elf_section_header
bool	elf_valid_elf_section_header_symtab_shndx(unsigned char class, const uint64_t symtab_index, const t_elf_h *ehdr);
bool	elf_valid_elf_section_headers(unsigned char elf_class, const t_elf_h *ehdr);

// elf_valid_elf_section_header_2
bool	elf_valid_section_header(unsigned char elf_class, const t_elf_sh *shdr, uint16_t index, const t_elf_h *ehdr, uint64_t *symtab);
bool	elf_valid_first_section_header(unsigned char elf_class, const t_elf_h *ehdr,
                          const t_elf_sh *shdr);
uint32_t	elf_get_section_header_type_by_index(const t_elf_h *ehdr,
                          uint16_t index);
t_elf_sh	elf_get_section_header_by_type(const t_elf_h *ehdr, uint32_t type);
bool	elf_valid_elf_section_header_handler(unsigned char elf_class,  uint64_t *symtab, const t_elf_h *ehdr, uint64_t index);


//analysis
  size_t elf_get_gnu_stack_phdr_ndx(const t_elf_h *ehdr, unsigned char class, uint64_t phnum);

  size_t elf_get_highest_used_vaddr_avalaible(const t_elf_h *ehdr, unsigned char class, uint64_t phnum);
  size_t elf_get_highest_loadable_vaddr_avalaible(const t_elf_h *ehdr, unsigned char class, uint64_t phnum);
  uint64_t elf_get_closest_higher_p_offset(const t_elf_h *ehdr, unsigned char class, uint64_t phnum, uint64_t p_offset);

// srcs/elf/packer/check_packer.c
int elf_packer(void *elf_mmap, off_t file_size, const char *file_path);
#endif
