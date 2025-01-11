#ifndef DATA_STRUCT_H
#define DATA_STRUCT_H

#include <unordered_map>
//#include "bst.h"
#include <vector>
#include <cstddef>

#define	KEY_TYPE	double
#define B_KEY_TYPE double

#define FEA_TYPE    unsigned int

//From rtree.h
//The interval structure of one dimension.
typedef struct 
{
	float min;
	float max;

}	range;

//The structure of the object.
typedef struct obj
{
	int			id;
	range*		MBR;

    FEA_TYPE    fea;
        
    struct bst*      inverted_list;              //storing the inverted list in Disk(o,d)    
    int* NeighborCountWithinD;
    int* NeighborCountWithinHalfD;

    // double* distance_bounds;  
}	obj_t;

//The structure for storing a set of features.
typedef struct fsi
{
    int			fea_n;
    FEA_TYPE*	feaset; //array
    B_KEY_TYPE  sup;
    struct fsi* next;   //use when fsi_t in fsi_set_t
    std::vector<struct obj_set *>* obj_set_list_v; //For joinless    
}	fsi_t;


//The structure for storing a set of feature sets.
typedef struct fsi_set
{
    int             fsi_n;
    fsi_t*          head;
    
    struct fsi_set* next;
}	fsi_set_t;

//From joinless_utility.h
//Inverted File related structures.
typedef struct k_node 
{
	KEY_TYPE		key;
	struct k_node*	next;

    int freq;
    
}	k_node_t;

//The structure for storing a set of keywords.
typedef struct psi
{
    int			key_n;
    k_node_t*	k_head;
    
}	psi_t;

//The node structure for storing an object pointer.
typedef struct obj_node
{
	obj_t*				obj_v;
	struct obj_node*	next;
    struct obj_set*     range = NULL;    
}	obj_node_t;

//The structure for storing a group of objects.
typedef struct obj_set
{
	int			obj_n;
	obj_node_t*	head;
    int fea_id; 

}	obj_set_t;


//From joinless_utility.h.

//The structure for storing a location.
typedef struct loc
{
	int		dim;
	float*	coord;
}	loc_t;

//Structure for storing a disk.
typedef struct disk
{
	loc_t*		center;
	B_KEY_TYPE	radius;
}	disk_t;


//Structure for storing a query.
typedef struct query
{
    loc_t*	loc_v;
    psi_t*	psi_v;
}	query_t;

//From data_utility.h
//The data structure for storing the statistics.
typedef struct colocation_stat
{
	//time.
	float		q_time;
	float		rtree_build_time;

	//memory.
	double		memory_v;
	double		memory_max;
	
	double		tree_memory_v;
	double		tree_memory_max;

    //Method 4
    double       S1_sum;
    double       S2_sum;
    double       S3_sum;
    double       S4_sum;
    double       S5_sum;
    
    double      S31_sum;
    double      S41_sum;
    
    
    double		S1_time;
    double		S2_time;
    double		S3_time;
    double		S4_time;
    double		S5_time;
    
}	colocation_stat_t;

#endif
