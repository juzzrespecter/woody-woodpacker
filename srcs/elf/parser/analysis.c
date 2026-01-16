#include "format_elf.h"
//elf/analisis
    size_t elf_get_gnu_stack_phdr_ndx(const t_elf_h *ehdr, unsigned char class, uint64_t phnum)
    {
        t_elf_ph phdr;
        uint64_t i;

        i = 0;
        while (i < phnum)
    	{
    	    phdr = elf_get_program_header(ehdr, i);
    	    if (elf_get_program_header_type(&phdr, class) == PT_GNU_STACK) // If not loadable ignore
    			return (i);
    	    i++;
    	}
        return  (0);
    }

    size_t elf_get_highest_used_vaddr_avalaible(const t_elf_h *ehdr, unsigned char class, uint64_t phnum)
    {
        t_elf_ph phdr;
        uint64_t i;
        size_t p_vaddr;
        size_t p_end_vaddr;
        size_t highest_used_addr = 0;

        i = 0;
        while (i < phnum)
    	{
    	    phdr = elf_get_program_header(ehdr, i);
    	    if (elf_get_program_header_type(&phdr, class) != PT_LOAD) // If not loadable ignore
    		{
    		    i++;
    		    continue ;
    		}
    	    p_vaddr = elf_get_program_header_vaddr(&phdr, class);
    	    p_end_vaddr = p_vaddr + elf_get_program_header_memsz(&phdr, class);
    	    if (highest_used_addr <= p_end_vaddr)
    			highest_used_addr = p_end_vaddr;
    	    i++;
    	}
        return  (highest_used_addr);
    }

size_t elf_get_highest_loadable_vaddr_avalaible(const t_elf_h *ehdr, unsigned char class, uint64_t phnum)
    {
    	return (align_up(elf_get_highest_used_vaddr_avalaible(ehdr, class, phnum), 0x1000));
    }

    uint64_t elf_get_closest_higher_p_offset(const t_elf_h *ehdr, unsigned char class, uint64_t phnum, uint64_t p_offset)
    {
        t_elf_ph phdr;
        uint64_t ph_ndx;
        uint64_t closest_offset = -1;
        uint64_t cur_offset;
        ph_ndx = 0;
        while (ph_ndx < phnum)
    	{
    	    phdr = elf_get_program_header(ehdr, ph_ndx);
    	    cur_offset = elf_get_program_header_offset(&phdr, class);
    	    if (cur_offset > p_offset && cur_offset < closest_offset)
    			closest_offset = cur_offset;
    	    ph_ndx++;
    	}
        return (closest_offset);
    }
