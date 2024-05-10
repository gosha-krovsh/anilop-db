#ifndef META_H_
#define META_H_

#include <string>

#include "serializable.h"

#include "memory/type.h"
#include "memory/memory.h"
#include "exception/exception.h"

class IMeta : public ISerializable{
public:
    size_t Serialize(byte* data, size_t max_volume) const override;
    size_t Deserialize(const byte* data, size_t max_volume) override;

    virtual size_t GetSize() const { return GetMagicWord().size() + 1; };

protected:
    virtual std::string GetMagicWord() const = 0;
};

class Meta : public IMeta {
    using BaseT = IMeta;

   public:
    size_t Serialize(byte* data, size_t max_volume) const override;
    size_t Deserialize(const byte* data, size_t max_volume) override;

    size_t GetSize() const override;

    uint64_t GetFreeListPage();
    void SetFreeListPage(uint64_t page);
    uint64_t GetRootPage();
    void SetRootPage(uint64_t page);

protected:
    std::string GetMagicWord() const override { return "ANILOPDB"; };

    uint64_t free_list_page_ = 0;
    uint64_t root_ = 0;
};

class LogMeta : public IMeta {
protected:
    std::string GetMagicWord() const override { return "ANILOPDBLOG"; }
};

class MemoryLogMeta : public IMeta {
    using BaseT = IMeta;

public:
    size_t Serialize(byte* data, size_t max_volume) const override;
    size_t Deserialize(const byte* data, size_t max_volume) override;

    size_t GetSize() const override;

    uint64_t GetDataStartPage() const { return data_page_; };
    void SetDataStartPage(uint64_t page) { data_page_ = page; }

    uint64_t GetDirtyPage() const { return dirty_page_; }
    void SetDirtyPage(uint64_t page) { dirty_page_ = page; }
    uint64_t GetAllocatedPage() const { return allocated_page_; }
    void SetAllocatedPage(uint64_t page) { allocated_page_ = page; }

protected:
    std::string GetMagicWord() const override { return "ANILOPDBMLOG"; }

private:
    uint64_t dirty_page_ = 0;
    uint64_t allocated_page_ = 0;
    uint64_t data_page_ = 0;
};

#endif  // META_H_
