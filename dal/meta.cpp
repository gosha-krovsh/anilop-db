#include "meta.h"

size_t Meta::Serialize(byte* data, size_t max_volume) {
    if (max_volume < 8) {

    }

    size_t size = free_list_.Serialize(data, max_volume);
    return size;
}

size_t Meta::Deserialize(byte* data, size_t max_volume) {
    size_t size = free_list_.Deserialize(data, max_volume);
    return size;
}

FreeList& Meta::FreeListPtr() {
    return free_list_;
}
