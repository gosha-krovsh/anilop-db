#include "log_dal.h"

LogDAL::LogDAL(const std::string &path, const settings::UserSettings &)
    : path_(path)
    , meta_(new LogMeta()) {
    bool file_exist = std::filesystem::exists(path);
    if (file_exist) {
        file_.open(path,  std::fstream::in | std::fstream::out);
    } else {
        file_.open(path,  std::fstream::in | std::fstream::out | std::fstream::trunc);
    }

    if (file_exist) {
        ReadMeta();
    } else {
        meta_->SetDataEndOffset(meta_->GetSize());
        WriteMeta();
    }
}

std::shared_ptr<LogMeta> LogDAL::GetMetaPtr() {
    return meta_;
}

std::vector<byte> LogDAL::ReadLogBuffer() {
    std::unique_lock lock(mutex_);
    if (!file_.is_open())
        throw dal_error::FileError("File is closed");

    file_.seekg(0, file_.end);
    uint64_t size = file_.tellg();
    size -= meta_->GetSize();
    // Put file read position to data
    file_.seekg(meta_->GetSize(), file_.beg);
    if (size == 0) {
        return {};
    }
    std::vector<byte> buffer(size);
    file_.read(buffer.data(), size);
    return buffer;
}

void LogDAL::WriteLog(const Log &log) {
    std::unique_lock lock(mutex_);
    if (!file_.is_open())
        throw dal_error::FileError("File is closed");

    std::vector<byte> data(log.GetByteLength());
    log.Serialize(data.data(), data.size());

    file_.seekp(meta_->GetDataEndOffset(), file_.beg);
    file_.write(data.data(), data.size());
    if (file_.fail()) {
        throw dal_error::FileError("Log file write failed.");
    }
    // Flush is important to keep data up to date
    file_.flush();
    if (file_.fail()) {
        throw dal_error::FileError("Log file flush failed.");
    }

    // Update offset
    meta_->SetDataEndOffset(meta_->GetDataEndOffset() + log.GetByteLength());
    WriteMeta();
}

void LogDAL::ClearLogs() {
    std::unique_lock lock(mutex_);
    if (!file_.is_open())
        throw dal_error::FileError("File is closed");

    // Open close to clear logs
    file_.close();
    file_.open(path_,  std::fstream::in | std::fstream::out | std::fstream::trunc);

    meta_->SetDataEndOffset(meta_->GetSize());
    WriteMeta();
}

void LogDAL::Close() {
    std::unique_lock lock(mutex_);
    if (!file_.is_open())
        throw dal_error::FileError("File is already closed");

    file_.close();
    if (file_.fail()) {
        throw dal_error::FileError("File Close failed.");
    }
}

LogDAL::~LogDAL() {
    std::unique_lock lock(mutex_);
    if (file_.is_open()) {
        Close();
    }
}

void LogDAL::WriteMeta() {
    std::unique_lock lock(mutex_);
    file_.seekp(meta_offset_, file_.beg);

    std::vector<byte> meta_buffer(meta_->GetSize());
    meta_->Serialize(meta_buffer.data(), meta_->GetSize());

    file_.write(meta_buffer.data(), meta_->GetSize());
}

void LogDAL::ReadMeta() {
    std::unique_lock lock(mutex_);
    file_.seekg(meta_offset_, file_.beg);

    std::vector<byte> meta_buffer(meta_->GetSize());
    file_.read(meta_buffer.data(), meta_->GetSize());

    meta_->Deserialize(meta_buffer.data(), meta_->GetSize());
}

void LogDAL::ClearLatest(uint64_t offset_to_end) {
    auto max_offset = meta_->GetDataEndOffset() - meta_->GetSize();
    if (offset_to_end > max_offset) {
        offset_to_end = max_offset;
    }

    meta_->SetDataEndOffset(meta_->GetDataEndOffset() - offset_to_end);
    WriteMeta();
}
