#include <math.h>

uint64_t udivmod64(uint64_t dividend, uint64_t divisor, uint64_t* remainder) {
    if (divisor == 0) {
        return 0;
    }

    uint64_t scaled_divisor = divisor;
    uint64_t remain = dividend;
    uint64_t result = 0;
    uint64_t multiple = 1;

    while (scaled_divisor < dividend) {
        scaled_divisor <<= 1;
        multiple <<= 1;
    }

    do {
        if (remain >= scaled_divisor) {
            remain -= scaled_divisor;
            result += multiple;
        }

        scaled_divisor >>= 1;
        multiple >>= 1;
    } while (multiple != 0);

    if (remainder) {
        *remainder = remain;
    }

    return result;
}
