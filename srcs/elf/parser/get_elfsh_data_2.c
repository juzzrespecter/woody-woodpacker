#include "format_elf.h"

uint64_t	elf_get_section_header_addralign(const t_elf_sh *shdr,
		unsigned char class)
{
	if (class == ELFCLASS32)
		return ((uint64_t)shdr->elf32->sh_addralign);
	return (shdr->elf64->sh_addralign);
}

uint32_t	elf_get_section_header_entsize(const t_elf_sh *shdr,
			unsigned char class)
{
	if (class == ELFCLASS32)
		return (shdr->elf32->sh_entsize);
	return (shdr->elf64->sh_entsize);
}

uint32_t	elf_get_section_header_addr(const t_elf_sh *shdr, unsigned char class)
{
	if (class == ELFCLASS32)
		return (shdr->elf32->sh_addr);
	return (shdr->elf64->sh_addr);
}
