#include "../../include/os/memory_metrics.h"
#include <sys/resource.h>
#include <fstream>
#include <iostream>
#include <string>

// Helper to parse /proc/meminfo for SwapFree and SwapTotal
long get_swap_used() {
    std::ifstream meminfo("/proc/meminfo");
    std::string line;
    long swap_total = 0, swap_free = 0;
    while (std::getline(meminfo, line)) {
        if (line.find("SwapTotal:") == 0) {
            sscanf(line.c_str(), "SwapTotal: %ld kB", &swap_total);
        } else if (line.find("SwapFree:") == 0) {
            sscanf(line.c_str(), "SwapFree: %ld kB", &swap_free);
        }
    }
    return swap_total - swap_free;
}

MetricsSnapshot MemoryMetrics::get_current_metrics() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    
    MetricsSnapshot snapshot;
    snapshot.minflt = usage.ru_minflt; // Minor page faults (soft)
    snapshot.majflt = usage.ru_majflt; // Major page faults (hard - required disk I/O)
    snapshot.swap_used_kb = get_swap_used();
    
    return snapshot;
}

void MemoryMetrics::print_diff(const MetricsSnapshot& start, const MetricsSnapshot& end) {
    long minflt_diff = end.minflt - start.minflt;
    long majflt_diff = end.majflt - start.majflt;
    long swap_diff = end.swap_used_kb - start.swap_used_kb;
    
    std::cout << "--- OS Memory Metrics ---" << std::endl;
    std::cout << "Minor Page Faults: " << minflt_diff << std::endl;
    std::cout << "Major Page Faults: " << majflt_diff << std::endl;
    std::cout << "Swap Usage Change: " << swap_diff << " kB" << std::endl;
    std::cout << "-------------------------" << std::endl;
}
