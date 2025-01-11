#ifndef improved_h
#define improved_h

#include <stdio.h>
#include "data_utility.h"
#include "rtree.h"
#include "data_struct_operation.h"


extern bool debug_mode;
extern RTree_t RTree_v;
extern bst_t* IF_v;

// extern int cost_tag;
extern double fea_highest_freq;
extern float dist_thr;
extern float min_pi;


/*------------------------ Apriori ------------------------ */

fsi_set_t** apriori(int alg_opt, int numOfObj, int numOfFea, double dist_thr);

fsi_set_t* const_L1_apriori();

FEA_TYPE join_check( fsi_t* fsi_v1, fsi_t* fsi_v2);

bool all_subsets_exist(fsi_set_t* fsi_set_v, fsi_t* fsi_v);
bool all_subsets_exist_with_distance(fsi_set_t* fsi_set_v, fsi_t* fsi_v, double dist_thr);

/*------------------------ Support Computation ------------------------ */

B_KEY_TYPE comp_support(int alg_opt, fsi_t* fsi_v, int numOfObj);

B_KEY_TYPE comp_support_partitioning(int alg_opt, fsi_t* fsi_v, int numOfObj);

B_KEY_TYPE comp_support_construction(int alg_opt, fsi_t* fsi_v, int numOfObj);

B_KEY_TYPE comp_support_enum(int alg_opt, fsi_t* fsi_v, int numOfObj);

void enum_sub(bst_t* IF_v, obj_set_t* S_0, obj_t* o, int& cnt);

B_KEY_TYPE comp_support_fraction(int alg_opt, fsi_t* fsi_v, int numOfObj, bool* RI);

/*------------------------ RI ------------------------ */

bool check_row_instance(int alg_opt,   fsi_t* fsi_v, obj_t* obj_v, bool* RI);

bool combinatorial( fsi_t* fsi_v, obj_t* obj_v, obj_set_t* S2, bool* RI);

obj_set_t* combinatorial_sub( bst_t* IF_v, obj_set_t* S_0, obj_t* o, B_KEY_TYPE d);

bool bst_check_plist_obj_n(bst_node_t* bst_node_v);

bool bst_check_plist(bst_t* bst_v, fsi_t* fsi_v, FEA_TYPE fea);

bool dia( fsi_t* fsi_v, obj_t* obj_v, obj_set_t* S2, bool* RI);

bool mck(fsi_t* fsi_v, obj_t* obj_v, obj_set_t* S2, bool* RI);

// Method 4

bool filter_and_verify(int alg_opt2,  fsi_t* fsi_v, obj_t* obj_v, bool* RI);

bool check_Nof_feasibility(fsi_t* fsi_v, obj_t* obj_v);

bool check_Nof2_feasibility(fsi_t* fsi_v, obj_t* obj_v);

int pruning_NNset( query_t* q, obj_t* obj_v);

/*------------------------ FractionScore ------------------------ */

B_KEY_TYPE min_frac_receive(fsi_t* fsi_v, obj_t* obj_v);

void precomputation(data_t* data_v, KEY_TYPE dist_thr);

#endif /* improved_h */
