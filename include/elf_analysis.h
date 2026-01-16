#ifndef ELF_ANALYSIS_H
# define ELF_ANALYSIS_H
# include "format_elf.h" //t_elf_h/ph, uint64_t
size_t		elf_get_gnu_stack_phdr_ndx(t_elf_h *ehdr, unsigned char class,
					   uint64_t phnum);
size_t		elf_get_highest_used_vaddr_avalaible(t_elf_h *ehdr,
						     unsigned char class, uint64_t phnum);
size_t		elf_get_highest_loadable_vaddr_avalaible(t_elf_h *ehdr,
							 unsigned char class, uint64_t phnum);
uint64_t	elf_get_closest_higher_p_offset(const t_elf_h *ehdr,
						unsigned char class, uint64_t phnum, uint64_t p_offset);
int		validate_elf_headers(t_elf_h *ehdr, uint64_t *shnum,
				     uint64_t *phnum, unsigned char *class);
#endif
