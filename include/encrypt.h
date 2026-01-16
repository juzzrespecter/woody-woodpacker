#ifndef ENCRYPT_H
# define ENCRYPT_H
# include <stddef.h>
# include "common.h"

t_buffer encrypt(const t_buffer *raw, char **key);
#endif
