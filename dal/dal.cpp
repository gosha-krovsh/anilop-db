#include "dal.h"

#include <memory>
#include <mutex>

DAL::DAL(const std::string& path,
         const settings::UserSettings&) :
      file_(),
      meta_(new Meta()),
      free_list_(new FreeList(settings::kMaxPage)) {
    // Check file existence and read metadata if needed
    bool file_exist = std::filesystem::exists(path);
    if (file_exist) {
        file_.open(path,  std::fstream::in | std::fstream::out);
    } else {
        file_.open(path,  std::fstream::in | std::fstream::out | std::fstream::trunc);
    }

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
    return std::make_shared<Page>(settings::kPageSize);
}

std::shared_ptr<Page> DAL::ReadPage(uint64_t page_num) {
    if (!file_.is_open())
        throw dal_error::FileError("File is closed");
    std::unique_lock lock(mutex_);

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
    if (!file_.is_open())
        throw dal_error::FileError("File is closed");
    std::unique_lock lock(mutex_);

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
    // Flush is important to keep data up to date
    file_.flush();
    if (file_.fail()) {
        throw dal_error::FileError("File flush failed.");
    }
}

uint64_t DAL::GetNextPage() {
    if (!file_.is_open())
        throw dal_error::FileError("File is closed");

    auto next_page = free_list_->GetNextPage();
    // Update freelist status
    writeFreeList();
    return next_page;
}

void DAL::ReleasePage(uint64_t page_num) {
    if (!file_.is_open())
        throw dal_error::FileError("File is closed");

    free_list_->ReleasePage(page_num);
    // Update freelist status
    writeFreeList();
}

void DAL::Close() {
    if (!file_.is_open())
        throw dal_error::FileError("File is closed");

    writeMeta();
    writeFreeList();

    file_.close();
    if (file_.fail()) {
        throw dal_error::FileError("File Close failed.");
    }
}

DAL::~DAL() {
    if (file_.is_open()) {
        Close();
    }
}

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

bool DAL::CanWrite() {
    return file_.is_open() && free_list_->HasFreePages();
}
