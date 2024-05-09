#include "log_storage.h"

LogStorage::LogStorage(std::shared_ptr<DAL> dal, std::shared_ptr<LogDAL> log_dal, const settings::UserSettings &settings)
    : settings_(settings), dal_(std::move(dal)), log_dal_(std::move(log_dal)) {
    auto log_buffer = log_dal_->ReadLogBuffer();
    auto log_buffer_ptr = log_buffer.data();
    uint64_t max_buffer_size = log_buffer.size();
    while (!log_buffer.empty()) {
        auto log = Log::readFromBuffer(log_buffer_ptr, max_buffer_size);
        log_buffer_ptr += log.GetLogSize();
        max_buffer_size -= log.GetLogSize();

        WriteLog(log);
    }
}

std::optional<std::vector<byte>> LogStorage::Find(const std::vector<byte> &key) {
    auto str_key = ConvertToStr(key);
    auto map_it = key_to_memory_log_.find(str_key);
    if (map_it != key_to_memory_log_.end()) {
        if (map_it->second.back()->GetCommand() != Log::Command::REMOVE)
            return std::make_optional(map_it->second.back()->GetValue());
    }
    return std::nullopt;
}

bool LogStorage::Put(const std::vector<byte>& key, const std::vector<byte>& value) {
    if (!dal_->CanWrite() || memory_log_.size() >= settings_.max_log_size) {
        return false;
    }
    Log log(Log::Command::PUT, key, value);
    auto str_key = ConvertToStr(key);

    WriteLog(log);
    return true;
}

bool LogStorage::Remove(const std::vector<byte> &key) {
    Log log(Log::Command::REMOVE, key);
    auto str_key = ConvertToStr(key);

    WriteLog(log);
    return true;
}

size_t LogStorage::Size() {
    return memory_log_.size();
}

std::string LogStorage::ConvertToStr(const std::vector<byte> &data) {
    return { data.begin(), data.end() };
}

std::vector<byte> LogStorage::ConvertFromStr(const std::string &data) {
    if (data.empty()) {
        return {};
    }
    return { data.begin(), std::prev(data.end()) };
}

std::vector<Log> &LogStorage::GetLogs() {
    return memory_log_;
}

const std::vector<Log> &LogStorage::GetLogs() const {
    return memory_log_;
}

void LogStorage::Clear() {
    memory_log_.clear();
    key_to_memory_log_.clear();

    log_dal_->ClearLogs();
}

void LogStorage::PushTransactionLogs(const std::vector<Log> &logs) {
    for (const auto& log : logs)
        WriteLog(log);

    WriteLog({ Log::Command::COMMIT });
}

void LogStorage::WriteLog(const Log &log) {
    log_dal_->WriteLog(log);
    memory_log_.push_back(log);

    if (log.GetCommand() == Log::Command::COMMIT)
        return;

    auto str_key = ConvertToStr(log.GetKey());
    key_to_memory_log_[str_key].push_back(--memory_log_.end());
}
