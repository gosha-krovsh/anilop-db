#include "TransactionImpl.h"

#include <ranges>
#include <unordered_set>

TransactionImpl::TransactionImpl(
        bool is_write,
        std::shared_ptr<Storage> storage,
        std::shared_mutex& mutex
)
: is_write_(is_write)
, mutex_(mutex)
, storage_(std::move(storage)) {
}

std::optional<Data> TransactionImpl::Find(const Data &key) {
    auto str_key = LogStorage::ConvertToStr(key);
    auto map_it = key_to_tx_memory_log_.find(str_key);
    if (map_it != key_to_tx_memory_log_.end()) {
        if (map_it->second.back()->GetCommand() != Log::Command::REMOVE)
            return std::nullopt;
        else
            return std::make_optional(map_it->second.back()->GetValue());
    }
    return storage_->Find(key);
}

void TransactionImpl::rollback() {
    if (!is_write_) {
        mutex_.unlock_shared();
        return;
    }

    memory_tx_log_.clear();
    key_to_tx_memory_log_.clear();
    mutex_.unlock();
}

void TransactionImpl::commit() {
    if (!is_write_) {
        mutex_.unlock_shared();
        return;
    }

    storage_->PushTransactionLogs(memory_tx_log_);

    memory_tx_log_.clear();
    key_to_tx_memory_log_.clear();
    mutex_.unlock();
}

void TransactionImpl::Put(const Data &key, const Data &data) {
    memory_tx_log_.emplace_back(Log::Command::PUT, key, data);

    auto str_key = LogStorage::ConvertToStr(key);
    key_to_tx_memory_log_[str_key].push_back(std::prev(memory_tx_log_.end()));
}

void TransactionImpl::Remove(const Data &key) {
    memory_tx_log_.emplace_back(Log::Command::REMOVE, key);

    auto str_key = LogStorage::ConvertToStr(key);
    key_to_tx_memory_log_[str_key].push_back(std::prev(memory_tx_log_.end()));
}
