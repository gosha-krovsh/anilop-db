#include "Table.h"

#include "settings/settings.h"
#include "storage/storage.h"

using namespace AnilopDB;

void Table::Close() {
    if (is_closed_)
        throw std::runtime_error("Table is already closed!");

    is_closed_ = true;
}

Table::Table(const std::string& code, const std::string& path, const Settings &settings)
    : code_(code)
    , path_(path) {
    settings::UserSettings user_settings;
    user_settings.max_log_size = settings.max_log_size;

    storage_ = std::make_shared<Storage>(path, user_settings);
}

Table::~Table() {
    if (!is_closed_) {
        Close();
    }
}
