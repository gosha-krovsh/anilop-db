#ifndef DAL_H_
#define DAL_H_

#include <fstream>
#include <filesystem>
#include <cstdint>
#include <memory>
#include <string>

#include "type.h"
#include "page.h"
#include "meta.h"
#include "freelist.h"
#include "exception/exception.h"

class DAL {
public:
  DAL(const std::string &path);
  DAL(const std::string &path, size_t page_size);

  std::shared_ptr<Page> AllocateEmptyPage();
  std::shared_ptr<Page> ReadPage(uint64_t page_num);
  void WritePage(const std::shared_ptr<Page>& page);

  void close();

  ~DAL();

private:
  void writeMeta();
  void readMeta();

  void readFreeList();
  void writeFreeList();

  size_t page_size_;
  std::fstream file_;

  const uint64_t meta_page_num_ = 0;
  std::shared_ptr<Meta> meta_;
  std::shared_ptr<FreeList> free_list_;
};

#endif  // DAL_H_
