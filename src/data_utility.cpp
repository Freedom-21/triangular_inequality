#include "data_utility.h"
#include "rtree.h"

#include <unordered_map>

#ifndef WIN32

// GetCurTime is used to get the current running time in the current process.
void GetCurTime(rusage* curTime)
{
    int ret = getrusage(RUSAGE_SELF, curTime);
    if (ret != 0) {
        fprintf(stderr, "The running time info couldn't be collected successfully.\n");
        //FreeData( 2);
        exit(0);
    }
}

//  GetTime is used to get the 'float' format time from the start and end rusage structure.
void GetTime(struct rusage* timeStart, struct rusage* timeEnd, float* userTime, float* sysTime)
{
    (*userTime) = ((float)(timeEnd->ru_utime.tv_sec - timeStart->ru_utime.tv_sec)) + ((float)(timeEnd->ru_utime.tv_usec - timeStart->ru_utime.tv_usec)) * 1e-6;
    (*sysTime) = ((float)(timeEnd->ru_stime.tv_sec - timeStart->ru_stime.tv_sec)) + ((float)(timeEnd->ru_stime.tv_usec - timeStart->ru_stime.tv_usec)) * 1e-6;
}

#endif

colocation_config_t* read_config_colocation()
{
    colocation_config_t* cfg;
    FILE* c_fp;

    cfg = (colocation_config_t*)malloc(sizeof(colocation_config_t));
    memset(cfg, 0, sizeof(colocation_config_t));

    if ((c_fp = fopen(COLOCATION_CONFIG_FILE, "r")) == NULL) {
        fprintf(stderr, "The colocation_config file cannot be opened.\n");
        exit(0);
    }

    //algorithm option.
    fscanf(c_fp, "%i", &cfg->alg_opt);

    //data.
    fscanf(c_fp, "%i%s", &cfg->obj_n, cfg->loc_file);
    fscanf(c_fp, "%i%s", &cfg->key_n, cfg->doc_file);
    fscanf(c_fp, "%s", cfg->tree_file);
    fscanf(c_fp, "%i", &cfg->query_n);

    //colocation pattern mining.
    fscanf(c_fp, "%f", &cfg->min_pi);
    fscanf(c_fp, "%f", &cfg->dist_thr);

    fclose(c_fp);

    return cfg;
}

void add_keyword_entry(k_node_t*& k_node_v, KEY_TYPE key)
{
    k_node_v->next = (k_node_t*)malloc(sizeof(k_node_t));
    memset(k_node_v->next, 0, sizeof(k_node_t));
    k_node_v->next->key = key;
    k_node_v = k_node_v->next;

    /*s*/
    stat_v.memory_v += sizeof(k_node_t);
    if (stat_v.memory_v > stat_v.memory_max)
        stat_v.memory_max = stat_v.memory_v;
    /*s*/
}

void print_colocation_stat(colocation_config_t* cfg, int cnt)
{
    FILE* s_fp;
    if (!(s_fp = fopen(COLOCATION_STAT_FILE, "w"))) {
        fprintf(stderr, "Cannot open the statistics file.\n");
        exit(EXIT_FAILURE);
    }

    fprintf(s_fp, "R-tree Build Time: %.6f seconds\n", stat_v.rtree_build_time);
    fprintf(s_fp, "Average Query Time: %.6f seconds\n\n", stat_v.q_time);

    fprintf(s_fp, "Memory usage (MB)\tR-tree memory usage (MB)\n");
    fprintf(s_fp, "%.2f\t\t\t%.2f\n\n",
            stat_v.memory_max / (1024 * 1024),
            stat_v.tree_memory_max / (1024 * 1024));

    fprintf(s_fp, "Method\t\t\tCount\t\t\tTime (seconds)\n");
    fprintf(s_fp, "Check 1\t\t\t%.0lf\t\t\t%.6f\n", stat_v.S1_sum, stat_v.S1_time);
    fprintf(s_fp, "Check 2\t\t\t%.0lf\t\t\t%.6f\n", stat_v.S2_sum, stat_v.S2_time);
    fprintf(s_fp, "Check 3\t\t\t%.0lf\t\t\t%.6f\n", stat_v.S3_sum, stat_v.S3_time);
    fprintf(s_fp, "Check 4\t\t\t%.0lf\t\t\t%.6f\n", stat_v.S5_sum, stat_v.S5_time);

    fclose(s_fp);
}

void alloc_obj(obj_t* obj_v, int dim)
{
    obj_v->MBR = (range*)malloc(dim * sizeof(range));
    memset(obj_v->MBR, 0, dim * sizeof(range));
}

data_t* alloc_data(int num)
{
    data_t* data_v;

    data_v = (data_t*)malloc(sizeof(data_t));
    memset(data_v, 0, sizeof(data_t));

    data_v->obj_n = num;

    data_v->obj_v = (obj_t*)malloc(sizeof(obj_t) * data_v->obj_n);
    memset(data_v->obj_v, 0, sizeof(obj_t) * data_v->obj_n);

    return data_v;
}

/*
 *	Read the data based on the colocation_config_t info.
 */
data_t* read_data_colocation(colocation_config_t* cfg)
{
    int i, j;
    KEY_TYPE key;
    char des;
    char keys[TEXT_COL_MAX];
    char* tok;
    data_t* data_v;

    FILE* i_fp;

    data_v = alloc_data(cfg->obj_n);

    data_v->dim = cfg->dim;
    data_v->key_n = cfg->key_n;

    if ((i_fp = fopen(cfg->loc_file, "r")) == NULL) {
        fprintf(stderr, "Cannot open the loc file.\n");
        exit(0);
    }

    for (i = 0; i < data_v->obj_n; i++) {
        alloc_obj(data_v->obj_v + i, data_v->dim);

        fscanf(i_fp, "%i	", &(data_v->obj_v[i].id));

        for (j = 0; j < data_v->dim; j++) {
            fscanf(i_fp, "%c%f", &des, &(data_v->obj_v[i].MBR[j].min));
            data_v->obj_v[i].MBR[j].max = data_v->obj_v[i].MBR[j].min;
        }
    }
    fclose(i_fp);

    //Read the IDs info.
    if ((i_fp = fopen(cfg->doc_file, "r")) == NULL) {
        fprintf(stderr, "Cannot open the IDs file.\n");
        exit(0);
    }

    for (i = 0; i < data_v->obj_n; i++) {

        fgets(keys, TEXT_COL_MAX, i_fp);

        tok = strtok(keys, " ,");
        while ((tok = strtok(NULL, " ,"))) {
            key = atoi(tok);
            data_v->obj_v[i].fea = key;
        }
    }
    fclose(i_fp);

    return data_v;
}

void release_k_list(k_node_t* k_node_v)
{
    k_node_t* k_node_v1;
    while (k_node_v->next != NULL) {
        k_node_v1 = k_node_v;
        k_node_v = k_node_v->next;
        free(k_node_v1);
    }
    free(k_node_v);
}

void release_data(data_t* data_v)
{
    int i;

    for (i = 0; i < data_v->obj_n; i++) {
        free(data_v->obj_v[i].MBR);
    }

    free(data_v->obj_v);
    free(data_v);
}