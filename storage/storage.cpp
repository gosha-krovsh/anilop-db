#include "storage.h"

Storage::Storage(const std::string& path, const settings::UserSettings& settings)
    : settings_(settings),
      dal_(new DAL(path, settings)),
      root_(dal_->GetMetaPtr()->GetRootPage()) {}

std::tuple<std::vector<byte>, bool> Storage::Find(const std::vector<byte>& key) {
    if (root_ == 0) {
        return std::forward_as_tuple(std::vector<byte>(), false);
    } else {
        auto [node, index, _] = FindKey(key, true);
        if (node == std::nullptr_t()) {
            return std::forward_as_tuple(std::vector<byte>(), false);
        } else {
            return std::forward_as_tuple(node->ItemsPtr()->operator[](index)->GetValue(), false);
        }
    }
}

void Storage::Put(const std::vector<byte> key, const std::vector<byte> value) {
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

    // Get all ancestores
    auto ancestors = GetNodes(ancestor_indices);

    // Split nodes, except root, if necessary
    for (uint64_t i = ancestors.size() - 2; i >= 0; --i) {
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

    if (node->isLeaf()) {
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
    for (int i = 0; i < node->ItemsPtr()->size(); ++i) {
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

uint64_t Storage::GetSplitIndex(const std::shared_ptr<Node>& node) {
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
    uint64_t split_index = GetSplitIndex(child);
    if (split_index == -1) {
        throw storage_error::InsertFailure(
            "Insert failed. Split called on lonely/underpopulated node.");
    }

    std::shared_ptr<Item> middle_item = child->ItemsPtr()->operator[](split_index);
    std::shared_ptr<Node> new_node = std::shared_ptr<Node>(new Node());

    if (child->isLeaf()) {
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
