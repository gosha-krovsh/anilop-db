#ifndef EXCEPTION_H_
#define EXCEPTION_H_

#include <stdexcept>
#include <string>

namespace dal_error {

class InsufficientBufferSize : public std::runtime_error {
   public:
    InsufficientBufferSize(const std::string& message);

   private:
    std::string message_;
};

class CorruptedBuffer : public std::runtime_error {
   public:
    CorruptedBuffer(const std::string& message);

   private:
    std::string message_;
};

class LowPageVolume : public std::runtime_error {
   public:
    LowPageVolume(const std::string& message);

   private:
    std::string message_;
};

class FileError : public std::runtime_error {
   public:
    FileError(const std::string& message);

   private:
    std::string message_;
};

}  // namespace data_layer

namespace storage_error {

class InsertFailure : public std::runtime_error {
   public:
    InsertFailure(const std::string& message);

   private:
    std::string message_;
};

}  // namespace data_layer

#endif  // EXCEPTION_H_ 
