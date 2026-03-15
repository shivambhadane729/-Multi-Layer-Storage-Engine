#pragma once
#include <vector>
#include <cstdint>
#include <random>

enum class QueryType {
    READ,
    WRITE
};

struct Query {
    QueryType type;
    uint64_t key;
    uint64_t value; // Dummy value for WRITE queries
};

class WorkloadGenerator {
public:
    WorkloadGenerator(size_t num_records, double read_ratio, double zipf_alpha);
    
    // Generate a list of queries
    std::vector<Query> generate(size_t num_queries);

private:
    size_t num_records_;
    double read_ratio_;
    double zipf_alpha_;

    std::mt19937 rng_;
    std::uniform_real_distribution<double> real_dist_;
    
    // Precompute probabilities for Zipf distribution
    std::vector<double> cdf_;
    void precompute_zipf();
};
