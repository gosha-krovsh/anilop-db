#include <bit>
#include <cstdint>
#include <cstring>

#include "type.h"

namespace memory {

void uint16_to_bytes(byte* dest, uint16_t value);
uint16_t bytes_to_uint16(const byte* src);

void uint64_to_bytes(byte* dest, uint64_t value);
uint64_t bytes_to_uint64(const byte* src);

}  // namespace convert
