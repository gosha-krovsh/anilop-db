#ifndef TYPE_H_
#define TYPE_H_

#include <cstdint>
#include <vector>

using size_t = decltype(sizeof(uint64_t));
using byte = char;

using Data = std::vector<byte>;

extern const size_t uint64_t_size;

#endif  // TYPE_H_