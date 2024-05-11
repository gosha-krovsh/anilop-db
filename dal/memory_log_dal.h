#ifndef ANILOP_MEMORYLOGDAL_H_
#define ANILOP_MEMORYLOGDAL_H_

#include <fstream>
#include <filesystem>
#include <cstdint>
#include <memory>
#include <string>

#include "log.h"
#include "page.h"
#include "meta.h"
#include "num_list.h"

#include "memory/type.h"
#include "settings/settings.h"
#include "exception/exception.h"


class MemoryLogDAL {
public:
    MemoryLogDAL(const std::string &path, const settings::UserSettings &user_settings);

    std::shared_ptr<MemoryLogMeta> GetMetaPtr();

    void SavePage(const std::shared_ptr<Page> &page);
    void SavePageAllocation(uint64_t page_num);

    std::vector<std::pair<uint64_t, std::shared_ptr<Page>>> GetSavedPages();
    std::vector<uint64_t> GetSavedPageAllocations();

    void Clear();

    void Close();

    ~MemoryLogDAL();

private:
    std::shared_ptr<Page> AllocateEmptyPage();

    std::shared_ptr<Page> ReadPage(uint64_t page_num);
    void WritePage(const std::shared_ptr<Page>& page);

    void WriteMeta();
    void ReadMeta();

    void WriteDirtyPages();
    void WriteNewPages();
    void ReadAllPages();

    std::fstream file_;

    const uint64_t meta_page_num_ = 0;
    std::shared_ptr <MemoryLogMeta> meta_;

    NumList dirty_pages_;
    NumList new_pages_;
};


#endif // ANILOP_MEMORYLOGDAL_H_
