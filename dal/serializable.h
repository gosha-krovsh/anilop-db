#ifndef SERIALIZABLE_H_
#define SERIALIZABLE_H_

#include <cstdint>

using size_t = decltype(sizeof(uint64_t));
using byte = char;

class ISerializable {
   public:
    virtual size_t Serialize(byte* data, size_t max_volume) = 0;
    virtual size_t Deserialize(byte* data, size_t max_volume) = 0;

    virtual inline ~ISerializable() {}
};

#endif  // SERIALIZABLE_H_
