#include "meta.h"

size_t IMeta::Serialize(byte *data, size_t max_volume) const {
    auto magic_word = GetMagicWord();

    size_t o_size = magic_word.size() + 1;
    if (max_volume < o_size) {
        throw dal_error::CorruptedBuffer("Buffer is too low for serialization.");
    }
    std::memcpy(data, magic_word.c_str(), magic_word.size() + 1);
    return o_size;
}

size_t IMeta::Deserialize(const byte *data, size_t max_volume) {
    auto magic_word = GetMagicWord();

    size_t r_size = magic_word.size() + 1;
    if (max_volume < r_size) {
        throw dal_error::CorruptedBuffer("Buffer is too low for deserialization.");
    }
    std::string temp_magic_word(data);
    temp_magic_word.resize(magic_word.size());
    if (temp_magic_word != GetMagicWord()) {
        throw dal_error::FileError("Magic word doesn't match");
    }
    return r_size;
}

size_t Meta::Serialize(byte* data, size_t max_volume) const {
    size_t o_base_size = BaseT::Serialize(data, max_volume);
    data += o_base_size;

    size_t o_size = 3 * uint64_t_size;
    if (max_volume < o_size) {
        throw dal_error::CorruptedBuffer("Buffer is too low for serialization.");
    }

    memory::uint64_to_bytes(data, root_);
    data += uint64_t_size;
    memory::uint64_to_bytes(data, free_list_page_);

    return o_base_size + o_size;
}

size_t Meta::Deserialize(const byte* data, size_t max_volume) {
    size_t r_base_size = BaseT::Deserialize(data, max_volume);
    data += r_base_size;

    size_t r_size = 3 * uint64_t_size;
    if (max_volume < r_size) {
        throw dal_error::CorruptedBuffer("Buffer is too low for deserialization.");
    }

    root_ = memory::bytes_to_uint64(data);
    data += uint64_t_size;
    free_list_page_ = memory::bytes_to_uint64(data);

    return r_base_size + r_size;
}

uint64_t Meta::GetFreeListPage() { return free_list_page_; }

void Meta::SetFreeListPage(uint64_t page) {
    free_list_page_ = page;
}

uint64_t Meta::GetRootPage() { return root_; }

void Meta::SetRootPage(uint64_t page) {
    root_ = page;
}

size_t Meta::GetSize() const {
    auto base_size = BaseT::GetSize();
    return base_size + (3 * uint64_t_size);
}


size_t LogMeta::Serialize(byte *data, size_t max_volume) const {
    size_t o_base_size = BaseT::Serialize(data, max_volume);
    data += o_base_size;

    size_t o_size = uint64_t_size;
    if (max_volume < o_size) {
        throw dal_error::CorruptedBuffer("Buffer is too low for serialization.");
    }
    memory::uint64_to_bytes(data, data_end_offset_);

    return o_base_size + o_size;
}

size_t LogMeta::Deserialize(const byte *data, size_t max_volume) {
    size_t r_base_size = BaseT::Deserialize(data, max_volume);
    data += r_base_size;

    size_t r_size = uint64_t_size;
    if (max_volume < r_size) {
        throw dal_error::CorruptedBuffer("Buffer is too low for deserialization.");
    }
    data_end_offset_ = memory::bytes_to_uint64(data);

    return r_base_size + r_size;
}

size_t LogMeta::GetSize() const {
    auto base_size = BaseT::GetSize();
    return base_size + uint64_t_size;
}

size_t MemoryLogMeta::Serialize(byte *data, size_t max_volume) const {
    size_t o_base_size = BaseT::Serialize(data, max_volume);
    data += o_base_size;

    size_t o_size = 3 * uint64_t_size;
    if (max_volume < o_size) {
        throw dal_error::CorruptedBuffer("Buffer is too low for serialization.");
    }

    memory::uint64_to_bytes(data, dirty_page_);
    data += uint64_t_size;
    memory::uint64_to_bytes(data, allocated_page_);
    data += uint64_t_size;
    memory::uint64_to_bytes(data, data_page_);

    return o_base_size + o_size;
}

size_t MemoryLogMeta::Deserialize(const byte *data, size_t max_volume) {
    size_t r_base_size = BaseT::Deserialize(data, max_volume);
    data += r_base_size;

    size_t r_size = 3 * uint64_t_size;
    if (max_volume < r_size) {
        throw dal_error::CorruptedBuffer("Buffer is too low for deserialization.");
    }

    dirty_page_ = memory::bytes_to_uint64(data);
    data += uint64_t_size;
    allocated_page_ = memory::bytes_to_uint64(data);
    data += uint64_t_size;
    data_page_ = memory::bytes_to_uint64(data);

    return r_base_size + r_size;
}

size_t MemoryLogMeta::GetSize() const {
    auto base_size = BaseT::GetSize();
    return base_size + (3 * uint64_t_size);
}
