#ifndef RTREE_H
#define RTREE_H

#include "bit_operator.h"
#include "bst.h"
#include "data_utility.h"
#include "data_struct.h"

#define M				32
#define m1				20

#define MAX_LINE_LENG	1027

//The node structure of the node of the rtree.
typedef struct node         
{
	int		num;            //The number of items in the node.
	int		loc;            //The location in its parent's node.

	int		level;			//from 'leaf(0)' to 'root', increased by 1 level by level.
	node*	parent;			//pointer to the parent node.
	void**	child;			//pointer array(because the inner-node and leaf-node has different chidren) 
	range** MBRs;			//closing MBRange within 'x' dimension.
	bst_t*	bst_v;
	
}	node_t;

//The info structure of the rtree.
typedef struct	RTree
{
	int		dim;
	
	int		split_opt;
	
	node_t*	root;			//root node.
	int		height;			//height.
	int		obj_n;			//the number of the objects indexed by the tree.

	int		inner_n;		//the number of inner-nodes.
	int		leaf_n;			//the number of leaf-nodes.
}	RTree_t;

//
struct objPointer	//typedef 
{
	obj_t*		p;
	objPointer* next;
};

//
struct nodePointer	//typedef 
{
	node_t*			p;
	nodePointer*	next;
};


extern RTree_t RTree_v;
	

/*---------------------Calculation Related APIs----------------------*/


void CreateNode( node_t* &pnode);
void ReleaseNode( node_t* pnode, int tag);
int IsOverlapped( range* MBR1, range* MBR2);
bool IsSame( range* MBR1, range* MBR2);
double GetArea( node_t* pnode);	
double GetArea_2( range* MBR1, range* MBR2);
double GetIncArea( node_t* pnode, range* MBR);
void NeedLeastArea( node_t* pnode, range* obj, int& loc);
void calc_MBR( node_t* pnode, range* &MBR);
range* get_MBR_node( node_t* pnode, int dim);
bool UpdateMRB( node_t* pnode, range* &MBR);
 
/*----------------------Insertion related APIs----------------------*/

void ChooseLeaf( node_t* root, range* MBR, node_t* &leaf_node);
void PickSeeds( node_t* pnode, range* MBR, int& g1, int& g2);
int PickNext( node_t* pnode, node_t* node_1, node_t* node_2, node_t* &node_c, range* MBR, bool* done, int rTag);
void AssignEntry( node_t* pnode, node_t* node_c, void* obj, range* MBR);
void SplitNode( node_t* &pnode, node_t* &node_2, void* obj, int opt);
void AdjustTree( node_t* pnode);
node_t* CreateNewRoot( node_t* node_1, node_t* node_2);
node_t* InsertSub( node_t* pnode, void* obj);
void Insert( obj_t* obj);

/*---------------------Deletion Related APIs------------------------*/

node_t* Search( node_t* root, range* MBR, int& loc);
void ChooseInner( node_t* root, range* MBR, node_t* &inner_node);
void CondenseSub( node_t* pnode, objPointer* h1, nodePointer* h2 );
void CondenseTree( node_t* pnode);
int Delete( obj_t* obj); 

/*---------------------Other Operation APIs------------------------*/

void ini_tree( );
bool print_and_check_tree( int o_tag, const char* tree_file);
void build_RTree( data_t* data_v);
void print_IF( node_t* node_v, FILE* fp, int tag);

/*---------------------RTree augmentation APIs------------------------*/

k_node_t* collect_keywords_list( k_node_t* k_head);
k_node_t* collect_keywords_bst( bst_t* bst_v);
k_node_t* collect_keywords_fea(FEA_TYPE fea);

//void release_k_list( k_node_t* k_head);

int add_IF_entry( bst_t* bst_v, KEY_TYPE key, int loc);
void adjust_IF( node_t* c_node, void* obj, int loc);
void const_IF( node_t* node_v);
void adjust_parent_IF( node_t* p_node, int loc);

#endif
