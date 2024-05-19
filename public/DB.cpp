#include "DB.h"

using namespace AnilopDB;

std::shared_ptr<DB> DB::db;

std::shared_ptr<DB> DB::Open(const std::unordered_map<std::string, std::string>& code_path_map,
                             const AnilopDB::Settings &settings) {
    if (!db) {
        db = std::shared_ptr<DB>(new DB());
    }

    for (const auto& [code, path]: code_path_map) {
        if (db->table_map_.contains(code))
            continue;

        db->table_map_[code] = std::shared_ptr<Table>(new Table(code, path, settings));
    }
    return db;
}

std::shared_ptr<Transaction> DB::newReadTx(const std::vector<std::string>& codes) {
    auto tx_tables = getTxTables(codes);
    for (auto table : tx_tables) {
        table->tx_mutex_.lock_shared();
    }

    return std::shared_ptr<Transaction>(new Transaction(false , tx_tables));
}

std::shared_ptr<Transaction> DB::newWriteTx(const std::vector<std::string>& codes) {
    auto tx_tables = getTxTables(codes);
    for (auto table : tx_tables) {
        table->tx_mutex_.lock();
    }
    return std::shared_ptr<Transaction>(new Transaction(true, tx_tables));
}

std::vector<std::shared_ptr<Table>> DB::getTxTables(const std::vector<std::string> &codes) {
    std::vector<std::shared_ptr<Table>> result;
    for (const auto& code : codes) {
        if (!table_map_.contains(code)) {
            throw std::runtime_error("Invalid table code.");
        }
        result.push_back(table_map_[code]);
    }
    return result;
}

std::optional<Data> DB::Find(const std::string &code, const Data &key) {
    auto tx = newReadTx({ code });
    auto result = tx->Find(code, key);
    tx->commit();
    return result;
}

void DB::Put(const std::string &code, const Data &key, const Data &data) {
    auto tx = newWriteTx({ code });
    tx->Put(code, key, data);
    tx->commit();
}

void DB::Remove(const std::string &code, const Data &key) {
    auto tx = newWriteTx({ code });
    tx->Remove(code, key);
    tx->commit();
}

std::optional<std::string> DB::Find(const std::string &code, const std::string &key) {
    auto data = AnilopDB::StringToData(key);
    auto data_opt = Find(code, data);
    if (data_opt.has_value()) {
        return AnilopDB::DataToString(data_opt.value());
    }
    return std::nullopt;
}

void DB::Put(const std::string &code, const std::string &key, const std::string &data) {
    Put(code, AnilopDB::StringToData(key), AnilopDB::StringToData(data));
}

void DB::Remove(const std::string &code, const std::string &key) {
    Remove(code, AnilopDB::StringToData(key));
}

void DB::Close() {
    for (auto [_, table] : table_map_) {
        table->Close();
    }
    table_map_.clear();
}
