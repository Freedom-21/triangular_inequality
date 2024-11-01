
#include "frac.h"
#include "distance_cache.h"
#include "data_struct.h" // Ensure this is included if not already
#include <cassert>
#include <limits>


int determine_max_feature_id(data_t* data_v);

/*
 *  [Apriori]
 */
fsi_set_t** apriori(int alg_opt, int numOfObj, int numOfFea, double dist_thr)
{
    printf("Starting Apriori with alg_opt=%d, numOfObj=%d, numOfFea=%d, dist_thr=%.5f\n", alg_opt, numOfObj, numOfFea, dist_thr);
    int i, j;
    fsi_set_t** result; // storing overall result
    fsi_set_t* fsi_set_cur; // prev = L_{k-1}, cur = L_{k}
    fsi_t *fsi_v1, *fsi_v2, *fsi_v3;

    FEA_TYPE fea_v1;
    B_KEY_TYPE sup;

    i = 0;
    // Initialize the structure for storing L_1, L_2, ..., L_{|F|}
    result = (fsi_set_t**)malloc(numOfFea * sizeof(fsi_set_t*));
    memset(result, 0, numOfFea * sizeof(fsi_set_t*));

    /*s*/
    stat_v.memory_v += numOfFea * sizeof(fsi_set_t*);
    if (stat_v.memory_v > stat_v.memory_max)
        stat_v.memory_max = stat_v.memory_v;
    /*s*/

#ifndef WIN32
    struct rusage query_sta, query_end;
    float sys_t, usr_t, usr_t_sum = 0;
    GetCurTime(&query_sta);
#endif

    // L_1
    printf("Constructing L_1...\n");
    result[0] = const_L1_apriori();

#ifndef WIN32
    GetCurTime(&query_end);

    GetTime(&query_sta, &query_end, &usr_t, &sys_t);
    printf("L_1 time:%0.5lf\n", usr_t);
    GetCurTime(&query_sta);
#endif

    // L_(i)
    for (i = 1; i < numOfFea; i++) {
    printf("Processing Level L_%d\n", i + 1);
    fsi_v1 = result[i -1]->head->next;
    if (fsi_v1 == NULL) {
        printf("No more frequent itemsets at Level L_%d. Terminating.\n", i + 1);
        break;
    }
    fsi_set_cur = alloc_fsi_set();

    // Iterate over all fsi_v1 in L_{k-1}
    while (fsi_v1 != NULL) {
        fsi_v2 = fsi_v1->next;
        while (fsi_v2 != NULL) {
            // Join step
            if ((fea_v1 = join_check(fsi_v1, fsi_v2)) != -1) {
                printf("Joining fsi_set with features:");
                for(int f = 0; f < fsi_v1->fea_n; f++) printf(" %d", fsi_v1->feaset[f]);
                printf(" and ");
                for(int f = 0; f < fsi_v2->fea_n; f++) printf(" %d", fsi_v2->feaset[f]);
                printf(" to form feature %d\n", fea_v1);

                // Create the candidate feature set by adding the new feature
                fsi_v3 = add_fsi(fsi_v1, fea_v1);

                // Prune step
                if (i > 1 && !all_subsets_exist_with_distance(result[i -1], fsi_v3, dist_thr)) {
                    printf("Pruning candidate set with features:");
                    for(int f = 0; f < fsi_v3->fea_n; f++) printf(" %d", fsi_v3->feaset[f]);
                    printf("\n");

                    /* Update memory statistics */
                    stat_v.memory_v -= sizeof(fsi_t) + fsi_v3->fea_n * sizeof(FEA_TYPE);
                    /* Update memory statistics */

                    release_fsi(fsi_v3);
                    fsi_v2 = fsi_v2->next;
                    continue;
                }

                // Support calculation
                sup = comp_support(alg_opt, fsi_v3, numOfObj);
                printf("Computed support for candidate set:");
                for(int f = 0; f < fsi_v3->fea_n; f++) printf(" %d", fsi_v3->feaset[f]);
                printf(" is %.5lf\n", sup);

                if (sup >= min_sup) {
                    printf("Adding candidate set to Level L_%d\n", i + 1);
                    add_fsi_set_entry(fsi_set_cur, fsi_v3);
                } else {
                    printf("Candidate set does not meet min_sup and is discarded.\n");
                    /* Update memory statistics */
                    stat_v.memory_v -= sizeof(fsi_t) + fsi_v3->fea_n * sizeof(FEA_TYPE);
                    /* Update memory statistics */

                    release_fsi(fsi_v3);
                }
            }
            fsi_v2 = fsi_v2->next;
        }

        // Move to the next fsi_v1 in L_{k-1}
        fsi_v1 = fsi_v1->next;
    }

#ifndef WIN32
    GetCurTime(&query_end);
    GetTime(&query_sta, &query_end, &usr_t, &sys_t);
    printf("L_%d \ttime:%0.5lf\t fsi_n:%d\n", i + 1, usr_t, fsi_set_cur->fsi_n);
    GetCurTime(&query_sta);
#endif

    // Assign the current level's feature sets to the result
    result[i] = fsi_set_cur;
}

printf("Apriori algorithm completed.\n");
return result;
}


/*
 *  Construct the L1 feature set.
 *
 *
 */
fsi_set_t* const_L1_apriori()
{
    fsi_set_t* fsi_set_v;
    fsi_t* fsi_v;
    bst_node_t* bst_node_v;
    B_KEY_TYPE sup;

    fsi_set_v = alloc_fsi_set();

    // for each feature
    bst_node_v = bst_get_min(IF_v->root);
    while (bst_node_v != NULL) {
        fsi_v = alloc_fsi(1);
        fsi_v->feaset[0] = bst_node_v->key;

        if (cost_tag == 2) {
            // L1 is number of objects with the feature
            sup = bst_node_v->p_list_obj->obj_n;
            sup = sup / fea_highest_freq; // normalization
        } else {
            sup = 1; // by definition
        }

        fsi_v->sup = sup;

        if (sup >= min_sup)
            add_fsi_set_entry(fsi_set_v, fsi_v);
        else
            release_fsi(fsi_v);

        bst_node_v = bst_successor(bst_node_v);
    }

    return fsi_set_v;
}

/*
 *	The checking for the join step.
 *
 *	Check whether two sets @fsi_v1 and @fsi_v2
 *	share s-1 features, where s is the number of features in @fsi_v1 or
 *@fsi_v2.
 *
 *	return  -1 if the join check fails,
 *	otherwise, return the (only) feature in fsi_v2 that is not contained in
 *fsi_v1.
 *
 */
FEA_TYPE join_check(fsi_t* fsi_v1, fsi_t* fsi_v2)
{
    FEA_TYPE fea_v;

    fea_v = -1;

    if (fsi_v1->fea_n != fsi_v2->fea_n)
        return -1;

    if (memcmp(fsi_v1->feaset, fsi_v2->feaset, (fsi_v1->fea_n - 1) * sizeof(FEA_TYPE)) == 0) {
        fea_v = fsi_v2->feaset[fsi_v2->fea_n - 1];
    }

    return fea_v;
}

/*
 *  The checking for the prune step.
 *
 *  return true if all subsets of @fsi_v appear in @fsi_set_v.
 *
 *  Method:
 *  ignore one of the feature one by one to form the k-1 size feature set
 *  thus, number of k-1 size feature set = k
 *
 *  for each such feature set, we check whether it is in @fsi_set_v
 *  if not, return false
 */
// all_subsets_exist_with_distance.cpp

// #include "distance_cache.h"
// #include "data_struct.h" // Ensure this is included if not already
// #include <vector>
// #include <cassert>

// Function to check if all (k-1)-sized subsets exist and satisfy distance constraints
bool all_subsets_exist_with_distance(fsi_set_t* fsi_set_v, fsi_t* fsi_v, double dist_thr) {
    // Iterate over each feature to generate subsets
    for (int i = 0; i < fsi_v->fea_n; i++) {
        std::vector<FEA_TYPE> subset;
        for (int j = 0; j < fsi_v->fea_n; j++) {
            if (j != i)
                subset.push_back(fsi_v->feaset[j]);
        }

        bool subset_exists = false;

        // Iterate through existing frequent itemsets
        fsi_t* current_fsi = fsi_set_v->head->next;
        while (current_fsi != NULL) {
            bool match = true;
            for (auto fea : subset) {
                bool found = false;
                for (int k = 0; k < current_fsi->fea_n; k++) {
                    if (current_fsi->feaset[k] == fea) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    match = false;
                    break;
                }
            }

            if (match) {
                // Implement triangular inequality-based distance checks here
                bool distance_ok = true;
                // Example: Compute or retrieve distance bounds
                // If any inferred distance exceeds dist_thr, set distance_ok to false
                // ...

                // For debugging, assume distance_ok is true
                distance_ok = true; // Placeholder

                if (distance_ok) {
                    subset_exists = true;
                    break; // Found a valid subset with distance constraints
                }
            }

            current_fsi = current_fsi->next;
        }

        if (!subset_exists) {
            printf("Pruning candidate set with features:");
            for(auto fea : subset) printf(" %d", fea);
            printf("\n");
            return false; // Prune the candidate as this subset doesn't exist or fails distance constraints
        }
    }

    return true; // All subsets exist and satisfy distance constraints
}


bool all_subsets_exist(fsi_set_t* fsi_set_v, fsi_t* fsi_v)
{
    int i, x, y;
    fsi_t* fsi_temp;

    // i is the pos to be igore to have k-1 size
    for (i = 0; i < fsi_v->fea_n; i++) {
        // check whether fsi_v\[i] exist in fsi_set_v
        fsi_temp = fsi_set_v->head->next;
        while (fsi_temp != NULL) {
            // check whether fsi_temp == fsi_v\[i]
            for (x = 0; x < fsi_v->fea_n; x++) {
                if (x == i)
                    continue;
                for (y = 0; y < fsi_temp->fea_n; y++) {
                    if (fsi_temp->feaset[y] == fsi_v->feaset[x])
                        break;
                }
                if (y == fsi_temp->fea_n) { // cannot find
                    break;
                }
            }
            if (x == fsi_v->fea_n) // fsi_temp matches fsi_v\[i]
            {
                break;
            }
            fsi_temp = fsi_temp->next;
        }
        if (fsi_temp == NULL) // no matches
        {
            return false;
        }
    }

    return true;
}



//---------------------------------------------------------


// B_KEY_TYPE comp_support(int alg_opt, fsi_t* fsi_v, int numOfObj)
// {
//     return comp_support(alg_opt, fsi_v, numOfObj);
// }

// /*
//  * The implementaiton of "SupportComputation" in the paper.
//  *  @fsi_v = current feature set C
//  *
//  *  Output: @sup = sup(C)
//  */
B_KEY_TYPE comp_support(int alg_opt, fsi_t* fsi_v, int numOfObj)
{
    B_KEY_TYPE sup = std::numeric_limits<B_KEY_TYPE>::infinity();
    B_KEY_TYPE sup_C_f = 0;

    // Allocate and initialize the Row Instance (RI) array
    bool* RI = new bool[numOfObj]();

    // Iterate over each feature in the feature set
    for (int i = 0; i < fsi_v->fea_n; i++) {
        sup_C_f = 0;
        FEA_TYPE current_fea = fsi_v->feaset[i];

        // Retrieve the inverted list for the current feature
        struct bst_node* bst_node = bst_search(IF_v, current_fea);
        if (bst_node == nullptr) {
            printf("Feature %d not found in inverted index.\n", current_fea);
            sup_C_f = 0.0;
        } else {
            obj_set_t* obj_set_v = bst_node->p_list_obj;
            obj_node_t* obj_node_v = obj_set_v->head->next;

            // Iterate over each object in the inverted list
            while (obj_node_v != NULL) {
                int obj_id = obj_node_v->obj_v->id - 1;
                assert(obj_id >= 0 && obj_id < numOfObj);
                bool flag = RI[obj_id];

                if (flag || check_row_instance(alg_opt, fsi_v, obj_node_v->obj_v, RI)) {
                    sup_C_f += 1.0; // Each valid row instance counts as 1
                }

                obj_node_v = obj_node_v->next;
            }

            // Normalize support by the number of objects containing the feature
            if (cost_tag == 1) {
                sup_C_f = sup_C_f / static_cast<B_KEY_TYPE>(obj_set_v->obj_n);
                printf("Feature %d: sup_C_f = %.5lf / %d = %.5lf\n", current_fea, sup_C_f * obj_set_v->obj_n, obj_set_v->obj_n, sup_C_f);
            }
        }

        // Maintain the smallest support across features
        if (sup_C_f <= sup) {
            sup = sup_C_f;
            printf("Updated minimum support to %.5lf\n", sup);

            // Early stopping if support falls below the threshold
            if (cost_tag == 1 && sup < min_sup) {
                printf("Early stopping: Support %.5lf < min_sup %.5lf\n", sup, min_sup);
                break;
            }
        }
    }

    // Assign the computed support to the feature set
    fsi_v->sup = sup;

    // Clean up the RI array
    delete[] RI;

    // Debug: Print final support
    printf("Final support for feature set:");
    for(int f = 0; f < fsi_v->fea_n; f++) printf(" %d", fsi_v->feaset[f]);
    printf(" is %.5lf\n", sup);

    return sup;
}


/*
 *  Perform check row instance operation
 *  @alg_opt:
 *  1 = combinational
 *  41 = filter and verify + 1
 *  @fsi_v: the current checking feature set C
 *  @obj_v: the current checking object
 */
bool check_row_instance(int alg_opt, fsi_t* fsi_v, obj_t* obj_v, bool* RI)
{
#ifndef WIN32
    float sys_t, S4_t = 0;
    struct rusage S4_sta, S4_end;
    GetCurTime(&S4_sta);
#endif

    bool flag = true;
    if (alg_opt == 1) // combinatorial
    {
        flag = combinatorial(fsi_v, obj_v, NULL, RI);
    } else // filter-and-verification
    {
        flag = filter_and_verify(alg_opt - 40, fsi_v, obj_v, RI);
    }

#ifndef WIN32
    GetCurTime(&S4_end);
    GetTime(&S4_sta, &S4_end, &S4_t, &sys_t);
    stat_v.S4_time += S4_t;
#endif

    return flag;
}

/*
 *  Method 1 : Combinatorial Approach
 *
 *  if we call this function from method 4, S2 = range_query(disk_v,q) to save
 * some time
 *  else S2 = NULL
 */
bool combinatorial(fsi_t* fsi_v, obj_t* obj_v, obj_set_t* S2, bool* RI)
{
    bst_t* inverted_list;
    obj_set_t *S_0, *S = NULL;
    obj_set_t* obj_set_v;
    loc_t* loc_v;
    disk_t* disk_v;
    query_t* q;

    // Step 1. Range query.

    loc_v = get_obj_loc(obj_v);
    disk_v = alloc_disk(IRTree_v.dim);
    set_disk(disk_v, loc_v, dist_thr);

    q = alloc_query();
    q->loc_v = loc_v;
    q->psi_v = fsi_to_psi(fsi_v, obj_v->fea);

    if (S2 == NULL) {
        obj_set_v = range_query(disk_v, q);

        // Step 2. Construct an IF structure.
        inverted_list = const_IF(obj_set_v, q->psi_v);

        release_obj_set(obj_set_v);
        release_disk(disk_v);
        release_query(q);

        // return false if any key is missing
        if (!bst_check_plist_obj_n(inverted_list->root)) {
            release_IF(inverted_list);
            return false;
        }
    } else {
        inverted_list = const_IF(S2, q->psi_v);
        release_obj_set(S2);
        release_disk(disk_v);
        release_query(q);
    }

    // Initialize the S_0.
    S_0 = alloc_obj_set();

    // Invoke the sub-procedure "recursively".
    S = combinatorial_sub(inverted_list, S_0, obj_v, dist_thr);

    // Release the resources.
    release_obj_set(S_0);
    release_IF(inverted_list);
    if (S != NULL && S->obj_n != 0) {
        //        add_obj_set_entry(obj_v, S);
        //        printf("S5:%f\t%f\n",comp_diameter(S), dist_thr);
        //---
        // update RI
        {
#ifndef WIN32
            float sys_t, S3_t = 0;
            struct rusage S3_sta, S3_end;
            GetCurTime(&S3_sta);
#endif
            obj_node_t* obj_node_v;
            obj_node_v = S->head->next;
            while (obj_node_v != NULL) {
                RI[obj_node_v->obj_v->id - 1] = true;
                obj_node_v = obj_node_v->next;
            }

#ifndef WIN32
            GetCurTime(&S3_end);
            GetTime(&S3_sta, &S3_end, &S3_t, &sys_t);
            stat_v.S3_time += S3_t;
#endif
        }
        //---
        release_obj_set(S);
        return true;
    }

    release_obj_set(S);
    return false;
}

/*
 *	The sub-procedure of combinatorial approach.
 *
 * we process as follows: for each object o in the root,
 *  we update the IF s.t. the keywords in o are removed (note that the root must be removed), then o is added to S_0, then we recursive call the function to find another object in the (new) root and update IF and add to S_0...., until the IF is empty, then S covers all the keywords.
 * note that if we do not consider the distance constraint, we can always find a feaisble set (e.g., by adding all objects in IF into S). When we consider the distance constraint, we may return NULL.
 */
obj_set_t* combinatorial_sub(bst_t* IF_v, obj_set_t* S_0, obj_t* o, B_KEY_TYPE d)
{
    obj_t* obj_v;
    obj_set_t* S;
    bst_node_t* bst_node_v;
    obj_node_t* obj_node_v;
    bst_node_list_t* bst_node_list_v;

    if (IF_v->node_n == 0)
        return alloc_obj_set(); // An empty one.

    bst_node_v = IF_v->root;
    obj_node_v = bst_node_v->p_list_obj->head->next;
    while (obj_node_v != NULL) {
        // Pick an object.
        obj_v = obj_node_v->obj_v;

        // Distance constraint checking.
        if (!check_dist_constraint(S_0, obj_v, o, d)) {
            obj_node_v = obj_node_v->next;
            continue;
        }

        // Update the IF_v.
        bst_node_list_v = update_IF_obj(IF_v, obj_v);

        // Update the S_0.
        // obj_v is added at the first place of S_0.
        add_obj_set_entry(obj_v, S_0);

        // Sub-procedure.
        S = combinatorial_sub(IF_v, S_0, o, d);

        // Restore the S_0.
        remove_obj_set_entry(S_0);

        // Restore the IF_v.
        restore_IF_bst_node_list(IF_v, bst_node_list_v);

        release_bst_node_list(bst_node_list_v);

        // Checking.
        if (S != NULL) {
            // Include obj_v into S.
            add_obj_set_entry(obj_v, S);

            return S;
        }

        // S == NULL.

        // Try the next object candidate.
        obj_node_v = obj_node_v->next;
    }
    return NULL;
}

/*
 * method for inverted file
 * return true if all nodes have p_list->obj_n > 0
 */
bool bst_check_plist_obj_n(bst_node_t* bst_node_v)
{
    if (bst_node_v == NULL)
        return true;

    return (bst_node_v->p_list_obj->obj_n > 0) && bst_check_plist_obj_n(bst_node_v->left) && bst_check_plist_obj_n(bst_node_v->right);
}

/*
 * method for inverted file
 * return true if all nodes of fsi_v->feaset[i] exist and have p_list->obj_n > 0
 */
bool bst_check_plist(bst_t* bst_v, fsi_t* fsi_v, FEA_TYPE fea)
{
    bst_node_t* bst_node_v;

    if (bst_v == NULL)
        return false;

    for (int i = 0; i < fsi_v->fea_n; i++) {
        if (fsi_v->feaset[i] == fea)
            continue;

        bst_node_v = bst_search(bst_v, fsi_v->feaset[i]);
        if (bst_node_v == NULL || bst_node_v->p_list_obj->obj_n == 0) {
            return false;
        }
    }
    return true;
}

/*
 *  Method 4 : Filtering-and-Verification Approach
 *
 *  @alg_opt2:
 *  1 = (method 1) combinatorial
 *  2 = (method 2) Dia
 *  3 = (method 3) mCK
 */
bool filter_and_verify(int alg_opt2, fsi_t* fsi_v, obj_t* obj_v, bool* RI)
{
    loc_t* loc_v;
    obj_set_t* obj_set_v = NULL; // range query Disk(o,d) or NULL
    query_t* q = NULL;
    int temp = -1; // 1:true,0:false,-1:null
    bool feasibleflag1 = false; // true = feasible, false = not feasible
    bool feasibleflag2 = false;
    bool feasibleflag3 = false;

//---------------------------------------------------
// Filter 2. feasiblility check in D(o,d) by N_o_f

#ifndef WIN32
    float sys_t, S1_t = 0;
    struct rusage S1_sta, S1_end;
    GetCurTime(&S1_sta);
#endif

    {
        feasibleflag1 = check_Nof_feasibility(fsi_v, obj_v);

#ifndef WIN32
        GetCurTime(&S1_end);
        GetTime(&S1_sta, &S1_end, &S1_t, &sys_t);
        stat_v.S1_time += S1_t;
#endif

        if (!feasibleflag1) {
            stat_v.S1_sum++;
            return false;
        }

        if (fsi_v->fea_n == 2 && feasibleflag1) {
            stat_v.S1_sum++;
            return true;
        }
    }
    //---------------------------------------------------
    // Filter 3. range query in D(o,d/2)

    loc_v = get_obj_loc(obj_v);
    q = alloc_query();
    q->loc_v = loc_v;
    q->psi_v = fsi_to_psi(fsi_v, obj_v->fea);

   {
#ifndef WIN32
        float S2_t = 0;
        struct rusage S2_sta, S2_end;
        GetCurTime(&S2_sta);
#endif

        feasibleflag2 = check_Nof2_feasibility(fsi_v, obj_v);

#ifndef WIN32
        GetCurTime(&S2_end);
        GetTime(&S2_sta, &S2_end, &S2_t, &sys_t);
        stat_v.S2_time += S2_t;
#endif

        if (feasibleflag2) {
            stat_v.S2_sum++;
            release_query(q);
            return true;
        }
    }
	
    //---------------------------------------------------
    // verification

    stat_v.S5_sum++;

//---------------------------------------------------

#ifndef WIN32
    float S5_t = 0;
    struct rusage S5_sta, S5_end;
    GetCurTime(&S5_sta);
#endif

    // note that obj_set_v is released inside the functions
    if (alg_opt2 == 1)
        feasibleflag3 = combinatorial(fsi_v, obj_v, obj_set_v, RI);
    // else if (alg_opt2 == 2)
    // //     feasibleflag3 = dia(fsi_v, obj_v, obj_set_v, RI);
    // else // if(alg_opt2 == 3)
    //     feasibleflag3 = mck(fsi_v, obj_v, obj_set_v, RI);

#ifndef WIN32
    GetCurTime(&S5_end);
    GetTime(&S5_sta, &S5_end, &S5_t, &sys_t);
    stat_v.S5_time += S5_t;
#endif

    release_query(q);
    return feasibleflag3;
}

/*
 * Check whether all fea in @fsi_v > 0 in obj_v->N_o_f
 * @return
 * true: yes
 * false: no
 */
bool check_Nof_feasibility(fsi_t* fsi_v, obj_t* obj_v)
{
    for (int i = 0; i < fsi_v->fea_n; i++) {
        if (fsi_v->feaset[i] == obj_v->fea)
            continue;

        if (obj_v->N_o_f[fsi_v->feaset[i] - 1] == 0) {
            // not found
            return false;
        }
    }
    return true;
}

/*
 * Check whether all fea in @fsi_v > 0 in obj_v->N_o_f2
 * @return
 * true: yes
 * false: no
 */
bool check_Nof2_feasibility(fsi_t* fsi_v, obj_t* obj_v)
{
    for (int i = 0; i < fsi_v->fea_n; i++) {
        if (fsi_v->feaset[i] == obj_v->fea)
            continue;

        if (obj_v->N_o_f2[fsi_v->feaset[i] - 1] == 0) {
            // not found
            return false;
        }
    }
    return true;
}


// void precomputation(data_t* data_v, B_KEY_TYPE dist_thr)
// {
//     k_node_t *k_head, *k_node_v;
//     disk_t* disk_v;
//     loc_t* loc_v;
//     obj_set_t* obj_set_v;
//     obj_node_t* obj_node_v;
//     //    std::unordered_map<FEA_TYPE,int>* map, * map2;
//     //    std::unordered_map<FEA_TYPE,float> * tmp;
//     int *map, *map2;
//     float* tmp;

//     k_head = collect_keywords_bst(IF_v);
	
//     /*s*/
//     stat_v.memory_v += (sizeof(float) * data_v->key_n) * data_v->obj_n;
//     stat_v.memory_v += (sizeof(int) * data_v->key_n) * data_v->obj_n * 2;
//     if (stat_v.memory_v > stat_v.memory_max)
//         stat_v.memory_max = stat_v.memory_v;
//     /*s*/

//     // for each object
//     for (int i = 0; i < data_v->obj_n; i++) {
//         data_v->obj_v[i].frac_f = new float[data_v->key_n]();
//         data_v->obj_v[i].N_o_f = new int[data_v->key_n]();
//         data_v->obj_v[i].N_o_f2 = new int[data_v->key_n]();
//     }

// // 	//*we iterate each object that contain frequent labels
// //     // for each object
//     for (int i = 0; i < data_v->obj_n; i++) {
		
// 		FEA_TYPE fea = data_v->obj_v[i].fea;
// 		bst_node_t* bst_node_v =  bst_search(IF_v, fea);
// 		if(bst_node_v !=NULL){
// 			double n = bst_node_v->p_list_obj->obj_n;
// 			if(n<fea_highest_freq*min_sup){
// 				continue;
// 			}
// 		}
		
//         map = data_v->obj_v[i].N_o_f;
//         map2 = data_v->obj_v[i].N_o_f2;

//         // 2. find all objects in range D(o,d)
//         loc_v = get_obj_loc(&data_v->obj_v[i]);

//         disk_v = alloc_disk(IRTree_v.dim);
//         set_disk(disk_v, loc_v, dist_thr);

//         obj_set_v = range_query(disk_v);

//         // 3. loop each obj in range
//         obj_node_v = obj_set_v->head->next;
//         while (obj_node_v != NULL) {
//             map[obj_node_v->obj_v->fea - 1]++;

//             if (calc_dist_obj(obj_node_v->obj_v, &data_v->obj_v[i]) <= dist_thr / 2)
//                 map2[obj_node_v->obj_v->fea - 1]++;

//             obj_node_v = obj_node_v->next;
//         }

// //         //-------------
//         obj_node_v = obj_set_v->head->next;
//         while (obj_node_v != NULL) {
//             tmp = obj_node_v->obj_v->frac_f;
//             tmp[data_v->obj_v[i].fea - 1] += 1 / ((double)map[obj_node_v->obj_v->fea - 1]);

//             if (tmp[data_v->obj_v[i].fea - 1] > 1)
//                 tmp[data_v->obj_v[i].fea - 1] = 1;

//             obj_node_v = obj_node_v->next;
//         }

// //         //-------------
//         release_disk(disk_v);
//         release_obj_set(obj_set_v);
//         release_loc(loc_v);
//     }

//     release_k_list(k_head);
// }
// precomputation.cpp

// Placeholder declarations for missing functions and variables
// extern bst_node_t* bst_search(struct bst* tree, FEA_TYPE fea);
// extern struct bst* IF_v; // Inverted index tree
// extern IRTree IRTree_v; // IRTree structure
// extern double fea_highest_freq;
// extern double min_sup;
// extern struct stat_struct stat_v; // Assuming this is defined elsewhere

// Function to determine the maximum feature ID
int determine_max_feature_id(data_t* data_v) {
    int max_fea_id = 0;
    for (int i = 0; i < data_v->obj_n; i++) {
        if (data_v->obj_v[i].fea > max_fea_id) {
            max_fea_id = data_v->obj_v[i].fea;
        }
    }
    return max_fea_id;
}

void precomputation(data_t* data_v, B_KEY_TYPE dist_thr)
{
    k_node_t *k_head, *k_node_v;
    disk_t* disk_v;
    loc_t* loc_v;
    obj_set_t* obj_set_v;
    obj_node_t* obj_node_v;
    int *map, *map2;
    float* tmp;

    // Collect keywords from BST
    k_head = collect_keywords_bst(IF_v);
	
    /* Update memory statistics */
    stat_v.memory_v += (sizeof(float) * data_v->key_n) * data_v->obj_n;
    stat_v.memory_v += (sizeof(int) * data_v->key_n) * data_v->obj_n * 2;
    if (stat_v.memory_v > stat_v.memory_max)
        stat_v.memory_max = stat_v.memory_v;

    /* Allocate memory for each object */
    for (int i = 0; i < data_v->obj_n; i++) {
        data_v->obj_v[i].frac_f = new float[data_v->key_n]();
        data_v->obj_v[i].N_o_f = new int[data_v->key_n]();
        data_v->obj_v[i].N_o_f2 = new int[data_v->key_n]();
        data_v->obj_v[i].distance_bounds = nullptr; // Initialize to nullptr
    }

    // Determine the maximum feature ID for distance_bounds allocation
    int max_fea_id = determine_max_feature_id(data_v);

    // Populate the object list for distance caching
    obj_list.reserve(data_v->obj_n);
    for (int i = 0; i < data_v->obj_n; i++) {
        obj_list.push_back(&data_v->obj_v[i]);
    }

    // Initialize feature to object IDs mapping
    initialize_fea_to_obj_id_map(data_v);

    // Precompute distances and populate distance_bounds
    for (int i = 0; i < data_v->obj_n; i++) {
        // Check if the feature frequency meets the threshold
        FEA_TYPE fea = data_v->obj_v[i].fea;
        bst_node_t* bst_node_v = bst_search(IF_v, fea);
        if (bst_node_v != NULL) {
            double n = static_cast<double>(bst_node_v->p_list_obj->obj_n);
            if (n < fea_highest_freq * min_sup) {
                continue;
            }
        }
		
        map = data_v->obj_v[i].N_o_f;
        map2 = data_v->obj_v[i].N_o_f2;

        // Find all objects within distance threshold using IRTree
        loc_v = get_obj_loc(&data_v->obj_v[i]);

        disk_v = alloc_disk(IRTree_v.dim);
        set_disk(disk_v, loc_v, dist_thr);

        obj_set_v = range_query(disk_v);

        // Initialize distance_bounds if not already allocated
        if (data_v->obj_v[i].distance_bounds == nullptr) {
            data_v->obj_v[i].distance_bounds = new double[max_fea_id + 1];
            // Initialize all distances to a large value
            for (int f = 0; f <= max_fea_id; f++) {
                data_v->obj_v[i].distance_bounds[f] = std::numeric_limits<double>::max();
            }
        }

        // Loop through each object in range to update maps and distance_bounds
        obj_node_v = obj_set_v->head->next;
        while (obj_node_v != NULL) {
            FEA_TYPE neighbor_fea = obj_node_v->obj_v->fea;
            int neighbor_id = obj_node_v->obj_v->id;

            // Increment feature counts
            map[neighbor_fea - 1]++;

            // Calculate exact distance and update map2
            double exact_dist = calc_dist_obj(obj_node_v->obj_v, &data_v->obj_v[i]);
            if (exact_dist <= dist_thr / 2.0) {
                map2[neighbor_fea - 1]++;
            }

            // Update distance_bounds with the minimum distance
            if (exact_dist < data_v->obj_v[i].distance_bounds[neighbor_fea]) {
                data_v->obj_v[i].distance_bounds[neighbor_fea] = exact_dist;
            }

            // Similarly, update the neighbor's distance_bounds
            // Allocate neighbor's distance_bounds if necessary
            if (data_v->obj_v[neighbor_id - 1].distance_bounds == nullptr) {
                data_v->obj_v[neighbor_id - 1].distance_bounds = new double[max_fea_id + 1];
                for (int f = 0; f <= max_fea_id; f++) {
                    data_v->obj_v[neighbor_id - 1].distance_bounds[f] = std::numeric_limits<double>::max();
                }
            }
            if (exact_dist < data_v->obj_v[neighbor_id - 1].distance_bounds[fea]) {
                data_v->obj_v[neighbor_id - 1].distance_bounds[fea] = exact_dist;
            }

            obj_node_v = obj_node_v->next;
        }

        // Update fraction features
        obj_node_v = obj_set_v->head->next;
        while (obj_node_v != NULL) {
            FEA_TYPE neighbor_fea = obj_node_v->obj_v->fea;
            int neighbor_id = obj_node_v->obj_v->id;

            // Update frac_f based on N_o_f
            if (map[neighbor_fea - 1] != 0) {
                data_v->obj_v[neighbor_id - 1].frac_f[fea - 1] += 1.0f / static_cast<float>(map[neighbor_fea - 1]);
                if (data_v->obj_v[neighbor_id - 1].frac_f[fea - 1] > 1.0f) {
                    data_v->obj_v[neighbor_id - 1].frac_f[fea - 1] = 1.0f;
                }
            }

            obj_node_v = obj_node_v->next;
        }

        // Release resources
        release_disk(disk_v);
        release_obj_set(obj_set_v);
        release_loc(loc_v);
    }
}
