#ifndef STORAGE_H_
#define STORAGE_H_

#include <memory>

#include "dal/node.h"
#include "dal/dal.h"
#include "settings/settings.h"
#include "memory/type.h"

class Storage {
   public:
    Storage(const std::string& path);

   private:
    // Memory workflow functions
    std::shared_ptr<Node> GetNode(uint64_t page_num);
    void WriteNode(const std::shared_ptr<Node>& node);
    void DeleteNode(uint64_t page_num);
    // B-Tree algorithms
    std::shared_ptr<Node> findNode();

    std::shared_ptr<DAL> dal_;
    uint64_t root_;
};

#endif  // STORAGE_H_