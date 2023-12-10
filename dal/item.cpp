#include "item.h"

Item::Item() : key_(), value_() {}

Item::Item(std::vector<byte> key, std::vector<byte> value)
    : key_(key), value_(value) {}

size_t Item::ByteLength() {
    return 2 * uint64_t_size + key_.size() + value_.size();
}

size_t Item::Serialize(byte* data, size_t max_volume) {
    size_t o_size = 2 * uint64_t_size + key_.size() + value_.size();
    if (max_volume < o_size) {
        throw dal_error::CorruptedBuffer("Buffer size is too low for serialisation.");
    }

    memory::uint64_to_bytes(data, key_.size());
    data += uint64_t_size;
    memory::uint64_to_bytes(data, value_.size());
    data += uint64_t_size;

    std::memcpy(data, key_.data(), key_.size());
    data += key_.size();
    std::memcpy(data, value_.data(), value_.size());
    return o_size;
}

size_t Item::Deserialize(const byte* data, size_t max_volume) {
    size_t basic_r_size = 2 * uint64_t_size;
    if (max_volume < basic_r_size) {
        throw dal_error::CorruptedBuffer(
            "Buffer size is too low for deserialisation.");
    }

    uint64_t key_size = memory::bytes_to_uint64(data);
    data += uint64_t_size;
    uint64_t value_size = memory::bytes_to_uint64(data);
    data += uint64_t_size;
    max_volume -= 2 * uint64_t_size;

    size_t r_size = key_size + value_size;
    if (max_volume < r_size) {
        throw dal_error::CorruptedBuffer(
            "Buffer size is too low for deserialisation.");
    }
    key_.reserve(key_size);
    value_.reserve(value_size);

    std::memcpy(key_.data(), data, key_size);
    data += key_size;
    std::memcpy(value_.data(), data, value_size);
    return basic_r_size + r_size;
}
