/*
 *	Author: Harry
 *	Email: khchanak@cse.ust.hk
 */

#include "yoo_alg.h"
#include "irtree.h"
#include "frac.h"
#include "data_utility.h"
#include <iostream>
#include <fstream>
#include <random>
// #include "SmallestEnclosingCircle.h"
#include <cstddef>
#include <cmath>
// #include "stringgenc.h"

using namespace std;
using std::size_t;
using std::vector;

IRTree_t IRTree_v;
bst_t* IF_v;

colocation_stat_t stat_v;

bool debug_mode = false;

//----------------------------------
//user parameter
float dist_thr;
float min_sup;
// int cost_tag; //1= participation based; 2=fraction based
double fea_highest_freq;

//------------------------------------
void colocation_patterns_support(int cost);

void colocation();

void build_IF(data_t* data_v);

// void gen_syn();

// double get_highest_freq(bst_node_t* bst_node_v);

fsi_set_t* read_patterns();

int main(int argc, char* argv[])
{

  //     batch_gen_syn_data2();
    colocation();

//    colocation_patterns_support(3);
    return 0;
}


void colocation()
{
    int i;
    colocation_config_t* cfg;
    FILE *r_fp, *r_fp2;
    data_t* data_v;
    fsi_set_t** result;

    memset(&stat_v, 0, sizeof(colocation_stat_t));

    //Read the cofig.
    printf("Reading configuration ...\n");
    cfg = read_config_colocation();
    cfg->dim = 2;

    // cost_tag = cfg->cost;
    dist_thr = cfg->dist_thr;
    min_sup = cfg->min_sup;
  //  min_conf = cfg->min_conf;

    //Read the data.
    printf("Reading data ...\n");
    data_v = read_data_colocation(cfg);

#ifndef WIN32
    float sys_t, usr_t, usr_t_sum = 0;
    struct rusage IR_tree_sta, IR_tree_end;

    GetCurTime(&IR_tree_sta);
#endif

    //Option 1: Build the tree from scratch.
    //Build the IR-tree.
    if (cfg->tree_tag == 0) {
        printf("Building IR-tree ...\n");
        build_IRTree(data_v);

        print_and_check_tree(1, cfg->tree_file);
        //check_IF( );
    } else {
        //Option 2: Read an existing tree.
        printf("Reading IR-Tree ...\n");
        read_tree(cfg->tree_file);
    }

#ifndef WIN32
    GetCurTime(&IR_tree_end);
    GetTime(&IR_tree_sta, &IR_tree_end, &stat_v.irtree_build_time, &sys_t);
#endif

    //Build Inverted file
    build_IF(data_v);

    //---
    if (cfg->alg_opt == 1) {
        printf("Redirected to the Joinless Algorithm with threshold prevalnce:%lf\n", min_sup);
    } else {
        printf("Redirected to the Improved Algorithm with threshold prevalnce:%lf\n", min_sup);
    }
    //---

#ifndef WIN32
	
	float pre_t = 0;
	struct rusage pre_sta, pre_end;
	GetCurTime(&pre_sta);
#endif
	
	//Pre-computation of NeighborCountWithinD
	precomputation(data_v, cfg->dist_thr);
	
#ifndef WIN32
	GetCurTime(&pre_end);
	GetTime(&pre_sta, &pre_end, &pre_t, &sys_t);
	printf("pre\ttime:%0.5lf\n", pre_t);
#endif
	
    for (i = 0; i < cfg->query_n; i++) //i==1
    {

#ifndef WIN32
        struct rusage query_sta, query_end;
        GetCurTime(&query_sta);

#endif

        printf("Query #%i ...\n\t", i + 1);

        // result = apriori(cfg->alg_opt, cfg->obj_n, cfg->key_n, cfg->dist_thr); // testing improved alone

        if (cfg->alg_opt == 1) {
            result = joinless_mining(data_v, cfg->obj_n, cfg->key_n);
        } else if (cfg->alg_opt == 2) {
            result = apriori(cfg->alg_opt, cfg->obj_n, cfg->key_n, cfg->dist_thr);
        }else {
            // Invalid alg_opt: Print error message and stop execution
            exit(EXIT_FAILURE);  
        }

#ifndef WIN32
        GetCurTime(&query_end);

        GetTime(&query_sta, &query_end, &usr_t, &sys_t);
        usr_t_sum += usr_t;

		printf("Time: %f\n\n", usr_t + pre_t);
		//pre-computation time should also be included in the query time
		stat_v.q_time = usr_t_sum / (i + 1) + pre_t;

#endif

        //Print the query results.
        if (i == 0) {
            if ((r_fp = fopen(COLOCATION_RESULT_FILE, "w")) == NULL) {
                fprintf(stderr, "Cannot open the colocation_result file.\n");
                exit(0);
            }

			fclose(r_fp);
			//fclose(r_fp2);

        }
        //release the result memory
        for (int k = 0; k < cfg->key_n; k++)
            if (result[k] != NULL)
                release_fsi_set(result[k]);

        print_colocation_stat(cfg, i + 1);

        //reset some statistcs for the next query
        stat_v.S1_sum = 0.0;
        stat_v.S2_sum = 0.0;
        stat_v.S3_sum = 0.0;
        stat_v.S5_sum = 0.0;

        stat_v.S1_time = 0.0;
        stat_v.S2_time = 0.0;
        stat_v.S3_time = 0.0;
        stat_v.S4_time = 0.0;
        stat_v.S5_time = 0.0;
    }

    free(cfg);
}

/*
 * Construct the inverted file @IF_v based on the data
 */
void build_IF(data_t* data_v)
{

    bst_node_t* bst_node_v;

    IF_v = bst_ini();

    //Insert all the objects to construct the IF
    for (int i = 0; i < data_v->obj_n; i++) {
        //        if(i%100==0)
        {

            bst_node_v = bst_search(IF_v, data_v->obj_v[i].fea);

            if (bst_node_v != NULL) {
                add_obj_set_entry(&data_v->obj_v[i], bst_node_v->p_list_obj);
            } else //bst_node_v = NULL
            {
                //Insert a new keyword in the binary tree.
                bst_node_v = (bst_node_t*)malloc(sizeof(bst_node_t));
                memset(bst_node_v, 0, sizeof(bst_node_t));

                /*s*/
                stat_v.memory_v += sizeof(bst_node_t);
                if (stat_v.memory_v > stat_v.memory_max)
                    stat_v.memory_max = stat_v.memory_v;
                /*s*/

                //Update the posting list.
                bst_node_v->key = data_v->obj_v[i].fea;
                bst_node_v->p_list_obj = alloc_obj_set();

                add_obj_set_entry(&data_v->obj_v[i], bst_node_v->p_list_obj);
                bst_insert(IF_v, bst_node_v);
            }
        }
    }
}
