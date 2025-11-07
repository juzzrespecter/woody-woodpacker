#include "cmn.h"

bool	is_between(uint64_t lower_limit, uint64_t upper_limit, uint64_t value)
{
  return (lower_limit <= value && value <= upper_limit);
}
