#include "convert.h"

void convert::uint64_to_bytes(byte* dest, uint64_t value) {
    if constexpr (std::endian::native == std::endian::little) {
        std::memcpy(dest, reinterpret_cast<char*>(&value), 8);
    } else {
        dest[0] = static_cast<char>(value);
        dest[1] = static_cast<char>(value >> 8);
        dest[2] = static_cast<char>(value >> 16);
        dest[3] = static_cast<char>(value >> 24);
        dest[4] = static_cast<char>(value >> 32);
        dest[5] = static_cast<char>(value >> 40);
        dest[6] = static_cast<char>(value >> 48);
        dest[7] = static_cast<char>(value >> 56);
    }
}

uint64_t convert::bytes_to_uint64(byte* src) {
    uint64_t value = 0;
    if constexpr (std::endian::native == std::endian::little) {
        std::memcpy(reinterpret_cast<char*>(&value), src, 8);
    } else {
        value += (uint64_t)src[0];
        value += (uint64_t)src[1] << 8;
        value += (uint64_t)src[2] << 16;
        value += (uint64_t)src[3] << 24;
        value += (uint64_t)src[4] << 32;
        value += (uint64_t)src[5] << 40;
        value += (uint64_t)src[6] << 48;
        value += (uint64_t)src[7] << 56;
    }

    return value;
}
