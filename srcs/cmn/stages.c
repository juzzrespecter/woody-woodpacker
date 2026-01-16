#include "common.h"

static const char	*g_binary_paths[FT_NUM][NUM_ARCHS][NUM_BIN_TYPES] = {
	[FT_ELF] = {
      [ARCH_X86] = {
          [STAGE_1]     = "bytecode/elf/x32/stage1.bin",
          [STAGE_2]     = "bytecode/elf/x32/stage2.bin",
          [STAGE_3]     = "bytecode/elf/x32/stage3.bin",
          [PAYLOAD_BIN] = "bytecode/elf/x32/payload.bin"
      },
      [ARCH_X64] = {
          [STAGE_1]     = "bytecode/elf/x64/stage1.bin",
          [STAGE_2]     = "bytecode/elf/x64/stage2.bin",
          [STAGE_3]     = "bytecode/elf/x64/stage3.bin",
          [PAYLOAD_BIN] = "bytecode/elf/x64/payload.bin"
      }
	},
	[FT_PE] = {
	 [ARCH_X86] = {
          [STAGE_1]     = "bytecode/pe/x32/stage1.bin",
          [STAGE_2]     = "bytecode/pe/x32/stage2.bin",
          [STAGE_3]     = "bytecode/pe/x32/stage3.bin",
          [PAYLOAD_BIN] = "bytecode/pe/x32/payload.bin"
      },
      [ARCH_X64] = {
          [STAGE_1]     = "bytecode/pe/x64/stage1.bin",
          [STAGE_2]     = "bytecode/pe/x64/stage2.bin",
          [STAGE_3]     = "bytecode/pe/x64/stage3.bin",
          [PAYLOAD_BIN] = "bytecode/pe/x64/payload.bin"
	  }
	}
  };


t_buffer	load_stage_buffer(t_arch arch, t_bin_type bin_type, t_file_type ft)
  {

      int						stage_fd;
      t_buffer				stage;

      stage_fd = open(g_binary_paths[ft][arch][bin_type], O_RDONLY);
      if (stage_fd < 0 || get_file_size(stage_fd, (off_t *)&stage.len) || !stage.len)
      {
  		if (stage_fd >= 0)
  			close(stage_fd);
  		return ((t_buffer){0});
      }
      stage.data = malloc(stage.len);
      if (!stage.data)
      {
  		close(stage_fd);
  		return ((t_buffer){0});
      }
      bool invalid = invalid_read(stage_fd, stage.data, stage.len);
      close(stage_fd);
      if (!invalid)
  		return (stage);
  	free(stage.data);
  	return ((t_buffer){0});
  }

  t_buffer	load_stage_elf_buffer(t_arch arch, t_bin_type bin_type)
  {
	  return (load_stage_buffer(arch, bin_type, FT_ELF));
  }

    t_buffer load_and_apply_patches_to_buffer(t_arch arch, t_bin_type bin_type, const t_patch_set *patch)
  {
  	t_buffer binary;

  	ft_bzero(&binary, sizeof(binary));
  	binary = load_stage_elf_buffer(arch, bin_type);
  	if (!invalid_buffer(&binary))
  		apply_patches_to_buffer(binary.data, patch);
  	return (binary); // Success
  }
