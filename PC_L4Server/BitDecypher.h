#pragma once
#include <algorithm>

template <typename IntegerType>
IntegerType bitsToInt(IntegerType& result, const unsigned char* bits, bool little_endian = true, const unsigned int size = UINT_MAX)
{
    result = 0;
    unsigned int length = __min(sizeof(result), size);
    if (little_endian)
        for (int n = length; n >= 0; n--)
            result = (result << 8) + bits[n];
    else
        for (unsigned n = 0; n < length; n++)
            result = (result << 8) + bits[n];
    return result;
}

template <typename IntegerType>
void intToBits(const IntegerType& result, unsigned char* bits, bool little_endian = true, const unsigned int size = UINT_MAX)
{
    auto in = result;
    int length = __min(sizeof(result), size);
    if (little_endian)
        for (unsigned n = 0; n < length; n++)
        {
            bits[n] = in & 255;
            in >>= 8;
        }
    else
        for (int n = length - 1; n >= 0; n--)
        {

            bits[n] = in & 255;
            in >>= 8;
        }
}