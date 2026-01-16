#include "common.h"

void apply_patches_to_buffer(char *buffer, const t_patch_set *patch)
{
	size_t	i;

	i = 0;
	while (i < patch->count)
	{
		ft_memcpy(buffer + patch->offsets[i], &patch->values[i].b8, patch->value_size);
		i++;
	}
}

  size_t compute_patch_offset(t_arch arch, size_t base_offset,
  	const t_patch_delta *deltas, size_t count)
{
	size_t delta;
  	size_t i;

  	delta = 0;
  	i = 0;
	while (i < count)
  	{
		if ((deltas[i].arch == arch || deltas[i].arch == ARCH_ALL)
  			&& deltas[i].offset <= base_offset)
  			delta += deltas[i].len * deltas[i].sign;
  		i++;
	}
	return (base_offset + delta);
}

t_patch_set ft_init_patch_set(const size_t *off,
    const t_type *val, size_t count, size_t value_size)
{
	t_patch_set patch = (t_patch_set){.offsets=off, .values=val, .count=count,
  									  .value_size=value_size};
	return (patch);
}

   void apply_patches_to_buffer_with_patch_desc(t_buffer *stage1,
  	const t_arch arch,
  	const t_patch_desc *patches_desc, size_t patch_count,
  	const t_patch_delta *deltas, size_t delta_count)
  {
      for (size_t i = 0; i < patch_count; ++i)
      {
          uint64_t final_off = compute_patch_offset(arch,
                                                    (size_t)ft_ternary((arch == ARCH_X64), (void *)patches_desc[i].base_off_x64
  															 , (void *)patches_desc[i].base_off_x86),
                                                    deltas, delta_count);

          size_t	len = (size_t)ft_ternary(arch == ARCH_X64, (void *)patches_desc[i].len_x64,(void *) patches_desc[i].len_x86);
          if (patches_desc[i].value && len)
              ft_memcpy(stage1->data + final_off, patches_desc[i].value, len);
      }
  }
