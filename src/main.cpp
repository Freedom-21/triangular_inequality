#include "joinless.h"
#include "rtree.h"
#include "improved.h"
#include "data_utility.h"
#include <iostream>
#include <fstream>
#include <random>
#include <cstddef>
#include <cmath>

using namespace std;
using std::size_t;
using std::vector;

RTree_t RTree_v;
bst_t* IF_v;

colocation_stat_t stat_v;

bool debug_mode = false;

float dist_thr;
float min_pi;
double fea_highest_freq;

void colocation();

void build_IF(data_t* data_v);

int main(int argc, char* argv[])
{
    colocation();
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

    dist_thr = cfg->dist_thr;
    min_pi = cfg->min_pi;
    printf("Reading data ...\n");
    data_v = read_data_colocation(cfg);

#ifndef WIN32
    float sys_t, usr_t, usr_t_sum = 0;
    struct rusage IR_tree_sta, IR_tree_end;

    GetCurTime(&IR_tree_sta);
#endif

//    if (cfg->tree_tag == 0) { 
    printf("Building RTree ...\n");
    build_RTree(data_v);
    print_and_check_tree(1, cfg->tree_file);
    // }

#ifndef WIN32
    GetCurTime(&IR_tree_end);
    GetTime(&IR_tree_sta, &IR_tree_end, &stat_v.rtree_build_time, &sys_t);
#endif

    //Build Inverted file
    build_IF(data_v);

    if (cfg->alg_opt == 1) {
        printf("Redirected to the Joinless Algorithm with threshold prevalnce:%lf\n", min_pi);
    } else {
        printf("Redirected to the Improved Algorithm with threshold prevalnce:%lf\n", min_pi);
    }

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

        if (cfg->alg_opt == 1) {
            result = joinless_mining(data_v, cfg->obj_n, cfg->key_n);
        } else if (cfg->alg_opt == 2) {
            result = improved(cfg->alg_opt, cfg->obj_n, cfg->key_n, cfg->dist_thr);
        }else {
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

            print_fsi_set(result, cfg->key_n, r_fp);

			fclose(r_fp);
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

void build_IF(data_t* data_v)
{

    bst_node_t* bst_node_v;

    IF_v = bst_ini();

    //Insert all the objects to construct the IF
    for (int i = 0; i < data_v->obj_n; i++) {
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
