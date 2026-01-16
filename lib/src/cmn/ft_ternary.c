#include "cmn.h"

const void *ft_ternary(const bool condition, const void *true_val,
		       const void *false_val)
{
  if (condition)
    return (true_val);
  return (false_val);
}
