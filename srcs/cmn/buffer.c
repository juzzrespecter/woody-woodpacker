#include "common.h"

bool invalid_buffer(t_buffer *buf)
{
  	return (!buf->data || !buf->len);
}

t_buffer alloc_concat_buffers(const t_buffer *a, const t_buffer *b)
{
  	t_buffer concat;

  	concat.len = a->len + b->len;
  	concat.data = malloc(concat.len);
  	if (!concat.data)
  		return (concat);
  	ft_memcpy(concat.data, a->data, a->len);
  	ft_memcpy(concat.data + a->len, b->data, b->len);
  	return (concat);
}
