#include "log.h"

#include "exception/exception.h"
#include "memory/memory.h"

#include <cassert>

Log::Log(Log::Command command, const std::vector<byte> &key, const std::vector<byte> &value)
    : command_(command), key_(key), value_(value) {
    assert(command_ == Command::PUT);
}

Log::Log(Log::Command command, const std::vector<byte> &key)
        : command_(command), key_(key) {
    assert(command_ == Command::REMOVE);
}

Log::Log() {
}

size_t Log::Serialize(byte *data, size_t max_volume) const {
    size_t o_size = 1 + 2 * uint64_t_size + key_.size() + value_.size();
    if (max_volume < o_size) {
        throw dal_error::InsufficientBufferSize("Max volume is too low for serialisation.");
    }

    *data = static_cast<byte>(command_);
    data += 1;
    memory::uint64_to_bytes(data, key_.size());
    data += uint64_t_size;
    memory::uint64_to_bytes(data, value_.size());
    data += uint64_t_size;

    std::memcpy(data, key_.data(), key_.size());
    data += key_.size();
    std::memcpy(data, value_.data(), value_.size());

    return o_size;
}

size_t Log::Deserialize(const byte *data, size_t max_volume) {
    size_t r_size = 1 + 2 * uint64_t_size;
    if (max_volume < r_size) {
        throw dal_error::CorruptedBuffer("Buffer size is too low for deserialization.");
    }
    max_volume -= r_size;

    command_ = static_cast<Command>(*data);
    data += 1;
    uint64_t key_size = memory::bytes_to_uint64(data);
    data += uint64_t_size;
    uint64_t value_size = memory::bytes_to_uint64(data);
    data += uint64_t_size;

    if (max_volume < key_size + value_size) {
        throw dal_error::CorruptedBuffer("Buffer size is too low for deserialization.");
    }

    key_.resize(key_size);
    value_.resize(value_size);

    std::memcpy(key_.data(), data, key_size);
    data += key_size;
    std::memcpy(value_.data(), data, value_size);

    return r_size + key_size + value_size;
}

Log::Command Log::GetCommand() const {
    return command_;
}

const std::vector<byte>& Log::GetKey() const {
    return key_;
}

const std::vector<byte>& Log::GetValue() const {
    return value_;
}

std::vector<byte>& Log::GetKey() {
    return key_;
}

std::vector<byte>& Log::GetValue() {
    return value_;
}

uint64_t Log::GetLogSize() const {
    return 2 + key_.size() + value_.size();
}

Log Log::readFromBuffer(byte *buffer, size_t max_size) {
    Log log;
    log.Deserialize(buffer, max_size);
    return log;
}
