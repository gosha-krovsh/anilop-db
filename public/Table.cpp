#include "Table.h"

#include "settings/settings.h"
#include "storage/storage.h"

using namespace AnilopDB;

std::unordered_map<std::string, std::shared_ptr<Table>> Table::table_map_;

std::shared_ptr<Table> Table::Open(
        const std::string &path,
        const Settings &settings
) {
    return std::shared_ptr<Table>(new Table(path, settings));
}

void Table::Close() {
    if (is_closed_)
        throw std::runtime_error("Table is already closed!");

    is_closed_ = true;
    table_map_.erase(path_);
}

std::string Table::getPath() const {
    return path_;
}

std::optional<Data> Table::Find(const Data &key) {
    auto tx = newReadTx();
    auto result = tx->Find(key);
    tx->commit();
    return result;
}

void Table::Put(const Data &key, const Data &data) {
    auto tx = newWriteTx();
    tx->Put(key, data);
    tx->commit();
}

void Table::Remove(const Data &key) {
    auto tx = newWriteTx();
    tx->Remove(key);
    tx->commit();
}

std::optional<std::string> Table::Find(const std::string& key) {
    auto data = AnilopDB::StringToData(key);
    auto data_opt = Find(data);
    if (data_opt.has_value()) {
        return AnilopDB::DataToString(data_opt.value());
    }
    return std::nullopt;
}

void Table::Put(const std::string &key, const std::string &data) {
    Put(AnilopDB::StringToData(key), AnilopDB::StringToData(data));
}

void Table::Remove(const std::string &key) {
    Remove(AnilopDB::StringToData(key));
}

Table::Table(const std::string& path, const Settings &settings)
    : path_(path) {
    settings::UserSettings user_settings;
    user_settings.max_log_size = settings.max_log_size;

    storage_ = std::make_shared<Storage>(path, user_settings);
}

std::shared_ptr<Transaction> Table::newReadTx() {
    tx_mutex_.lock_shared();
    return std::shared_ptr<Transaction>(new Transaction(false, storage_, tx_mutex_));
}

std::shared_ptr<Transaction> Table::newWriteTx() {
    tx_mutex_.lock();
    return std::shared_ptr<Transaction>(new Transaction(true, storage_, tx_mutex_));
}
