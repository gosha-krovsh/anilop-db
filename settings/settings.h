#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "memory/type.h"
#include <cstdint>

namespace settings {

extern size_t kMaxPage;

struct UserSettings {
    size_t page_size = 4096;
    size_t max_log_size = 100;
    double min_fill_percent = 0.2;
    double max_fill_percent = 0.95;
};

}  // namespace settings

#endif  // SETTINGS_H_