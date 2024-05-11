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