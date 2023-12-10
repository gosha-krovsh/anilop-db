#include "page.h"

Page::Page(const std::vector<byte>& data)
    : data_(std::move(data)) {
    if (data_.size() > settings::kPageSize) {
        throw dal_error::LowPageVolume("Page size is not big enough to store data.");
    }
    data_.resize(settings::kPageSize);
}

Page::Page() : data_(settings::kPageSize) {}

void Page::SetPageNum(uint64_t page_num) { 
    page_num_ = page_num;
}

uint64_t Page::GetPageNum() const { 
    return page_num_; 
}

byte* Page::Data() { 
    return data_.data(); 
}

const byte* Page::Data() const { 
    return data_.data(); 
}
