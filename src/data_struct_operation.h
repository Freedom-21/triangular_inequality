#ifndef data_struct_operation_h
#define data_struct_operation_h

#define BIT_TYPE	unsigned int

#include "data_utility.h"
#include "joinless_utility.h"
#include "irtree.h"
#include <math.h>


/* ------------------------- create/add/release  ------------------------- */

fsi_t* alloc_fsi( int key_n);

fsi_set_t* alloc_fsi_set();

void release_fsi( fsi_t* fsi_v);

void release_fsi_set( fsi_set_t* fsi_set_v);

void add_fsi_set_entry( fsi_set_t* fsi_set_v, fsi_t* fsi_v);

fsi_t* add_fsi(fsi_t* fsi_v, FEA_TYPE fea);

bst_t* const_IF(obj_set_t* obj_set_v);

void release_IF_sub( bst_node_t* x);

void release_IF( bst_t* T);

/* ------------------------- print / checking / convert  ------------------------- */

void print_fsi(fsi_t* fsi_v, FILE* o_fp);

void print_fsi_set(fsi_set_t** result, int numOfFea, FILE* o_fp);

psi_t* fsi_to_psi(fsi_t* fsi_v, FEA_TYPE feaToExclude);

/* ------------------------- R-tree  ------------------------- */

void retrieve_sub_tree( node_t* node_v, obj_set_t* &obj_set_v);

void range_query_sub( node_t* node_v, disk_t* disk_v, obj_set_t* &obj_set_v);

obj_set_t* range_query( disk_t* disk_v);

#endif /* data_struct_operation_h */
