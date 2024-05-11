#include "type.h"

#include <cstring>

namespace AnilopDB {

    std::string DataToString(const Data &data) {
        std::string result(data.size(), ' ');
        std::memcpy(result.data(), data.data(), data.size());
        return result;
    }

    Data StringToData(const std::string &str) {
        Data result(str.size() + 1);
        std::memcpy(result.data(), str.data(), str.size() + 1);
        return result;
    }

}