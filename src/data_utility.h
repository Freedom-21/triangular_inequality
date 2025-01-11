#ifndef DATA_UTILITY_H
#define	DATA_UTILITY_H


#include <unordered_map>
#include "data_struct.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <random>
using namespace std;


#ifndef WIN32
#include<sys/resource.h>
#endif

#define TEXT_COL_MAX			1075
#define MAX_FILENAME_LENG		256	

#define	CONFIG_FILE				"RTree_config.txt"
#define	COLOCATION_CONFIG_FILE	"param.txt"
#define	COLOCATION_STAT_FILE	"usages.txt"
#define	COLOCATION_RESULT_FILE	"patterns.txt"

//The structure for storing the objects.
typedef struct 
{
	int			dim;
	int			obj_n;
	obj_t*		obj_v;

	int			key_n;

}	data_t;

//The structure for storing the configuration info.
typedef	struct
{
    int		dim;
    int		obj_n;
    int		key_n;
    
    int		split_opt;
    
    char	loc_file[ MAX_FILENAME_LENG];
    char	doc_file[ MAX_FILENAME_LENG];
    
    char	tree_file[ MAX_FILENAME_LENG];
        
}	RTree_config_t;


//The structure for storing the configuration information for the co-location pattern mining problem.
typedef struct colocation_config
{
	int		dim;
	int		obj_n; //|O|
	int		key_n; // total number of keywords appear in O
	char	loc_file[ MAX_FILENAME_LENG];
	char	doc_file[ MAX_FILENAME_LENG];
    char	tree_file[ MAX_FILENAME_LENG];
	int		alg_opt; //Method 1 or 2    
    int     query_n;
	float    min_pi;
    float    dist_thr;    
    
}	colocation_config_t;

extern colocation_stat_t stat_v;

#ifndef WIN32

void GetCurTime( rusage* curTime);
void GetTime( struct rusage* timeStart, struct rusage* timeEnd, float* userTime, float* sysTime);

#endif

colocation_config_t* read_config_colocation( );
void add_keyword_entry( k_node_t* &k_node_v, KEY_TYPE key);
void print_colocation_stat( colocation_config_t* cfg, int cnt);
void alloc_obj( obj_t* obj_v, int dim);
data_t* alloc_data( int num);
data_t*	read_data_colocation( colocation_config_t* cfg);
void release_k_list( k_node_t* k_node_v);
void release_data( data_t* data_v);

#endif
