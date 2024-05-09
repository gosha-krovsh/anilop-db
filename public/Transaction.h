#ifndef ANILOP_TRANSACTION_H_
#define ANILOP_TRANSACTION_H_

#include <memory>
#include <optional>
#include <shared_mutex>

#include "type.h"

class Storage;
class TransactionImpl;

namespace AnilopDB {
    class Table;

    class Transaction {
    public:
        friend class Table;

        Transaction(const Transaction &) = delete;

        Transaction(Transaction &&) = delete;

        Transaction &operator=(const Transaction &) = delete;

        Transaction &operator=(Transaction &&) = delete;

        std::optional<Data> Find(const Data &key);

        void Put(const Data &key, const Data &data);

        void Remove(const Data &key);

        void commit();

        void rollback();

        ~Transaction();

    private:
        Transaction(bool is_write, std::shared_ptr<Storage> storage, std::shared_mutex &mutex);

        TransactionImpl* impl_;
    };
}

#endif // ANILOP_TRANSACTION_H_
