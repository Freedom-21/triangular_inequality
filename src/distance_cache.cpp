// distance_cache.cpp

#include "distance_cache.h"
// #include <stdio.h> // Removed as it's not needed
#include <cmath>    // For sqrt
#include <cassert>  // For assert (optional, for testing purposes)

// Initialize the distance cache
std::unordered_map<std::pair<int, int>, double, pair_hash> distance_cache;

// Initialize the object list
std::vector<obj_t*> obj_list;

// Initialize the feature to object IDs mapping
std::unordered_map<FEA_TYPE, std::vector<int>> fea_to_obj_ids_map;

// Function to compute Euclidean distance between two objects (2D)
double compute_distance(int id1, int id2) {
    // Adjust indexing if IDs are not zero-based
    assert(id1 > 0 && id1 <= static_cast<int>(obj_list.size()));
    assert(id2 > 0 && id2 <= static_cast<int>(obj_list.size()));

    const obj_t* obj1 = obj_list[id1 - 1];
    const obj_t* obj2 = obj_list[id2 - 1];
    
    // Ensure MBR is not null
    assert(obj1->MBR != nullptr && obj2->MBR != nullptr);

    double dx = static_cast<double>(obj1->MBR->min) - static_cast<double>(obj2->MBR->min);
    double dy = static_cast<double>(obj1->MBR->max) - static_cast<double>(obj2->MBR->max);
    
    return std::sqrt(dx * dx + dy * dy);
}

// Function to compute or retrieve cached distance
double compute_distance_cached(int id1, int id2) {
    // Replace std::minmax with manual ordering to avoid compiler errors
    std::pair<int, int> key = (id1 < id2) ? std::make_pair(id1, id2) : std::make_pair(id2, id1);
    
    // Check if distance is already cached
    auto it = distance_cache.find(key);
    if (it != distance_cache.end()) {
        return it->second;
    }
    
    // Compute the distance as it's not cached
    double dist = compute_distance(id1, id2);
    
    // Store the computed distance in the cache
    distance_cache[key] = dist;
    
    return dist;
}

// Function to initialize the feature to object IDs mapping
void initialize_fea_to_obj_id_map(data_t* data_v) {
    for (int i = 0; i < data_v->obj_n; i++) {
        FEA_TYPE fea = data_v->obj_v[i].fea;
        int obj_id = data_v->obj_v[i].id;
        fea_to_obj_ids_map[fea].push_back(obj_id);
    }
}

// Function to get all object IDs by feature
std::vector<int> get_obj_ids_by_fea(FEA_TYPE fea) {
    auto it = fea_to_obj_ids_map.find(fea);
    if (it != fea_to_obj_ids_map.end()) {
        return it->second;
    }
    return {}; // Returns an empty vector if not found
}
