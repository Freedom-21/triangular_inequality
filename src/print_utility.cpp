// #include "print_utility.h"
// #include <unordered_set>

// // Auxiliary function to print combined object sets
// void print_combined_obj_set(const combined_obj_set_t* combinedSet) {
//     if (combinedSet == nullptr || combinedSet->colocated_objects.empty()) {
//         std::cout << "Combined object set is empty." << std::endl;
//         return;
//     }

//     std::cout << "Colocated objects: ";
//     for (const auto& obj : combinedSet->colocated_objects) {
//         std::cout << obj->id << " ";
//     }
//     std::cout << std::endl;
// }

// // Auxiliary function to check if adding a new object forms a valid combination
// bool is_valid_combination(const std::vector<obj_t*>& colocated_objects, obj_t* new_obj, B_KEY_TYPE dist_thr) {
//     if (colocated_objects.empty()) {
//         return true; // If no colocated objects, it's trivially a valid combination
//     }

//     // Create a temporary obj_set_t to store colocated objects
//     obj_set_t* obj_set_v = alloc_obj_set();
//     for (const auto& obj : colocated_objects) {
//         add_obj_set_entry(obj, obj_set_v);
//     }

//     // Use the first colocated object as the reference for distance checking
//     obj_t* reference_obj = colocated_objects.front();

//     // Check if adding the new object forms a valid combination
//     bool is_valid = check_dist_constraint(obj_set_v, new_obj, reference_obj, dist_thr);

//     // Free the allocated obj_set_t after use
//     release_obj_set(obj_set_v);

//     return is_valid;
// }
// // Define a helper function to create a unique key for each combination
// std::string create_combination_key(const std::vector<obj_t*>& objects) {
//     std::vector<int> ids;
//     for (const auto& obj : objects) {
//         ids.push_back(obj->id);
//     }
//     std::sort(ids.begin(), ids.end()); // Sort the IDs to ensure the key is consistent
//     std::string key;
//     for (const auto& id : ids) {
//         key += std::to_string(id) + "-";
//     }
//     return key;
// }