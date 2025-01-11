#include "rtree.h"

// CreateNode malloc the storage space for a pointer to 'node'.

void CreateNode( node_t* &pnode)
{
	pnode = ( node_t*)malloc( sizeof( node));
	memset( pnode, 0, sizeof( node));

	pnode->child = ( void**)malloc( ( M+1) * sizeof( void*));
	memset( pnode->child, 0, ( M + 1) * sizeof( void*));
	pnode->MBRs = ( range**)malloc( ( M+1) * sizeof( range*));
	memset( pnode->MBRs, 0, ( M + 1) * sizeof( range*));

	/*s*/
	float tmp = stat_v.memory_v;
	/*s*/

	//IR-tree augmentation.
	pnode->bst_v = bst_ini( );

	/*s*/
	stat_v.tree_memory_v += stat_v.memory_v - tmp;
	stat_v.memory_v = tmp;

	stat_v.tree_memory_v += sizeof( node) + ( M + 1) * ( sizeof( void*) + sizeof( range*));
	if( stat_v.tree_memory_v > stat_v.tree_memory_max)
		stat_v.tree_memory_max = stat_v.tree_memory_v;
	/*s*/
}

/*
 *	Release a node structure.
 */
void ReleaseNode( node_t* pnode, int tag)
{
	int i;

	if( tag && pnode->level > 0)
	{
		for( i=0; i<pnode->num; i++)
			free( pnode->MBRs[ i]);

		/*s*/
		stat_v.tree_memory_v -= IRTree_v.dim * sizeof( range);
		/*s*/
	}

	free( pnode->child);
	free( pnode->MBRs);

	/*s*/
	float tmp = stat_v.memory_v;
	/*s*/

	bst_release( pnode->bst_v);

	/*s*/
	stat_v.tree_memory_v -= tmp - stat_v.memory_v;
	stat_v.memory_v = tmp;
	/*s*/

	free( pnode);

	/*s*/
	stat_v.tree_memory_v -= sizeof( node) + ( M + 1) * ( sizeof( void*) + sizeof( range*));
	/*s*/
}

/*
* IsOverlapped determines whether MBR overlaps the obj.
*
* param MBR indicates the MBRangle.
* param obj is the object.
*
* return 1 if overlapped; 0 if crossed; else(departed) -1.
*/
int IsOverlapped(range* MBR, range* obj)
{
	int  i, tag;
	tag = 0;
	for( i=0; i<IRTree_v.dim; i++)	//Travel each dimension.
	{
		//Departed;
		if( MBR[ i].min > obj[ i].max ||
			MBR[ i].max < obj[ i].min)
			return -1;
		//Crossed;
		else if( !( MBR[ i].min >= obj[ i].min &&
			MBR[ i].max <= obj[ i].max))
			tag = 1;
		//Contained;
		else 
			continue;
	}
	if( tag = 1)
		return 0;
	else
		return 1;
}

/*
* IsSame is used to judge whether the two MBRangles are the same.
*
*  MBR1, MBR2 indicate the two MBRangles respectively.
*
* return true if same, else false.
*/
bool IsSame( range* MBR1, range* MBR2)
{
	int i;
	for( i=0; i<IRTree_v.dim; i++)
	{
		if( !(MBR1[i].max == MBR2[i].max && 
			MBR1[i].min == MBR2[i].min))
			return false;
	}
	return true;
}

/*
* calc_MBR is used to find the closing MBRangle for a node.
*
*  pnode indicates the node in rtree.
*  MBR gets back the closing MBRangle(allocated outside the function).
* 
* return void.
*/
void calc_MBR( node_t* pnode, range* &MBR)
{
	//The interface of "MBR" is due to the R-tree implementation.
	int i, j;
	for( j=0; j<IRTree_v.dim; j++)         //dimensions.
	{
		MBR[j].min = FLT_MAX;
		MBR[j].max = -FLT_MAX;

		for( i=0; i<pnode->num; i++)      //entries.
		{
			if( pnode->MBRs[i][j].min < MBR[j].min)
				MBR[j].min = pnode->MBRs[i][j].min;
			if( pnode->MBRs[i][j].max > MBR[j].max)
				MBR[j].max = pnode->MBRs[i][j].max;
		}
	}
}

/*
 *	Same usage as calc_MBR with a different interface.
 */
range* get_MBR_node( node_t* pnode, int dim)
{
	range* MBR;

	MBR = ( range*)malloc( dim * sizeof( range));
	memset( MBR, 0, dim * sizeof( range));

	calc_MBR( pnode, MBR);

	return MBR;
}

/*
* GetArea is used to get the area of closing MBRangle of a node.
*
*  pnode indicates the node.
*
* return the area MBRs.
**/
double GetArea( node_t* pnode)
{
	int i;
	double area;
	range* MBR;

	MBR = get_MBR_node( pnode, IRTree_v.dim);

	area = 1;
	for(i=0; i<IRTree_v.dim; i++)
		area *= MBR[i].max - MBR[i].min;

	free( MBR);
	return area;
}

/*
* GetArea_2 is used to get the closing area of two MBRangles.
* 
*  MBR1, MBR2 indicates two MBRangles respectively.
*
* return the area.
**/ 
double GetArea_2( range* MBR1, range* MBR2)
{
	double area;
	float high, low;
	int i;

	area = 1;
	for( i=0; i<IRTree_v.dim; i++)
	{	
		high = MBR1[i].max >= MBR2[i].max ? MBR1[i].max : MBR2[i].max;
		low = MBR1[i].min <= MBR2[i].min ? MBR1[i].min : MBR2[i].min;
		
		area *= high - low;
	}

	return area;
}

/*
* GetIncArea is used to get the increased area when adding a MBRangle into a node.
*
*  pnode indicates the node.
*  MBR indicates the MBRangle.
*
* return the increased area.
**/
double GetIncArea( node_t* pnode, range* MBR)
{
	double area1, area2;
	
	//Original Area in pnode.
	area1 = GetArea( pnode);

	//Area after adding MBR into node_1.
	pnode->MBRs[ pnode->num] = MBR;
	pnode->num ++;
	area2 = GetArea( pnode);
	pnode->num --;
	
	return area2 - area1;
}

/*
*This funtion is used to locate the closing MBRangle 
*which need the least enlargement in order to enclose the obj.
*
*  pnode indicates the node in the rtree.
*  obj indicates the object.
*  loc get back the location of subtree to be propagate.
*
* return void.
**/
void NeedLeastArea( node_t* pnode, range* obj, int& loc)
{
	int i, j;
	double ori, enl, pre_area, min_inc_area;
	range* MBR, *MBR_tmp;

	//use MBR_tmp to denote the enlarged MBRangle.
	MBR_tmp = ( range*)malloc( IRTree_v.dim * sizeof( range));
	memset( MBR_tmp, 0, IRTree_v.dim * sizeof( range));

	loc = 0;
	min_inc_area = DBL_MAX;
	for(i=0; i<pnode->num; i++)            //entries
	{
		ori = 1;
		enl = 1;
		MBR = pnode->MBRs[i];
		for( j=0; j<IRTree_v.dim; j++)              //dimensions
		{
			MBR_tmp[ j].min = MBR[ j].min <= obj[ j].min ?
				MBR[ j].min : obj[ j].min;
			MBR_tmp[ j].max = MBR[ j].max >= obj[ j].max ?
				MBR[ j].max : obj[ j].max;

			ori *= MBR[ j].max - MBR[ j].min;
			enl *= MBR_tmp[ j].max - MBR_tmp[ j].min;
		}//for(j)
		
		if( min_inc_area > enl-ori)
		{
			min_inc_area = enl - ori;
			loc = i;
			pre_area = ori;
		}
		else if( min_inc_area == enl-ori)
		{
			if( ori < pre_area)
			{
				pre_area = ori;
				loc = i;
			}
		}
	}//for(i)

	free(MBR_tmp);

	return;
}

/*
* UpdateMRB is used to find the closing MBRangle & judgement of enlargement.
*
*  same with 'calc_MBR's.
*
* return true if enlarged, else false.
*/
bool UpdateMRB( node_t* pnode, range* &MBR)
{
	int i, j;
	float min, max;
	bool res;

	res = false;
	for( i=0; i<IRTree_v.dim; i++)		//dimensions.
	{
		min = FLT_MAX;
		max = -FLT_MAX;

		for(j=0; j<pnode->num; j++)		//entries.
		{
			if( pnode->MBRs[j][i].min < min)
				min = pnode->MBRs[j][i].min;
			if( pnode->MBRs[j][i].max > max)
				max = pnode->MBRs[j][i].max;
		}

		//Be careful of the 'initial' conditions of MBR--bug (bigger OR smaller).
		if(MBR[i].max != max)
		{
			MBR[i].max = max;
			res = true;
		}
		if(MBR[i].min != min)
		{
			MBR[i].min = min;
			res = true;
		}
	}

	return res;
}  

/*------------------------Insertion related APIs-------------------------*/

void ChooseLeaf( node_t* root, range* MBR, node_t* &leaf_node)
{
	int loc = 0;
	while(1)
	{	
		if(root->level==0)
		{
			//root node is a leaf-node.
			leaf_node = root;
			return;
		}		
		NeedLeastArea( root, MBR, loc);
		root = ( node_t*)( root->child[loc]);
	}
}

// This function is used to pick up two seed MBRangles.

void PickSeeds( node_t* pnode, range* MBR, int& g1, int& g2)
{
	int i, j, k;
	double area, area1, area2, max = -DBL_MAX;
	range* MBR1, *MBR2, *MBR_tmp;
	
	MBR_tmp = ( range*)malloc( sizeof(range));
	memset( MBR_tmp, 0, sizeof( range));
	
	MBR1 = MBR;
	for(i=-1; i<pnode->num; )			
	{
		for(j=0; j<pnode->num; j++)	
		{
			MBR2 = pnode->MBRs[j];

			if( j != i)
			{
				area = 1;
				area1 = 1;
				area2 = 1;

				for(k=0; k<IRTree_v.dim; k++)
				{	
					//MBR_tmp is used to enclose the two MBRs: MBR1, MBR2.
					MBR_tmp->max = (MBR1[k].max >= MBR2[k].max)? MBR1[k].max : MBR2[k].max;
					MBR_tmp->min = (MBR1[k].min <= MBR2[k].min)? MBR1[k].min : MBR2[k].min;
					area *= MBR_tmp->max - MBR_tmp->min;
					area1 *= MBR1[k].max - MBR1[k].min;
					area2 *= MBR2[k].max - MBR2[k].min;
				}//
				
				if( area - area1 - area2 > max)
				{
					max = area - area1 - area2;
					g1 = i;
					g2 = j;
				}
			}//if(j!=i)
		}//if(j)

		MBR1 = pnode->MBRs[ ++i];
	}//if(i)
	
	free( MBR_tmp);
}

// PickNext is used to pick the new item to be grouped.
int PickNext( node_t* pnode, node_t* node_1, node_t* node_2, node_t* &node_c, range* MBR, bool* done, int rTag)
{	
	int i, next;
	double max = -DBL_MAX, d1, d2, dif;
	range* MBR_tmp;

	if( rTag == 0)
	{
		i = -1;
		MBR_tmp = MBR;
	}
	else
	{
		i = 0;
		MBR_tmp = pnode->MBRs[ i];
	}

	for( ; i<pnode->num; )
	{	
		if( !( i>= 0 && done[ i] == true))
		{				
			d1 = GetIncArea( node_1, MBR_tmp);
			d2 = GetIncArea( node_2, MBR_tmp);			
			dif = d2 - d1;	
			
			if( fabs( dif) > max )
			{
				max = fabs( dif);
				next = i;
				//Decide the node.
				if(dif > 0)
					node_c = node_1;
				else if( dif == 0)		
				{
					d1 = GetArea( node_1);
					d2 = GetArea( node_2);
					if( d1 < d2)
						node_c = node_1;
					else if( d1 == d2)
					{
						if( node_1->num <= node_2->num)
							node_c = node_1;
						else
							node_c = node_2;
					}
					else
						node_c = node_2;
				}
				else
					node_c = node_2;
			}
		}
		i++;
		MBR_tmp = pnode->MBRs[ i];
	}
	return next;
}

// 	Assigne an entry to a node.

void AssignEntry( node_t* pnode, node_t* node_c, void* obj, range* MBR)
{
	node_c->child[ node_c->num] = obj;
	node_c->MBRs[ node_c->num] = MBR;
	
	if( pnode->level > 0)
	{
		( ( node_t*)obj)->parent = node_c;
		( ( node_t*)obj)->loc = node_c->num;
	}
	node_c->num ++;
}

// Split node into two when needed
void SplitNode( node_t* &pnode, node_t* &node_2, void* obj, int opt)
{   
	int i, g1, g2, rTag = 0, qCase, next = 0;
	node_t* node_c, *node_1;

	CreateNode( node_1);
	CreateNode( node_2);
	
	//done marks the elements in pnode that have been grouped.
	bool* done = (bool*)malloc( sizeof( bool) * M);
	memset( done, 0, sizeof( bool) * M);

	//PickSeeds.
	range* MBR;
	if( pnode->level > 0)
	{	
		//The inserted is a subtree.
		MBR = ( range*)malloc( IRTree_v.dim * sizeof( range));
		memset( MBR, 0, IRTree_v.dim * sizeof( range));

		calc_MBR( ( node_t*)obj, MBR);
	}
	else
	{
		//The inserted is an object.
		MBR =( ( obj_t*)obj)->MBR;
	}

	PickSeeds( pnode, MBR, g1, g2);
	
	//Assign the two entries indicated by g1, g2.
	if( g1 == -1)    //obj
	{
		//g1 indicates the obj.
		AssignEntry( pnode, node_2, obj, MBR);
		rTag = 1;
	}
	else
	{
		//g1 indicates an entry of pnode.
		AssignEntry( pnode, node_2, pnode->child[ g1], pnode->MBRs[ g1]);
		done[ g1] = true;   
	}

	AssignEntry( pnode, node_1, pnode->child[ g2], pnode->MBRs[ g2]);
	done[ g2] = true;       
	
	//Assign the rest entries.
	while( true)
	{
		if( node_1->num + node_2->num == M + 1 )
		{   
			// 1. All entries have been assigned.
			qCase = 1;
			break;
		}
		else if( node_1->num == M + 1 - m1)
		{	
			// 2. The rest should be assigned to node_2.
			qCase = 2;
			break;
		}
		else if( node_2->num == M + 1 - m1)
		{	
			// 3. The rest should be assigned to node_1.
			qCase = 3;
			break;
		}

		next = PickNext( pnode, node_1, node_2, node_c, MBR, done, rTag);

		//Assign the 'next' entry.
		if( next == -1)	//the 'next' is obj.
		{
			AssignEntry( pnode, node_2, obj, MBR);
			rTag = 1;
		}
		else
		{
			AssignEntry( pnode, node_c, pnode->child[ next], pnode->MBRs[ next]);
			done[ next] = true;
		}
	}//while(1)
	
	//Assign the rest entries if necessary.
	if( qCase == 2)
		node_c = node_2;
	else if( qCase == 3)
		node_c = node_1;
	else
		goto E;

	if( rTag == 0)
	{
		AssignEntry( pnode, node_c, obj, MBR);
		rTag = 1;
	}	
	
	for(i=0; i<pnode->num; i++)
	{
		if( done[i] == false)
		{
			AssignEntry( pnode, node_c, pnode->child[ i], pnode->MBRs[ i]);
			done[ i] = true;
		}
	}
	
	//
E:	node_1->parent = pnode->parent;	
	node_1->loc = pnode->loc;
	node_1->level = pnode->level;
	//((node_t*)(pnode->parent))->child[pnode->loc] = node_1;
	
	node_2->level = pnode->level;
	//Note that node_2's 'parent' & 'loc' haven't been set.
	
	node_c = pnode;
	pnode = node_1;

	//Free the resource.
	ReleaseNode( node_c, 0);
	free( done);
}

/*
 *	Create a new root due to the split operation.
 *	node_1 is the original root.
 *	node_2 is the new split node from node_1.
 */
node_t* CreateNewRoot( node_t* node_1, node_t* node_2)
{
	int i;
	node_t* nRoot;

	CreateNode( nRoot);
	nRoot->num = 2;
	nRoot->parent = NULL;
	nRoot->level = node_1->level + 1;

	for( i=0; i<2; i++)
	{
		nRoot->child[ i] = ( node_t*)malloc( sizeof(node));
		memset( nRoot->child[ i], 0, sizeof( node));

		nRoot->MBRs[ i] = ( range*)malloc( IRTree_v.dim * sizeof( range));
		memset( nRoot->MBRs[ i], 0, IRTree_v.dim * sizeof( range));
	}

	/*s*/
	stat_v.tree_memory_v += 2 * ( sizeof( node) + IRTree_v.dim * sizeof( range));
	if( stat_v.tree_memory_v > stat_v.tree_memory_max)
		stat_v.tree_memory_max = stat_v.tree_memory_v;
	/*s*/

	//Adjust the two children of nRoot: node_1, node_2;
	node_1->parent = nRoot;
	node_1->loc = 0;
	calc_MBR( node_1, nRoot->MBRs[ 0]);
	nRoot->child[ 0] = node_1;

	node_2->parent = nRoot;
	node_2->loc = 1;
	calc_MBR( node_2, nRoot->MBRs[ 1]);
	nRoot->child[ 1] = node_2;

	const_IF( nRoot);

	IRTree_v.inner_n ++;

	return nRoot;
}

/*
* InsertSub is used to insert a node/subtree recursively.
*/
node_t* InsertSub( node_t* pnode, void* obj)
{
	if( pnode->num < M)
	{
		//pnode has room for the new entry.
		if( pnode->level > 0)
		{
			//iner-node case.
			pnode->MBRs[ pnode->num] = ( range*)malloc( IRTree_v.dim * sizeof( range));
			memset( pnode->MBRs[ pnode->num], 0, IRTree_v.dim * sizeof( range));
			
			calc_MBR( ( node_t*)obj, pnode->MBRs[ pnode->num]);
			( (node_t*)obj)->parent = pnode;
			( (node_t*)obj)->loc = pnode->num;                      //bug.

			/*s*/
			stat_v.tree_memory_v += IRTree_v.dim * sizeof( range);
			if( stat_v.tree_memory_v > stat_v.tree_memory_max)
				stat_v.tree_memory_max = stat_v.tree_memory_v;
			/*s*/
		}
		else
		{
			//leaf-node case.
			pnode->MBRs[ pnode->num] = (( obj_t*)obj)->MBR;
		}

		pnode->child[ pnode->num] = obj;
		pnode->num ++;

		//Adjust the tree.
		AdjustTree( pnode);

		//Update the IF information.
		adjust_IF( pnode, obj, pnode->num - 1);

		return NULL;

	}//pnode->num < IRTree_v.
	else
	{
		//No free room for the inserted object, need split node.
		node_t* node_2;
		
		//SplitNode: QuadraticSplit.
		SplitNode( pnode, node_2, obj, IRTree_v.split_opt);

		if( pnode->level == 0)
			IRTree_v.leaf_n ++;
		else
			IRTree_v.inner_n ++;

		//Construct the IFs for the newly-created nodes.
		const_IF( pnode);
		const_IF( node_2);

		//Insert the split node recursively.
		if( pnode->parent == NULL)
		{	
			//The current node is the root node.
			//Create a new root.
			return CreateNewRoot( pnode, node_2);
		}

		node_t* pP;
		pP = pnode->parent;
		pP->child[ pnode->loc] = pnode;	

		AdjustTree( pnode);

		adjust_parent_IF( pP, pnode->loc);

	return InsertSub( pP, node_2);
	}
}

/*
* AddjustTree is used to adjust the rtree upward after inserting.
*
*  pnode indicates the current node.
* 
* return void.
*/
void AdjustTree( node_t* pnode)
{
	node_t* pP;

	pP = pnode->parent;
	while( pP != NULL)
	{
		if( !UpdateMRB( pnode, pP->MBRs[ pnode->loc]))
			return;
		else
		{
			pnode = pP;
			pP = pnode->parent;
		}			
	}
}	

/*
* Insert is used to insert object into the rtree(root).
**/
void Insert( obj_t* obj)
{
	node_t* pnode, *res;

	ChooseLeaf( IRTree_v.root, obj->MBR, pnode);
    ///pnode is the node the insert in
	res = InsertSub( pnode, obj );

	if( res != NULL)
	{	
		//The root has been changed.
		IRTree_v.root = res;
		IRTree_v.height ++;
	}

	return;
}

/*----------------------Delettion related APIs-------------------------*/

/*
*This function is used to 'Search' the object.
* return the leaf-node that includes the object, else NULL.
**/
node_t* Search( node_t* root, range* MBR, int& loc)
{
	int i;
	node_t* node_c;
	
	if( root->level > 0)
	{
		//if the current node is non-leaf node.
		for( i=0; i<root->num; i++)
		{
			if( IsOverlapped(root->MBRs[i], MBR) == 1)
			{
				node_c = Search( ( node_t*)( root->child[ i]), MBR, loc);
				if( node_c != NULL)
					return node_c;
			}
		}
	}
	else
	{
		//if the current node is leaf-node.
		for(i=0; i<root->num; i++)
		{
			if( IsSame( root->MBRs[i], MBR))	
			{
				loc = i;
				return root;
			}
		}
	}//else	

	return NULL;
}

// ChooseInner is used to choose the inner node for Re-Insertion w.r.t. pnode.

void ChooseInner( node_t* root, node_t* pnode, node_t* &inner_node)
{
	int loc;
	range* MBR;
	
	MBR = ( range*)malloc( sizeof( range));
	memset( MBR, 0, sizeof( range));

	calc_MBR( pnode, MBR);

	while(1)
	{
		if( root->level == pnode->level+1)
		{
			inner_node = root;
			return;
		}

		NeedLeastArea( root, MBR, loc);
		root = ( node_t*)( root->child[loc]);
	}

	free( MBR);
}

/*
* CondenseSub is used as a sub function in CondenseTree to generate the two chains.
* return void.
**/
void CondenseSub( node_t* pnode, objPointer* h1, nodePointer* h2 )
{
	int i;
	node_t* pParent;
	range* rTmp;

	if( pnode->parent == NULL)
	{	
		//root node.
		return;
	}
	else if( pnode->num >= m1)
	{	
		//Adjust the tree.
		AdjustTree( pnode);	
	}
	else	//pnode->num < m
	{
		//pnode is not a "full" enough.
		//Thus, all children of pnode need to be re-inserted.

		//Delete pnode's entry from its parent node.
		pParent = pnode->parent;
		if( pnode->loc != pParent->num-1)
		{	
			rTmp = pParent->MBRs[ pnode->loc];
			pParent->child[ pnode->loc] = pParent->child[ pParent->num-1];	//child
			pParent->MBRs[ pnode->loc] = pParent->MBRs[ pParent->num-1];		//MBRs
			
			( ( node_t*)( pParent->child[ pnode->loc]))->loc = pnode->loc;		//
		}
		else
		{
			rTmp = pParent->MBRs[ pParent->num-1];
		}

		pParent->num --;
		free( rTmp);	
		
		//Add the entries in pnode into two chains.
		if(pnode->level > 0)
		{
			//pnode is a non-leaf node.
			//Add the entries in pnode into head2 (node chain).
			for(i=0; i<pnode->num; i++)
			{
				h2->p = ( node_t*)( pnode->child[ i]);
				h2->next = ( nodePointer*)malloc( sizeof( nodePointer));
				memset( h2->next, 0, sizeof( nodePointer));

				h2 = h2->next;
			}
		}
		else	//pnode->level == 0
		{
			//pnode is a leaf node.
			//Add the entries in pnode into head1 (obj chain).
			for(i=0; i<pnode->num; i++)
			{
				h1->p = ( obj_t*)( pnode->child[i]);
				h1->next = ( objPointer*)malloc( sizeof( objPointer));
				memset( h1->next, 0, sizeof( objPointer));

				h1 = h1->next;
			}
		}
		
		//Release the pnode.
		ReleaseNode( pnode, 1);

		if( pnode->level == 0)
			IRTree_v.leaf_n --;
		else
			IRTree_v.inner_n --;

		//Condense upward recursively.
		CondenseSub( pParent, h1, h2);
	}//else
}

/*
* CondenseTree is used to condense the tree after deletion operation.
* 
*  pnode indicates the node, where deletion has occured.
*
* return void.
*/ 
void CondenseTree( node_t* pnode)
{
	objPointer* head1, *h1, *eTag1;
	nodePointer* head2, *h2, *eTag2;

	//Create two chains to store the "re-insert entries".
	head1 = ( objPointer*)malloc( sizeof( objPointer));
	memset( head1, 0, sizeof( objPointer));
	head2 = ( nodePointer*)malloc( sizeof( nodePointer));
	memset( head2, 0, sizeof( nodePointer));

	h1 = head1;
	h2 = head2;
	
	//CondenseSub.
	CondenseSub( pnode, h1, h2);
	
	//Perform the re-insertion operations for head1.
	h1 = head1;
	eTag1 = h1->next;
	while( h1 != NULL)
	{
			node_t* lnode, *res;
			ChooseLeaf( IRTree_v.root, h1->p->MBR, lnode);
			res = InsertSub( lnode, h1->p);
			if( res)
			{
				IRTree_v.root = res;
				IRTree_v.height ++;
			}

		free(h1);

		if( eTag1 == NULL)
			break;

		h1 = eTag1;
		eTag1 = h1->next;
	}

	free(h1);

	//Re-Insertion operation for head2.
	h2 = head2;
	eTag2 = h2->next;
	while( h2 != NULL)
	{
			node_t* inNode, *res;

			ChooseInner( IRTree_v.root, h2->p, inNode);
			res = InsertSub( inNode, h2->p);
			if( res)
			{
				//The root has been changed.
				IRTree_v.root = res;
				IRTree_v.height ++;
			}

		free(h2);

		if( eTag2 == NULL)
			break;

		h2 = eTag2;
		eTag2 = h2->next;
	}	

	free(h2);
}

//  Delete is used to delete an entry from the rtree.

int Delete( obj_t* obj)
{
	int loc;
	node_t* lnode, *root;
	obj_t* oTmp;

	root = IRTree_v.root;
	lnode = Search( root, obj->MBR, loc);

	if( lnode == NULL)
	{	
		return 0;
	}

	oTmp = ( obj_t*)( lnode->child[loc]);

	lnode->child[ loc] = lnode->child[ lnode->num-1];
	lnode->MBRs[ loc] = lnode->MBRs[ lnode->num-1];
	lnode->num --;
	IRTree_v.obj_n --;

	free( oTmp->MBR);
	free( oTmp);

	//CondenseTree.
	CondenseTree( lnode);
	
	//Update the level if necessary (the root has only one child).
	if( IRTree_v.root->num == 1 && IRTree_v.root->level > 0)
	{
		lnode = IRTree_v.root; 
		IRTree_v.root = ( node_t*)( IRTree_v.root->child[0]);
		IRTree_v.root->parent = NULL;
		IRTree_v.root->loc = 0;
		IRTree_v.height --;

		//Release the original root node.
		ReleaseNode( lnode, 1);

		IRTree_v.inner_n --;
	}
	else if( IRTree_v.root->num == 0)
	{	
		//Extreme case.
		IRTree_v.height = 0;
	}

	return 1;
}

/*---------------------------Other Operation APIs---------------------------------*/

//  ini_tree is used to initialize a rtree

void ini_tree( )
{
	memset( &IRTree_v, 0, sizeof( IRTree_t));

	CreateNode( IRTree_v.root);
	IRTree_v.leaf_n ++;
}

/*
* Close is used to *Test* tree, save the rtree ( rTreeSaveFile), and free the resource.
*
*  cfg involves the configuration info.
*  print the tree to the file if o_tag = 1;
* otherwise, print the tree in the console.
*
* return true if succeed; otherwise return false.
*/
bool print_and_check_tree( int o_tag, const char* tree_file)
{
	int i, j, cnt = 1, inner_cnt, leaf_cnt;
	float min, max;
	FILE* fp;
	
	//Using the 'Circle Queue' to enlarge the space utility.
	if( o_tag == 1)
		fp = fopen( tree_file, "w");
	else 
		fp = stdout;
	if( fp == NULL)
	{
		fprintf( stderr, "Cannot create Save_File.\n");
		exit(0);
	}
	
	fprintf( fp, "                                                  R-Tree Information\n");
	fprintf( fp, "M\tm\tInner-nodes\tLeaf-nodes\tObjects\tDimension\n");
	fprintf( fp, "%i\t%i\t%i\t\t\t%i\t\t%i\t%i\n\n", 
				M, m1, IRTree_v.inner_n, IRTree_v.leaf_n, IRTree_v.obj_n, IRTree_v.dim);
	fprintf( fp, "num   level   loc addr              parent\n");
	
	int sta, rear;
	node *c_node, *root;
	node_t** queue;

	queue = ( node_t**)malloc( ( IRTree_v.obj_n + 1) * sizeof( node_t*));
	memset( queue, 0, ( IRTree_v.obj_n + 1) * sizeof( node_t*));
	
	sta = rear = 0;
	root = IRTree_v.root;
	queue[ rear] = root;
	rear = ( rear + 1) % ( IRTree_v.obj_n + 1);	//macro's pit.

	inner_cnt = leaf_cnt = 0;
	while( sta != rear)
	{
		c_node = queue[ sta];
		sta = ( sta + 1) % ( IRTree_v.obj_n + 1);

		if( c_node->level == 0)
			leaf_cnt ++;
		else
			inner_cnt ++;
		
		//Print the information of c_node.
		fprintf( fp, "%-4i  %-4i    %-4i%-12X      %-8X\n", 
			c_node->num, c_node->level, c_node->loc, c_node, c_node->parent);
		
		if( c_node->num == 0)
			break;
		for( i=0; i<c_node->num; i++)
		{
			if( c_node->level > 0)
				fprintf( fp, "%-18X", c_node->child[i]);
			else
				fprintf( fp, "%-18i", ( ( obj_t*)(c_node->child[ i]))->id);
		}

		fprintf( fp, "%-8s  %-8s", "min", "max");
		fprintf( fp, "\n");
		
		//Print the 'child' and 'MBR' info.
		for( j=0; j<IRTree_v.dim; j++)
		{	
			min = FLT_MAX;
			max = -FLT_MAX;

			for( i=0; i<c_node->num; i++)
			{
				fprintf( fp, "%-8.2f  %-8.2f", 
					c_node->MBRs[i][j].min, c_node->MBRs[i][j].max );

				if( c_node->MBRs[i][j].min < min)
					min = c_node->MBRs[i][j].min;
				if( c_node->MBRs[i][j].max > max)
					max = c_node->MBRs[i][j].max;
			}
/*t*/
			//Test the MBR informatin of the tree.
			if( c_node->parent != NULL)
			{
				if( !( min == c_node->parent->MBRs[ c_node->loc][j].min && 
					max == c_node->parent->MBRs[ c_node->loc][j].max))
					fprintf( stderr, "**");
			}
/*t*/
			fprintf( fp, "%-8.2f  %-8.2f", min, max);
			fprintf( fp, "\n");
		}

		//Print the IF info.
		if( o_tag == 1)
			print_IF( c_node, fp, 2);
		else
			print_IF( c_node, stdout, 2);
		
		//Enqueue the children of the current node.
		if( c_node->level > 0)
		{
			for(i=0; i<c_node->num; i++)
			{
				queue[ rear] = ( node_t*)( c_node->child[i]);
				rear = ( rear + 1) % ( IRTree_v.obj_n + 1);	//macro's pit.

				//Test the 'loc' and 'level' information.
/*t*/
				if( ((node_t*)(c_node->child[i]))->loc != i)
					fprintf( stderr, "The %i'th child's loc is wrong.\n", i+1);
				if( ((node_t*)(c_node->child[i]))->level != c_node->level-1)
					fprintf( stderr, "The %i'th child's level is wrong.\n", i+1);
/*t*/
			}
		}

		fprintf( fp, "\n");
	}//while
	
	free( queue);
	if( o_tag == 1)
		fclose( fp);

	if( IRTree_v.inner_n != inner_cnt)
		printf( "Inconsistent inner_n info!\n");
	if( IRTree_v.leaf_n != leaf_cnt)
		printf( "Inconsistent leaf_n info!\n");

	return true;
}

/*---------------------Added APIs-2011-03-09------------------------*/

/*
*	build_IRTree constructs the IR-tree based on the data file.
*/
void build_IRTree( data_t* data_v)
{
	int i;
    printf("initilizing tree\n");
	//Initialize a IR-tree.
	ini_tree( );
	IRTree_v.dim = data_v->dim;
    printf("inserting objects\n");
	
	//Insert all the objects to construct the IR-tree.
	for( i=0; i<data_v->obj_n; i++)
	{
        Insert(data_v->obj_v + i);
        // printf("%d\t",(data_v->obj_v + i)->id);
		IRTree_v.obj_n ++;
	}
}
/*
 *	Print the IF information of a node.
 */
void print_IF( node_t* node_v, FILE* fp, int p_tag)
{
	int tag;
	bst_node_t* x;

	tag = 0;
	x = node_v->bst_v->root;
	
	while( get_next_in_order( x, tag))
	{
		fprintf( fp, "%.0lf:\t", x->key);
		
		if( p_tag == 1)
		{	
			//Option 1: printing for reading by human.
			int i;
			for( i=0; i<M; i++)
			{
				if( get_k_bit( x->p_list, i))
					fprintf( fp, "%i ", i);
			}
			fprintf( fp, "\n");
		}
		else
		{
			//Option 2: printing for read_tree.
			fprintf( fp, "%u\n", x->p_list);
		}

		if( !in_order_sub( x, tag))
			break;
	}
	fprintf( fp, "-1\n");
}

/*---------------------IR-tree augmentation APIs------------------------*/

/*
 *	Get the keywords from a keyword list.
 *
 *	k_head is the keyword list.
 *
 *	return the keyword lit.
 */
k_node_t* collect_keywords_list( k_node_t* k_head)
{
	k_node_t* k_node_v, *k_node_v1, *k_node_v2;
	
	k_node_v = ( k_node_t*)malloc( sizeof( k_node_t));
	memset( k_node_v, 0, sizeof( k_node_t));

	k_node_v1 = k_node_v;
	k_node_v2 = k_head->next;
	while( k_node_v2 != NULL)
	{
		add_keyword_entry( k_node_v1, k_node_v2->key);

		k_node_v2 = k_node_v2->next;
	}

	return k_node_v;
}

/*
 *	Get all the keywords in a binary tree.
 *	Method: a non-recursion implementation of the "in-order" traversal.
 *
 *	bst_v is the binary tree.
 *
 *	return the keyword list.
 */
k_node_t* collect_keywords_bst( bst_t* bst_v)
{
	int tag;
	k_node_t* k_node_v, *k_node_v1;
	bst_node_t* x;

	k_node_v = ( k_node_t*)malloc( sizeof( k_node_t));
	memset( k_node_v, 0, sizeof( k_node_t));

	//Traverse the binary search tree: pre-order, DFS.	
	x = bst_v->root;
	tag = 0;

	k_node_v1 = k_node_v;
	while( get_next_in_order( x, tag))
	{
		add_keyword_entry( k_node_v1, x->key);

		if( !in_order_sub( x, tag))
			break;
	}

	return k_node_v;
}


/*
 *	Get the keywords from a fea
 *
 *	k_head is the keyword list.
 *
 *	return the keyword lit.
 */
k_node_t* collect_keywords_fea(FEA_TYPE fea)
{
    k_node_t* k_node_v, *k_node_v1;
    
    k_node_v = ( k_node_t*)malloc( sizeof( k_node_t));
    memset( k_node_v, 0, sizeof( k_node_t));
    
    k_node_v1 = k_node_v;
    
    add_keyword_entry( k_node_v1, fea);
    
    return k_node_v;
}

/*
 *
 *	return 1 if the key is a new keyword; otherwise, return 0.
 */
int add_IF_entry( bst_t* bst_v, KEY_TYPE key, int loc)
{
	bst_node_t* bst_node_v;

	bst_node_v = bst_search( bst_v, key);
	if( bst_node_v != NULL)
	{
		//Update the posting list.
		insert_k_bit( bst_node_v->p_list, loc);
		
		return 0;
	}
	else	//bst_node_v = NULL
	{
		//Insert a new keyword in the binary tree.
		bst_node_v = ( bst_node_t*)malloc( sizeof( bst_node_t));
		memset( bst_node_v, 0, sizeof( bst_node_t));

		/*s*/
		stat_v.tree_memory_v += sizeof( bst_node_t);
		if( stat_v.tree_memory_v > stat_v.tree_memory_max)
			stat_v.tree_memory_max = stat_v.tree_memory_v;
		/*s*/

		//Update the posting list.
		bst_node_v->key = key;
		insert_k_bit( bst_node_v->p_list, loc);

		bst_insert( bst_v, bst_node_v);

		return 1;
	}
}

/*
 *	Adjust the IFs of the node and its anscestors 
 *	when an obj/subtree is inserted in this node.
 *
 *	c_node is the node.
 *	obj is the obj/subtree.
 *	loc is the loc of obj.
 */
void adjust_IF( node_t* c_node, void* obj, int loc)
{
	int res;
	k_node_t* new_keys, *k_node_v1, *k_node_v2;

	new_keys = ( k_node_t*)malloc( sizeof( k_node_t));
	memset( new_keys, 0, sizeof( k_node_t));
	
	//The list for maintaining the new keywords.
	//Retrieve the set of keywords.
	if( c_node->level == 0)
	{
		//obj corresponds to an object.
		//Collect the keywords from the keyword list of obj.
		//new_keys = collect_keywords_list( ( ( obj_t*)obj)->k_head);
       new_keys = collect_keywords_fea( ( ( obj_t*)obj)->fea);
	}
	else
	{
		//obj corresponds to a subtree.
		//Collect the keywords from the binary tree of obj.
		new_keys = collect_keywords_bst( ( ( node_t*)obj)->bst_v);
	}

	//Set the loc.
	//loc = c_node->num;

	while( true)
	{		
		//For each keyword t in new_keys,
		//add the loc to the posting list of t in the IF of c_node.
		//( t, loc)
		k_node_v2 = new_keys;
		k_node_v1 = new_keys->next;
		while( k_node_v1 != NULL)
		{
			res = add_IF_entry( c_node->bst_v, k_node_v1->key, loc);

			if( res == 0)
			{
				//k_node_v1->key exists in c_node->bst_v.
				//Delete k_node_v1 from the new_keys list.
				k_node_v2->next = k_node_v1->next;
				free( k_node_v1);
				k_node_v1 = k_node_v2->next;
			}
			else	//res == 1
			{
				//k_node_v1->key is a new keyword.
				//k_node_v1 should be kept for updating the further ancesters.
				k_node_v2 = k_node_v1;
				k_node_v1 = k_node_v1->next;
			}
		}//while( k_node_v1)

		//Propagate the update to the ancestors.
		if( new_keys->next != NULL && c_node->parent != NULL)
		{
			loc = c_node->loc;
			c_node = c_node->parent;
		}
		else
			break;
	}//while( true)

	//Release the resource.
	release_k_list( new_keys);
}

/*
 *	Construct the IF information for a node.
 *	Method: traverse the keyword list of each entry of the node.
 */
void const_IF( node_t* node_v)
{
	int loc;
	k_node_t* k_node_v, *k_node_v1;
	
	for( loc=0; loc<node_v->num; loc++)
	{
		if( node_v->level == 0)
        {
            k_node_v = collect_keywords_fea( ( ( obj_t*)( node_v->child[ loc]))->fea);
        }else	//
			k_node_v = collect_keywords_bst( ( ( node_t*)( node_v->child[ loc]))->bst_v);
		k_node_v1 = k_node_v->next;
		while( k_node_v1 != NULL)
		{
			add_IF_entry( node_v->bst_v, k_node_v1->key, loc);
			k_node_v1 = k_node_v1->next;
		}
        release_k_list( k_node_v);
	}
}

/*
 *	Delete the "loc" from all posting lists of a IF represented by a bst.
 *	Method: in-order traversal.
 */
void delete_IF_loc( bst_t* bst_v, int loc)
{
	int tag;
	bst_node_t* x;

	//Traverse the binary search tree: pre-order, DFS.
	x = bst_v->root;
	tag = 0;

	while( get_next_in_order( x, tag))
	{
		delete_k_bit( x->p_list, loc);
		if( !in_order_sub( x, tag))
			break;
	}
}

void adjust_parent_IF( node_t* p_node, int loc)
{
	//Remove the entry corresponding to loc in p_node's IF.
	delete_IF_loc( p_node->bst_v, loc);

	//Adjust the IFs of p_node and its ancestors.
	adjust_IF( p_node, ( void*)( p_node->child[ loc]), loc);
}