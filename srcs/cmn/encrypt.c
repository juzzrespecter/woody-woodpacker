#include "common.h"

//                                        unused ret_size,
void *encrypt(void *raw, size_t raw_size, size_t *ret_size, int *key)
{
  	if (raw_size)
		*ret_size = raw_size;
	if (key)
		ft_bzero(key, 128/8);
  	return (raw);
}

t_encrypt wrapper_encrypt(t_buffer *raw)
{
	t_encrypt crypt;

	crypt.buffer.data = encrypt(raw->data, raw->len, &crypt.buffer.len, (int *)&crypt.key.ints[0]);
	return (crypt);
}
