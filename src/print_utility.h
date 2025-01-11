#include "improved.h"
#include <cstddef>
#include <iostream>
#include <fstream>
#include <memory>  // For unique_ptr
#include <vector>
#include <algorithm>
#include "data_struct.h"
#include <set>

bool is_valid_combination(const std::vector<obj_t*>& colocated_objects, obj_t* new_obj, B_KEY_TYPE dist_thr);
void print_combined_obj_set(const combined_obj_set_t* combinedSet);
std::string create_combination_key(const std::vector<obj_t*>& objects);