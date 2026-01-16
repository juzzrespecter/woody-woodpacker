#include "cmn.h"

bool ft_in_array(void **arr, uint64_t arr_size, void *value)
{
  while (arr_size && arr[arr_size - 1] != value)
    arr_size--;
  return (arr_size);// if arr_size == 0 False
}
