#include "meta.h"

size_t IMeta::Serialize(byte *data, size_t max_volume) const {
    auto magic_word = GetMagicWord();

    size_t o_size = magic_word.size();
    if (max_volume < o_size) {
        throw dal_error::CorruptedBuffer("Buffer is too low for serialization.");
    }
    std::memcpy(data, magic_word.c_str(), magic_word.size() + 1);
    return o_size;
}

size_t IMeta::Deserialize(const byte *data, size_t max_volume) {
    auto magic_word = GetMagicWord();

    size_t r_size = magic_word.size();
    if (max_volume < r_size) {
        throw dal_error::CorruptedBuffer("Buffer is too low for deserialization.");
    }
    std::string temp_magic_word(data);
    if (temp_magic_word != GetMagicWord()) {
        throw dal_error::FileError("Magic word doesn't match");
    }
    return r_size;
}

size_t Meta::Serialize(byte* data, size_t max_volume) const {
    size_t o_base_size = BaseT::Serialize(data, max_volume);
    size_t o_size = 1 + 2 * uint64_t_size;
    if (max_volume < o_size) {
        throw dal_error::CorruptedBuffer("Buffer is too low for serialization.");
    }

    auto magic_word = GetMagicWord();
    std::memcpy(data, magic_word.c_str(), magic_word.size() + 1);
    data += 8;
    memory::uint64_to_bytes(data, root_);
    data += uint64_t_size;
    memory::uint64_to_bytes(data, free_list_page_);
    data += uint64_t_size;
    memory::uint64_to_bytes(data, page_size_);

    return o_base_size + o_size;
}

size_t Meta::Deserialize(const byte* data, size_t max_volume) {
    size_t r_base_size = BaseT::Deserialize(data, max_volume);
    size_t r_size = 1 + 2 * uint64_t_size;
    if (max_volume < r_size) {
        throw dal_error::CorruptedBuffer("Buffer is too low for deserialization.");
    }
    std::string temp_magic_word(data);
    if (temp_magic_word != GetMagicWord()) {
        throw dal_error::FileError("Magic word doesn't match");
    }
    data += 8;

    root_ = memory::bytes_to_uint64(data);
    data += uint64_t_size;
    free_list_page_ = memory::bytes_to_uint64(data);
    data += uint64_t_size;
    page_size_ = memory::bytes_to_uint64(data);

    return r_base_size + r_size;
}

uint64_t Meta::GetPageSize() {
    return page_size_;
}

void Meta::SetPageSize(uint64_t page_size) {
    page_size_ = page_size;
}

uint64_t Meta::GetFreeListPage() { return free_list_page_; }

void Meta::SetFreeListPage(uint64_t page) {
    free_list_page_ = page;
}

uint64_t Meta::GetRootPage() { return root_; }

void Meta::SetRootPage(uint64_t page) {
    root_ = page;
}
