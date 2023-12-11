#ifndef PAGE_H_
#define PAGE_H_

#include <cstdint>
#include <vector>

#include "memory/type.h"
#include "settings/settings.h"
#include "exception/exception.h"

class Page {
   public:
    Page();
    Page(const std::vector<byte>& data);

    void SetPageNum(uint64_t page_num);
    uint64_t GetPageNum() const;

    byte* Data();
    const byte* Data() const;

   private:
    u_int64_t page_num_;
    std::vector<byte> data_;
};

#endif  // PAGE_H_
