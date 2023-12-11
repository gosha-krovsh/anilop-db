#include "storage.h"

Storage::Storage(const std::string& path,
                 const settings::UserSettings& settings)
    : dal_(new DAL(path, settings)), root_(dal_->GetMetaPtr()->GetRootPage()) {
}

std::shared_ptr<Node> Storage::GetNode(uint64_t page_num) {
    std::shared_ptr<Page> page = dal_->ReadPage(page_num);
    std::shared_ptr<Node> node(new Node());

    node->SetPageNum(page_num);
    node->Deserialize(page->Data(), dal_->GetMetaPtr()->GetPageSize());
    return node;
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

std::tuple<std::shared_ptr<Node>, size_t> Storage::FindKey(
    const std::vector<byte>& key) {
    std::shared_ptr<Node> root_node = GetNode(root_);
    return FindKeyRecursive(root_node, key);
}

std::tuple<std::shared_ptr<Node>, size_t> Storage::FindKeyRecursive(
    const std::shared_ptr<Node>& node, const std::vector<byte>& key) {
    auto [index, was_found] = FindKeyInNode(node, key);
    if (was_found) {
        return std::tie(node, index);
    }

    if (node->isLeaf()) {
        return std::forward_as_tuple(std::nullptr_t(), -1);
    }

    uint64_t child_page_num = (*node->ChildNodesPtr())[index];
    std::shared_ptr<Node> child_node = GetNode(child_page_num);
    return FindKeyRecursive(child_node, key);
}

std::tuple<size_t, bool> Storage::FindKeyInNode(
    const std::shared_ptr<Node>& node, const std::vector<byte>& key) {
    for (int i = 0; i < node->ItemsPtr()->size(); ++i) {
        std::shared_ptr<Item> item = (*node->ItemsPtr())[i];
        int comp_result = std::memcmp(item->KeyData(), key.data(),
                                      std::min(key.size(), item->KeySize()));
        if (comp_result == 0) {
            return std::forward_as_tuple(i, true);
        }

        if (comp_result > 0) {
            return std::forward_as_tuple(i, false);
        }
    }
    return std::forward_as_tuple(node->ItemsPtr()->size(), false);
}
