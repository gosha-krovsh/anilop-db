#include "memory_log_dal.h"

MemoryLogDAL::MemoryLogDAL(const std::string &path, const settings::UserSettings& user_settings) {
    bool file_exist = std::filesystem::exists(path);
    file_.open(path, std::ios::binary);

    if (file_exist) {
        ReadMeta();
        ReadAllPages();
    } else {
        // Set up meta
        meta_->SetPageSize(user_settings.page_size);
        meta_->SetDirtyPage(1);
        meta_->SetAllocatedPage(2);
        meta_->SetDataStartPage(3);
        WriteMeta();
        // Write empty ones by default
        WriteNewPages();
        WriteDirtyPages();
    }
}

std::shared_ptr<MemoryLogMeta> MemoryLogDAL::GetMetaPtr() {
    return meta_;
}

void MemoryLogDAL::SavePage(uint64_t page_num, const std::shared_ptr<Page> &page) {
    if (!file_.is_open())
        throw dal_error::FileError("File is closed");

    page->SetPageNum(meta_->GetDataStartPage() + dirty_pages_.GetDataPtr()->size());
    WritePage(page);

    dirty_pages_.GetDataPtr()->push_back(page_num);
    WriteDirtyPages();
}

void MemoryLogDAL::SavePageAllocation(uint64_t page_num) {
    if (!file_.is_open())
        throw dal_error::FileError("File is closed");

    new_pages_.GetDataPtr()->push_back(page_num);
    WriteNewPages();
}

std::vector<std::pair<uint64_t, std::shared_ptr<Page>>> MemoryLogDAL::GetSavedPages() {
    if (!file_.is_open())
        throw dal_error::FileError("File is closed");

    std::vector<std::pair<uint64_t, std::shared_ptr<Page>>> result(dirty_pages_.GetDataPtr()->size());

    for (size_t i = 0; i < dirty_pages_.GetDataPtr()->size(); ++i) {
        auto page_num = meta_->GetDataStartPage() + i;
        auto page = ReadPage(page_num);
        result.emplace_back(dirty_pages_.GetDataPtr()->operator[](i), page);
    }
    return result;
}

std::vector<uint64_t> MemoryLogDAL::GetSavedPageAllocations() {
    if (!file_.is_open())
        throw dal_error::FileError("File is closed");

    return *new_pages_.GetDataPtr();
}

void MemoryLogDAL::Clear() {
    dirty_pages_.Clear();
    new_pages_.Clear();

    file_.clear();

    WriteMeta();
    // Write empty ones by default
    WriteNewPages();
    WriteDirtyPages();
}

void MemoryLogDAL::Close() {
    if (!file_.is_open())
        throw dal_error::FileError("File is closed");

    file_.close();
}

MemoryLogDAL::~MemoryLogDAL() {
    Close();
}

std::shared_ptr<Page> MemoryLogDAL::AllocateEmptyPage() {
    return std::make_shared<Page>(meta_->GetPageSize());
}

std::shared_ptr<Page> MemoryLogDAL::ReadPage(uint64_t page_num) {
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

void MemoryLogDAL::WritePage(const std::shared_ptr<Page> &page) {
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

void MemoryLogDAL::WriteMeta() {
    std::shared_ptr<Page> page = AllocateEmptyPage();
    page->SetPageNum(meta_page_num_);

    meta_->Serialize(page->Data(), meta_->GetPageSize());
    WritePage(page);
}

void MemoryLogDAL::ReadMeta() {
    std::shared_ptr<Page> page = AllocateEmptyPage();
    page->SetPageNum(meta_page_num_);

    meta_->Serialize(page->Data(), meta_->GetPageSize());
    WritePage(page);
}

void MemoryLogDAL::WriteDirtyPages() {
    std::shared_ptr<Page> page = AllocateEmptyPage();
    page->SetPageNum(meta_->GetDirtyPage());

    dirty_pages_.Serialize(page->Data(), meta_->GetPageSize());
    WritePage(page);
}

void MemoryLogDAL::WriteNewPages() {
    std::shared_ptr<Page> page = AllocateEmptyPage();
    page->SetPageNum(meta_->GetAllocatedPage());

    new_pages_.Serialize(page->Data(), meta_->GetPageSize());
    WritePage(page);
}

void MemoryLogDAL::ReadAllPages() {
    auto dirty_page = ReadPage(meta_->GetDirtyPage());
    auto new_page = ReadPage(meta_->GetAllocatedPage());

    dirty_pages_.Deserialize(dirty_page->Data(), meta_->GetPageSize());
    new_pages_.Deserialize(new_page->Data(), meta_->GetPageSize());
}


