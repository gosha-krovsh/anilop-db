#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "memory/type.h"
#include <cstdint>

namespace settings {

extern const size_t kMaxPage;
extern const size_t kPageSize;

struct UserSettings {
    size_t max_log_size = 100;
    double min_fill_percent = 0.2;
    double max_fill_percent = 0.95;
};

}  // namespace settings

#endif  // SETTINGS_H_