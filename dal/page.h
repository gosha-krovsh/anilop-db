#include <cstdint>
#include <vector>

using byte = char;

class Page {
   public:
    Page(size_t page_size, const std::vector<byte>& data);
    Page(size_t page_size);

    void SetPageNum(uint64_t page_num);
    uint64_t GetPageNum() const;

    byte* Data();
    const byte* Data() const;

   private:
    u_int64_t page_num_;
    std::vector<byte> data_;
};
