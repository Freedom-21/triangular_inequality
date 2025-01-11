
#include "frac.h"
// #include "distance_cache.h"
#include "data_struct.h" // Ensure this is included if not already
#include <cassert>
#include <limits>


// int determine_max_feature_id(data_t* data_v);

/*
 *  [Apriori]
 */
fsi_set_t** apriori(int alg_opt, int numOfObj, int numOfFea, double dist_thr)
{
    printf("Starting Improved Algorithm with, objects=%d, Features=%d, dist_thr=%.5f\n", numOfObj, numOfFea, dist_thr);
    int i, j;
    fsi_set_t** result; // storing overall result
    fsi_set_t* fsi_set_cur; // prev = P_{k-1}, cur = P_{k}
    fsi_t *fsi_v1, *fsi_v2, *fsi_v3;

    FEA_TYPE fea_v1;
    B_KEY_TYPE sup;

    i = 0;
    // Initialize the structure for storing P_1, P_2, ..., P_{|F|}
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

    // P_1
    // printf("Constructing P_1...\n");
    result[0] = const_L1_apriori();

#ifndef WIN32
    GetCurTime(&query_end);

    GetTime(&query_sta, &query_end, &usr_t, &sys_t);
    printf("Pattern_1 \ttime:%0.5lf\n", usr_t);
    GetCurTime(&query_sta);
#endif

    // P_(i)
    for (i = 1; i < numOfFea; i++) {
    // printf("Processing Level Pattern_%d\n", i + 1);
    fsi_v1 = result[i -1]->head->next;
    if (fsi_v1 == NULL) {
        printf("No more frequent itemsets at Level P_%d. Terminating.\n", i + 1);
        break;
    }
    fsi_set_cur = alloc_fsi_set();

    // Iterate over all fsi_v1 in P_{k-1}
    while (fsi_v1 != NULL) {
        fsi_v2 = fsi_v1->next;
        while (fsi_v2 != NULL) {
            // Join step
            if ((fea_v1 = join_check(fsi_v1, fsi_v2)) != -1) {
                // printf("Joining fsi_set with features:");
                // for(int f = 0; f < fsi_v1->fea_n; f++) printf(" %d", fsi_v1->feaset[f]);
                // printf(" and ");
                // for(int f = 0; f < fsi_v2->fea_n; f++) printf(" %d", fsi_v2->feaset[f]);
                // printf(" to form feature %d\n", fea_v1);

                // Create the candidate feature set by adding the new feature
                fsi_v3 = add_fsi(fsi_v1, fea_v1);

                // Prune step
                if (i > 1 && !all_subsets_exist(result[i -1], fsi_v3)) {
                    // printf("Pruning candidate set with features:");
                    // for(int f = 0; f < fsi_v3->fea_n; f++) printf(" %d", fsi_v3->feaset[f]);
                    // printf("\n");

                    /* Update memory statistics */
                    stat_v.memory_v -= sizeof(fsi_t) + fsi_v3->fea_n * sizeof(FEA_TYPE);
                    /* Update memory statistics */

                    release_fsi(fsi_v3);
                    fsi_v2 = fsi_v2->next;
                    continue;
                }

                // Support calculation
                sup = comp_support(alg_opt, fsi_v3, numOfObj);
                // printf("Computed support for candidate set:");
                // for(int f = 0; f < fsi_v3->fea_n; f++) printf(" %d", fsi_v3->feaset[f]);
                // printf(" is %.5lf\n", sup);

                if (sup >= min_sup) {
                    // printf("Adding candidate set to Level P_%d\n", i + 1);
                    add_fsi_set_entry(fsi_set_cur, fsi_v3);
                } else {
                    // printf("Candidate set does not meet min_sup and is discarded.\n");
                    /* Update memory statistics */
                    stat_v.memory_v -= sizeof(fsi_t) + fsi_v3->fea_n * sizeof(FEA_TYPE);
                    /* Update memory statistics */

                    release_fsi(fsi_v3);
                }
            }
            fsi_v2 = fsi_v2->next;
        }

        // Move to the next fsi_v1 in P_{k-1}
        fsi_v1 = fsi_v1->next;
    }

#ifndef WIN32
    GetCurTime(&query_end);
    GetTime(&query_sta, &query_end, &usr_t, &sys_t);
    printf("Pattern_%d \ttime:%0.5lf\t # of colocation patterns:%d\n", i + 1, usr_t, fsi_set_cur->fsi_n);
    GetCurTime(&query_sta);
#endif

    // Assign the current level's feature sets to the result
    result[i] = fsi_set_cur;
}

printf("Improved algorithm completed.\n");
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

        sup = 1; // by definition

        fsi_v->sup = sup;

        add_fsi_set_entry(fsi_set_v, fsi_v);

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
        B_KEY_TYPE sup, sup_C_f;
    obj_node_t* obj_node_v;
    obj_set_t* obj_set_v;

    sup = INFINITY;
    sup_C_f = 0;

    //    FEA_TYPE fea_worst;
    //    obj_set_t* obj_set_worst;

    // keep track objects that are involved in row instances that have been found
    // note that obj_id-1 is needed inside the array
    bool* RI = new bool[numOfObj]();

    // for each feature f in C
    for (int i = 0; i < fsi_v->fea_n; i++) {
        sup_C_f = 0;

        // the corresponding inverted list in IF
        obj_set_v = ((bst_node_t*)bst_search(IF_v, fsi_v->feaset[i]))->p_list_obj;
        obj_node_v = obj_set_v->head->next;
        // for each object o with the feature f
        while (obj_node_v != NULL) {
            bool flag = false;
         {
#ifndef WIN32
                float sys_t, S3_t = 0;
                struct rusage S3_sta, S3_end;
                GetCurTime(&S3_sta);
#endif
                flag = RI[obj_node_v->obj_v->id - 1];
                if (flag)
                    stat_v.S3_sum++;
#ifndef WIN32
                GetCurTime(&S3_end);
                GetTime(&S3_sta, &S3_end, &S3_t, &sys_t);
                stat_v.S3_time += S3_t;
#endif
            }

            if (flag || check_row_instance(alg_opt, fsi_v, obj_node_v->obj_v, RI)) {
                // if (cost_tag == 1)
                sup_C_f += 1; //each group is counted as 1
            }

            obj_node_v = obj_node_v->next;
        }

        // if (cost_tag == 1)
        sup_C_f = sup_C_f / (double)obj_set_v->obj_n;

        // maintain the smallest one here
        if (sup_C_f <= sup) {
            sup = sup_C_f;

            // early stopping: sup of this fea set < threshold
            if (sup < min_sup)
                break;
        }
    }
    fsi_v->sup = sup;
    delete[] RI;

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
        flag = filter_and_verify(alg_opt - 1, fsi_v, obj_v, RI);
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
// Filter 2. feasiblility check in D(o,d) by NeighborCountWithinD

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

#ifndef WIN32
    GetCurTime(&S5_end);
    GetTime(&S5_sta, &S5_end, &S5_t, &sys_t);
    stat_v.S5_time += S5_t;
#endif

    release_query(q);
    return feasibleflag3;
}

/*
 * Check whether all fea in @fsi_v > 0 in obj_v->NeighborCountWithinD
 * @return
 * true: yes
 * false: no
 */
bool check_Nof_feasibility(fsi_t* fsi_v, obj_t* obj_v)
{
    for (int i = 0; i < fsi_v->fea_n; i++) {
        if (fsi_v->feaset[i] == obj_v->fea)
            continue;

        if (obj_v->NeighborCountWithinD[fsi_v->feaset[i] - 1] == 0) {
            // not found
            return false;
        }
    }
    return true;
}

/*
 * Check whether all fea in @fsi_v > 0 in obj_v->NeighborCountWithinHalfD
 * @return
 * true: yes
 * false: no
 */
bool check_Nof2_feasibility(fsi_t* fsi_v, obj_t* obj_v)
{
    for (int i = 0; i < fsi_v->fea_n; i++) {
        if (fsi_v->feaset[i] == obj_v->fea)
            continue;

        if (obj_v->NeighborCountWithinHalfD[fsi_v->feaset[i] - 1] == 0) {
            // not found
            return false;
        }
    }
    return true;
}


void precomputation(data_t* data_v, B_KEY_TYPE dist_thr)
{
    k_node_t *k_head, *k_node_v;
    disk_t* disk_v;
    loc_t* loc_v;
    obj_set_t* obj_set_v;
    obj_node_t* obj_node_v;
    //    std::unordered_map<FEA_TYPE,int>* map, * map2;
    //    std::unordered_map<FEA_TYPE,float> * tmp;
    int *map, *map2;
    float* tmp;

    k_head = collect_keywords_bst(IF_v);
	
    /*s*/
    stat_v.memory_v += (sizeof(float) * data_v->key_n) * data_v->obj_n;
    stat_v.memory_v += (sizeof(int) * data_v->key_n) * data_v->obj_n * 2;
    if (stat_v.memory_v > stat_v.memory_max)
        stat_v.memory_max = stat_v.memory_v;
    /*s*/

    // for each object
    for (int i = 0; i < data_v->obj_n; i++) {
        // data_v->obj_v[i].frac_f = new float[data_v->key_n]();
        data_v->obj_v[i].NeighborCountWithinD = new int[data_v->key_n]();
        data_v->obj_v[i].NeighborCountWithinHalfD = new int[data_v->key_n]();
    }

// 	//*we iterate each object that contain frequent labels
//     // for each object
    for (int i = 0; i < data_v->obj_n; i++) {
		
		FEA_TYPE fea = data_v->obj_v[i].fea;
		bst_node_t* bst_node_v =  bst_search(IF_v, fea);
		if(bst_node_v !=NULL){
			double n = bst_node_v->p_list_obj->obj_n;
			if(n<fea_highest_freq*min_sup){
				continue;
			}
		}
		
        map = data_v->obj_v[i].NeighborCountWithinD;
        map2 = data_v->obj_v[i].NeighborCountWithinHalfD;

        // 2. find all objects in range D(o,d)
        loc_v = get_obj_loc(&data_v->obj_v[i]);

        disk_v = alloc_disk(IRTree_v.dim);
        set_disk(disk_v, loc_v, dist_thr);

        obj_set_v = range_query(disk_v);

        // 3. loop each obj in range
        obj_node_v = obj_set_v->head->next;
        while (obj_node_v != NULL) {
            map[obj_node_v->obj_v->fea - 1]++;

            if (calc_dist_obj(obj_node_v->obj_v, &data_v->obj_v[i]) <= dist_thr / 2)
                map2[obj_node_v->obj_v->fea - 1]++;

            obj_node_v = obj_node_v->next;
        }

        release_disk(disk_v);
        release_obj_set(obj_set_v);
        release_loc(loc_v);
    }

        // -----------------------------------------------------------
    // OPTIONAL: Print out the arrays for debugging/inspection.
    // -----------------------------------------------------------
    // printf("\n--- Neighbor Count Information ---\n");
    // for (int i = 0; i < data_v->obj_n; i++) {
    //     printf("Object ID %d:\n", data_v->obj_v[i].id);
    //     for (int f = 0; f < data_v->key_n; f++) {
    //         int countD   = data_v->obj_v[i].NeighborCountWithinD[f];
    //         int countD2  = data_v->obj_v[i].NeighborCountWithinHalfD[f];
    //         // Print only if it's non-zero or if you want to see everything, remove the if.
    //         if (countD > 0 || countD2 > 0) {
    //             printf("  Feature %2d => Within D: %3d, Within D/2: %3d\n",
    //                    f + 1, countD, countD2);
    //         }
    //     }
    //     printf("\n");
    // }

    release_k_list(k_head);
}