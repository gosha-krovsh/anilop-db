#include <stdexcept>
#include <string>

namespace data_layer {

class LowPageVolume : public std::runtime_error {
   public:
    LowPageVolume(const std::string& message);

   private:
    std::string message_;
};

class IncorrectPageSize : public std::runtime_error {
   public:
    IncorrectPageSize(const std::string& message);

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
