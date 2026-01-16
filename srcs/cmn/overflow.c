#include <stdint.h>
#include <stdbool.h>

bool	unsigned_overflow_limit(uint64_t times, uint64_t size, uint64_t limit)
{
        if (!size || !times)
                return (false);
        return (times > limit / size);
}

bool	signed_overflow_limit(int64_t times, int64_t size, int64_t limit)
{
        int64_t	result;

        if (!times || !size)
                return (false);
        result = limit / size;
        if (result < 0)
                return (times < result);
        return (times > result);
}

bool	unsigned_int_mult_overflow(uint64_t times, uint64_t size,
                        uint64_t size_bits)
{
        uint64_t	limit;

        if (!times || !size)
                return (false);
        if (!size_bits || size_bits > 64)
                return (true);
        limit = 0;
        while (--size_bits)
                limit = limit << 1 | 1;
        return (unsigned_overflow_limit(times, size, limit));
}

bool	int_mult_overflow(int64_t times, int64_t size, uint64_t size_bits)
{
        int64_t	limit;
        int64_t	bit_mask;

        if (!times || !size)
                return (false);
        if (!size_bits || size_bits > 64)
                return (true);
        limit = 1 << (size_bits - 1);
        bit_mask = 1;
        while (--size_bits)
                bit_mask = bit_mask << 1 | 1;
        if ((limit == size && times == -1) || (limit == times && -1 == size))
                return (true);
        return (signed_overflow_limit(times, size, limit) // Negative min
                || signed_overflow_limit(times, size, ~limit & bit_mask));// Positive Max
}

bool	uint_mult_overflow(unsigned int times, unsigned int size)
{
        return (unsigned_int_mult_overflow(times, size, sizeof(size) * 8));
}
