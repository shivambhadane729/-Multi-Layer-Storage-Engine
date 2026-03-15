#include "../../include/os/mmap_manager.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

MMapManager::MMapManager(const std::string& filepath, size_t file_size)
    : filepath_(filepath), file_size_(file_size), fd_(-1), mapped_data_(MAP_FAILED) {}

MMapManager::~MMapManager() {
    unmap_file();
}

bool MMapManager::map_file() {
    fd_ = open(filepath_.c_str(), O_RDWR | O_CREAT, 0666);
    if (fd_ == -1) {
        std::cerr << "Error opening file for mmap: " << filepath_ << std::endl;
        return false;
    }

    // Ensure the file is large enough
    struct stat st;
    if (fstat(fd_, &st) == 0 && st.st_size < file_size_) {
        // Expand file if necessary
        if (ftruncate(fd_, file_size_) != 0) {
            std::cerr << "Error truncating file." << std::endl;
            close(fd_);
            return false;
        }
    }

    mapped_data_ = mmap(nullptr, file_size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
    if (mapped_data_ == MAP_FAILED) {
        std::cerr << "Error mmapping the file." << std::endl;
        close(fd_);
        return false;
    }
    return true;
}

void MMapManager::unmap_file() {
    if (mapped_data_ != MAP_FAILED) {
        munmap(mapped_data_, file_size_);
        mapped_data_ = MAP_FAILED;
    }
    if (fd_ != -1) {
        close(fd_);
        fd_ = -1;
    }
}

void MMapManager::flush() {
    if (mapped_data_ != MAP_FAILED) {
        msync(mapped_data_, file_size_, MS_SYNC);
    }
}

Record* MMapManager::get_record_at(size_t offset) {
    if (mapped_data_ == MAP_FAILED || offset + sizeof(Record) > file_size_) {
        return nullptr;
    }
    return reinterpret_cast<Record*>(static_cast<char*>(mapped_data_) + offset);
}

void MMapManager::write_record_at(size_t offset, const Record& record) {
    if (mapped_data_ != MAP_FAILED && offset + sizeof(Record) <= file_size_) {
        std::memcpy(static_cast<char*>(mapped_data_) + offset, &record, sizeof(Record));
    }
}
