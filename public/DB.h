#ifndef ANILOP_DB_H_
#define ANILOP_DB_H_

#include <memory>
#include <vector>
#include <unordered_map>
#include <optional>
#include <shared_mutex>

#include "type.h"
#include "Table.h"
#include "Transaction.h"
#include "Settings.h"


namespace AnilopDB {

    class DB {
    public:
        static std::shared_ptr<DB> Open(const std::unordered_map<std::string, std::string>& code_path_map,
                                        const Settings& settings);

        std::optional<Data> Find(const std::string& code, const Data &key);
        void Put(const std::string& code, const Data &key, const Data &data);
        void Remove(const std::string& code, const Data &key);

        std::optional<std::string> Find(const std::string& code, const std::string& key);
        void Put(const std::string& code, const std::string&key, const std::string& data);
        void Remove(const std::string& code, const std::string& key);

        std::shared_ptr<Transaction> newReadTx(const std::vector<std::string>& codes);
        std::shared_ptr<Transaction> newWriteTx(const std::vector<std::string>& codes);

    private:
        static std::shared_ptr<DB> db;
        explicit DB(std::unordered_map<std::string, std::string> code_path_map, const Settings &settings);

        std::vector<std::shared_ptr<Table>> getTxTables(const std::vector<std::string>& codes);

        std::unordered_map<std::string, std::shared_ptr<Table>> table_map_;
    };

}

#endif  // ANILOP_DB_H_
