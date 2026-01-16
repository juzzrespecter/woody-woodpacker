#ifndef ELF_SETTERS_EHDR_H
# define ELF_SETTERS_EHDR_H
# include "format_elf.h"//t_elf_h

void elf_ehdr_set_shoff(t_elf_h *ehdr, uint64_t value, unsigned char class);
void elf_ehdr_set_shentsize(t_elf_h *ehdr, uint64_t value, unsigned char class);
void elf_ehdr_set_shstrndx(t_elf_h *ehdr, uint64_t value, unsigned char class);
void elf_ehdr_set_entry(t_elf_h *ehdr, uint64_t value, unsigned char class);
void elf_ehdr_set_phnum(t_elf_h *ehdr, uint64_t value, unsigned char class);
void elf_ehdr_set_shnum(t_elf_h *ehdr, uint64_t value, unsigned char class);
#endif
