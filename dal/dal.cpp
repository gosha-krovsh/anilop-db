#include "dal.h"

DAL::DAL(const std::string& path) : DAL::DAL(path, 4096) {}

DAL::DAL(const std::string& path, size_t page_size)
    : page_size_(page_size),
      file_(),
      meta_(new Meta()),
      free_list_(new FreeList()) {
    // Check page_size correctness
    if (page_size < 1024) {
        throw data_layer::LowPageVolume(
            "[page_size < 1024] Page size is too small.");
    }
    size_t check = page_size & (page_size - 1);
    if (check != 0) {
        throw data_layer::IncorrectPageSize("Page size must be a power of 2.");
    }
    // Check file existance and read metadata if needed
    bool file_exist = std::filesystem::exists(path);
    file_.open(path);

    if (file_exist) {
        readMeta();
        readFreeList();
    } else {
        // Gets a page for free_list_ and updates metadata
        meta_->SetFreeListPage(free_list_->GetNextPage());
        writeMeta();
    }
}

std::shared_ptr<Page> DAL::AllocateEmptyPage() {
    return std::shared_ptr<Page>(new Page(page_size_));
}

std::shared_ptr<Page> DAL::ReadPage(uint64_t page_num) {
    std::shared_ptr<Page> page = AllocateEmptyPage();
    // Page offset in file
    uint64_t offset = page_num * page_size_;
    // Retrieve page from file
    file_.seekg(offset);
    if (file_.fail()) {
        throw data_layer::FileError("File is corrupted.");
    }
    file_.readsome(page->Data(), page_size_);
    if (file_.fail()) {
        throw data_layer::FileError("File read failed.");
    }

    return page;
}

void DAL::WritePage(const std::shared_ptr<Page>& page) {
    uint64_t offset = page->GetPageNum() * page_size_;
    // Write page into file
    file_.seekp(offset);
    if (file_.fail()) {
        throw data_layer::FileError("File is corrupted.");
    }
    file_.write(page->Data(), page_size_);
    if (file_.fail()) {
        throw data_layer::FileError("File write failed.");
    }
}

void DAL::close() {
    file_.close();
    if (file_.fail()) {
        throw data_layer::FileError("File close failed.");
    }
}

DAL::~DAL() { close(); }

void DAL::writeMeta() {
    std::shared_ptr<Page> page = AllocateEmptyPage();
    page->SetPageNum(meta_page_num_);
    
    meta_->Serialize(page->Data(), page_size_);
    WritePage(page);
}

void DAL::readMeta() {
    std::shared_ptr<Page> page = ReadPage(meta_page_num_);
    meta_->Deserialize(page->Data(), page_size_);
}

void DAL::writeFreeList() {
    std::shared_ptr<Page> page = AllocateEmptyPage();
	page->SetPageNum(meta_->GetFreeListPage());
	
    free_list_->Serialize(page->Data(), page_size_);
    WritePage(page);
}

void DAL::readFreeList() {
    std::shared_ptr<Page> page = ReadPage(meta_->GetFreeListPage());
    free_list_->Deserialize(page->Data(), page_size_);
}
