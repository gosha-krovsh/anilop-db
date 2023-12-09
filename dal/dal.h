#include <fstream>
#include <filesystem>
#include <cstdint>
#include <memory>
#include <string>

#include "exception/exception.h"
#include "meta.h"
#include "page.h"

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

  size_t page_size_;
  std::fstream file_;

  const uint64_t meta_page_num_ = 0;
  Meta meta_;
};
