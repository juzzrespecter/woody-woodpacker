#include "format_elf.h"

void elf_phdr_set_memsz(t_elf_ph *phdr, uint64_t value, unsigned char class)
{
    if (class == ELFCLASS32)
	{
	    /* printf("phdr set memsz elf32\n"); */
	    phdr->elf32->p_memsz = (uint32_t) value;
	    return ;
	}
    /* printf("phdr set memsz elf64\n"); */
    phdr->elf64->p_memsz = value;
}

void elf_phdr_set_offset(t_elf_ph *phdr, uint64_t value, unsigned char class)
{
    if (class == ELFCLASS32)
	{
	    /* printf("phdr set offset elf32\n"); */
	    phdr->elf32->p_offset = (uint32_t) value;
	    return ;
	}
    /* printf("phdr set offset elf64\n"); */
    phdr->elf64->p_offset = value;
}

void elf_phdr_set_vaddr(t_elf_ph *phdr, uint64_t value, unsigned char class)
{
    if (class == ELFCLASS32)
	{
	    /* printf("phdr set vaddr elf32\n"); */
	    phdr->elf32->p_vaddr = (uint32_t) value;
	    return ;
	}
    /* printf("phdr set vaddr elf64\n"); */
    phdr->elf64->p_vaddr = value;
}

void elf_phdr_set_paddr(t_elf_ph *phdr, uint64_t value, unsigned char class)
{
    if (class == ELFCLASS32)
	{
	    /* printf("phdr set paddr elf32\n"); */
	    phdr->elf32->p_paddr = (uint32_t) value;
	    return ;
	}
    /* printf("phdr set paddr elf64\n"); */
    phdr->elf64->p_paddr = value;
}

void elf_phdr_set_filesz(t_elf_ph *phdr, uint64_t value, unsigned char class)
{
    if (class == ELFCLASS32)
	{
	    /* printf("phdr set addr elf32\n"); */
	    phdr->elf32->p_filesz = (uint32_t) value;
	    return ;
	}
    /* printf("phdr set addr elf64\n"); */
    phdr->elf64->p_filesz = value;
}

void elf_phdr_set_align(t_elf_ph *phdr, uint64_t value, unsigned char class)
{
    if (class == ELFCLASS32)
	{
	    /* printf("phdr set addr elf32\n"); */
	    phdr->elf32->p_align = (uint32_t) value;
	    return ;
	}
    /* printf("phdr set addr elf64\n"); */
    phdr->elf64->p_align = value;
}

void elf_phdr_set_flags(t_elf_ph *phdr, uint64_t value, unsigned char class)
{
    if (class == ELFCLASS32)
	{
	    /* printf("phdr set addr elf32\n"); */
	    phdr->elf32->p_flags = (uint32_t) value;
	    return ;
	}
    /* printf("phdr set addr elf64\n"); */
    phdr->elf64->p_flags = value;
}

void elf_phdr_set_type(t_elf_ph *phdr, uint64_t value, unsigned char class)
{
    if (class == ELFCLASS32)
	{
	    /* printf("phdr set addr elf32\n"); */
	    phdr->elf32->p_type = (uint32_t) value;
	    return ;
	}
    /* printf("phdr set addr elf64\n"); */
    phdr->elf64->p_type = value;
}
