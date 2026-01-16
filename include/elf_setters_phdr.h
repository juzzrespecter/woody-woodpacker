#ifndef ELF_SETTERS_PHDR_H
# define ELF_SETTERS_PHDR_H
# include "format_elf.h"//t_elf_ph

void elf_phdr_set_type(t_elf_ph *phdr, uint64_t value, unsigned char class);
void elf_phdr_set_flags(t_elf_ph *phdr, uint64_t value, unsigned char class);
void elf_phdr_set_align(t_elf_ph *phdr, uint64_t value, unsigned char class);
void elf_phdr_set_filesz(t_elf_ph *phdr, uint64_t value, unsigned char class);
void elf_phdr_set_paddr(t_elf_ph *phdr, uint64_t value, unsigned char class);
void elf_phdr_set_vaddr(t_elf_ph *phdr, uint64_t value, unsigned char class);
void elf_phdr_set_offset(t_elf_ph *phdr, uint64_t value, unsigned char class);
void elf_phdr_set_memsz(t_elf_ph *phdr, uint64_t value, unsigned char class);

#endif
