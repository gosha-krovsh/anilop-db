#include "Transaction.h"

#include "storage/TransactionImpl.h"

using namespace AnilopDB;

Transaction::Transaction(bool is_write,
                         const std::vector<std::shared_ptr<Table>>& tables) {
    for (const auto& table : tables) {
        impls_.push_back(new TransactionImpl(is_write, table->storage_, table->tx_mutex_));
        code_impl_[table->code_] = impls_.back();
    }
}

std::optional<Data> Transaction::Find(const std::string& code, const Data& key) {
    if (!code_impl_.contains(code))
        throw std::runtime_error("Invalid table code.");

    return code_impl_[code]->Find(key);
}

void Transaction::Put(const std::string& code, const Data &key, const Data &data) {
    if (!code_impl_.contains(code))
        throw std::runtime_error("Invalid table code.");

    code_impl_[code]->Put(key, data);
}

void Transaction::Remove(const std::string& code, const Data &key) {
    if (!code_impl_.contains(code))
        throw std::runtime_error("Invalid table code.");

    code_impl_[code]->Remove(key);
}

std::optional<std::string> Transaction::Find(const std::string& code, const std::string& key) {
    auto data = AnilopDB::StringToData(key);
    auto data_opt = Find(code, data);
    if (data_opt.has_value()) {
        return AnilopDB::DataToString(data_opt.value());
    }
    return std::nullopt;
}

void Transaction::Put(const std::string& code, const std::string &key, const std::string &data) {
    Put(code, AnilopDB::StringToData(key), AnilopDB::StringToData(data));
}

void Transaction::Remove(const std::string& code, const std::string &key) {
    Remove(code, AnilopDB::StringToData(key));
}

void Transaction::commit() {
    for (auto impl : impls_) {
        impl->commit();
    }
}

void Transaction::rollback() {
    for (auto impl : impls_) {
        impl->rollback();
    }
}

Transaction::~Transaction() {
    for (auto impl : impls_) {
        delete impl;
    }
}
