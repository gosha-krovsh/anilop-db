#include "dal.h"

DAL::DAL(const std::string& path) : DAL::DAL(path, 4096) {}

DAL::DAL(const std::string& path, size_t page_size)
    : page_size_(page_size), file_(std::move(path)) {
    if (page_size < 1024) {
        throw data_layer::LowPageVolume(
            "[page_size < 1024] Page size is too small.");
    }
    size_t check = page_size & (page_size - 1);
    if (check != 0) {
        throw data_layer::IncorrectPageSize("Page size must be a power of 2.");
    }
}

Page DAL::AllocateEmptyPage() {
    return Page(page_size_);
}

Page DAL::ReadPage(uint64_t page_num) { 
    Page page = AllocateEmptyPage();
    // Page offset in file
    uint64_t offset = page_num * page_size_;
    // Retrieve page from file
    file_.seekg(offset);
    if (file_.fail()) {
        throw data_layer::FileError("File is corrupted.");
    }
    file_.readsome(page.Data(), page_size_);
    if (file_.fail()) {
        throw data_layer::FileError("File read failed.");
    }

    return page;
}

void DAL::WritePage(const Page& page) {
    uint64_t offset = page.GetPageNum() * page_size_;
    // Write page into file
    file_.seekp(offset);
    if (file_.fail()) {
        throw data_layer::FileError("File is corrupted.");
    }
    file_.write(page.Data(), page_size_);
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

DAL::~DAL() {
    close();
}
