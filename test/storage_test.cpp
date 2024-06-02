#include <gtest/gtest.h>

#define private public
#define protected public

#include "storage/storage.h"

TEST(Storage, TreeWorkflow) {
    settings::UserSettings settings;
    Storage storage("storage_test.db", settings);
    {
        std::vector<byte> key(8);
        std::memcpy(key.data(), "Goodbye", 8);
        auto result = storage.FindInTree(key);
        ASSERT_FALSE(result.has_value());
    }
    {
        std::vector<byte> key(8);
        std::memcpy(key.data(), "Goodbye", 8);
        std::vector<byte> data(6);
        std::memcpy(data.data(), "World", 6);
        ASSERT_NO_THROW(storage.PutInTree(key, data));
    }
    {
        std::vector<byte> key(8);
        std::memcpy(key.data(), "Goodbye", 8);
        std::vector<byte> data(6);
        std::memcpy(data.data(), "World", 6);
        auto result = storage.FindInTree(key);
        ASSERT_TRUE(result.has_value());
        ASSERT_EQ(result.value(), data);
    }
    {
        std::vector<byte> key(8);
        std::memcpy(key.data(), "Goodbye", 8);
        ASSERT_NO_THROW(storage.RemoveInTree(key));
    }
    {
        std::vector<byte> key(8);
        std::memcpy(key.data(), "Goodbye", 8);
        auto result = storage.FindInTree(key);
        ASSERT_FALSE(result.has_value());
    }
}

TEST(Storage, StorageCrash) {
    if (std::filesystem::exists("storage.db.log")) {
        std::filesystem::remove("storage.db.log");
    }
    if (std::filesystem::exists("storage.db.mlog")) {
        std::filesystem::remove("storage.db.mlog");
    }
    if (std::filesystem::exists("storage.db")) {
        std::filesystem::remove("storage.db");
    }
    settings::UserSettings settings;
    auto storage = std::make_shared<Storage>("storage.db", settings);

    {
        std::vector<byte> key(8);
        std::memcpy(key.data(), "Goodbye", 8);
        std::vector<byte> data(6);
        std::memcpy(data.data(), "World", 6);
        ASSERT_NO_THROW(storage->Put(key, data));
        throw std::runtime_error("Boom");
    }
}

TEST(Storage, StorageRecovery) {
    settings::UserSettings settings;
    auto storage = std::make_shared<Storage>("storage.db", settings);

    {
        std::vector<byte> key(8);
        std::memcpy(key.data(), "Goodbye", 8);
        std::vector<byte> data(6);
        std::memcpy(data.data(), "World", 6);

        auto data_opt = storage->Find(key);
        ASSERT_TRUE(data_opt.has_value());
        ASSERT_EQ(*data_opt, data);
    }
}