#include "meta.h"

size_t Meta::Serialize(byte* data, size_t max_volume) {
    size_t o_size = (magic_word.size() + 1 + 2 * uint64_t_size);
    if (max_volume < o_size) {
        throw dal_error::CorruptedBuffer("Buffer is too low for serialization.");
    }
    std::memcpy(data, magic_word.c_str(), magic_word.size() + 1);
    data += 8;
    memory::uint64_to_bytes(data, root_);
    data += uint64_t_size;
    memory::uint64_to_bytes(data, free_list_page_);

    return o_size;
}

size_t Meta::Deserialize(const byte* data, size_t max_volume) {
    size_t r_size = (magic_word.size() + 1 + 2 * uint64_t_size);
    if (max_volume < r_size) {
        throw dal_error::CorruptedBuffer("Buffer is too low for deserialization.");
    }
    std::string temp_magic_word(data);
    if (temp_magic_word != magic_word) {
        throw dal_error::FileError("Magic word dosn't match");
    }
    data += 8;

    root_ = memory::bytes_to_uint64(data);
    data += uint64_t_size;
    free_list_page_ = memory::bytes_to_uint64(data);

    return r_size;
}

uint64_t Meta::GetFreeListPage() { return free_list_page_; }

void Meta::SetFreeListPage(uint64_t page) {
    free_list_page_ = page;
}

uint64_t Meta::GetRootPage() { return root_; }

void Meta::SetRootPage(uint64_t page) {}
