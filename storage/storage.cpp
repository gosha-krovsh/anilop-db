#include "storage.h"

#include <memory>
#include <thread>

Storage::Storage(
        const std::string& path,
        const settings::UserSettings& settings)
    : settings_(settings),
      dal_(new DAL(path, path + ".log", settings)),
      root_(dal_->GetMetaPtr()->GetRootPage()),
      log_storage_(dal_, settings_){}

std::optional<std::vector<byte>> Storage::Find(const std::vector<byte>& key) {
    std::unique_lock lock(mutex_);
    auto log_result = log_storage_.Find(key);
    if (log_result.has_value()) {
        return log_result;
    }

    return FindInTree(key);
}

void Storage::Put(const std::vector<byte>& key, const std::vector<byte>& value) {
    std::unique_lock lock(mutex_);
    // Log workflow
    if (log_storage_.Put(key, value)) {
        PushLogAsync();
        return;
    }
    // Tree workflow
    PutInTree(key, value);
}

void Storage::Remove(const std::vector<byte>& key) {
    std::unique_lock lock(mutex_);
    // Log workflow
    if (log_storage_.Remove(key)) {
        return;
    }
    // Tree workflow
    RemoveInTree(key);
}

std::optional<std::vector<byte>> Storage::FindInTree(const std::vector<byte>& key) {
    if (root_ == 0) {
        return std::nullopt;
    } else {
        auto [node, index, _] = FindKey(key, true);
        if (node == std::nullptr_t()) {
            return std::nullopt;
        } else {
            return std::make_optional(node->ItemsPtr()->operator[](index)->GetValue());
        }
    }
}

void Storage::PutInTree(const std::vector<byte>& key, const std::vector<byte>& value) {
    std::shared_ptr<Item> new_item = std::make_shared<Item>(key, value);
    std::shared_ptr<Node> root_node;
    if (root_ == 0) {
        root_node = std::make_shared<Node>();
        root_node->AddItem(new_item, 0);
    } else {
        root_node = GetNode(root_);
    }

    // Find node for insert and all ancestor page_nums
    auto [insert_node, insert_index, ancestor_indices] = FindKey(key, false);
    // Add new item to the leaf node
    if (!insert_node->ItemsPtr()->empty()) {
        auto item = insert_node->ItemsPtr()->operator[](insert_index);
        if (std::memcmp(item->KeyData(), key.data(), std::min(item->ByteLength(), key.size())) == 0) {
            insert_node->ItemsPtr()->operator[](insert_index) = new_item;
        }
    } else {
        insert_node->AddItem(new_item, insert_index);
    }
    WriteNode(insert_node);

    // Get all ancestors
    auto ancestors = GetNodes(ancestor_indices);

    // Split nodes, except root, if necessary
    for (int64_t i = ancestors.size() - 2; i >= 0; --i) {
        auto parent_node = ancestors[i];
        auto child_node = ancestors[i + 1];
        auto child_index = ancestor_indices[i + 1];
        if (IsOverPopulated(child_node)) {
            Split(parent_node, child_node, child_index);
        }
    }

    // Split root, if necessary
    if (IsOverPopulated(root_node)) {
        std::shared_ptr<Node> new_root = std::make_shared<Node>();
        new_root->SetPageNum(root_node->GetPageNum());
        Split(new_root, root_node, 0);

        WriteNode(new_root);
        root_ = new_root->GetPageNum();
    }
}

void Storage::RemoveInTree(const std::vector<byte>& key) {
    auto root_node = GetNode(root_);

    auto [remove_node, remove_index, ancestor_indices] = FindKey(key, true);
    if (remove_node == std::nullptr_t()) {
        return;
    }

    if (remove_node->IsLeaf()) {
        RemoveFromLeaf(remove_node, remove_index);
    } else {
        auto affected_nodes = RemoveFromInternal(remove_node, remove_index);
        ancestor_indices.insert(ancestor_indices.end(), affected_nodes.begin(), affected_nodes.end());
    }

    auto ancestors = GetNodes(ancestor_indices);
    for (int64_t i = ancestors.size() - 2; i >= 0; --i) {
        auto parent_node = ancestors[i];
        auto node = ancestors[i + 1];
        if (IsUnderPopulated(node)) {
            RemoveAndRebalance(parent_node, node, ancestor_indices[i + 1]);
        }
    }

    if (root_node->ItemsPtr()->empty() && !root_node->ChildNodesPtr()->empty()) {
        root_ = ancestors[1]->GetPageNum();
    }
}

std::shared_ptr<Node> Storage::GetNode(uint64_t page_num) {
    std::shared_ptr<Page> page = dal_->ReadPage(page_num);
    std::shared_ptr<Node> node(new Node());

    node->SetPageNum(page_num);
    node->Deserialize(page->Data(), dal_->GetMetaPtr()->GetPageSize());
    return node;
}

std::vector<std::shared_ptr<Node>> Storage::GetNodes(const std::vector<uint64_t>& page_nums) {
    std::vector<std::shared_ptr<Node>> result;
    for (auto page_num : page_nums) {
        result.emplace_back(GetNode(page_num));
    }
    return result;
}

void Storage::WriteNode(const std::shared_ptr<Node>& node) {
    std::shared_ptr<Page> page = dal_->AllocateEmptyPage();
    if (node->GetPageNum() == 0) {
        uint64_t new_page_num = dal_->GetNextPage();
        node->SetPageNum(new_page_num);
    }
    page->SetPageNum(node->GetPageNum());

    node->Serialize(page->Data(), dal_->GetMetaPtr()->GetPageSize());
}

void Storage::DeleteNode(uint64_t page_num) { dal_->ReleasePage(page_num); }

double Storage::MaxThreshhold() {
    return settings_.max_fill_percent * dal_->GetMetaPtr()->GetPageSize();
}

double Storage::MinThreshhold() {
    return settings_.min_fill_percent * dal_->GetMetaPtr()->GetPageSize();
}

bool Storage::IsOverPopulated(const std::shared_ptr<Node>& node) {
    return node->ByteLength() > MaxThreshhold() && node->ItemsPtr()->size() > 1;
}

bool Storage::IsUnderPopulated(const std::shared_ptr<Node>& node) {
    return node->ByteLength() < MinThreshhold();
}

std::tuple<std::shared_ptr<Node>, size_t, std::vector<uint64_t>> Storage::FindKey(
    const std::vector<byte>& key,
    bool exact_key) {
    std::shared_ptr<Node> root_node = GetNode(root_);
    std::vector<uint64_t> ancestors;
    auto [node, index] = FindKeyRecursive(root_node, key, exact_key, &ancestors);
    return std::tie(node, index, ancestors);
}

std::tuple<std::shared_ptr<Node>, size_t> Storage::FindKeyRecursive(
    const std::shared_ptr<Node>& node, 
    const std::vector<byte>& key,
    bool exact_key,
    std::vector<uint64_t>* ancestors) {
    ancestors->emplace_back(node->GetPageNum());

    auto [index, was_found] = FindKeyInNode(node, key);
    if (was_found) {
        return std::tie(node, index);
    }

    if (node->IsLeaf()) {
        if (!exact_key) {
            return std::tie(node, index);
        }
        return std::forward_as_tuple(std::nullptr_t(), 0);
    }

    uint64_t child_page_num = (*node->ChildNodesPtr())[index];
    std::shared_ptr<Node> child_node = GetNode(child_page_num);
    return FindKeyRecursive(child_node, key, exact_key, ancestors);
}

std::tuple<size_t, bool> Storage::FindKeyInNode(const std::shared_ptr<Node>& node,
                                                const std::vector<byte>& key) {
    for (size_t i = 0; i < node->ItemsPtr()->size(); ++i) {
        std::shared_ptr<Item> item = (*node->ItemsPtr())[i];
        int comp_result =
            std::memcmp(item->KeyData(), key.data(), std::min(key.size(), item->KeySize()));
        if (comp_result == 0) {
            return std::forward_as_tuple(i, true);
        }

        if (comp_result > 0) {
            return std::forward_as_tuple(i, false);
        }
    }
    return std::forward_as_tuple(node->ItemsPtr()->size(), false);
}

int64_t Storage::GetSplitIndex(const std::shared_ptr<Node>& node) {
    size_t byte_length = node->HeaderByteLength();
    size_t items_size = node->ItemsPtr()->size();
    for (size_t i = 0; i < items_size; ++i) {
        byte_length += node->ItemsPtr()->operator[](i)->ByteLength();

        if (1. * byte_length > MinThreshhold() && i < items_size - 1) {
            return i + 1;
        }
    }

    // This behavior is usually caused by other broken Insert logic
    return -1;
}

void Storage::Split(const std::shared_ptr<Node>& parent, const std::shared_ptr<Node>& child,
                    size_t child_index) {
    int64_t split_index = GetSplitIndex(child);
    if (split_index == -1) {
        throw storage_error::InsertFailure(
            "Insert failed. Split called on lonely/underpopulated node.");
    }

    std::shared_ptr<Item> middle_item = child->ItemsPtr()->operator[](split_index);
    std::shared_ptr<Node> new_node = std::make_shared<Node>();

    if (child->IsLeaf()) {
        (*new_node->ItemsPtr()) = {child->ItemsPtr()->begin() + split_index + 1,
                                   child->ItemsPtr()->end()};
        WriteNode(new_node);
        child->ItemsPtr()->resize(split_index);
    } else {
        (*new_node->ItemsPtr()) = {child->ItemsPtr()->begin() + split_index + 1,
                                   child->ItemsPtr()->end()};
        (*new_node->ChildNodesPtr()) = {child->ChildNodesPtr()->begin() + split_index + 1,
                                        child->ChildNodesPtr()->end()};
        WriteNode(new_node);
        child->ItemsPtr()->resize(split_index);
        child->ChildNodesPtr()->resize(split_index + 1);
    }
    parent->AddItem(middle_item, child_index);

    if (parent->ChildNodesPtr()->size() == child_index + 1) {
        // Add child to the back
        parent->ChildNodesPtr()->emplace_back(new_node->GetPageNum());
    } else {
        // Split child nodes and insert in the middle
        parent->ChildNodesPtr()->insert(parent->ChildNodesPtr()->begin() + child_index + 1,
                                        new_node->GetPageNum());
    }
    WriteNode(parent);
    WriteNode(child);
}

void Storage::RemoveFromLeaf(const std::shared_ptr<Node>& node, size_t item_index) {
    node->ItemsPtr()->erase(node->ItemsPtr()->begin() + item_index);
    WriteNode(node);
}

std::vector<size_t> Storage::RemoveFromInternal(const std::shared_ptr<Node>& parent_node,
                                                size_t item_index) {
    /* Finds an element, that can replace current. Applies changes and returns touched nodes */
    std::vector<size_t> affected_children = {item_index};

    auto current_node_page = parent_node->ChildNodesPtr()->operator[](item_index);
    auto current_node = GetNode(current_node_page);
    while (!current_node->IsLeaf()) {
        size_t next_index = current_node->ChildNodesPtr()->size() - 1;
        current_node_page = parent_node->ChildNodesPtr()->operator[](next_index);
        current_node = GetNode(current_node_page);

        affected_children.emplace_back(next_index);
    }

    auto& parent_items = *parent_node->ItemsPtr();
    auto& current_items = *current_node->ItemsPtr();
    parent_items[item_index] = current_items[current_items.size() - 1];

    WriteNode(parent_node);
    WriteNode(current_node);
    return affected_children;
}

void Storage::LeftRotate(const std::shared_ptr<Node>& lhs, const std::shared_ptr<Node>& mhs,
                         const std::shared_ptr<Node>& rhs, size_t r_node_index) {
    // Remove left element from rhs node
    std::shared_ptr<Item> r_item = rhs->ItemsPtr()->front();
    rhs->ItemsPtr()->erase(rhs->ItemsPtr()->begin());

    // Update parents(middle) element
    size_t parent_r_item = r_node_index;
    if (r_node_index == mhs->ChildNodesPtr()->size() - 1) {
        parent_r_item = r_node_index - 1;
    }
    std::shared_ptr<Item> m_item = mhs->ItemsPtr()->operator[](parent_r_item);
    mhs->ItemsPtr()->operator[](parent_r_item) = r_item;

    // Update left element
    lhs->ItemsPtr()->emplace_back(m_item);

    // Update leaves
    if (!rhs->IsLeaf()) {
        auto child_node_ptr = rhs->ChildNodesPtr()->operator[](0);
        rhs->ChildNodesPtr()->erase(rhs->ChildNodesPtr()->begin());

        lhs->ChildNodesPtr()->emplace_back(child_node_ptr);
    }
}

void Storage::RightRotate(const std::shared_ptr<Node>& lhs, const std::shared_ptr<Node>& mhs,
                          const std::shared_ptr<Node>& rhs, size_t r_node_index) {
    // Remove right element from lhs node
    std::shared_ptr<Item> l_item = lhs->ItemsPtr()->back();
    lhs->ItemsPtr()->pop_back();

    // Update parents(middle) element
    size_t parent_r_item = 0;
    if (r_node_index != 0) {
        parent_r_item = r_node_index - 1;
    } else {
        parent_r_item = 0;
    }
    std::shared_ptr<Item> m_item = mhs->ItemsPtr()->operator[](parent_r_item);
    mhs->ItemsPtr()->operator[](parent_r_item) = l_item;

    // Update right element
    rhs->ItemsPtr()->insert(rhs->ItemsPtr()->begin(), l_item);

    // Update leaves
    if (!lhs->IsLeaf()) {
        auto child_node_ptr = lhs->ChildNodesPtr()->back();
        lhs->ChildNodesPtr()->pop_back();

        rhs->ChildNodesPtr()->insert(rhs->ChildNodesPtr()->begin(), child_node_ptr);
    }
}

void Storage::Merge(const std::shared_ptr<Node>& parent, const std::shared_ptr<Node>& unbalanced,
                    size_t u_node_index) {
    uint64_t lhs_node_ptr = parent->ChildNodesPtr()->operator[](u_node_index - 1);
    auto lhs_node = GetNode(lhs_node_ptr);

    // Update parent. Move item from parent to left_node
    auto parent_item = parent->ItemsPtr()->operator[](u_node_index - 1);
    parent->ItemsPtr()->erase(parent->ItemsPtr()->begin() + u_node_index - 1);
    lhs_node->ItemsPtr()->emplace_back(parent_item);

    // Add unbalanced items to left node
    for (auto item : *unbalanced->ItemsPtr()) {
        lhs_node->ItemsPtr()->emplace_back(item);
        if (!lhs_node->IsLeaf()) {
            lhs_node->ItemsPtr()->emplace_back(item);    
        }
    }
    if (!lhs_node->IsLeaf()) {
        for (auto child_ptr: *unbalanced->ChildNodesPtr()) {
            lhs_node->ChildNodesPtr()->emplace_back(child_ptr);
        }
    }

    WriteNode(parent);
    WriteNode(lhs_node);
    DeleteNode(unbalanced->GetPageNum());
}

void Storage::RemoveAndRebalance(const std::shared_ptr<Node>& parent,
                              const std::shared_ptr<Node>& unbalanced, size_t u_node_index) {
    // Right rotate, if we can
    if (u_node_index != 0) {
        auto lhs_node = GetNode(parent->ChildNodesPtr()->operator[](u_node_index - 1));
        if (!IsUnderPopulated(lhs_node)) {
            RightRotate(lhs_node, parent, unbalanced, u_node_index);
            WriteNode(lhs_node);
            WriteNode(parent);
            WriteNode(unbalanced);

            return;
        }
    }

    // Left rotate, if we can
    if (u_node_index != parent->ChildNodesPtr()->size() - 1) {
        auto rhs_node = GetNode(parent->ChildNodesPtr()->operator[](u_node_index + 1));
        if (!IsUnderPopulated(rhs_node)) {
            LeftRotate(unbalanced, parent, rhs_node, u_node_index);
            WriteNode(rhs_node);
            WriteNode(parent);
            WriteNode(unbalanced);

            return;
        }
    }

    // Nothing worked. Merge
    if (u_node_index == 0) {
        auto rhs_node = GetNode(parent->ChildNodesPtr()->operator[](u_node_index + 1));
        Merge(parent, rhs_node, u_node_index);

        return;
    }
    Merge(parent, unbalanced, u_node_index);
}

void Storage::PushLog() {
    std::unique_lock lock(mutex_);

    const auto& logs = log_storage_.GetLogs();
    for (const auto& log : logs) {
        if (log.GetCommand() == Log::Command::PUT) {
            PutInTree(log.GetKey(), log.GetValue());
        } else (log.GetCommand() == Log::Command::REMOVE) {
            PutInTree(log.GetKey());
        }
    }
    log_storage_.Clear();
}

void Storage::PushLogAsync() {
    auto thread = std::thread([this]() mutable {
        PushLog();
    });
    // Thread is not joined, call is a non-blocking operation
}

Storage::~Storage() {
    PushLog();
}
