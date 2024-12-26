#ifndef COSTENUM_H
#define	COSTENUM_H

#include "data_struct.h"
#include "irtree.h"
#include "frac.h"

extern  float dist_thr;
extern IRTree_t IRTree_v;


loc_t* alloc_loc( int dim);
loc_t* get_obj_loc( obj_t* obj_v);
loc_t* copy_loc( loc_t* loc_v);

void release_loc( loc_t* loc_v);
psi_t* alloc_psi( );
void add_psi_entry( psi_t* psi_v, KEY_TYPE key);
void release_psi( psi_t* psi_v);
query_t* alloc_query( );
void print_query( query_t* q, FILE* o_fp);
query_t* read_query( FILE* i_fp);
void release_query( query_t* q);
disk_t* alloc_disk( int dim);
void set_disk( disk_t* disk_v, loc_t* loc_v, B_KEY_TYPE radius);
disk_t* const_disk( loc_t* loc_v, B_KEY_TYPE radius);
void release_disk( disk_t* disk_v);
obj_set_t* alloc_obj_set( );
obj_set_t* copy_obj_set( obj_set_t* obj_set_v);
bool has_same_content_obj_set( obj_set_t* obj_set_v1, obj_set_t* obj_set_v2);
void remove_identical_obj( obj_set_t* obj_set_v);
void print_obj_set( obj_set_t* obj_set_v, FILE* o_fp);
void release_obj_set( obj_set_t* obj_set_v);
B_KEY_TYPE calc_dist_loc( loc_t* loc_v1, loc_t* loc_v2);
B_KEY_TYPE calc_dist_obj( obj_t* obj_v1, obj_t* obj_v2);
bool has_key_obj( obj_t* obj_v, KEY_TYPE key);
bool is_relevant_obj( obj_t* obj_v, query_t* q);

BIT_TYPE has_key_node( node_t* node_v, KEY_TYPE key);
BIT_TYPE is_relevant_node( node_t* node_v, query_t* q);
B_KEY_TYPE calc_minDist( range* MBR, loc_t* loc_v);
B_KEY_TYPE calc_maxDist( range* MBR, loc_t* loc_v);
B_KEY_TYPE calc_minDist_node( node_t* node_v, loc_t* loc_v);

bool is_overlap( range* MBR, disk_t* disk_v); 
bool is_enclosed( range* MBR, disk_t* disk_v);
void add_obj_set_entry( obj_t* obj_v, obj_set_t* obj_set_v);
void remove_obj_set_entry( obj_set_t* obj_set_v);
void retrieve_sub_tree( node_t* node_v, obj_set_t* &obj_set_v, query_t* q);
void range_query_sub( node_t* node_v, disk_t* disk_v, obj_set_t* &obj_set_v, query_t* q);

obj_set_t* range_query( disk_t* disk_v, query_t* q);
bst_t* const_IF( obj_set_t* obj_set_v, psi_t* psi_v);
// obj_set_t* const_obj_set( tri_t* triplet_v);clea
bool check_dist_constraint( obj_set_t* obj_set_v, obj_t* obj_v, obj_t* o, B_KEY_TYPE d);
bst_node_list_t* update_IF_obj( bst_t* IF_v, obj_t* obj_v);
void release_bst_node_list( bst_node_list_t* bst_node_list_v);
void restore_IF_bst_node_list( bst_t* IF_v, bst_node_list_t* bst_node_list_v);

#endif
