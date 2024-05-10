#include <gtest/gtest.h>

#define private public
#define protected public

#include "dal/item.h"
#include "dal/node.h"
#include "dal/freelist.h"
#include "dal/num_list.h"
#include "dal/meta.h"
#include "dal/log.h"


TEST(Meta, All) {
    Meta meta;
    meta.SetRootPage(1);
    meta.SetFreeListPage(0);
    auto meta_magic = meta.GetMagicWord();
    auto meta_size = meta.GetSize();

    std::vector<byte> data(meta_size);
    meta.Serialize(data.data(), meta_size);

    Meta saved_meta;
    saved_meta.Deserialize(data.data(), meta_size);
    ASSERT_EQ(saved_meta.GetSize(), meta_size);
    ASSERT_EQ(saved_meta.GetMagicWord(), meta_magic);
    ASSERT_EQ(saved_meta.GetRootPage(), 1);
    ASSERT_EQ(saved_meta.GetFreeListPage(), 0);
}

TEST(MemoryLogMeta, All) {
    MemoryLogMeta meta;
    meta.SetAllocatedPage(1);
    meta.SetDirtyPage(2);
    meta.SetDataStartPage(3);
    auto meta_magic = meta.GetMagicWord();
    auto meta_size = meta.GetSize();

    std::vector<byte> data(meta_size);
    meta.Serialize(data.data(), meta_size);

    MemoryLogMeta saved_meta;
    saved_meta.Deserialize(data.data(), meta_size);
    ASSERT_EQ(saved_meta.GetSize(), meta_size);
    ASSERT_EQ(saved_meta.GetMagicWord(), meta_magic);
    ASSERT_EQ(saved_meta.GetDirtyPage(), 2);
    ASSERT_EQ(saved_meta.GetAllocatedPage(), 1);
    ASSERT_EQ(saved_meta.GetDataStartPage(), 3);
}

TEST(LogMeta, All) {
    LogMeta meta;
    auto meta_magic = meta.GetMagicWord();
    auto meta_size = meta.GetSize();

    std::vector<byte> data(meta_size);
    meta.Serialize(data.data(), meta_size);

    LogMeta saved_meta;
    saved_meta.Deserialize(data.data(), meta_size);
    ASSERT_EQ(saved_meta.GetSize(), meta_size);
    ASSERT_EQ(saved_meta.GetMagicWord(), meta_magic);
}

TEST(FreeList, All) {
    FreeList freeList(3000);
    freeList.GetNextPage();
    freeList.GetNextPage();

    std::vector<byte> data(4096);
    freeList.Serialize(data.data(), 4096);

    FreeList saved_freelist;
    saved_freelist.Deserialize(data.data(), 4096);

    ASSERT_EQ(freeList.max_page_, saved_freelist.max_page_);
    ASSERT_EQ(freeList.current_max_page_, saved_freelist.current_max_page_);
    ASSERT_EQ(freeList.released_pages_, saved_freelist.released_pages_);
}

TEST(Item, All) {
    std::vector<byte> key(6);
    std::memcpy(key.data(), "Hello", 6);
    std::vector<byte> data(6);
    std::memcpy(data.data(), "World", 6);

    Item item(key, data);
    std::vector<byte> memory(item.ByteLength());
    item.Serialize(memory.data(), item.ByteLength());

    Item saved_item;
    saved_item.Deserialize(memory.data(), item.ByteLength());

    ASSERT_EQ(item.key_, saved_item.key_);
    ASSERT_EQ(item.value_, saved_item.value_);
}

TEST(Node, All) {
    std::vector<byte> key(6);
    std::memcpy(key.data(), "Hello", 6);
    std::vector<byte> data(6);
    std::memcpy(data.data(), "World", 6);
    std::shared_ptr<Item> item = std::make_shared<Item>(key, data);

    Node node;
    node.AddItem(item, 0);

    std::vector<byte> memory(node.ByteLength());
    node.Serialize(memory.data(), node.ByteLength());

    Node saved_node;
    saved_node.Deserialize(memory.data(), node.ByteLength());

    ASSERT_EQ(node.child_nodes_, node.child_nodes_);

    ASSERT_EQ(saved_node.items_.size(), 1);
    ASSERT_EQ(node.items_[0]->key_, saved_node.items_[0]->key_);
    ASSERT_EQ(node.items_[0]->value_, saved_node.items_[0]->value_);
}

TEST(NumList, All) {
    NumList num_list;
    num_list.GetDataPtr()->push_back(10);
    num_list.GetDataPtr()->push_back(22);
    num_list.GetDataPtr()->push_back(3);

    std::vector<byte> memory(num_list.GetByteLength());
    num_list.Serialize(memory.data(), num_list.GetByteLength());

    NumList saved_list;
    saved_list.Deserialize(memory.data(), num_list.GetByteLength());

    ASSERT_EQ(num_list.num_data_, saved_list.num_data_);
}

TEST(Log, All) {
    std::vector<byte> key(6);
    std::memcpy(key.data(), "Hello", 6);
    std::vector<byte> data(6);
    std::memcpy(data.data(), "World", 6);
    {
        Log log(Log::Command::PUT, key, data);

        std::vector<byte> memory(log.GetByteLength());
        log.Serialize(memory.data(), log.GetByteLength());

        Log saved_log;
        saved_log.Deserialize(memory.data(), log.GetByteLength());

        ASSERT_EQ(log.command_, saved_log.command_);
        ASSERT_EQ(log.key_, saved_log.key_);
        ASSERT_EQ(log.value_, saved_log.value_);
    }
    {
        Log log(Log::Command::REMOVE, key);

        std::vector<byte> memory(log.GetByteLength());
        log.Serialize(memory.data(), log.GetByteLength());

        Log saved_log;
        saved_log.Deserialize(memory.data(), log.GetByteLength());

        ASSERT_EQ(log.command_, saved_log.command_);
        ASSERT_EQ(log.key_, saved_log.key_);
        ASSERT_EQ(log.value_, saved_log.value_);
    }

    {
        Log log(Log::Command::COMMIT);

        std::vector<byte> memory(log.GetByteLength());
        log.Serialize(memory.data(), log.GetByteLength());

        Log saved_log;
        saved_log.Deserialize(memory.data(), log.GetByteLength());

        ASSERT_EQ(log.command_, saved_log.command_);
        ASSERT_EQ(log.key_, saved_log.key_);
        ASSERT_EQ(log.value_, saved_log.value_);
    }
}
