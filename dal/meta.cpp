#include "meta.h"

size_t Meta::Serialize(byte* data, size_t max_volume) {
    size_t o_size = (magic_word.size() + 1 + 2 * uint64_t_size);
    if (max_volume < o_size) {
        throw data_layer::CorruptedBuffer("Buffer is too low for serialization.");
    }
    std::memcpy(data, magic_word.c_str(), magic_word.size() + 1);
    data += 8;
    convert::uint64_to_bytes(data, root_);
    data += uint64_t_size;
    convert::uint64_to_bytes(data, free_list_page_);

    return o_size;
}

size_t Meta::Deserialize(byte* data, size_t max_volume) {
    size_t r_size = (magic_word.size() + 1 + 2 * uint64_t_size);
    if (max_volume < r_size) {
        throw data_layer::CorruptedBuffer("Buffer is too low for deserialization.");
    }
    std::string temp_magic_word(data);
    if (temp_magic_word != magic_word) {
        throw data_layer::FileError("Magic word dosn't match");
    }
    data += 8;

    root_ = convert::bytes_to_uint64(data);
    data += uint64_t_size;
    free_list_page_ = convert::bytes_to_uint64(data);

    return r_size;
}

uint64_t Meta::GetFreeListPage() { return free_list_page_; }

void Meta::SetFreeListPage(uint64_t page) {
    free_list_page_ = page;
}

uint64_t Meta::GetRootPage() { return 0; }

void Meta::SetRootPage(uint64_t page) {}
