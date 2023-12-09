#include <bit>
#include <cstdint>
#include <cstring>

using byte = char;

namespace convert {

static constexpr size_t uint64_t_size = sizeof(uint64_t); 

void uint64_to_bytes(byte* dest, uint64_t value);
uint64_t bytes_to_uint64(byte* src);

}  // namespace convert
