// Headers Begin
  #include <string.h>
  #include "woody.h"
  #include "common.h"
  #include "format_elf.h" // align_up, elf_get_gnu_phdr_ndx
  #include <stdbool.h>
  #include <unistd.h>
  #include <stdio.h>
  #include <stdint.h>
  #include <fcntl.h>
  #include <errno.h>
  #include <sys/mman.h>
  #include <stdlib.h> //Malloc


// Headers End
    static void set_elf_headers(t_elf_h *ehdr, uint64_t *shnum, uint64_t *phnum, unsigned char *class)
    {
    	*shnum = elf_get_elfh_shnum(ehdr);
    	*phnum = elf_get_elfh_phnum(ehdr);
    	*class = elf_get_elfh_e_ident_class((unsigned char *)ehdr->elf32);
    }

    // stages

    //payload
    static t_compress_encrypt compress_encrypt_with_header(t_buffer *input, size_t value_size)
    {
    	t_compress_encrypt compress_encrypt;
        t_buffer head_compress;

        ft_bzero(&compress_encrypt, sizeof(compress_encrypt));
        head_compress = lz77_get_headed_buffer(input, value_size);
        if (invalid_buffer(&head_compress))
    		return (compress_encrypt);
    	compress_encrypt.head_compress_len = head_compress.len;
    	compress_encrypt.encrypt = wrapper_encrypt(&head_compress);
        if (compress_encrypt.encrypt.buffer.data != head_compress.data)
    		free(head_compress.data);
        if (invalid_buffer(&compress_encrypt.encrypt.buffer))
    		free(compress_encrypt.encrypt.buffer.data);
        return (compress_encrypt);
    }

    // payload/stage2
    t_patch_set init_patch_stage2(t_arch arch, size_t elf_size, uint64_t s3_vaddr)
    {
    	static const size_t stage2_patch_offsets[NUM_ARCHS][2] = {
    		[ARCH_X86] = { 0x98 - 5, 0x98 - 5},
    		[ARCH_X64] = { 0xD7 - (0x22 + 10), 0xD7 - (0x22 + 10) }
    	};
    	static t_type patches[2];

    	patches[1].b8 = s3_vaddr;
    	patches[0].b8 = elf_size;
    	return (ft_init_patch_set(stage2_patch_offsets[arch], patches, 2,
    							  (size_t)ft_ternary(arch, (char *)sizeof(uint64_t),
    												 (void *)sizeof(uint32_t))));
    }

    static t_stage2_result build_stage2_buffer(t_arch arch, size_t elf_size, uint64_t s3_vaddr)
    {
    	const t_patch_set patch = init_patch_stage2(arch, elf_size, s3_vaddr);
    	t_stage2_result out;
    	t_buffer stage2;
    	t_buffer raw_payload;

    	ft_bzero(&out, sizeof(out));
    	stage2 = load_and_apply_patches_to_buffer(arch, STAGE_2, &patch);
    	if (invalid_buffer(&stage2))
    		return (out);
    	raw_payload = load_stage_elf_buffer(arch, PAYLOAD_BIN);
    	if (invalid_buffer(&raw_payload))
    	{
    		free(stage2.data);
    		return (out);
    	}
    	out.payload_raw_size = raw_payload.len;
    	out.buffer = alloc_concat_buffers(&raw_payload, &stage2);
    	free(raw_payload.data);
    	free(stage2.data);
    	return (out);
    }

    static inline bool is_exec_load_phdr(t_elf_ph *phdr, unsigned char class)
    {
    	return (PT_LOAD == elf_get_program_header_type(phdr, class) && PF_X & elf_get_program_header_flags(phdr, class));
    }


    static uint64_t get_vaddr_gap_fits(const t_elf_ph *phdr, unsigned char class,
    								   size_t gap_size)
    {
        const uint64_t	cur_top = elf_get_program_header_vaddr(phdr, class)
    		+ elf_get_program_header_memsz(phdr, class);
        const uint64_t aligned_top = align_up(cur_top, 0x1000);

        if ((aligned_top - cur_top) < gap_size)
            return (0);
        return (cur_top);
    }


    static uint64_t get_offset_gap_fits(const t_elf_h *ehdr, const t_elf_ph *phdr,
    									unsigned char class, uint64_t phnum,
    									size_t gap_size)
    {
        const uint64_t cur_top = elf_get_program_header_offset(phdr, class)
    		+ elf_get_program_header_memsz(phdr, class);
        uint64_t next;

    	next = elf_get_closest_higher_p_offset(ehdr, class, phnum, cur_top);
        if (next == (uint64_t)-1)
            next = elf_get_elfh_shoff(ehdr);

        if ((next - cur_top) < gap_size)
            return (0);
        return (cur_top);
    }

    static t_s3_find_result elf_get_s3_info(t_s3_find_params *params)
    {
        t_s3_find_result	res;
        uint64_t			ph_ndx;

        if (!params || !params->ehdr || !params->phnum || !params->gap_size)
            return ((t_s3_find_result){0});
        ph_ndx = 0;
        while (ph_ndx < params->phnum)
        {
            res.phdr = elf_get_program_header(params->ehdr, ph_ndx++);
            if (!is_exec_load_phdr(&res.phdr, params->class))
                continue;
            res.vaddr = get_vaddr_gap_fits(&res.phdr, params->class, params->gap_size);
            if (res.vaddr == 0)
                continue;
            res.offset = get_offset_gap_fits(params->ehdr, &res.phdr, params->class,
    										 params->phnum, params->gap_size);
            if (res.offset < params->gap_size)
                continue;
            res.found = true;
            return res;
        }
        return ((t_s3_find_result){0});
    }


    static uint64_t elf_inject_s3(void *elf, t_elf_h *ehdr, unsigned char class)
    {
        t_buffer stage3;
        t_s3_find_params params;
    	t_s3_find_result res;

        stage3 = load_stage_elf_buffer(class == ELFCLASS64, STAGE_3);
        if (invalid_buffer(&stage3))
    		return (-1);
        params = (t_s3_find_params) { .class = class, .ehdr = ehdr,
    								  .phnum = elf_get_elfh_phnum(ehdr),
    								  .gap_size = stage3.len };
    	res = elf_get_s3_info(&params);
        if (res.found)
    		ft_memcpy(elf + res.offset, stage3.data, stage3.len);
        free(stage3.data);
        if (!res.found)
    		return (res.vaddr);
        elf_phdr_set_filesz(&res.phdr, elf_get_program_header_filesz(&res.phdr, class)
    						+ stage3.len, class);
        elf_phdr_set_memsz(&res.phdr, elf_get_program_header_memsz(&res.phdr, class)
    					   + stage3.len, class);
        return (res.vaddr);
    }


    t_stage1_ctx stage1_compute_ctx(unsigned char class, const t_elf_h *ehdr,
    								t_stage1_params *params)
    {
    	t_stage1_ctx ctx;

    	ctx.arch = (t_arch) ft_ternary(class == ELFCLASS64, (void*)ARCH_X64, (void*)ARCH_X86);
        ctx.value_size = (size_t)ft_ternary(ctx.arch, (void*)sizeof(uint64_t),
    										(void*)sizeof(uint32_t));
    	ctx.phnum = elf_get_elfh_phnum(ehdr);
    	ctx.prefix_size = elf_get_struct_size_elf_header((char *)ehdr->elf32)
    		+ elf_get_elfh_phentsize(ehdr) *
    		(2 + (0 != elf_get_gnu_stack_phdr_ndx(ehdr, class, ctx.phnum)));
    	ctx.s2_entry_offset = params->payload_raw_size + 0x12 + ctx.arch * 0x5;
    	return (ctx);
    }

    static const t_patch_delta g_stage1[] = {
    	(t_patch_delta){ .offset = 0xd, .len = 34, .arch = ARCH_X86, .sign = 1 },
    	(t_patch_delta){ .offset = 0x3e, .len = 2, .arch = ARCH_X86, .sign = -1 }, /* -2 B to the insertion affect aes key.... */
    	(t_patch_delta){ .offset = 0x3e, .len = 1, .arch = ARCH_X86, .sign = -1 }, /* -1 B to the insertion affect aes key.... */
    	(t_patch_delta){ .offset = 0xa0, .len = 2, .arch = ARCH_X86, .sign = -1 }, /* -2 B to the insertion affect expected va */
    	(t_patch_delta){ .offset = 0x64, .len = 2, .arch = ARCH_X86, .sign = -1 }, /* -2 B affect to compress hdr len */
    	(t_patch_delta){ .offset = 0x66, .len = 5, .arch = ARCH_X86, .sign = 1 }, /* -2 B affect passed compress hdr len */
    	(t_patch_delta){ .offset = 0x66, .len = 1, .arch = ARCH_X86, .sign = -1 }, /* -2 B affect passed compress hdr len */
    };

    static void stage1_build_patch_desc(
        t_patch_desc desc[STAGE1_PATCH_COUNT],
        t_stage1_patch_values *vals,
        const t_stage1_ctx *ctx,
        const t_stage1_params *params,
        const t_elf_h *ehdr,
        unsigned char class,
        const t_buffer *stage1)
    {
        vals->expected_va =
            elf_get_highest_loadable_vaddr_avalaible(ehdr, class, ctx->phnum)
            + ctx->prefix_size
            + ((ctx->arch == ARCH_X64)
                ? 0x5
                : compute_patch_offset(ctx->arch, 0x8e,
                                       g_stage1,
                                       sizeof(g_stage1) / sizeof(*g_stage1)));
        vals->s1_len = stage1->len + ctx->prefix_size + params->encrypted.len;

        desc[STAGE1_PATCH_EXPECTED_VADDR] = (t_patch_desc){
            .base_off_x86 = 0xa1,
            .base_off_x64 = 0x08,
            .len_x86 = sizeof(uint32_t),
            .len_x64 = sizeof(uint64_t),
            .value = (void *)&vals->expected_va
        };
        desc[STAGE1_PATCH_S1_SIZE] = (t_patch_desc){
            .base_off_x86 = 0xb3,
            .base_off_x64 = 0x1f,
            .len_x86 = sizeof(uint32_t),
            .len_x64 = sizeof(uint64_t),
            .value = (void *)&vals->s1_len
        };
        desc[STAGE1_PATCH_STAGE2_SIZE] = (t_patch_desc){
            .base_off_x86 = 0x0c,
            .base_off_x64 = 0x60,
            .len_x86 = sizeof(uint32_t),
            .len_x64 = sizeof(uint64_t),
            .value = (void *)&params->stage2_total_size
        };
    	desc[STAGE1_PATCH_COMPRESSED_HDR_SIZE] = (t_patch_desc){
            .base_off_x86 = 0x65,
            .base_off_x64 = 0xc0,
            .len_x86 = sizeof(uint32_t),
            .len_x64 = sizeof(uint64_t),
            .value = (void *)&params->compressed_hdr_size
        };
        desc[STAGE1_PATCH_S2_ENTRY_OFFSET] = (t_patch_desc){
            .base_off_x86 = 0xc6,
            .base_off_x64 = 0xf0,
            .len_x86 = sizeof(uint32_t),
            .len_x64 = sizeof(uint64_t),
            .value = (void *)&ctx->s2_entry_offset
        };
        desc[STAGE1_PATCH_AES_KEY] = (t_patch_desc){
            .base_off_x86 = 0x3f,
            .base_off_x64 = 0x98,
            .len_x86 = sizeof(params->key.ints),
            .len_x64 = sizeof(params->key.ints),
            .value = (void *)&params->key.ints[0]
        };
    }

    t_buffer build_stage1_patched(unsigned char class, const t_elf_h *ehdr,
    							  t_stage1_params *params)
    {
    	const size_t patchs_delta_count = sizeof(g_stage1)/sizeof(*g_stage1);
    	const t_stage1_ctx ctx = stage1_compute_ctx(class, ehdr, params);
        t_buffer stage1;
    	t_stage1_patch_values patch_vals;
    	t_patch_desc patches_desc[STAGE1_PATCH_COUNT];

    	stage1 = load_stage_elf_buffer(ctx.arch, STAGE_1);
        if (invalid_buffer(&stage1))
    	{
    		return (stage1);
    	}
    	stage1_build_patch_desc(patches_desc, &patch_vals, &ctx, params, ehdr,
            class, &stage1);
    	apply_patches_to_buffer_with_patch_desc(&stage1, ctx.arch, patches_desc,
    		sizeof(patches_desc) / sizeof (*patches_desc), g_stage1,
    		patchs_delta_count);
        return (stage1);
    }

    t_stage2_result generate_stage2(t_arch arch, t_buffer *elf)
    {
        t_stage2_result stage2;
        t_elf_h elf_header;
        uint64_t s3_va;

        elf_header = (t_elf_h){.elf32 = elf->data};
        ft_bzero(&stage2, sizeof(stage2));
        s3_va = elf_inject_s3(elf->data, &elf_header,
    						  elf_get_elfh_e_ident_class(elf->data));
        if (s3_va == (uint64_t)-1)
    		return (stage2);
        return (build_stage2_buffer(arch, elf->len, s3_va));
    }

    t_stage1_params	prepare_stage1_prereq(unsigned char class, t_buffer *elf)
    {
        const t_arch arch = (class == ELFCLASS64) ? ARCH_X64 : ARCH_X86;
        const size_t value_size = arch ? sizeof(uint64_t) : sizeof(uint32_t);
        t_compress_encrypt com_cypher;
        t_stage2_result payload_stage2;
        t_buffer payload_stage2_elf;

        payload_stage2 = generate_stage2(arch, elf);
        if (invalid_buffer(&payload_stage2.buffer))
    		return ((t_stage1_params){0});
        payload_stage2_elf = alloc_concat_buffers(&payload_stage2.buffer, elf);
        free(payload_stage2.buffer.data);
        if (invalid_buffer(&payload_stage2_elf))
    		return ((t_stage1_params){0});
        com_cypher = compress_encrypt_with_header(&payload_stage2_elf, value_size);
        free(payload_stage2_elf.data);
        if (invalid_buffer(&com_cypher.encrypt.buffer))
    		return ((t_stage1_params){0});

        return ((t_stage1_params){.stage2_total_size=payload_stage2_elf.len,
    							  .encrypted=com_cypher.encrypt.buffer,
    							  .compressed_hdr_size = com_cypher.head_compress_len,
    							  .payload_raw_size = payload_stage2.payload_raw_size,
    							  .key = com_cypher.encrypt.key});
    }

    bool invalid_s1_params(t_stage1_params *val)
    {
    	return (val->stage2_total_size == 0 || val->encrypted.len == 0
    			|| val->compressed_hdr_size == 0 );
    }

t_buffer *prepare_payload_prerequisites(t_buffer *elf)
    {
    	const unsigned char class = elf_get_elfh_e_ident_class(elf->data);
    	const t_elf_h elf_header = { .elf32 = elf->data};
    	t_stage1_params params = prepare_stage1_prereq(class, elf);
    	t_buffer *result;

    	if (invalid_s1_params(&params))
    	{
    		free(params.encrypted.data);
    		return (NULL);
    	}
    	result = malloc(sizeof(*result) * 2);
    	if (result == NULL)
    		return (result);
    	result[0] = build_stage1_patched(class, &elf_header, &params);
    	if (invalid_buffer(&result[0]))
    	{
    		free(result);
    		free(params.encrypted.data);
    		return (NULL);
    	}
    	result[1] = params.encrypted;
    	return (result);
    }

static    t_buffer generate_payload(t_buffer *elf)
    {
    	t_buffer payload;
    	t_buffer *payload_prereq;

    	ft_bzero(&payload, sizeof(payload));
    	payload_prereq = prepare_payload_prerequisites(elf);
    	if (payload_prereq == NULL)
    		return (payload);
    	payload = alloc_concat_buffers(&payload_prereq[0], &payload_prereq[1]);
    	free(payload_prereq[0].data);
    	free(payload_prereq[1].data);
    	free(payload_prereq);
    	return (payload);
    }

    // elf/write
    //t_elf_write_ctx
    static int write_ehdr(t_elf_write_ctx *ctx)
    {
    	size_t ehdr_len = elf_get_struct_size_elf_header((char *)ctx->ehdr->elf32);
    	const uint64_t new_phnum = 2 + (0 != elf_get_gnu_stack_phdr_ndx(ctx->ehdr,
    																	ctx->class,
    																	ctx->phnum));

    	elf_ehdr_set_entry(ctx->ehdr, ctx->new_phdr_vaddr + ehdr_len
    					   + elf_get_elfh_phentsize(ctx->ehdr) * new_phnum,
    					   ctx->class);
    	elf_ehdr_set_phnum(ctx->ehdr, new_phnum, ctx->class);
    	elf_ehdr_set_shoff(ctx->ehdr, 0, ctx->class);
    	elf_ehdr_set_shnum(ctx->ehdr, 0, ctx->class);
    	elf_ehdr_set_shentsize(ctx->ehdr, 0, ctx->class);
    	elf_ehdr_set_shstrndx(ctx->ehdr, 0, ctx->class);
    	if (invalid_write(ctx->fd, ctx->ehdr->elf32, ehdr_len))
    		return (ERR_WRITE_HEADER);
    	return (ERR_SUCCESS);
    }

    void apply_phdr_layout(t_elf_ph *phdr, const t_phdr_layout *l,
    					   unsigned char class)
    {
        elf_phdr_set_offset(phdr, l->offset, class);
        elf_phdr_set_vaddr(phdr,  l->vaddr,  class);
        elf_phdr_set_paddr(phdr,  l->paddr,  class);
        elf_phdr_set_filesz(phdr, l->filesz, class);
        elf_phdr_set_memsz(phdr,  l->memsz,  class);
        elf_phdr_set_align(phdr,  l->align,  class);
        elf_phdr_set_type(phdr,   l->type,   class);
        elf_phdr_set_flags(phdr,  l->flags,  class);
    }

    t_phdr_layout phdr_rw_layout(const t_elf_h *ehdr, unsigned char class,
    							 uint64_t phnum)
    {
        const uint64_t align = 0x1000;
        t_phdr_layout l;
    	t_elf_ph phdr;

        l.align  = align;
        l.offset = 0;
        l.type   = PT_LOAD;
        l.flags  = PF_R | PF_W;
    	phdr = elf_get_program_header(ehdr, 0);
        l.vaddr = elf_get_program_header_vaddr(&phdr, class) & ~(align - 1);
        l.paddr = l.vaddr;
        l.memsz = elf_get_highest_used_vaddr_avalaible(ehdr, class, phnum) - l.vaddr;
        l.filesz = align;
        return l;
    }


        //t_elf_write_ctx
    static t_phdr_layout phdr_rx_layout(const t_elf_write_ctx *ctx)
    {
        const uint64_t ehdr_sz = elf_get_struct_size_elf_header((char *)ctx->ehdr->elf32);
        const uint64_t phdr_sz = elf_get_struct_size_elf_ph(ctx->class);
        const uint64_t ph_count =
            2 + (0 != elf_get_gnu_stack_phdr_ndx(ctx->ehdr, ctx->class, ctx->phnum));
        t_phdr_layout l;

        l.align  = 0x1000;
        l.offset = 0;
        l.type   = PT_LOAD;
        l.flags  = PF_R | PF_X;
        l.vaddr  = ctx->new_phdr_vaddr;
        l.paddr  = ctx->new_phdr_vaddr;
        l.memsz  = ctx->payload_size + ehdr_sz + phdr_sz * ph_count;
        l.filesz = l.memsz;
        return l;
    }

    static int write_program_headers(t_elf_write_ctx *ctx)
    {
    	const uint64_t		phdr_struct_size = elf_get_struct_size_elf_ph(ctx->class);
    	const uint64_t		ndx = elf_get_gnu_stack_phdr_ndx(ctx->ehdr, ctx->class,
    														 ctx->phnum);
    	const t_phdr_layout	rw = phdr_rw_layout(ctx->ehdr, ctx->class, ctx->phnum);
    	const t_phdr_layout	rx = phdr_rx_layout(ctx);
    	t_elf_ph			phdr;

    	phdr = elf_get_program_header(ctx->ehdr, 0);
    	apply_phdr_layout(&phdr, &rw, ctx->class);
        if (invalid_write(ctx->fd, phdr.elf32, phdr_struct_size))
    	{
            return (ERR_WRITE_PHDR);
    	}
    	apply_phdr_layout(&phdr, &rx, ctx->class);
        if (invalid_write(ctx->fd, phdr.elf32, phdr_struct_size))
            return (ERR_WRITE_PHDR);
        if (!ndx)
    		return (ndx);
    	phdr = elf_get_program_header(ctx->ehdr, ndx);
    	if (invalid_write(ctx->fd, phdr.elf32, phdr_struct_size))
    		return (ERR_WRITE_PHDR);
    	return (0);
    }

    int generate_packed_file(void *elf_mmap, t_buffer *payload, int packed)
    {
        t_elf_h ehdr = (t_elf_h){.elf32 = elf_mmap};
        uint64_t shnum;
        uint64_t phnum;
        unsigned char class;
        uint64_t new_phdr_vaddr;

    	set_elf_headers(&ehdr, &shnum, &phnum, &class);
        new_phdr_vaddr = elf_get_highest_loadable_vaddr_avalaible(&ehdr, class, phnum);
    	t_elf_write_ctx ctx = {.ehdr = &ehdr, .fd = packed, .class = class,
    						   .phnum = phnum, .new_phdr_vaddr = new_phdr_vaddr,
    						   .payload_size = payload->len};
        if (write_ehdr(&ctx))
    		return (ERR_WRITE_HEADER);
        if (write_program_headers(&ctx))
    		return (ERR_WRITE_PHDR);
        if (invalid_write(packed, payload->data, payload->len))
    		return (ERR_WRITE_PAYLOAD);
        /* ft_printf("Successfully packed\n"); */
        return (0);
    }

    //elf/packer/elf_packer.c

    int elf_packer(void *elf_mmap, off_t file_size, const char *file_path)
    {
        t_buffer	elf = (t_buffer){.data=elf_mmap, .len=(size_t)file_size};
        t_buffer	payload = generate_payload(&elf);
    	int			packed;
    	int			result;

        if (invalid_buffer(&payload))
    		return (1);
        packed = open(file_path, O_CREAT | O_TRUNC | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
        if (packed < 0)
        {
    		free(payload.data);
    		return (2);
        }
        result = generate_packed_file(elf.data, &payload, packed);
    	free(payload.data);
    	close(packed);
    	if (result)
    		return (3);
        return (0);
    }
