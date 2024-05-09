#ifndef FREELIST_H_
#define FREELIST_H_

#include <cstdint>
#include <vector>

#include "serializable.h"

#include "memory/type.h"
#include "memory/memory.h"
#include "exception/exception.h"

class FreeList : public ISerializable {
   public:
    FreeList();
    explicit FreeList(uint64_t max_page);

    size_t Serialize(byte* data, size_t max_volume) const override;
    size_t Deserialize(const byte* data, size_t max_volume) override;

    uint64_t GetMaxPage() const;
    void SetMaxPage(uint64_t max_page);

    uint64_t GetNextPage();
    void ReleasePage(uint64_t page_num);
    void ReleaseAllPages(uint64_t start_page_num);

    bool HasFreePages();

   private:
    uint64_t max_page_;
    uint64_t current_max_page_;
    std::vector<uint64_t> released_pages_;
};

#endif  // FREELIST_H_
