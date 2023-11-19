#include <fstream>
#include <string>
#include <stdint.h>

#include "exception/exception.h"
#include "page.h"

class DAL {
public:
  DAL(const std::string &path);
  DAL(const std::string &path, size_t page_size);

  Page AllocateEmptyPage();
  Page ReadPage(uint64_t page_num);
  void WritePage(const Page& page);

  void close();

  ~DAL();

private:
  size_t page_size_ = 4096;
  std::fstream file_; 
};
