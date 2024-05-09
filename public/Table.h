#ifndef ANILOP_DB_H_
#define ANILOP_DB_H_

#include <memory>
#include <vector>
#include <unordered_map>
#include <optional>
#include <shared_mutex>

#include "type.h"
#include "Transaction.h"
#include "Settings.h"

class Storage;

namespace AnilopDB {

    class Table {
    public:
        static std::shared_ptr<Table> Open(
                const std::string& path,
                const Settings &settings
        );

        Table(const Table&) = delete;
        Table(Table&&) = delete;
        void operator=(const Table&) = delete;
        void operator=(Table&&) = delete;

        std::string getPath() const;

        std::optional<Data> Find(const Data& key);
        void Put(const Data& key, const Data& data);
        void Remove(const Data& key);

        std::shared_ptr<Transaction> newReadTx();
        std::shared_ptr<Transaction> newWriteTx();

        void Close();

    private:
        static std::unordered_map<std::string, std::shared_ptr<Table>> table_map_;

        explicit Table(const std::string& path, const Settings &settings);

        bool is_closed_ = false;

        std::string path_;
        std::shared_ptr<Storage> storage_;
        std::shared_mutex tx_mutex_;
    };

}

#endif // ANILOP_DB_H_
