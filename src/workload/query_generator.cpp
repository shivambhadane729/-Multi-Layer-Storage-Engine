#include "../../include/workload/query_generator.h"
#include <cmath>
#include <algorithm>
#include <iostream>

WorkloadGenerator::WorkloadGenerator(size_t num_records, double read_ratio, double zipf_alpha)
    : num_records_(num_records), read_ratio_(read_ratio), zipf_alpha_(zipf_alpha),
      rng_(std::random_device{}()), real_dist_(0.0, 1.0) {
    std::cout << "Precomputing Zipf distribution (alpha=" << zipf_alpha_ << ")..." << std::endl;
    precompute_zipf();
}

void WorkloadGenerator::precompute_zipf() {
    cdf_.reserve(num_records_ + 1);
    double c = 0.0;
    
    // Calculate normalization constant
    for (size_t i = 1; i <= num_records_; ++i) {
        c = c + (1.0 / std::pow(static_cast<double>(i), zipf_alpha_));
    }
    c = 1.0 / c;
    
    double sum_prob = 0.0;
    cdf_.push_back(0.0);
    for (size_t i = 1; i <= num_records_; ++i) {
        sum_prob += c * (1.0 / std::pow(static_cast<double>(i), zipf_alpha_));
        cdf_.push_back(sum_prob);
    }
}

std::vector<Query> WorkloadGenerator::generate(size_t num_queries) {
    std::vector<Query> queries;
    queries.reserve(num_queries);
    
    for (size_t i = 0; i < num_queries; ++i) {
        Query q;
        
        // Determine Read vs Write
        if (real_dist_(rng_) < read_ratio_) {
            q.type = QueryType::READ;
        } else {
            q.type = QueryType::WRITE;
        }
        
        // Generate Zipf key using CDF inversion
        double rnd = real_dist_(rng_);
        auto it = std::lower_bound(cdf_.begin(), cdf_.end(), rnd);
        size_t rank = std::distance(cdf_.begin(), it);
        
        if (rank == 0) rank = 1;
        if (rank > num_records_) rank = num_records_;
        
        q.key = rank; // Keys correspond to a 1-based rank
        q.value = 1000 + rank; // Dummy value logic
        
        queries.push_back(q);
    }
    return queries;
}
