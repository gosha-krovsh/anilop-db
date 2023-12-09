#ifndef META_H_
#define META_H_

#include <string>

#include "freelist.h"
#include "serializable.h"

using byte = char;

class Meta : public ISerializable {
    friend FreeList;

   public:
    Meta() = default;

    size_t Serialize(byte* data, size_t max_volume) override;
    size_t Deserialize(byte* data, size_t max_volume) override;

    FreeList& FreeListPtr();

   private:
    std::string magic_word = "ANILOPDB";
    FreeList free_list_;
};

#endif  // META_H_
