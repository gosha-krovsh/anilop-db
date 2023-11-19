#include <cstdint>
#include <vector>

class FreeList {
   public:
    FreeList();

    uint64_t GetMaxPage();
    void SetMaxPage(uint64_t max_page);

    uint64_t GetNextPage();
    void RealeasePage(uint64_t page_num);

   private:
    uint64_t max_page_;
    std::vector<uint64_t> realeased_pages_;
};
