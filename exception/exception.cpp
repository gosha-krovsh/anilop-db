#include "exception.h"

dal_error::LowPageVolume::LowPageVolume(const std::string& message)
    : std::runtime_error(message) {}

dal_error::FileError::FileError(const std::string& message)
    : std::runtime_error(message) {}

dal_error::InsufficientBufferSize::InsufficientBufferSize(
    const std::string& message)
    : std::runtime_error(message) {}

dal_error::CorruptedBuffer::CorruptedBuffer(const std::string& message)
    : std::runtime_error(message) {}

storage_error::InsertFailure::InsertFailure(const std::string& message)
    : std::runtime_error(message) {}
