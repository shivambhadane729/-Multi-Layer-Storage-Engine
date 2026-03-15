#pragma once
#include <cstdint>

struct MetricsSnapshot {
    long minflt;  // Minor page faults (RAM hits)
    long majflt;  // Major page faults (Disk hits)
    long swap_used_kb; // Swap usage
};

class MemoryMetrics {
public:
    static MetricsSnapshot get_current_metrics();
    static void print_diff(const MetricsSnapshot& start, const MetricsSnapshot& end);
};
