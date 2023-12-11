#ifndef ITEM_H_
#define ITEM_H_

#include <cstring>
#include <cstdint>
#include <vector>

#include "serializable.h"

#include "memory/type.h"
#include "memory/memory.h"
#include "exception/exception.h"

class Item : public ISerializable {
   public:
    Item();
    Item(std::vector<byte> key, std::vector<byte> value);

    size_t ByteLength();

    size_t KeySize();
    byte* KeyData();

    size_t ValueSize();
    byte* ValueData();

    size_t Serialize(byte* data, size_t max_volume) override;
    size_t Deserialize(const byte* data, size_t max_volume) override;

   private:
    std::vector<byte> key_;
    std::vector<byte> value_;
};

#endif  // ITEM_H_
