#ifndef LOG_STORAGE_H_
#define LOG_STORAGE_H_

#include <optional>
#include <unordered_map>
#include <list>
#include <cstring>
#include <memory>
#include <tuple>

#include "dal/dal.h"
#include "dal/log_dal.h"
#include "dal/log.h"
#include "memory/type.h"
#include "settings/settings.h"

class LogStorage {
public:
    LogStorage(std::shared_ptr<DAL> dal, std::shared_ptr<LogDAL> log_dal, const settings::UserSettings& settings);

    std::optional<std::vector<byte>> Find(const std::vector<byte>& key);
    bool Put(const std::vector<byte>& key, const std::vector<byte>& value);
    bool Remove(const std::vector<byte>& key);

    void PushTransactionLogs(const std::vector<Log>& logs);

    std::list<Log>& GetLogs();
    const std::list<Log>& GetLogs() const;
    size_t Size();

    void Clear();

    static std::string ConvertToStr(const std::vector<byte> &data);
    static std::vector<byte> ConvertFromStr(const std::string& data);

private:
    void WriteLog(const Log& log);
    void WriteLogToMemory(const Log& log);

    void WriteMeta();
    void ReadMeta();

    std::list<Log> memory_log_;
    std::unordered_map<std::string, std::vector<std::list<Log>::iterator>> key_to_memory_log_;

    settings::UserSettings settings_;
    std::shared_ptr<DAL> dal_;
    std::shared_ptr<LogDAL> log_dal_;
};

#endif  // LOG_STORAGE_H_
