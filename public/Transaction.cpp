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

void Transaction::commit() {
    impl_->commit();
}

void Transaction::rollback() {
    impl_->rollback();
}

Transaction::~Transaction() {
    delete impl_;
}
