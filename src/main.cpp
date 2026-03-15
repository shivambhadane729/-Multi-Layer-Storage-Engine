#include <iostream>
#include <vector>
#include <chrono>

#include "../include/cache/lru_cache.h"
#include "../include/ram_index/skiplist.h"
#include "../include/disk/bplus_tree.h"
#include "../include/os/mmap_manager.h"
#include "../include/os/memory_metrics.h"
#include "../include/workload/query_generator.h"

int main() {
    const size_t NUM_RECORDS = 100000;
    const size_t NUM_QUERIES = 50000;
    const size_t CACHE_SIZE = 5000;
    const double READ_RATIO = 0.8;
    const double ZIPF_ALPHA = 1.2;

    std::cout << "Adaptive Storage Engine Initialization" << std::endl;
    std::cout << "Records: " << NUM_RECORDS << ", Queries: " << NUM_QUERIES << std::endl;

    // 1. Initialize MMap
    size_t file_size = NUM_RECORDS * sizeof(Record);
    MMapManager mmap_manager("data/records.bin", file_size);
    if (!mmap_manager.map_file()) {
        std::cerr << "Failed to map file." << std::endl;
        return 1;
    }

    // 2. Pre-fill Disk Data and B+ Tree
    std::cout << "Populating data and B+ Tree Disk Index..." << std::endl;
    BPlusTree disk_index;
    for (size_t i = 1; i <= NUM_RECORDS; ++i) {
        Record r;
        r.id = i;
        size_t offset = (i - 1) * sizeof(Record);
        mmap_manager.write_record_at(offset, r);
        disk_index.insert(i, offset);
    }
    mmap_manager.flush(); // Ensure written to disk

    // 3. Initialize Workload
    WorkloadGenerator workload(NUM_RECORDS, READ_RATIO, ZIPF_ALPHA);
    auto queries = workload.generate(NUM_QUERIES);

    // Naive Engine simulation
    std::cout << "\n-----------------------" << std::endl;
    std::cout << "Running Naive Storage Engine (Direct Disk)" << std::endl;
    auto naive_start_metrics = MemoryMetrics::get_current_metrics();
    auto naive_start_time = std::chrono::high_resolution_clock::now();

    size_t disk_reads = 0;
    for (const auto& q : queries) {
        if (q.type == QueryType::READ) {
            uint64_t offset;
            if (disk_index.search(q.key, offset)) {
                Record* r = mmap_manager.get_record_at(offset);
                volatile uint64_t temp = r->id; // Read memory to trigger page fault
                disk_reads++;
            }
        }
    }

    auto naive_end_time = std::chrono::high_resolution_clock::now();
    auto naive_end_metrics = MemoryMetrics::get_current_metrics();
    double naive_duration = std::chrono::duration<double, std::milli>(naive_end_time - naive_start_time).count();
    
    std::cout << "Naive Execution Time: " << naive_duration << " ms" << std::endl;
    std::cout << "Avg Latency: " << (naive_duration / NUM_QUERIES) << " ms" << std::endl;
    std::cout << "Disk Hits: " << disk_reads << std::endl;
    MemoryMetrics::print_diff(naive_start_metrics, naive_end_metrics);


    // Adaptive Engine Simulation
    std::cout << "\n-----------------------" << std::endl;
    std::cout << "Running Adaptive Multi-Layer Engine" << std::endl;
    
    LRUCache lru_cache(CACHE_SIZE);
    SkipList ram_index(16, 0.5);

    auto adapt_start_metrics = MemoryMetrics::get_current_metrics();
    auto adapt_start_time = std::chrono::high_resolution_clock::now();

    size_t adapt_cache_hits = 0;
    size_t adapt_ram_hits = 0;
    size_t adapt_disk_hits = 0;

    for (const auto& q : queries) {
        if (q.type == QueryType::READ) {
            Record r;
            // 1. Check LRU Cache
            if (lru_cache.get(q.key, r)) {
                adapt_cache_hits++;
                continue;
            }

            // 2. Check RAM Index
            if (ram_index.search(q.key, r)) {
                adapt_ram_hits++;
                // Promote to Cache
                auto evict = lru_cache.put(q.key, r);
                // Currently discarding evicted from cache, in full engine could demote to RAM
                continue;
            }

            // 3. Check Disk Index
            uint64_t offset;
            if (disk_index.search(q.key, offset)) {
                Record* mapped_r = mmap_manager.get_record_at(offset);
                if (mapped_r) {
                    r = *mapped_r;
                    adapt_disk_hits++;
                    // Promote to RAM/Cache
                    ram_index.insert(q.key, r);
                }
            }
        } else {
            // Write handles updates in disk and evicts cache
            uint64_t offset;
            if (disk_index.search(q.key, offset)) {
                Record r;
                r.id = q.key;
                mmap_manager.write_record_at(offset, r);
            }
        }
    }

    auto adapt_end_time = std::chrono::high_resolution_clock::now();
    auto adapt_end_metrics = MemoryMetrics::get_current_metrics();
    double adapt_duration = std::chrono::duration<double, std::milli>(adapt_end_time - adapt_start_time).count();
    
    std::cout << "Adaptive Execution Time: " << adapt_duration << " ms" << std::endl;
    std::cout << "Avg Latency: " << (adapt_duration / NUM_QUERIES) << " ms" << std::endl;
    std::cout << "Cache Hits: " << adapt_cache_hits << std::endl;
    std::cout << "RAM Hits: " << adapt_ram_hits << std::endl;
    std::cout << "Disk Hits: " << adapt_disk_hits << std::endl;
    MemoryMetrics::print_diff(adapt_start_metrics, adapt_end_metrics);

    return 0;
}
