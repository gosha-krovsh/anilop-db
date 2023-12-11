#ifndef NODE_H_
#define NODE_H_

#include <memory>
#include <cstdint>
#include <vector>

#include "serializable.h"
#include "item.h"

#include "memory/type.h"
#include "memory/memory.h"
#include "settings/settings.h"
#include "exception/exception.h"

class Node : public ISerializable {
   public:
    Node();

    bool isLeaf();

    void SetPageNum(uint64_t page_num);
    uint64_t GetPageNum() const;

    size_t Serialize(byte* data, size_t max_volume) override;
    size_t Deserialize(const byte* data, size_t max_volume) override;

   private:
    uint64_t page_num_;
    std::vector<uint64_t> child_nodes_;
    std::vector<std::shared_ptr<Item>> items_;
};

#endif  // NODE_H_
