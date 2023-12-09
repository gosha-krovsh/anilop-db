#include <bit>
#include <cstdint>
#include <cstring>

#include "dal/type.h"

namespace convert {

void uint64_to_bytes(byte* dest, uint64_t value);
uint64_t bytes_to_uint64(byte* src);

}  // namespace convert
