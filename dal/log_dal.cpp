#include "log_dal.h"

LogDAL::LogDAL(const std::string &path, const settings::UserSettings &)
    : meta_(new LogMeta()) {
    bool file_exist = std::filesystem::exists(path);
    if (file_exist) {
        file_.open(path,  std::fstream::in | std::fstream::out);
    } else {
        file_.open(path,  std::fstream::in | std::fstream::out | std::fstream::trunc);
    }

    if (file_exist) {
        ReadMeta();
    } else {
        WriteMeta();
    }
}

std::shared_ptr<LogMeta> LogDAL::GetMetaPtr() {
    return meta_;
}

std::vector<byte> LogDAL::ReadLogBuffer() {
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
    if (!file_.is_open())
        throw dal_error::FileError("File is closed");

    std::vector<byte> data(log.GetByteLength());
    log.Serialize(data.data(), data.size());

    file_.seekp(meta_->GetSize(), file_.beg);
    file_.write(data.data(), data.size());
    if (file_.fail()) {
        throw dal_error::FileError("Log file write failed.");
    }
    // Flush is important to keep data up to date
    file_.flush();
    if (file_.fail()) {
        throw dal_error::FileError("Log file flush failed.");
    }
}

void LogDAL::ClearLogs() {
    if (!file_.is_open())
        throw dal_error::FileError("File is closed");

    file_.clear();
    WriteMeta();
}

void LogDAL::Close() {
    if (!file_.is_open())
        throw dal_error::FileError("File is already closed");

    file_.close();
    if (file_.fail()) {
        throw dal_error::FileError("File Close failed.");
    }
}

LogDAL::~LogDAL() { Close(); }

void LogDAL::WriteMeta() {
    file_.seekp(meta_offset_, file_.beg);

    std::vector<byte> meta_buffer(meta_->GetSize());
    meta_->Serialize(meta_buffer.data(), meta_->GetSize());

    file_.write(meta_buffer.data(), meta_->GetSize());
}

void LogDAL::ReadMeta() {
    file_.seekg(meta_offset_, file_.beg);

    std::vector<byte> meta_buffer(meta_->GetSize());
    file_.read(meta_buffer.data(), meta_->GetSize());

    meta_->Deserialize(meta_buffer.data(), meta_->GetSize());
}

