#include <gtest/gtest.h>

#define private public
#define protected public

#include "dal/dal.h"
#include "dal/log_dal.h"
#include "dal/memory_log_dal.h"

class DalTest : public ::testing::Test {
protected:
    std::shared_ptr<DAL> dal_;

    virtual void SetUp() {
        if (std::filesystem::exists("test.db")) {
            std::filesystem::remove("test.db");
            std::filesystem::copy_file("../test/data/test.db", "test.db");
        }
        settings::UserSettings settings;
        dal_ = std::make_shared<DAL>("test.db", settings);
    }
};

class EmptyDalTest : public ::testing::Test {
protected:
    std::shared_ptr<DAL> dal_;

    virtual void SetUp() {
        if (std::filesystem::exists("test.db")) {
            std::filesystem::remove("test.db");
        }
        settings::UserSettings settings;
        dal_ = std::make_shared<DAL>("test.db", settings);
    }
};


TEST(Dal, OpenClose) {
    settings::UserSettings settings;

    DAL dal("test.db", settings);
    dal.Close();

    DAL dal2("test.db", settings);
    dal2.Close();
}

TEST_F(DalTest, Workflow) {
    ASSERT_TRUE(dal_->CanWrite());
    size_t pg_num;
    {
        auto page = dal_->AllocateEmptyPage();
        page->SetPageNum(dal_->GetNextPage());
        pg_num = page->GetPageNum();

        page->Data()[0] = '#';
        ASSERT_NO_THROW(dal_->WritePage(page));
    }
    {
        auto page = dal_->ReadPage(pg_num);
        ASSERT_EQ(page->Data()[0], '#');
    }
}

TEST_F(EmptyDalTest, Workflow) {
    ASSERT_TRUE(dal_->CanWrite());
    size_t pg_num;
    {
        auto page = dal_->AllocateEmptyPage();
        page->SetPageNum(dal_->GetNextPage());
        pg_num = page->GetPageNum();

        page->Data()[0] = '#';
        ASSERT_NO_THROW(dal_->WritePage(page));
    }
    {
        auto page = dal_->ReadPage(pg_num);
        ASSERT_EQ(page->Data()[0], '#');
    }
}

class LogDalTest : public ::testing::Test {
protected:
    std::shared_ptr<LogDAL> dal_;

    virtual void SetUp() {
        if (std::filesystem::exists("test.db.log")) {
            std::filesystem::remove("test.db.log");
            std::filesystem::copy_file("../test/data/test.db.log", "test.db.log");
        }
        settings::UserSettings settings;
        dal_ = std::make_shared<LogDAL>("test.db.log", settings);
    }
};

class EmptyLogDalTest : public ::testing::Test {
protected:
    std::shared_ptr<LogDAL> dal_;

    virtual void SetUp() {
        if (std::filesystem::exists("test.db.log")) {
            std::filesystem::remove("test.db.log");
        }
        settings::UserSettings settings;
        dal_ = std::make_shared<LogDAL>("test.db.log", settings);
    }
};


TEST(LogDAL, OpenClose) {
    settings::UserSettings settings;

    LogDAL dal("test.db.log", settings);
    dal.Close();

    LogDAL dal2("test.db.log", settings);
    dal2.Close();
}

TEST_F(LogDalTest, Workflow) {
    std::vector<byte> key(6);
    std::memcpy(key.data(), "Hello", 6);
    std::vector<byte> data(6);
    std::memcpy(data.data(), "World", 6);
    Log log(Log::Command::PUT, key, data);

    auto buffer = dal_->ReadLogBuffer();
    auto saved_log = Log::readFromBuffer(buffer.data(), log.GetByteLength());

    ASSERT_EQ(saved_log.command_, log.command_);
    ASSERT_EQ(saved_log.key_, log.key_);
    ASSERT_EQ(saved_log.value_, log.value_);
}

TEST_F(EmptyLogDalTest, Workflow) {
    std::vector<byte> key(6);
    std::memcpy(key.data(), "Hello", 6);
    std::vector<byte> data(6);
    std::memcpy(data.data(), "World", 6);
    Log log(Log::Command::PUT, key, data);

    ASSERT_NO_THROW(dal_->WriteLog(log));
    auto buffer = dal_->ReadLogBuffer();
    auto saved_log = Log::readFromBuffer(buffer.data(), log.GetByteLength());

    ASSERT_EQ(saved_log.command_, log.command_);
    ASSERT_EQ(saved_log.key_, log.key_);
    ASSERT_EQ(saved_log.value_, log.value_);
}

class MemoryLogDalTest : public ::testing::Test {
protected:
    std::shared_ptr<MemoryLogDAL> dal_;

    virtual void SetUp() {
        if (std::filesystem::exists("test.db.mlog")) {
            std::filesystem::remove("test.db.mlog");
            std::filesystem::copy_file("../test/data/test.db.mlog", "test.db.mlog");
        }
        settings::UserSettings settings;
        dal_ = std::make_shared<MemoryLogDAL>("test.db.mlog", settings);
    }
};

class EmptyMemoryLogDalTest : public ::testing::Test {
protected:
    std::shared_ptr<MemoryLogDAL> dal_;

    virtual void SetUp() {
        if (std::filesystem::exists("test.db.mlog")) {
            std::filesystem::remove("test.db.mlog");
        }
        settings::UserSettings settings;
        dal_ = std::make_shared<MemoryLogDAL>("test.db.mlog", settings);
    }
};

TEST(MemoryLogDAL, OpenClose) {
    settings::UserSettings settings;

    MemoryLogDAL dal("test.db.mlog", settings);
    dal.Close();

    MemoryLogDAL dal2("test.db.mlog", settings);
    dal2.Close();
}

TEST_F(EmptyMemoryLogDalTest, Workflow) {
    {
        auto page = dal_->AllocateEmptyPage();
        page->SetPageNum(10);
        page->Data()[0] = '#';

        ASSERT_NO_THROW(dal_->SavePage(page));
        ASSERT_NO_THROW(dal_->SavePageAllocation(11));
    }
    {
        auto pages = dal_->GetSavedPages();
        ASSERT_EQ(pages[0].first, 10);
        ASSERT_EQ(pages[0].second->Data()[0], '#');
        auto allocations = dal_->GetSavedPageAllocations();
        ASSERT_EQ(allocations[0], 11);
    }
}

TEST_F(MemoryLogDalTest, Workflow) {
    auto pages = dal_->GetSavedPages();
    ASSERT_EQ(pages.size(), 1);
    ASSERT_EQ(pages[0].first, 10);
    ASSERT_EQ(pages[0].second->Data()[0], '#');
    auto allocations = dal_->GetSavedPageAllocations();
    ASSERT_EQ(allocations.size(), 1);
    ASSERT_EQ(allocations[0], 11);
}
