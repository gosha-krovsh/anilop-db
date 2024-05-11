#include "Transaction.h"

#include "storage/TransactionImpl.h"

using namespace AnilopDB;

Transaction::Transaction(
        bool is_write,
        std::shared_ptr<Storage> storage,
        std::shared_mutex &mutex)
        : impl_(new TransactionImpl (is_write,
                                     std::move(storage),
                                     mutex))
{}

std::optional<Data> Transaction::Find(const Data& key) {
    return impl_->Find(key);
}

void Transaction::Put(const Data &key, const Data &data) {
    impl_->Put(key, data);
}

void Transaction::Remove(const Data &key) {
    impl_->Remove(key);
}

std::optional<std::string> Transaction::Find(const std::string& key) {
    auto data = AnilopDB::StringToData(key);
    auto data_opt = Find(data);
    if (data_opt.has_value()) {
        return AnilopDB::DataToString(data_opt.value());
    }
    return std::nullopt;
}

void Transaction::Put(const std::string &key, const std::string &data) {
    Put(AnilopDB::StringToData(key), AnilopDB::StringToData(data));
}

void Transaction::Remove(const std::string &key) {
    Remove(AnilopDB::StringToData(key));
}

void Transaction::commit() {
    impl_->commit();
}

void Transaction::rollback() {
    impl_->rollback();
}

Transaction::~Transaction() {
    delete impl_;
}
