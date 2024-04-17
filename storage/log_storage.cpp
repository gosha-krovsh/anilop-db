#include "log_storage.h"

LogStorage::LogStorage(std::shared_ptr<DAL> dal, const settings::UserSettings &settings)
    : settings_(settings), dal_(std::move(dal)) {
    auto log_buffer = dal_->ReadLogBuffer();
    auto log_buffer_ptr = log_buffer.data();
    uint64_t max_buffer_size = log_buffer.size();
    while (!log_buffer.empty()) {
        auto log = Log::readFromBuffer(log_buffer_ptr, max_buffer_size);
        log_buffer_ptr += log.GetLogSize();
        max_buffer_size -= log.GetLogSize();

        auto str_key = ConvertToStr(log.GetKey());
        memory_log_.push_back(std::move(log));
        key_to_memory_log_.emplace(str_key, --memory_log_.end());
    }
}

std::optional<std::vector<byte>> LogStorage::Find(const std::vector<byte> &key) {
    auto str_key = ConvertToStr(key);
    auto map_it = key_to_memory_log_.find(str_key);
    if (map_it != key_to_memory_log_.end()) {
        return std::make_optional(map_it->second->GetValue());
    }
    return std::nullopt;
}

bool LogStorage::Put(const std::vector<byte>& key, const std::vector<byte>& value) {
    if (!dal_->canWriteLog() || memory_log_.size() >= settings_.max_log_size) {
        return false;
    }
    Log log(Log::Command::PUT, key, value);
    auto str_key = ConvertToStr(key);

    dal_->WriteLog(log);
    memory_log_.emplace_back(std::move(log));
    key_to_memory_log_.emplace(str_key, --memory_log_.end());

    return true;
}

bool LogStorage::Remove(const std::vector<byte> &key) {
    Log log(Log::Command::REMOVE, key);
    auto str_key = ConvertToStr(key);

    dal_->WriteLog(log);
    memory_log_.emplace_back(std::move(log));
    key_to_memory_log_.emplace(str_key, --memory_log_.end());

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
    return { data.begin(), data.end() - 1 };
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

    dal_->ClearLogs();
}
