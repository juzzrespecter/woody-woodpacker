#include "compress.h"


  static t_buffer lz77_get_buffer(t_buffer *input)
  {
      t_list *compressed_list;
      t_buffer compressed;

      compressed_list  = lz77_compress_to_list((unsigned char *)input->data, input->len);
      if (!compressed_list)
  		return (compressed);
      compressed.data = lz77_get_compress_arr(compressed_list, &compressed.len);
      ft_lstclear(&compressed_list, free);
  	return (compressed);
  }

  t_buffer lz77_get_headed_buffer(t_buffer *input, size_t value_size)
  {
  	t_buffer tmp = {.data=&input->len, .len=value_size};
      t_buffer compressed;
      t_buffer compressed_with_header;

  	compressed = lz77_get_buffer(input);
  	if (invalid_buffer(&compressed))
  		return (compressed);
  	compressed_with_header = alloc_concat_buffers(&tmp, &compressed);
  	free(compressed.data);
  	return (compressed_with_header);
  }


t_list *lz77_compress_to_list(const unsigned char *data, size_t data_len) {
    if (!data || data_len == 0) return NULL;

    t_list *lst = NULL;
    size_t i = 0;

    while (i < data_len) {
	size_t best_offset = 0;
	size_t best_length = 0;

	// Define window start
	size_t window_start = (i > COMPRESS_LZ77_SLIDING_WINDOW_SIZE) ?
	    i - COMPRESS_LZ77_SLIDING_WINDOW_SIZE : 0;

	// Search for the longest match in sliding window
	for (size_t offset = 1; offset <= (i - window_start) && offset <= COMPRESS_LZ77_OFFSET_MAX_LIMIT; offset++) {
	    size_t match_length = 0;
	    size_t max_possible = (data_len - i > COMPRESS_LZ77_MAX_MATCH_LEN) ?
		COMPRESS_LZ77_MAX_MATCH_LEN : (data_len - i);

	    while (match_length < max_possible &&
		   data[i - offset + match_length] == data[i + match_length]) {
		match_length++;
	    }

	    if (match_length >= COMPRESS_LZ77_MIN_MATCH_LEN && match_length > best_length) {
		best_length = match_length;
		best_offset = offset;
	    }
	}

	// Allocate token
	t_lz77 *token = malloc(sizeof(t_lz77));
	if (!token) {
	    ft_lstclear(&lst, free);
	    return NULL;
	}
	token->distance_len = 0;
	token->byte = 0;

	if (best_length >= COMPRESS_LZ77_MIN_MATCH_LEN) {
	    // Valid match
	    if (best_offset > 0xFFF) {
		/* fprintf(stderr, "Offset %zu exceeds 12-bit limit\n", best_offset); */
		free(token);
		ft_lstclear(&lst, free);
		return NULL;
	    }
	    size_t token_len = best_length - 2;
	    token->distance_len = (uint16_t)((best_offset << 4) | (token_len & 0x0F));
	    token->byte = (i + best_length < data_len) ? data[i + best_length] : 0;
	    i += best_length + 1;
	} else {
	    token->distance_len = 0;
	    token->byte = data[i];
	    i += 1;
	}

	// Create list node
	t_list *node = ft_lstnew(token);
	if (!node) {
	    free(token);
	    ft_lstclear(&lst, free);
	    return NULL;
	}

	ft_lstadd_back(&lst, node);
    }

    return lst;
}

unsigned char *lz77_get_compress_arr(t_list *lst, size_t *arr_size)
{
    *arr_size = ft_lstsize(lst) * 3;
    unsigned char *str = malloc(*arr_size);
    unsigned char *str_pos = str;
    if (!str)
	return (str);
    t_list *tmp = lst;
    while (tmp)
	{
	    t_lz77 *token = (t_lz77 *)tmp->content;

	    uint16_t dist_len = token->distance_len;
	    str_pos[0] = dist_len & 0xFF;         // low byte
	    str_pos[1] = (dist_len >> 8) & 0xFF;  // high byte
	    str_pos[2] = token->byte;             // literal byte
	    str_pos += 3;
	    tmp = tmp->next;
	}
    return ((void *)str);
}
