#include "format_elf.h"

uint64_t	elf_get_elfh_entry(const t_elf_h *ehdr)
{
	const unsigned char	class = elf_get_elfh_e_ident_class(ehdr->elf32->e_ident);

	if (class == ELFCLASS32)
		return ((uint64_t)ehdr->elf32->e_entry);
	return (ehdr->elf64->e_entry);
}

uint64_t	elf_get_elfh_phoff(const t_elf_h *ehdr)
{
	const unsigned char	class = elf_get_elfh_e_ident_class(ehdr->elf32->e_ident);

	if (class == ELFCLASS32)
		return ((uint64_t)ehdr->elf32->e_phoff);
	return (ehdr->elf64->e_phoff);
}

uint64_t	elf_get_elfh_shoff(const t_elf_h *ehdr)
{
	const unsigned char	class = elf_get_elfh_e_ident_class(ehdr->elf32->e_ident);

	if (class == ELFCLASS32)
		return ((uint64_t)ehdr->elf32->e_shoff);
	return (ehdr->elf64->e_shoff);
}

int	elf_get_struct_size_elf_header(char *elf_mmap)
{
	if (elf_mmap[EI_CLASS] == ELFCLASS32)
		return (sizeof (Elf32_Ehdr));
	else if (elf_mmap[EI_CLASS] == ELFCLASS64)
		return (sizeof (Elf64_Ehdr));
	return (0);
}
