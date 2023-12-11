#include "dal.h"

DAL::DAL(const std::string& path) :
      file_(),
      meta_(new Meta()),
      free_list_(new FreeList(settings::kMaxPage)) {
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

std::shared_ptr<Meta> DAL::GetMetaPtr() {
    return meta_;
}

std::shared_ptr<Page> DAL::AllocateEmptyPage() {
    return std::shared_ptr<Page>(new Page());
}

std::shared_ptr<Page> DAL::ReadPage(uint64_t page_num) {
    std::shared_ptr<Page> page = AllocateEmptyPage();
    // Page offset in file
    uint64_t offset = page_num * settings::kPageSize;
    // Retrieve page from file
    file_.seekg(offset);
    if (file_.fail()) {
        throw dal_error::FileError("File is corrupted.");
    }
    file_.readsome(page->Data(), settings::kPageSize);
    if (file_.fail()) {
        throw dal_error::FileError("File read failed.");
    }

    return page;
}

void DAL::WritePage(const std::shared_ptr<Page>& page) {
    uint64_t offset = page->GetPageNum() * settings::kPageSize;
    // Write page into file
    file_.seekp(offset);
    if (file_.fail()) {
        throw dal_error::FileError("File is corrupted.");
    }
    file_.write(page->Data(), settings::kPageSize);
    if (file_.fail()) {
        throw dal_error::FileError("File write failed.");
    }
}

uint64_t DAL::GetNextPage() {
    return free_list_->GetNextPage(); 
}

void DAL::ReleasePage(uint64_t page_num) {
    free_list_->RealeasePage(page_num);
}

void DAL::close() {
    file_.close();
    if (file_.fail()) {
        throw dal_error::FileError("File close failed.");
    }
}

DAL::~DAL() { close(); }

void DAL::writeMeta() {
    std::shared_ptr<Page> page = AllocateEmptyPage();
    page->SetPageNum(meta_page_num_);
    
    meta_->Serialize(page->Data(), settings::kPageSize);
    WritePage(page);
}

void DAL::readMeta() {
    std::shared_ptr<Page> page = ReadPage(meta_page_num_);
    meta_->Deserialize(page->Data(), settings::kPageSize);
}

void DAL::writeFreeList() {
    std::shared_ptr<Page> page = AllocateEmptyPage();
	page->SetPageNum(meta_->GetFreeListPage());
	
    free_list_->Serialize(page->Data(), settings::kPageSize);
    WritePage(page);
}

void DAL::readFreeList() {
    std::shared_ptr<Page> page = ReadPage(meta_->GetFreeListPage());
    free_list_->Deserialize(page->Data(), settings::kPageSize);
}
