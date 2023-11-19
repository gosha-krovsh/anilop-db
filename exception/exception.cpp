#include "exception.h"

data_layer::LowPageVolume::LowPageVolume(const std::string& message) : 
    std::runtime_error(message) {
}

data_layer::IncorrectPageSize::IncorrectPageSize(const std::string& message) : 
    std::runtime_error(message) {
}

data_layer::FileError::FileError(const std::string& message) : 
    std::runtime_error(message) {
}
