#ifndef TYPE_H_
#define TYPE_H_

#include <cstdint>

using size_t = decltype(sizeof(uint64_t));
using byte = char;

extern const size_t uint64_t_size = sizeof(uint64_t);

#endif  // TYPE_H_