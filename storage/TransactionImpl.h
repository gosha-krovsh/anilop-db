#ifndef ANILOP_TRANSACTIONIMPL_H
#define ANILOP_TRANSACTIONIMPL_H

#include <optional>
#include <shared_mutex>

#include "memory/type.h"
#include "storage/storage.h"


class TransactionImpl {
public:
    TransactionImpl(
            bool is_write,
            std::shared_ptr<Storage> storage_,
            std::shared_mutex& mutex
    );

    std::optional<Data> Find(const Data &key);

    void Put(const Data &key, const Data &data);

    void Remove(const Data &key);

    void commit();

    void rollback();

private:
    bool is_write_;
    std::shared_mutex& mutex_;

    std::shared_ptr<Storage> storage_;

    std::vector<Log> memory_tx_log_;
    std::unordered_map<std::string, std::vector<std::vector<Log>::iterator>> key_to_tx_memory_log_;
};


#endif //ANILOP_TRANSACTIONIMPL_H
