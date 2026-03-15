#pragma once
#include <cstdint>

// Fixed size record to simulate a database row or page (256 bytes)
struct Record {
    uint64_t id;
    char data[248];
};
