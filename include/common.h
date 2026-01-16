#ifndef COMMON_H
# define COMMON_H
# include <stdint.h>  // uint64_t, uint32_t
# include <stdbool.h> // bool
# include <stddef.h> // size_t
# include <unistd.h>//write, read, lseek, open
# include <fcntl.h> // O_RDONLY, close
# include "libft.h" //ft_bzero, ft_memcpy

typedef union u_type
{
	uint8_t	b;
	uint16_t	b2;
	uint32_t	b4;
	uint64_t	b8;
} t_type;

typedef struct	s_patch_set {
	const size_t	*offsets;
	const t_type	*values; // change for void **
	const size_t	count;
	const size_t	value_size;// change for len
}	t_patch_set;
// Architecture indices
typedef enum	e_arch{
	ARCH_X86,
	ARCH_X64,
	NUM_ARCHS,
	ARCH_ALL=NUM_ARCHS
}	t_arch;


typedef struct s_patch_delta {
	size_t	offset;   // base offset
	size_t	len;      // bytes added or removed
	t_arch	arch;     // ARCH_X86, ARCH_X64, or NUM_ARCHS for "all"
  	int8_t	sign;     // 1 or -1
} t_patch_delta;

typedef struct	s_patch_desc {
	size_t	base_off_x86;
	size_t	base_off_x64;
	size_t	len_x86;
	size_t	len_x64;
	void	*value;
}	t_patch_desc;

typedef struct	s_aes128_key {
	uint32_t ints[4];  // 128 bits
}	t_aes128_key;

typedef struct	s_buffer {
    void	*data;
    size_t	len;
}	t_buffer;

typedef struct	s_encrypt {
  	t_buffer buffer;
  	t_aes128_key key;
}	t_encrypt;

typedef enum e_file_type {
	FT_ELF,
	/* FT_MACH_O, */
	FT_PE,  // FT_DOS?
	FT_UNKNOW,
	FT_NUM=FT_UNKNOW
}	t_file_type;

typedef enum e_bin_type {
	STAGE_1,
	STAGE_2,
	STAGE_3,
	PAYLOAD_BIN,
	NUM_BIN_TYPES
}	t_bin_type;



// overflow
//stdbool (bool), stdint (uint64_t)
bool	unsigned_overflow_limit(uint64_t times, uint64_t size, uint64_t limit);
bool	signed_overflow_limit(int64_t times, int64_t size, int64_t limit);
bool	int_mult_overflow(int64_t times, int64_t size, uint64_t size_bits);
bool	uint_mult_overflow(unsigned int times, unsigned int size);
bool	unsigned_int_mult_overflow(uint64_t times, uint64_t size, uint64_t size_bits);

//align
//stddef.h (size_t)
//stdint.h (uint64_t)
size_t align_down(uint64_t size, uint64_t alignment);
size_t align_up(uint64_t size, uint64_t alignment);

//encrypt
//libft.h (ft_bzero)
void *encrypt(void *raw, size_t raw_size, size_t *ret_size, int *key);
t_encrypt wrapper_encrypt(t_buffer *raw);

//buffer
bool invalid_buffer(t_buffer *buf);
t_buffer alloc_concat_buffers(const t_buffer *a, const t_buffer *b);

//invalid
  bool invalid_read(int fd, void *ptr, size_t len);
  bool invalid_write(int fd, void *ptr, size_t len);

void apply_patches_to_buffer(char *buffer, const t_patch_set *patch);
size_t compute_patch_offset(t_arch arch, size_t base_offset,
	const t_patch_delta *deltas, size_t count);
  t_patch_set ft_init_patch_set(const size_t *off,
    const t_type *val, size_t count, size_t value_size);
void apply_patches_to_buffer_with_patch_desc(t_buffer *stage1,
  	const t_arch arch,
  	const t_patch_desc *patches_desc, size_t patch_count,
  	const t_patch_delta *deltas, size_t delta_count);

//stages
t_buffer	load_stage_buffer(t_arch arch, t_bin_type bin_type, t_file_type ft);
t_buffer	load_stage_elf_buffer(t_arch arch, t_bin_type bin_type);
t_buffer	load_and_apply_patches_to_buffer(t_arch arch, t_bin_type bin_type, const t_patch_set *patch);

//file validation
int		get_file(const char *path, int *fd); //open
int 	get_file_size(int fd, off_t *file_size);//lseek

bool valid_cpu_x86(void);

# endif
