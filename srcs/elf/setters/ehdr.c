// begin setter ehdr
#include "format_elf.h"

void elf_ehdr_set_shoff(t_elf_h *ehdr, uint64_t value, unsigned char class)
{
    if (class == ELFCLASS32)
	{
	    /* printf("ehdr set shoff elf32\n"); */
	    ehdr->elf32->e_shoff = (uint32_t) value;
	    return ;
	}
    /* printf("ehdr set shoff elf64\n"); */
    ehdr->elf64->e_shoff = value;
}

void elf_ehdr_set_shentsize(t_elf_h *ehdr, uint64_t value, unsigned char class)
{
    if (class == ELFCLASS32)
	{
	    /* printf("ehdr set shoff elf32\n"); */
	    ehdr->elf32->e_shentsize = (uint32_t) value;
	    return ;
	}
    /* printf("ehdr set shoff elf64\n"); */
    ehdr->elf64->e_shentsize = value;
}

void elf_ehdr_set_shstrndx(t_elf_h *ehdr, uint64_t value, unsigned char class)
{
    if (class == ELFCLASS32)
	{
	    /* printf("ehdr set shoff elf32\n"); */
	    ehdr->elf32->e_shstrndx = (uint32_t) value;
	    return ;
	}
    /* printf("ehdr set shoff elf64\n"); */
    ehdr->elf64->e_shstrndx = value;
}

void elf_ehdr_set_entry(t_elf_h *ehdr, uint64_t value, unsigned char class)
{
    if (class == ELFCLASS32)
	{
	    /* printf("ehdr set entry elf32\n"); */
	    ehdr->elf32->e_entry = (uint32_t) value;
	    return ;
	}
    /* printf("ehdr set entry elf64\n"); */
    ehdr->elf64->e_entry = value;
}

void elf_ehdr_set_phnum(t_elf_h *ehdr, uint64_t value, unsigned char class)
{
    if (class == ELFCLASS32)
	{
	    /* printf("ehdr set phnum elf32\n"); */
	    ehdr->elf32->e_phnum = (uint32_t) value;
	    return ;
	}
    /* printf("ehdr set phnum elf64\n"); */
    ehdr->elf64->e_phnum = value;
}

void elf_ehdr_set_shnum(t_elf_h *ehdr, uint64_t value, unsigned char class)
{
    if (class == ELFCLASS32)
	{
	    /* printf("ehdr set shnum elf32\n"); */
	    ehdr->elf32->e_shnum = (uint32_t) value;
	    return ;
	}
    /* printf("ehdr set shnum elf64\n"); */
    ehdr->elf64->e_shnum = value;
}
