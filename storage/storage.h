#ifndef STORAGE_H_
#define STORAGE_H_

#include <cstring>
#include <memory>
#include <tuple>
#include <shared_mutex>
#include <mutex>

#include "dal/dal.h"
#include "dal/log_dal.h"
#include "dal/memory_log_dal.h"
#include "dal/node.h"
#include "memory/type.h"
#include "settings/settings.h"
#include "storage/log_storage.h"

class Storage {
   public:
    Storage(const std::string& path,
            const settings::UserSettings& settings);
    ~Storage();

    std::optional<std::vector<byte>> Find(const std::vector<byte>& key);
    void Put(const std::vector<byte>& key, const std::vector<byte>& value);
    void Remove(const std::vector<byte>& key);

    void PushTransactionLogs(const std::vector<Log>& logs);

    /// @brief Restores saved state
    void Restore();

   private:
    /// @brief Clears saved state
    void ClearState();

    // Tree functions
    std::optional<std::vector<byte>> FindInTree(const std::vector<byte>& key);
    void PutInTree(const std::vector<byte>& key, const std::vector<byte>& value);
    void RemoveInTree(const std::vector<byte>& key);
    // Tree functions impls
    std::optional<std::vector<byte>> FindInTreeImpl(const std::vector<byte>& key);
    void PutInTreeImpl(const std::vector<byte>& key, const std::vector<byte>& value);
    void RemoveInTreeImpl(const std::vector<byte>& key);

    // Memory workflow functions
    std::shared_ptr<Node> GetNode(uint64_t page_num);
    std::vector<std::shared_ptr<Node>> GetNodes(const std::vector<uint64_t>& page_nums);
    void WriteNode(const std::shared_ptr<Node>& node, bool is_new);
    /// @warning Forbidden to change state of node, before delete
    void DeleteNode(const std::shared_ptr<Node>& node);
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
    int64_t GetSplitIndex(const std::shared_ptr<Node>& node);
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

    void PushLog();
    void PushLogAsync();

    void UpdateSaveProcess();

    std::shared_mutex mutex_;

    settings::UserSettings settings_;

    std::shared_ptr<DAL> dal_;
    std::shared_ptr<LogDAL> log_dal_;
    bool save_started_ = false;
    std::shared_ptr<MemoryLogDAL> memory_log_dal_;

    uint64_t root_;

    // Storage extension
    LogStorage log_storage_;
};

#endif  // STORAGE_H_