#ifndef ANILOP_TRANSACTION_H_
#define ANILOP_TRANSACTION_H_

#include <memory>
#include <optional>
#include <shared_mutex>
#include <unordered_map>

#include "Table.h"
#include "type.h"

class Storage;
class TransactionImpl;

namespace AnilopDB {

    class Transaction {
    public:
        friend class DB;

        Transaction() = delete;

        Transaction(const Transaction &) = delete;
        Transaction(Transaction &&) = delete;

        Transaction &operator=(const Transaction &) = delete;
        Transaction &operator=(Transaction &&) = delete;

        std::optional<Data> Find(const std::string& code, const Data &key);
        void Put(const std::string& code, const Data &key, const Data &data);
        void Remove(const std::string& code, const Data &key);

        std::optional<std::string> Find(const std::string& code, const std::string& key);
        void Put(const std::string& code, const std::string& key, const std::string& data);
        void Remove(const std::string& code, const std::string& key);

        void commit();

        void rollback();

        ~Transaction();

    private:
        Transaction(bool is_write, const std::vector<std::shared_ptr<Table>>& tables);

        std::unordered_map<std::string, TransactionImpl*> code_impl_;
        // Order is important
        std::vector<TransactionImpl*> impls_;
    };
}

#endif // ANILOP_TRANSACTION_H_
