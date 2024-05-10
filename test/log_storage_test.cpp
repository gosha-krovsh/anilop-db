#include <gtest/gtest.h>

#define private public
#define protected public

#include "storage/log_storage.h"

class EmptyLogStorageTest : public ::testing::Test {
protected:
    settings::UserSettings settings_;
    std::shared_ptr<LogDAL> log_dal_;
    std::shared_ptr<DAL> dal_;

    virtual void SetUp() {
        if (std::filesystem::exists("log_storage_test.db")) {
            std::filesystem::remove("log_storage_test.db");
        }
        if (std::filesystem::exists("log_storage_test.db.log")) {
            std::filesystem::remove("log_storage_test.db.log");
        }
        dal_ = std::make_shared<DAL>("log_storage_test.db", settings_);
        log_dal_ = std::make_shared<LogDAL>("log_storage_test.db.log", settings_);
    }
};

TEST_F(EmptyLogStorageTest, Workflow) {
    LogStorage log_storage(dal_, log_dal_, settings_);
    {
        std::vector<byte> key(6);
        std::memcpy(key.data(), "Hello", 6);
        std::vector<byte> data(6);
        std::memcpy(data.data(), "World", 6);
        ASSERT_NO_THROW(log_storage.Put(key, data));
    }
    {
        std::vector<byte> key(8);
        std::memcpy(key.data(), "Goodbye", 8);
        std::vector<byte> data(6);
        std::memcpy(data.data(), "World", 6);
        ASSERT_NO_THROW(log_storage.Put(key, data));
    }
    {
        std::vector<byte> key(8);
        std::memcpy(key.data(), "Goodbye", 8);
        ASSERT_NO_THROW(log_storage.Remove(key));
    }

    {
        std::vector<byte> key(6);
        std::memcpy(key.data(), "Hello", 6);
        ASSERT_TRUE(log_storage.Find(key).has_value());
    }
    {
        std::vector<byte> key(8);
        std::memcpy(key.data(), "Goodbye", 8);
        ASSERT_FALSE(log_storage.Find(key).has_value());
    }
}

class LogStorageTest : public ::testing::Test {
protected:
    settings::UserSettings settings_;
    std::shared_ptr<LogDAL> log_dal_;
    std::shared_ptr<DAL> dal_;

    virtual void SetUp() {
        if (std::filesystem::exists("log_storage_test.db")) {
            std::filesystem::remove("log_storage_test.db");
            std::filesystem::copy_file("../test/data/log_storage_test.db", "log_storage_test.db");
        }
        if (std::filesystem::exists("log_storage_test.db.log")) {
            std::filesystem::remove("log_storage_test.db.log");
            std::filesystem::copy_file("../test/data/log_storage_test.db.log", "log_storage_test.db.log");
        }
        dal_ = std::make_shared<DAL>("log_storage_test.db", settings_);
        log_dal_ = std::make_shared<LogDAL>("log_storage_test.db.log", settings_);
    }
};

TEST_F(LogStorageTest, Workflow) {
    LogStorage log_storage(dal_, log_dal_, settings_);
    {
        std::vector<byte> key(6);
        std::memcpy(key.data(), "Hello", 6);
        ASSERT_TRUE(log_storage.Find(key).has_value());
    }
    {
        std::vector<byte> key(8);
        std::memcpy(key.data(), "Goodbye", 8);
        ASSERT_FALSE(log_storage.Find(key).has_value());
    }
}
