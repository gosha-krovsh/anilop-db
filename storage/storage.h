#ifndef STORAGE_H_
#define STORAGE_H_

#include <cstring>
#include <memory>
#include <tuple>

#include "dal/node.h"
#include "dal/dal.h"
#include "settings/settings.h"
#include "memory/type.h"

class Storage {
   public:
    Storage(const std::string& path, const settings::UserSettings& settings);
    
    // std::vecor<byte> Find(const std::vector<byte> key);

   private:
    // Memory workflow functions
    std::shared_ptr<Node> GetNode(uint64_t page_num);
    void WriteNode(const std::shared_ptr<Node>& node);
    void DeleteNode(uint64_t page_num);
    // B-Tree algorithms
    std::tuple<std::shared_ptr<Node>, size_t> FindKey(const std::vector<byte>& key);
    std::tuple<std::shared_ptr<Node>, size_t> FindKeyRecursive(const std::shared_ptr<Node>& node, const std::vector<byte>& key);
    std::tuple<size_t, bool> FindKeyInNode(const std::shared_ptr<Node>& node, const std::vector<byte>& key);

    std::shared_ptr<DAL> dal_;
    uint64_t root_;
};

#endif  // STORAGE_H_