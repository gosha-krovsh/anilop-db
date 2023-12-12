#ifndef STORAGE_H_
#define STORAGE_H_

#include <cstring>
#include <memory>
#include <tuple>

#include "dal/dal.h"
#include "dal/node.h"
#include "memory/type.h"
#include "settings/settings.h"

class Storage {
   public:
    Storage(const std::string& path, const settings::UserSettings& settings);

    std::tuple<std::vector<byte>, bool> Find(const std::vector<byte>& key);
    void Put(const std::vector<byte> key, const std::vector<byte> value);
    void Remove(const std::vector<byte> key);

   private:
    // Memory workflow functions
    std::shared_ptr<Node> GetNode(uint64_t page_num);
    std::vector<std::shared_ptr<Node>> GetNodes(const std::vector<uint64_t>& page_nums);
    void WriteNode(const std::shared_ptr<Node>& node);
    void DeleteNode(uint64_t page_num);
    // Threshold calls
    double MaxThreshhold();
    double MinThreshhold();
    bool IsOverPopulated(const std::shared_ptr<Node>& node);
    bool IsUnderPopulated(const std::shared_ptr<Node>& node);
    // B-Tree algorithms

    // Find helpers
    std::tuple<std::shared_ptr<Node>, size_t, std::vector<uint64_t>> FindKey(
        const std::vector<byte>& key, bool exact_key);
    std::tuple<std::shared_ptr<Node>, size_t> FindKeyRecursive(const std::shared_ptr<Node>& node,
                                                               const std::vector<byte>& key,
                                                               bool exact_key,
                                                               std::vector<uint64_t>* ancestors);
    std::tuple<size_t, bool> FindKeyInNode(const std::shared_ptr<Node>& node,
                                           const std::vector<byte>& key);
    // Put helpers
    uint64_t GetSplitIndex(const std::shared_ptr<Node>& node);
    void Split(const std::shared_ptr<Node>& parent, const std::shared_ptr<Node>& child,
               size_t childIndex);
    // Remove helpers
    void RemoveFromLeaf(const std::shared_ptr<Node>& node, size_t item_index);
    std::vector<uint64_t> RemoveFromInternal(const std::shared_ptr<Node>& parent_node,
                                             size_t item_index);
    void LeftRotate(const std::shared_ptr<Node>& lhs, const std::shared_ptr<Node>& mhs,
                    const std::shared_ptr<Node>& rhs, size_t r_node_index);
    void RightRotate(const std::shared_ptr<Node>& lhs, const std::shared_ptr<Node>& mhs,
                     const std::shared_ptr<Node>& rhs, size_t r_node_index);
    void Merge(const std::shared_ptr<Node>& parent, const std::shared_ptr<Node>& unbalanced,
               size_t u_node_index);
    void RemoveAndRebalance(const std::shared_ptr<Node>& parent, const std::shared_ptr<Node>& unbalanced,
                         size_t u_node_index);

    settings::UserSettings settings_;
    std::shared_ptr<DAL> dal_;
    uint64_t root_;
};

#endif  // STORAGE_H_