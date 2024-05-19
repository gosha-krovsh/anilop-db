#ifndef ANILOP_TABLE_H_
#define ANILOP_TABLE_H_

#include <memory>
#include <vector>
#include <unordered_map>
#include <optional>
#include <shared_mutex>

#include "type.h"
#include "Settings.h"

class Storage;

namespace AnilopDB {

    class Table {
        friend class DB;
        friend class Transaction;

    public:
        Table(const Table&) = delete;
        Table(Table&&) = delete;
        void operator=(const Table&) = delete;
        void operator=(Table&&) = delete;

        ~Table();

        void Close();

    private:
        explicit Table(const std::string& code, const std::string& path, const Settings &settings);

        bool is_closed_ = false;

        std::string code_;
        std::string path_;
        std::shared_ptr<Storage> storage_;
        std::shared_mutex tx_mutex_;
    };

}

#endif // ANILOP_TABLE_H_
