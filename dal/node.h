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

    bool IsLeaf() const;

    void SetPageNum(uint64_t page_num);
    uint64_t GetPageNum() const;

    void AddItem(const std::shared_ptr<Item>& item, size_t pos);

    std::vector<uint64_t>* ChildNodesPtr();
    std::vector<std::shared_ptr<Item>>* ItemsPtr();

    size_t HeaderByteLength() const;
    size_t ByteLength() const;

    size_t Serialize(byte* data, size_t max_volume) const override;
    size_t Deserialize(const byte* data, size_t max_volume) override;

   private:
    void CheckPtrInterDeser(const char* left, const char* right);

    uint64_t page_num_;
    std::vector<uint64_t> child_nodes_;
    std::vector<std::shared_ptr<Item>> items_;
};

#endif  // NODE_H_
