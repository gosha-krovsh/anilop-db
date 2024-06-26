#include "log_storage.h"

LogStorage::LogStorage(std::shared_ptr<DAL> dal, std::shared_ptr<LogDAL> log_dal, const settings::UserSettings &settings)
    : settings_(settings), dal_(std::move(dal)), log_dal_(std::move(log_dal)) {
    auto log_buffer = log_dal_->ReadLogBuffer();
    auto log_buffer_ptr = log_buffer.data();
    uint64_t buffer_size_left = log_buffer.size();

    size_t start_count = 0;
    size_t commit_count = 0;
    while (buffer_size_left > 0) {
        auto log = Log::readFromBuffer(log_buffer_ptr, buffer_size_left);
        log_buffer_ptr += log.GetByteLength();
        buffer_size_left -= log.GetByteLength();

        if (log.GetCommand() == Log::Command::START)
            start_count++;
        if (log.GetCommand() == Log::Command::COMMIT)
            commit_count++;
        WriteLogToMemory(log);
    }

    auto log_index = memory_log_.size() - 1;
    uint64_t clear_offset = 0;
    while (log_index > 0 && commit_count != start_count) {
        auto log_it = memory_log_.begin();
        std::advance(log_it, log_index);

        auto log = *log_it;
        if (log.GetCommand() == Log::Command::START)
            --start_count;
        if (log.GetCommand() == Log::Command::COMMIT)
            --commit_count;

        clear_offset += log.GetByteLength();

        auto str_key = ConvertToStr(log.GetKey());
        key_to_memory_log_[str_key].pop_back();

        memory_log_.erase(log_it);
        --log_index;
    }
    if (clear_offset > 0) {
        log_dal_->ClearLatest(clear_offset);
    }
}

std::optional<std::vector<byte>> LogStorage::Find(const std::vector<byte> &key) {
    auto str_key = ConvertToStr(key);
    auto map_it = key_to_memory_log_.find(str_key);
    if (map_it != key_to_memory_log_.end()) {
        const auto& log = map_it->second.back();
        if (log->GetCommand() != Log::Command::REMOVE)
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

std::list<Log> &LogStorage::GetLogs() {
    return memory_log_;
}

const std::list<Log> &LogStorage::GetLogs() const {
    return memory_log_;
}

void LogStorage::Clear() {
    memory_log_.clear();
    key_to_memory_log_.clear();

    log_dal_->ClearLogs();
}

void LogStorage::PushTransactionLogs(const std::vector<Log> &logs) {
    WriteLog({ Log::Command::START });
    for (const auto& log : logs)
        WriteLog(log);
    WriteLog({ Log::Command::COMMIT });
}

void LogStorage::WriteLog(const Log &log) {
    log_dal_->WriteLog(log);
    WriteLogToMemory(log);
}

void LogStorage::WriteLogToMemory(const Log &log) {
    memory_log_.push_back(log);

    if (log.GetCommand() == Log::Command::START
        || log.GetCommand() == Log::Command::COMMIT)
        return;

    auto str_key = ConvertToStr(log.GetKey());
    key_to_memory_log_[str_key].push_back(std::prev(memory_log_.end()));
}
