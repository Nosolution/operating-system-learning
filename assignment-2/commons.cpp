#include "commons.h"


/**
 * Parse bytes to number
 * @param   bytes   bytes to be convert   
 * @param   n       number of bytes
 * @return  number that parsed into
 */
number bs2n(const byte *bytes, int n)
{
    number res = 0;
    const short base = 256;
    for (int i = n - 1; i >= 0; i--)
    {
        byte b = bytes[i];
        res *= base;
        number bit = 0;
        for (int j = 7; j >= 0; j--)
        {
            bit *= 2;
            if (b & 0x80)
            {
                bit++;
            }
            b = b << 1;
        }
        res += bit;
    }
    return res;
};