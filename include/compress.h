#ifndef COMPRESS_H
# define COMPRESS_H
#include "common.h"// size_t, t_list, t_buffer, uint16_t

typedef enum	e_compress_lz77
  {
    COMPRESS_LZ77_MIN_MATCH_LEN       = 3,
    COMPRESS_LZ77_MAX_MATCH_LEN       = 17,
    COMPRESS_LZ77_SLIDING_WINDOW_SIZE = 0x1000,
    COMPRESS_LZ77_OFFSET_MAX_LIMIT    = 0x0FFF
  }	t_compress_lz77;

typedef struct s_lz77
{
  uint16_t distance_len;
  unsigned char byte;
}	__attribute__((packed)) t_lz77;


unsigned char *lz77_get_compress_arr(t_list *lst, size_t *arr_size);
t_list *lz77_compress_to_list(const unsigned char *data, size_t data_len);
t_buffer lz77_get_headed_buffer(t_buffer *input, size_t value_size);

#endif
