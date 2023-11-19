#include "freelist.h"

FreeList::FreeList() : max_page_(0), realeased_pages_() {}

uint64_t FreeList::GetMaxPage() { 
    return max_page_; 
}

void FreeList::SetMaxPage(uint64_t max_page) {
    max_page_ = max_page;
}

uint64_t FreeList::GetNextPage() {
    if (realeased_pages_.size() > 0) {
        uint64_t page_num = realeased_pages_.back();
        realeased_pages_.pop_back();

        return page_num;
    }

    ++max_page_;
    return max_page_; 
}

void FreeList::RealeasePage(uint64_t page_num) {
    realeased_pages_.push_back(page_num);
}
