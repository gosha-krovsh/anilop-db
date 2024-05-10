#include "num_list.h"

size_t NumList::Serialize(byte *data, size_t max_volume) const {
    size_t o_size = GetByteLength();
    if (max_volume < o_size) {
        throw dal_error::InsufficientBufferSize("Max volume is too low for serialisation.");
    }

    memory::uint64_to_bytes(data, num_data_.size());
    data += uint64_t_size;
    for (uint64_t page : num_data_) {
        memory::uint64_to_bytes(data, page);
        data += uint64_t_size;
    }
    return o_size;
}

size_t NumList::Deserialize(const byte *data, size_t max_volume) {
    size_t r_size = sizeof(uint64_t);
    if (max_volume < r_size) {
        throw dal_error::CorruptedBuffer("Buffer size is too low for deserialization.");
    }
    max_volume -= r_size;

    size_t size = memory::bytes_to_uint64(data);
    data += uint64_t_size;

    if (size > 0) {
        if (max_volume < size * uint64_t_size) {
            throw dal_error::CorruptedBuffer("Buffer size is too low for deserialization.");
        }

        num_data_.resize(size);
        for (size_t i = 0; i < size; ++i) {
            num_data_[i] = memory::bytes_to_uint64(data);
            data += uint64_t_size;
        }
    }
    return r_size + size * uint64_t_size;
}

std::vector<uint64_t> *NumList::GetDataPtr() {
    return &num_data_;
}

const std::vector<uint64_t> *NumList::GetDataPtr() const {
    return &num_data_;
}

void NumList::Clear() {
    num_data_.clear();
}

size_t NumList::GetByteLength() const {
    return (num_data_.size() + 1) * uint64_t_size;
}
