#include "freelist.h"

#include <numeric>

FreeList::FreeList() : FreeList(0) {}

FreeList::FreeList(uint64_t max_page)
: max_page_(max_page)
, current_max_page_(0)
, released_pages_() {}

size_t FreeList::Serialize(byte* data, size_t max_volume) const {
    size_t o_size = released_pages_.size() * uint64_t_size + 3 * uint64_t_size;
    if (max_volume < o_size) {
        throw dal_error::InsufficientBufferSize("Max volume is too low for serialisation.");
    }

    memory::uint64_to_bytes(data, max_page_);
    data += uint64_t_size;
    memory::uint64_to_bytes(data, current_max_page_);
    data += uint64_t_size;
    memory::uint64_to_bytes(data, released_pages_.size());
    data += uint64_t_size;
    for (uint64_t page : released_pages_) {
        memory::uint64_to_bytes(data, page);
        data += uint64_t_size;
    }
    return o_size;
}

size_t FreeList::Deserialize(const byte* data, size_t max_volume) {
    size_t r_size = 3 * sizeof(uint64_t);
    if (max_volume < r_size) {
        throw dal_error::CorruptedBuffer("Buffer size is too low for deserialization.");
    }
    max_volume -= r_size;

    max_page_ = memory::bytes_to_uint64(data);
    data += uint64_t_size;
    current_max_page_ = memory::bytes_to_uint64(data);
    data += uint64_t_size;
    uint64_t released_pages_size = memory::bytes_to_uint64(data);
    data += uint64_t_size;
    
    if (released_pages_size > 0) {
        if (max_volume < released_pages_size * uint64_t_size) {
            throw dal_error::CorruptedBuffer("Buffer size is too low for deserialization.");
        }

        released_pages_.resize(released_pages_size);
        for (size_t i = 0; i < released_pages_size; ++i) {
            released_pages_[i] = memory::bytes_to_uint64(data);
        }
    }
    return r_size + released_pages_size * uint64_t_size;
}

uint64_t FreeList::GetNextPage() {
    if (!released_pages_.empty()) {
        uint64_t page_num = released_pages_.back();
        released_pages_.pop_back();

        return page_num;
    } else if (current_max_page_ == max_page_) {
        throw dal_error::FileError("Low memory. Unable to allocate more data");
    }

    ++current_max_page_;
    return current_max_page_;
}

void FreeList::ReleasePage(uint64_t page_num) {
    released_pages_.push_back(page_num);
}

void FreeList::ReleaseAllPages(uint64_t start_page_num) {
    size_t new_size = max_page_ - start_page_num;
    released_pages_.resize(new_size);

    std::iota(released_pages_.begin(), released_pages_.end(), start_page_num);
}

bool FreeList::HasFreePages() {
    return current_max_page_ < max_page_ || !released_pages_.empty();
}
