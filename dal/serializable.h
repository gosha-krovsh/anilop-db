#ifndef SERIALIZABLE_H_
#define SERIALIZABLE_H_

#include "memory/type.h"

class ISerializable {
   public:
    virtual size_t Serialize(byte* data, size_t max_volume) const = 0;
    virtual size_t Deserialize(const byte* data, size_t max_volume) = 0;

    virtual inline ~ISerializable() {}
};

#endif  // SERIALIZABLE_H_
