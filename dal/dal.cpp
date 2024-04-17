#include "dal.h"

#include <memory>

DAL::DAL(const std::string& path,
         const std::string& log_path,
         const settings::UserSettings& user_settings) :
      file_(path, std::ios::binary),
      log_file_(log_path, std::ios::binary),
      meta_(new Meta()),
      free_list_(new FreeList(settings::kMaxPage)) {
    // Check file existence and read metadata if needed
    bool file_exist = std::filesystem::exists(path);
    file_.open(path);

    if (file_exist) {
        readMeta();
        readFreeList();
    } else {
        // Gets a page for free_list_ and updates metadata
        meta_->SetFreeListPage(free_list_->GetNextPage());
        meta_->SetPageSize(user_settings.page_size);
        writeMeta();
    }
}

std::shared_ptr<Meta> DAL::GetMetaPtr() {
    return meta_;
}

std::shared_ptr<Page> DAL::AllocateEmptyPage() {
    return std::make_shared<Page>(meta_->GetPageSize());
}

std::shared_ptr<Page> DAL::ReadPage(uint64_t page_num) {
    std::shared_ptr<Page> page = AllocateEmptyPage();
    // Page offset in file
    uint64_t offset = page_num * meta_->GetPageSize();
    // Retrieve page from file
    file_.seekg(offset);
    if (file_.fail()) {
        throw dal_error::FileError("File is corrupted.");
    }
    file_.readsome(page->Data(), meta_->GetPageSize());
    if (file_.fail()) {
        throw dal_error::FileError("File read failed.");
    }

    return page;
}

void DAL::WritePage(const std::shared_ptr<Page>& page) {
    uint64_t offset = page->GetPageNum() * meta_->GetPageSize();
    // Write page into file
    file_.seekp(offset);
    if (file_.fail()) {
        throw dal_error::FileError("File is corrupted.");
    }
    file_.write(page->Data(), meta_->GetPageSize());
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
    return free_list_->GetNextPage(); 
}

void DAL::ReleasePage(uint64_t page_num) {
    free_list_->ReleasePage(page_num);
}

void DAL::close() {
    file_.close();
    if (file_.fail()) {
        throw dal_error::FileError("File close failed.");
    }
    log_file_.close();
    if (file_.fail()) {
        throw dal_error::FileError("File close failed.");
    }
}

DAL::~DAL() { close(); }

void DAL::writeMeta() {
    std::shared_ptr<Page> page = AllocateEmptyPage();
    page->SetPageNum(meta_page_num_);
    
    meta_->Serialize(page->Data(), meta_->GetPageSize());
    WritePage(page);
}

void DAL::readMeta() {
    std::shared_ptr<Page> page = ReadPage(meta_page_num_);
    meta_->Deserialize(page->Data(), meta_->GetPageSize());
}

void DAL::writeFreeList() {
    std::shared_ptr<Page> page = AllocateEmptyPage();
	page->SetPageNum(meta_->GetFreeListPage());
	
    free_list_->Serialize(page->Data(), meta_->GetPageSize());
    WritePage(page);
}

void DAL::readFreeList() {
    std::shared_ptr<Page> page = ReadPage(meta_->GetFreeListPage());
    free_list_->Deserialize(page->Data(), meta_->GetPageSize());
}

bool DAL::canWriteLog() {
    return free_list_->HasFreePages();
}

std::vector<byte> DAL::ReadLogBuffer() {
    auto size = log_file_.tellg();
    if (size == 0) {
        return {};
    }
    std::vector<byte> buffer(size);
    log_file_.read(buffer.data(), size);
    return buffer;
}

void DAL::ClearLog() {
    log_file_.clear();
}

void DAL::WriteLog(const Log &log) {
    std::vector<byte> data(log.GetLogSize());
    log.Serialize(data.data(), data.size());

    log_file_.write(data.data(), data.size());
    if (log_file_.fail()) {
        throw dal_error::FileError("Log file write failed.");
    }
    // Flush is important to keep data up to date
    log_file_.flush();
    if (log_file_.fail()) {
        throw dal_error::FileError("Log file flush failed.");
    }
}
