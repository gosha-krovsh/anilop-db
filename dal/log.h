#ifndef LOG_H
#define LOG_H

#include <vector>
#include <cstring>

#include "dal/serializable.h"
#include "memory/type.h"

class Log : public ISerializable {
public:
    enum class Command : byte {
        PUT,
        REMOVE
    };

    Log(Command command, const std::vector<byte>& key, const std::vector<byte>& value);
    Log(Command command, const std::vector<byte>& key);

    static Log readFromBuffer(byte* buffer, size_t max_size);

    size_t Serialize(byte* data, size_t max_volume) const override;
    size_t Deserialize(const byte* data, size_t max_volume) override;

    uint64_t GetLogSize() const;

    Command GetCommand() const;

    std::vector<byte>& GetKey();
    std::vector<byte>& GetValue();
    const std::vector<byte>& GetKey() const;
    const std::vector<byte>& GetValue() const;

private:
    Log();

    Command command_;
    std::vector<byte> key_;
    std::vector<byte> value_;
};

#endif //LOG_H
