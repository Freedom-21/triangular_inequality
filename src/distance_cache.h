// distance_cache.h

#ifndef DISTANCE_CACHE_H
#define DISTANCE_CACHE_H

#include "data_struct.h" // Include the data structures
#include "data_utility.h"
#include <unordered_map>
#include <utility>
#include <vector>
#include <algorithm> // For std::min and std::max

// Remove the typedef to avoid duplicate specifier
// typedef unsigned int FEA_TYPE;

// Custom hash function for unordered pairs of integers
struct pair_hash {
    std::size_t operator() (const std::pair<int, int>& p) const {
        // Order the pair to ensure (A, B) and (B, A) are treated the same
        int first = std::min(p.first, p.second);
        int second = std::max(p.first, p.second);
        // Combine the hashes of the two integers
        return std::hash<int>()(first) ^ (std::hash<int>()(second) << 16);
    }
};

// Declare the distance cache
extern std::unordered_map<std::pair<int, int>, double, pair_hash> distance_cache;

// Function to compute cached distance between two object IDs
double compute_distance_cached(int id1, int id2);

// Declare the object list
extern std::vector<obj_t*> obj_list;

// Declare the feature to object IDs mapping
extern std::unordered_map<FEA_TYPE, std::vector<int>> fea_to_obj_ids_map;

// Function to initialize the feature to object IDs mapping
void initialize_fea_to_obj_id_map(data_t* data_v);

// Function to get all object IDs by feature
std::vector<int> get_obj_ids_by_fea(FEA_TYPE fea);

#endif // DISTANCE_CACHE_H
