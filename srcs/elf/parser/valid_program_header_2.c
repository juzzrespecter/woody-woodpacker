#include "format_elf.h"

 uint64_t	elf_get_struct_size_elf_ph(unsigned char class)
 {
         if (class == ELFCLASS32)
                 return (sizeof(Elf32_Phdr));
         return (sizeof(Elf64_Phdr));
 }

 t_elf_ph	elf_get_program_header(const t_elf_h *ehdr, uint16_t index)
 {
         const uint64_t	offset = elf_get_elfh_phoff(ehdr);
         const uint64_t	size = elf_get_struct_size_elf_ph(
                         elf_get_elfh_e_ident_class((void *)ehdr->elf32));

         if (unsigned_int_mult_overflow(size, index, sizeof(size) * 8))
                 return ((t_elf_ph){.elf32 = NULL});
         return ((t_elf_ph){.elf32 = (void *)ehdr->elf32 + offset
                 + size * index});
 }

 uint64_t elf_get_program_header_align(const t_elf_ph *phdr,
                         unsigned char class)
 {
         if (class == ELFCLASS32)
                 return (phdr->elf32->p_align);
         return (phdr->elf64->p_align);
 }

 uint32_t	elf_get_program_header_offset(const t_elf_ph *phdr,
                         unsigned char class)
 {
         if (class == ELFCLASS32)
                 return (phdr->elf32->p_offset);
         return (phdr->elf64->p_offset);
 }

 uint64_t	elf_get_program_header_filesz(const t_elf_ph *phdr,
                         unsigned char class)
 {
         if (class == ELFCLASS32)
                 return (phdr->elf32->p_filesz);
         return (phdr->elf64->p_filesz);
 }

 uint64_t	elf_get_program_header_memsz(const t_elf_ph *phdr,
                         unsigned char class)
 {
         if (class == ELFCLASS32)
                 return (phdr->elf32->p_memsz);
         return (phdr->elf64->p_memsz);
 }

   uint64_t	elf_get_program_header_vaddr(const t_elf_ph *phdr,
                           unsigned char class)
   {
           if (class == ELFCLASS32)
                   return (phdr->elf32->p_vaddr);
           return (phdr->elf64->p_vaddr);
   }

   uint64_t	elf_get_program_header_paddr(const t_elf_ph *phdr,
                           unsigned char class)
   {
           if (class == ELFCLASS32)
                   return (phdr->elf32->p_paddr);
           return (phdr->elf64->p_paddr);
   }

uint64_t elf_get_program_header_type(t_elf_ph *phdr, unsigned char class)
{
  if (class == ELFCLASS32)
      return (phdr->elf32->p_type);
  return (phdr->elf64->p_type);
}

uint64_t elf_get_program_header_flags(t_elf_ph *phdr, unsigned char class)
{
  if (class == ELFCLASS32)
      return (phdr->elf32->p_flags);
  return (phdr->elf64->p_flags);
}
