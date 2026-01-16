#include "format_elf.h"

bool	elf_valid_section_header(unsigned char elf_class, const t_elf_sh *shdr, uint16_t index, const t_elf_h *ehdr, uint64_t *symtab)
{
        const uint32_t	type = elf_get_section_header_type(shdr, elf_class);
        const uint32_t	link = elf_get_section_header_link(shdr, elf_class);
        const uint64_t	real_shnum = elf_get_real_shnum(ehdr, elf_class);

        if (real_shnum <= link)
                return (false);
        if ((elf_get_section_header_flags(shdr, elf_class) & SHF_INFO_LINK
                        || type == SHT_RELA || type == SHT_REL)
                && (elf_get_section_header_info(shdr, elf_class) >= real_shnum
                        || elf_get_section_header_info(shdr, elf_class) == index))
                return (false);
        if (link == index)
                return (false);
        if (type == SHT_SYMTAB_SHNDX)
                symtab[0] = true;
        else if (type == SHT_SYMTAB)
                symtab[1] = index;
        return (true);
}

bool	elf_valid_first_section_header(unsigned char elf_class, const t_elf_h *ehdr,
                        const t_elf_sh *shdr)
{
        if (elf_get_elfh_shnum(ehdr) == SHN_UNDEF && !elf_get_section_header_size(shdr, elf_class))
          return (false);
        if (elf_get_elfh_shstrndx(ehdr) == SHN_XINDEX && !elf_get_section_header_link(shdr, elf_class))
          return (false);
        if (elf_get_elfh_phnum(ehdr) == PN_XNUM &&!elf_get_section_header_info(shdr, elf_class))
          return (false);
        return (!uint_mult_overflow(elf_get_real_shnum(ehdr, elf_class),
                        elf_get_section_header_struct_size(ehdr)));
}

uint32_t	elf_get_section_header_type_by_index(const t_elf_h *ehdr,
                        uint16_t index)
{
        const unsigned char	class = elf_get_elfh_e_ident_class((void *)ehdr->elf32);
        const t_elf_sh		shdr = elf_get_section_header(ehdr, index);

        return (elf_get_section_header_type(&shdr, class));
}

t_elf_sh	elf_get_section_header_by_type(const t_elf_h *ehdr, uint32_t type)
{
        const unsigned char	class = elf_get_elfh_e_ident_class((void *)ehdr->elf32);
        const uint16_t		shnum = elf_get_real_shnum(ehdr, class);
        uint16_t			index;
        t_elf_sh			shdr;

        index = 0;
        while (index < shnum)
        {
                shdr = elf_get_section_header(ehdr, index++);
                if (shdr.elf32 && elf_get_section_header_type(&shdr, class) == type)
                        return (shdr);
        }
        shdr.elf32 = NULL;
        return (shdr);
}

bool	elf_valid_elf_section_header_handler(unsigned char elf_class, uint64_t *symtab, const t_elf_h *ehdr,
                        uint64_t index)
{
        const t_elf_sh	shdr = elf_get_section_header(ehdr, index);

        if (!index)
                return (elf_valid_first_section_header(elf_class, ehdr, &shdr));
        return (elf_valid_section_header(elf_class, &shdr, index, ehdr, symtab));
}
