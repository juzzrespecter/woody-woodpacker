#include "format_elf.h"

 uint32_t	elf_get_section_header_type(const t_elf_sh *shdr, unsigned char class)
 {
         if (class == ELFCLASS32)
                 return (shdr->elf32->sh_type);
         return (shdr->elf64->sh_type);
 }

 uint32_t	elf_get_section_header_flags(const t_elf_sh *shdr, unsigned char class)
 {
         if (class == ELFCLASS32)
                 return (shdr->elf32->sh_flags);
         return (shdr->elf64->sh_flags);
 }

 uint64_t	elf_get_section_header_size(const t_elf_sh *shdr, unsigned char class)
 {
         if (class == ELFCLASS32)
                 return (shdr->elf32->sh_size);
         return (shdr->elf64->sh_size);
 }

 uint32_t	elf_get_section_header_link(const t_elf_sh *shdr, unsigned char class)
 {
         if (class == ELFCLASS32)
                 return (shdr->elf32->sh_link);
         return (shdr->elf64->sh_link);
 }

 uint32_t	elf_get_section_header_info(const t_elf_sh *shdr, unsigned char class)
 {
         if (class == ELFCLASS32)
                 return (shdr->elf32->sh_info);
         return (shdr->elf64->sh_info);
 }
