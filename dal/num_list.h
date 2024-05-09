#ifndef ANILOP_NUM_LIST_H
#define ANILOP_NUM_LIST_H


#include <cstdint>
#include <vector>

#include "serializable.h"

#include "memory/type.h"
#include "memory/memory.h"
#include "exception/exception.h"

class NumList : public ISerializable {
public:
    size_t Serialize(byte* data, size_t max_volume) const override;
    size_t Deserialize(const byte* data, size_t max_volume) override;

    std::vector<uint64_t>* GetDataPtr();
    const std::vector<uint64_t>* GetDataPtr() const;

    void Clear();

private:
    std::vector<uint64_t> num_data_;
};


#endif //ANILOP_NUM_LIST_H
