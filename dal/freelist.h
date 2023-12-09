#ifndef FREELIST_H_
#define FREELIST_H_

#include <cstdint>
#include <vector>

#include "type.h"
#include "serializable.h"
#include "convert/convert.h"
#include "exception/exception.h"

class FreeList : public ISerializable {
   public:
    FreeList();

    size_t Serialize(byte* data, size_t max_volume) override;
    size_t Deserialize(byte* data, size_t max_volume) override;

    uint64_t GetMaxPage();
    void SetMaxPage(uint64_t max_page);

    uint64_t GetNextPage();
    void RealeasePage(uint64_t page_num);

   private:
    uint64_t max_page_;
    std::vector<uint64_t> realeased_pages_;
};

#endif  // FREELIST_H_
