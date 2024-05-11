#ifndef ANILOP_TYPE_H_
#define ANILOP_TYPE_H_

#include <vector>
#include <string>

namespace AnilopDB {

    using byte = char;
    using Data = std::vector<byte>;

    std::string DataToString(const Data &data);
    Data StringToData(const std::string &str);

}

#endif // ANILOP_TYPE_H_
