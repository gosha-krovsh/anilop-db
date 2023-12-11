#include "storage.h"

Storage::Storage(const std::string& path) : dal_(new DAL(path)), root_(dal_->GetMetaPtr()->GetRootPage()) {
    if (root_ == -1) {
        // This means, that current table is new
        std::shared_ptr<Node> node(new Node());
        
        WriteNode(node);
        root_ = node->GetPageNum();
    }
}

std::shared_ptr<Node> Storage::GetNode(uint64_t page_num) {
    std::shared_ptr<Page> page = dal_->ReadPage(page_num);
    std::shared_ptr<Node> node(new Node());

    node->SetPageNum(page_num);
    node->Deserialize(page->Data(), settings::kPageSize);
    return node;
}

void Storage::WriteNode(const std::shared_ptr<Node>& node) {
    std::shared_ptr<Page> page = dal_->AllocateEmptyPage();
    if (node->GetPageNum() == 0) {
        uint64_t new_page_num = dal_->GetNextPage();
        node->SetPageNum(new_page_num);
    }
    page->SetPageNum(node->GetPageNum());

    node->Serialize(page->Data(), settings::kPageSize);
}

void Storage::DeleteNode(uint64_t page_num) {
    dal_->ReleasePage(page_num);
}
