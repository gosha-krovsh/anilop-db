#include "node.h"

Node::Node() {}

bool Node::isLeaf() { return child_nodes_.size() == 0; }

void Node::SetPageNum(uint64_t page_num) {
    page_num_ = page_num;
}

uint64_t Node::GetPageNum() const { 
    return page_num_; 
}

size_t Node::Serialize(byte* data, size_t max_volume) { 
    if (max_volume != settings::kPageSize) {
        throw dal_error::CorruptedBuffer("Buffer size is too low for serialisation."); 
    }
    // Serialize leaf status
    char leaf_bit = 0; 
    if (isLeaf()) {
        leaf_bit = 1;
    }
    data[0] = leaf_bit;
    data += 1;
    // Serialize len of key-value pairs
    memory::uint16_to_bytes(data, static_cast<uint16_t>(items_.size()));
    data += 2;
    // Serialize header and item footer
    char* left_ptr = data;
    char* right_ptr = data + max_volume;
    for (size_t i = 0; i < items_.size(); ++i) {
        std::shared_ptr<Item> item = items_[i];
        
        if (!isLeaf()) {  // Serialize child node page_num 
            uint64_t child_node = child_nodes_[i];
            memory::uint64_to_bytes(data, child_node);
            left_ptr += uint64_t_size;
        }

        // Serialize offset
        size_t offset = item->ByteLength();
        memory::uint64_to_bytes(left_ptr, offset);
        left_ptr += uint64_t_size;
        // Serialize item
        right_ptr -= offset;
        item->Serialize(right_ptr, item->ByteLength());
    }
    // Serialize last child
    if (!isLeaf()) {
        memory::uint64_to_bytes(data, child_nodes_.back());
    }

    return settings::kPageSize;
}

size_t Node::Deserialize(const byte* data, size_t max_volume) { 
    if (max_volume != settings::kPageSize) {
        throw dal_error::CorruptedBuffer("Buffer size is too low for serialisation."); 
    }
    items_.clear();
    child_nodes_.clear(); 
    // Deserialize leaf status
    char leaf_bit = 0; 
    leaf_bit = data[0];
    
    data += 1;
    max_volume -= 1;
    // Deserialize len of key-value pairs
    uint16_t items_size = memory::bytes_to_uint16(data);
    data += 2;
    max_volume -= 2;
    // Deserialize header and item footer
    const char* left_ptr = data;
    const char* right_ptr = data + max_volume;
    for (size_t i = 0; i < items_size; ++i) {
        Item* item = new Item;
        
        if (leaf_bit == 0) {  // Derialize child node page_num 
            child_nodes_.emplace_back(memory::bytes_to_uint64(data));
            left_ptr += uint64_t_size;
        }

        // Deserialize offset
        size_t offset = memory::bytes_to_uint64(left_ptr);
        left_ptr += uint64_t_size;
        // Deserialize item
        right_ptr -= offset;
        item->Deserialize(right_ptr, offset);

        items_.emplace_back(item);
    }
    // Deserialize last child
    if (leaf_bit == 0) {
        child_nodes_.emplace_back(memory::bytes_to_uint64(left_ptr));
    }

    return settings::kPageSize;    
}
