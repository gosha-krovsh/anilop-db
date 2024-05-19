#ifndef ANILOP_LOG_DAL_H
#define ANILOP_LOG_DAL_H

#include <fstream>
#include <filesystem>
#include <cstdint>
#include <memory>
#include <string>
#include <mutex>

#include "log.h"
#include "page.h"
#include "meta.h"

#include "memory/type.h"
#include "settings/settings.h"
#include "exception/exception.h"

class LogDAL {
public:
    LogDAL(const std::string &path, const settings::UserSettings &user_settings);

    std::shared_ptr<LogMeta> GetMetaPtr();

    std::vector<byte> ReadLogBuffer();
    void WriteLog(const Log &log);

    void ClearLogs();
    void ClearLatest(uint64_t offset_to_end);
    void Close();

    ~LogDAL();

private:
    void WriteMeta();
    void ReadMeta();

    std::string path_;
    std::fstream file_;

    const uint64_t meta_offset_ = 0;
    std::shared_ptr<LogMeta> meta_;

    std::recursive_mutex mutex_;
};


#endif //ANILOP_LOG_DAL_H
