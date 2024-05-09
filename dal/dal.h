#ifndef DAL_H_
#define DAL_H_

#include <fstream>
#include <filesystem>
#include <cstdint>
#include <memory>
#include <string>

#include "log.h"
#include "page.h"
#include "meta.h"
#include "freelist.h"

#include "memory/type.h"
#include "settings/settings.h"
#include "exception/exception.h"

class DAL {
public:
  DAL(const std::string& path,
      const settings::UserSettings& user_settings);

  std::shared_ptr<Meta> GetMetaPtr();

  std::shared_ptr<Page> AllocateEmptyPage();
  std::shared_ptr<Page> ReadPage(uint64_t page_num);
  void WritePage(const std::shared_ptr<Page>& page);

  uint64_t GetNextPage();
  void ReleasePage(uint64_t page_num);

  bool CanWrite();

  void close();
  ~DAL();

private:
  void writeMeta();
  void readMeta();

  void readFreeList();
  void writeFreeList();

  std::fstream file_;
  std::fstream log_file_;
  std::fstream memory_log_file_;

  const uint64_t meta_page_num_ = 0;
  std::shared_ptr<MemoryLogMeta> memory_log_meta_;
  std::shared_ptr<Meta> meta_;
  std::shared_ptr<FreeList> free_list_;
};

#endif  // DAL_H_
