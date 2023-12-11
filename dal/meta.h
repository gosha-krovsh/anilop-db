#ifndef META_H_
#define META_H_

#include <string>

#include "serializable.h"

#include "memory/type.h"
#include "memory/memory.h"
#include "exception/exception.h"

class Meta : public ISerializable {
   public:
    Meta() = default;

    size_t Serialize(byte* data, size_t max_volume) override;
    size_t Deserialize(const byte* data, size_t max_volume) override;

    uint64_t GetFreeListPage();
    void SetFreeListPage(uint64_t page);
    uint64_t GetRootPage();
    void SetRootPage(uint64_t page);

   private:
    std::string magic_word = "ANILOPDB";
    uint64_t free_list_page_ = -1;
    uint64_t root_ = -1;
};

#endif  // META_H_
