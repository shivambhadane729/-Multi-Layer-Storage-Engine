#pragma once
#include <string>
#include <cstddef>
#include "../common/record.h"

class MMapManager {
public:
    MMapManager(const std::string& filepath, size_t file_size);
    ~MMapManager();

    bool map_file();
    void unmap_file();
    void flush();

    // Gets a pointer directly into the mmapped area
    Record* get_record_at(size_t offset);
    
    // Writes a record directly to the mmapped area
    void write_record_at(size_t offset, const Record& record);

private:
    std::string filepath_;
    size_t file_size_;
    int fd_;
    void* mapped_data_;
};
