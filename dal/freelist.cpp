#include "freelist.h"

FreeList::FreeList() : max_page_(0), realeased_pages_() {}

size_t FreeList::Serialize(byte* data, size_t max_volume) {
    size_t size = realeased_pages_.size() * convert::uint64_t_size + 2 * convert::uint64_t_size;
    if (max_volume < size) {
        throw data_layer::InsufficientBufferSize("Max volume is too low for serialisation.");
    }

    convert::uint64_to_bytes(data, max_page_);
    data += convert::uint64_t_size;
    convert::uint64_to_bytes(data, realeased_pages_.size());
    data += convert::uint64_t_size;
    for (uint64_t page : realeased_pages_) {
        convert::uint64_to_bytes(data, page);
        data += convert::uint64_t_size;
    }
    return size;
}

size_t FreeList::Deserialize(byte* data, size_t max_volume) {
    size_t basic_size = 2 * sizeof(uint64_t);
    if (max_volume < basic_size) {
        throw data_layer::CorruptedBuffer("Buffer size is too low for deserialisation."); 
    }
    max_page_ = convert::bytes_to_uint64(data);
    data += convert::uint64_t_size;
    uint64_t released_pages_size = convert::bytes_to_uint64(data);
    data += convert::uint64_t_size;
    max_volume -= 2 * convert::uint64_t_size;
    
    if (released_pages_size > 0) {
        if (max_volume < released_pages_size * convert::uint64_t_size) {
            throw data_layer::CorruptedBuffer("Buffer size is too low for deserialisation."); 
        }
        realeased_pages_.resize(released_pages_size);
        for (size_t i = 0; i < released_pages_size; ++i) {
            realeased_pages_[i] = convert::bytes_to_uint64(data);
        }
    }
    return basic_size + released_pages_size * convert::uint64_t_size;
}

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
