#include "joinless_utility.h"

/*
 *	Allocate a loc_t structure.
 */
loc_t* alloc_loc( int dim)
{
	loc_t* loc_v;

	loc_v = ( loc_t*)malloc( sizeof( loc_t));
	memset( loc_v, 0, sizeof( loc_t));

	loc_v->coord = ( float*)malloc( dim * sizeof( float));
	memset( loc_v->coord, 0, dim * sizeof( float));

	loc_v->dim = dim;

	return loc_v;
}

/*
 *	Retrieve the loc_t information of an obj_t structure @obj_v.
 */
loc_t* get_obj_loc( obj_t* obj_v)
{
	int i;
	loc_t* loc_v;

	loc_v = alloc_loc( RTree_v.dim);
	for( i=0; i<RTree_v.dim; i++)
		loc_v->coord[ i] = obj_v->MBR[ i].min;
	
	return loc_v;
}

/*
 *	Copy a loc_t structure.
 */
loc_t* copy_loc( loc_t* loc_v)
{
	int j;
	loc_t* loc_v1;

	loc_v1 = alloc_loc( loc_v->dim);

	for( j=0; j<loc_v->dim; j++)
		loc_v1->coord[ j] = loc_v->coord[ j];

	return loc_v1;
}

/*
 *	Release a loc_t structure.
 */
void release_loc( loc_t* loc_v)
{
	free( loc_v->coord);
	free( loc_v);
}

/*
 *	Allocate a psi_t structure.
 */
psi_t* alloc_psi( )
{
	psi_t* psi_v;

	psi_v = ( psi_t*)malloc( sizeof( psi_t));
	memset( psi_v, 0, sizeof( psi_t));

	psi_v->k_head = ( k_node_t*)malloc( sizeof( k_node_t));
	memset( psi_v->k_head, 0, sizeof( k_node_t));

    /*s*/
    stat_v.memory_v += sizeof(psi_t) + sizeof( k_node_t);
    if( stat_v.memory_v > stat_v.memory_max)
        stat_v.memory_max = stat_v.memory_v;
    /*s*/

	return psi_v;
}

/*
 *	Add an keyword into the psi_t structure.
 */
void add_psi_entry( psi_t* psi_v, KEY_TYPE key)
{
	k_node_t* k_node_v;

	k_node_v = ( k_node_t*)malloc( sizeof( k_node_t));
	memset( k_node_v, 0, sizeof( k_node_t));

    /*s*/
    stat_v.memory_v += sizeof( k_node_t);
    if( stat_v.memory_v > stat_v.memory_max)
        stat_v.memory_max = stat_v.memory_v;
    /*s*/

    
	k_node_v->key = key;
	k_node_v->next =  psi_v->k_head->next;
	psi_v->k_head->next = k_node_v;
	psi_v->key_n ++;
}

/*
 *	Release a psi_t structure.
 */
void release_psi( psi_t* psi_v)
{
	k_node_t* k_node_v1, *k_node_v2;

    if(psi_v==NULL)
        return;
    
    /*s*/
    stat_v.memory_v -= sizeof( k_node_t)* (psi_v->key_n+1);
    stat_v.memory_v -= sizeof( psi_t);
    /*s*/
    
	k_node_v1 = psi_v->k_head;
	while( k_node_v1->next != NULL)
	{
		k_node_v2 = k_node_v1->next;
		free( k_node_v1);
		k_node_v1 = k_node_v2;
	}
	free( k_node_v1);
    free( psi_v);
    
    
}

/*
 *	Allocate a query_t structure.
 */
query_t* alloc_query( )
{
	query_t* q;
	q = ( query_t*)malloc( sizeof( query_t));
	memset( q, 0, sizeof( query_t));
	return q;
}

/*
 *	Print a query_t structure.
 */
void print_query( query_t* q, FILE* o_fp)
{
	int i;
	k_node_t* k_node_v;

	//Location.
	fprintf( o_fp, "%f", q->loc_v->coord[ 0]);
	for( i=1; i<q->loc_v->dim; i++)
	{
		fprintf( o_fp, ",%f", q->loc_v->coord[ i]);
	}
	fprintf( o_fp, "\n");

	//Keywords.
	fprintf( o_fp, "%i", q->psi_v->key_n);
	k_node_v = q->psi_v->k_head->next;
	while( k_node_v != NULL)
	{
		fprintf( o_fp, ",%i", ( int)( k_node_v->key));
		k_node_v = k_node_v->next;
	}
	fprintf( o_fp, "\n\n");

	return;
}

/*
 *	Read a query instance from a file stream @i_fp.
 */
query_t* read_query( FILE* i_fp)
{
	int i;
	char des;
	KEY_TYPE key;
	query_t* q;
	k_node_t* k_node_v;

	q = alloc_query( );

	q->loc_v = alloc_loc( RTree_v.dim);
	q->psi_v = alloc_psi( );

	//Location.
	if( fscanf( i_fp, "%f", &q->loc_v->coord[ 0]) == EOF)
	{
		release_query( q);
		return NULL;
	}

	for( i=1; i<RTree_v.dim; i++)
		fscanf( i_fp, "%c%f", &des, &q->loc_v->coord[ i]);

	//Keywords.
	k_node_v = q->psi_v->k_head;
	fscanf( i_fp, "%i", &q->psi_v->key_n);
	for( i=0; i<q->psi_v->key_n; i++)
	{
		fscanf( i_fp, "%c%lf", &des, &key);
		add_keyword_entry( k_node_v, key);
	}

	return q;		
}

/*
 *	Release a query_t structure.
 */
void release_query( query_t* q)
{
    
    if(q==NULL) return;
	release_loc( q->loc_v);
	release_psi( q->psi_v);

	free( q);
}

/*
 *	Allocate the memory for a disk_t structure.
 */
disk_t* alloc_disk( int dim)
{
	disk_t* disk_v;

	disk_v = ( disk_t*)malloc( sizeof( disk_t));
	memset( disk_v, 0, sizeof( disk_t));

	disk_v->center = alloc_loc( dim);

	/*s*/
	stat_v.memory_v += sizeof( disk_t);
	if( stat_v.memory_v > stat_v.memory_max)
		stat_v.memory_max = stat_v.memory_v;
	/*s*/
	

	return disk_v;
}

/*
 *	Initialize a disk_t structure.
 */
void set_disk( disk_t* disk_v, loc_t* loc_v, B_KEY_TYPE radius)
{
	int i;

	disk_v->radius = radius;
	
	for( i=0; i<loc_v->dim; i++)
		disk_v->center->coord[ i] = loc_v->coord[ i];
}

/*
 *	Construct a disk_t structure with its center of @loc_v and
 *	its radius of @radius.
 */
disk_t* const_disk( loc_t* loc_v, B_KEY_TYPE radius)
{
	int i;
	disk_t* disk_v;


	disk_v = alloc_disk( RTree_v.dim);
	for( i=0; i<RTree_v.dim; i++)
		disk_v->center->coord[ i] = loc_v->coord[ i];
	disk_v->center->dim = RTree_v.dim;

	disk_v->radius = radius;

	return disk_v;
}

/*
 *	Release a disk_t structure.
 */
void release_disk( disk_t* disk_v)
{
    if(disk_v == NULL) return;
    
	release_loc( disk_v->center);
	free( disk_v);

	/*s*/
	stat_v.memory_v -= sizeof( disk_t);
	/*s*/
}

/*
 *	Allocate the memory for an obj_set_t structure.
 */
obj_set_t* alloc_obj_set( )
{
	obj_set_t* obj_set_v;

	obj_set_v = ( obj_set_t*)malloc( sizeof( obj_set_t));
	memset( obj_set_v, 0, sizeof( obj_set_t));

	obj_set_v->head = ( obj_node_t*)malloc( sizeof( obj_node_t));
	memset( obj_set_v->head, 0, sizeof( obj_node_t));

	/*s*/
	stat_v.memory_v += sizeof( obj_set_t) + sizeof( obj_node_t);
	if( stat_v.memory_v > stat_v.memory_max)
		stat_v.memory_max = stat_v.memory_v;
	/*s*/

	return obj_set_v;
}

/*
 *	Copy an obj_set_t structure.
 */
obj_set_t* copy_obj_set( obj_set_t* obj_set_v)
{
	obj_set_t* rtn;
	obj_node_t* obj_node_v, *obj_node_iter;

	rtn= alloc_obj_set( );
	obj_node_v = rtn->head;

	obj_node_iter = obj_set_v->head->next;
	while( obj_node_iter != NULL)
	{
		obj_node_v->next = ( obj_node_t*)malloc( sizeof( obj_node_t));
		memset( obj_node_v->next, 0, sizeof( obj_node_t));

		obj_node_v = obj_node_v->next;
		obj_node_v->obj_v = obj_node_iter->obj_v;
		
		obj_node_iter = obj_node_iter->next;
	}

	rtn->obj_n = obj_set_v->obj_n;

	/*s*/
	stat_v.memory_v += rtn->obj_n * sizeof( obj_node_t);
	if( stat_v.memory_v > stat_v.memory_max)
		stat_v.memory_max = stat_v.memory_v;
	/*s*/

	return rtn;		
}

/*
 *	Check whether two obj_set_t structures obj_set_v1 and obj_set_v2 have the same content.
 */
bool has_same_content_obj_set( obj_set_t* obj_set_v1, obj_set_t* obj_set_v2)
{
	obj_node_t* iter1, *iter2;

	if( obj_set_v1->obj_n != obj_set_v2->obj_n)
		return false;

	iter1 = obj_set_v1->head->next;
	while( iter1 != NULL)
	{
		iter2 = obj_set_v2->head->next;
		while( iter2 != NULL)
		{
			if( iter2->obj_v == iter1->obj_v)
				break;

			iter2 = iter2->next;
		}

		if( iter2 == NULL)
			return false;

		iter1 = iter1->next;
	}

	return true;
}

/*
 *	Remove the identical objs from @obj_set_v.
 */
void remove_identical_obj( obj_set_t* obj_set_v)
{
	obj_node_t* obj_node_iter1, *obj_node_iter2, *tmp;

	obj_node_iter1 = obj_set_v->head->next;
	while( obj_node_iter1->next != NULL)
	{
		tmp = obj_node_iter1;	
		obj_node_iter2 = obj_node_iter1->next;
		while( obj_node_iter2 != NULL)
		{
			if( obj_node_iter2->obj_v == obj_node_iter1->obj_v)
			{
				//remove.
				tmp->next = obj_node_iter2->next;
				obj_set_v->obj_n --;
				
				free( obj_node_iter2);
				
				/*s*/
				stat_v.memory_v -= sizeof( obj_node_t);
				/*s*/

				obj_node_iter2 = tmp->next;
				continue;
			}

			tmp = obj_node_iter2;
			obj_node_iter2 = obj_node_iter2->next;
		}

		obj_node_iter1 = obj_node_iter1->next;
		if( obj_node_iter1 == NULL)
			break;
	}

	return;
}

/*
 *	Print an obj_set_t structure.
 */
void print_obj_set( obj_set_t* obj_set_v, FILE* o_fp)
{
	int i;
	obj_node_t* obj_node_iter;
	obj_t* obj_v;

	if( !obj_set_v)
		return;

	for( i=0; i<20; i++)
		fprintf( o_fp, "==");
	fprintf( o_fp, "\n");

	fprintf( o_fp, "%i\n\n", obj_set_v->obj_n);
	obj_node_iter = obj_set_v->head->next;
	while( obj_node_iter != NULL)
	{
		obj_v = obj_node_iter->obj_v;
		fprintf( o_fp, "%i:\t", obj_v->id);
        
//        print_k_list( obj_v->k_head, o_fp);
        fprintf( o_fp, "%d \n ", obj_v->fea);

		obj_node_iter = obj_node_iter->next;
	}
	fprintf( o_fp, "\n");
}

/*
 *	Release the memory of an obj_set_t structure.
 */
void release_obj_set( obj_set_t* obj_set_v)
{
	obj_node_t* obj_node_v1, *obj_node_v2;

	if( obj_set_v == NULL)
		return;

	/*s*/
	stat_v.memory_v -= sizeof( obj_set_t) + 
						( obj_set_v->obj_n + 1) * sizeof( obj_node_t);
	/*s*/

	obj_node_v1 = obj_set_v->head;
	while( obj_node_v1->next != NULL)
	{
		obj_node_v2 = obj_node_v1->next;
		free( obj_node_v1);

		obj_node_v1 = obj_node_v2;
	}
	free( obj_node_v1);

	free( obj_set_v);
}

/*
 *	Calculate the distance between two locations @loc_v1 and @loc_v2.
 */
B_KEY_TYPE calc_dist_loc( loc_t* loc_v1, loc_t* loc_v2)
{
	int i;
	B_KEY_TYPE dist;

	dist = 0;
	for( i=0; i<loc_v1->dim; i++)
		dist += pow( loc_v1->coord[ i] - loc_v2->coord[ i], 2);

	return sqrt( dist);
}

/*
 *	Calculate the distance between two objects @obj_v1 and @obj_v2.
 */
B_KEY_TYPE calc_dist_obj( obj_t* obj_v1, obj_t* obj_v2)
{
	B_KEY_TYPE dist;
	loc_t* loc_v1, *loc_v2;

	loc_v1 = get_obj_loc( obj_v1);
	loc_v2 = get_obj_loc( obj_v2);

	dist = calc_dist_loc( loc_v1, loc_v2);

	release_loc( loc_v1);
	release_loc( loc_v2);

	return dist;	
}

/*
 *	Check whether an object @obj_v contains the keyword @key.
 */
bool has_key_obj( obj_t* obj_v, KEY_TYPE key)
{
    if(obj_v->fea==key)return true;
    
	return false;
}

/*
 *	Check whether an object @obj_v is "relevant" to the query @q.
 *	That is, whether @obj_v contains a keyword in the query @q.
 */
bool is_relevant_obj( obj_t* obj_v, query_t* q)
{
	KEY_TYPE key;
	k_node_t* k_node_v;

	k_node_v = q->psi_v->k_head->next;
	while( k_node_v != NULL)
	{
		key = k_node_v->key;
		if( has_key_obj( obj_v, key))
			return true;

		k_node_v = k_node_v->next;
	}

	return false;		
}

/*
 *	Check whether the sub-tree rooted at a node @node_v
 *	contains a keyword @key.
 *
 */
BIT_TYPE has_key_node( node_t* node_v, KEY_TYPE key)
{
	bst_node_t* bst_node_v;

	if( ( bst_node_v = bst_search( node_v->bst_v, key)))
		return bst_node_v->p_list;
	
	return 0;
}

/*
 *	Check whether a node @node_v is "relevant" or not.
 */
BIT_TYPE is_relevant_node( node_t* node_v, query_t* q)
{
	BIT_TYPE res, res_t;

	k_node_t* k_node_v;

	res = 0;
	k_node_v = q->psi_v->k_head->next;
	while( k_node_v != NULL)
	{
		res_t = has_key_node( node_v, k_node_v->key);
		union_bit( res, res_t);

		if( res == UINT_MAX)
			return res;

		k_node_v = k_node_v->next;
	}

	return res;
}

/*
 *	Calculate the minimum distance between the MBR of a node @node_v 
 *	and a location @loc_v.
 */
B_KEY_TYPE calc_minDist( range* MBR, loc_t* loc_v)
{
	int i;
	B_KEY_TYPE m_dist;
	
	//Calculate the minimum distance between the MBR and the location.
	m_dist = 0;
	for( i=0; i<loc_v->dim; i++)
	{
		if( loc_v->coord[ i] < MBR[ i].min)
			m_dist += pow( MBR[ i].min - loc_v->coord[ i], 2);
		else if( loc_v->coord[ i] > MBR[ i].max)
			m_dist += pow( loc_v->coord[ i] - MBR[ i].max, 2);
	}

	return sqrt( m_dist);
}

/*
 *	Calculate the maxDist between a MBR @MBR and 
 */
B_KEY_TYPE calc_maxDist( range* MBR, loc_t* loc_v)
{
	int i;
	B_KEY_TYPE maxDist;

	maxDist = 0;
	for( i=0; i<loc_v->dim; i++)
	{
		if( loc_v->coord[ i] <= ( MBR[ i].min + MBR[ i].max) / 2)
			maxDist += pow( MBR[ i].max - loc_v->coord[ i], 2);
		else
			maxDist += pow( loc_v->coord[ i] - MBR[ i].min, 2);
	}

	return sqrt( maxDist);
}

/*
 *	Calculate the minimum distance between the MBR of a node @node_v and a location @loc_v.
 */
B_KEY_TYPE calc_minDist_node( node_t* node_v, loc_t* loc_v)
{
	B_KEY_TYPE dist;
	range* MBR;

	if( node_v->parent == NULL)
		MBR = get_MBR_node( node_v, RTree_v.dim);
	else
		MBR = node_v->parent->MBRs[ node_v->loc];

	dist = calc_minDist( MBR, loc_v);

	if( node_v->parent == NULL)
	{
		free( MBR);

		/*s*/
		stat_v.memory_v -= sizeof( RTree_v.dim * sizeof( range));
		/*s*/
	}

	return dist;
}

/*
 *	Check whether a MBR @MBR overlaps with a disk @disk_v.
 */
bool is_overlap( range* MBR, disk_t* disk_v)
{
	B_KEY_TYPE min_dist;

	min_dist = calc_minDist( MBR, disk_v->center);
	if(  min_dist <= disk_v->radius)
		return true;
	else
		return false;
}
/*
 *	Check whether a MBR @MBR is enclosed entirely by a disk @disk_v.
 */
bool is_enclosed( range* MBR, disk_t* disk_v)
{
   // printf("a.%f\tb.%f\n", calc_maxDist( MBR, disk_v->center), disk_v->radius);
	if( calc_maxDist( MBR, disk_v->center) <= disk_v->radius)
		return true;
	else
		return false;
}

/*
 *	Add an object entry @obj_v to @obj_set_v.
 */
void add_obj_set_entry( obj_t* obj_v, obj_set_t* obj_set_v)
{
	obj_node_t* obj_node_v;

	obj_node_v = ( obj_node_t*)malloc( sizeof( obj_node_t));
	memset( obj_node_v, 0, sizeof( obj_node_t));

	obj_node_v->obj_v = obj_v;
	obj_node_v->next =  obj_set_v->head->next;
	obj_set_v->head->next = obj_node_v;
	obj_set_v->obj_n ++;

	/*s*/
	stat_v.memory_v += sizeof( obj_node_t);
	if( stat_v.memory_v > stat_v.memory_max)
		stat_v.memory_max = stat_v.memory_v;
	/*s*/
}

/*
 *	Remove the first entry from the list of objects @obj_set_v.
 */	
void remove_obj_set_entry( obj_set_t* obj_set_v)
{
	obj_node_t* obj_node_v;

	obj_node_v = obj_set_v->head->next;
	obj_set_v->head->next = obj_node_v->next;
	obj_set_v->obj_n --;

	free( obj_node_v);

	/*s*/
	stat_v.memory_v -= sizeof( obj_node_t);
	/*s*/
}

/*
 *	Retrieve all the objects located at the sub-tree rooted at @node_v.
 *	The retrieved objects are stored in obj_set_v.
 */
void retrieve_sub_tree( node_t* node_v, obj_set_t* &obj_set_v, query_t* q)
{
	int i;
	BIT_TYPE p_list;

	if( node_v->level == 0)
	{
		for( i=0; i<node_v->num; i++)
		{
			if( is_relevant_obj( ( obj_t*)( node_v->child[ i]), q))
				add_obj_set_entry( ( obj_t*)( node_v->child[ i]), obj_set_v);			
		}
	}
	else
	{
		p_list = is_relevant_node( node_v, q);
		for( i=0; i<node_v->num; i++)
		{
			if( get_k_bit( p_list, i))
				retrieve_sub_tree( ( node_t*)( node_v->child[ i]), obj_set_v, q);
		}
	}
}

/*
 *	Range query on the sub-tree rooted at @node_v.
 *	@disk_v indicates the range which is a circle.
 *
 *	The results are stored in @obj_set_v.
 */
void range_query_sub( node_t* node_v, disk_t* disk_v, obj_set_t* &obj_set_v, query_t* q)
{
	int i;
	BIT_TYPE p_list;
	range* MBR;

	if( node_v->parent == NULL)
		MBR = get_MBR_node( node_v, RTree_v.dim);
	else
		MBR = node_v->parent->MBRs[ node_v->loc];

	//No overlapping.
	if( !is_overlap( MBR, disk_v))
		return;

	//Enclosed entrely.
	if( is_enclosed( MBR, disk_v))
	{
		retrieve_sub_tree( node_v, obj_set_v, q);
		if( node_v->parent == NULL)
		{
			free( MBR);

			/*s*/
			stat_v.memory_v -= RTree_v.dim * sizeof( range);
			/*s*/
		}

		return;
	}

	//The remaining cases.
	if( node_v->level == 0)
	{
		//node_v is a leaf-node.
		for( i=0; i<node_v->num; i++)
		{
			if( is_enclosed( ( ( obj_t*)( node_v->child[ i]))->MBR, disk_v) &&
				is_relevant_obj( ( obj_t*)( node_v->child[ i]), q))
				add_obj_set_entry( ( obj_t*)( node_v->child[ i]), obj_set_v);
		}
	}
	else
	{
		//node_v is an inner-node.
		p_list = is_relevant_node( node_v, q);
		
		for( i=0; i<node_v->num; i++)
		{
			if( get_k_bit( p_list, i))
				range_query_sub( ( node_t*)( node_v->child[ i]), disk_v, obj_set_v, q);
		}
	}

	if( node_v->parent == NULL)
	{
		free( MBR);
		/*s*/
		stat_v.memory_v -= RTree_v.dim * sizeof( range);
		/*s*/
	}
}

/*
 *	Circle range query.
 *
 *	DFS: recursive implementation.
 */
obj_set_t* range_query( disk_t* disk_v, query_t* q)
{
	obj_set_t* obj_set_v;

	obj_set_v = alloc_obj_set( );
	range_query_sub( RTree_v.root, disk_v, obj_set_v, q);

	return obj_set_v;
}

/*
 *	Construct the IF on a set of objects @obj_set_v for the keywords in @psi_v.
 *
 *	1. The IF structure is indexed by a binary search tree.
 *	2. No ordering is imposed in IF.
 */
bst_t* const_IF( obj_set_t* obj_set_v, psi_t* psi_v)
{
	int i;
	bst_t* IF_v;
	k_node_t* k_node_v;
	obj_node_t* obj_node_v;
	bst_node_t* bst_node_v;

	//IF_v = alloc_IF( psi_v->key_n);
	IF_v = bst_ini( );

	k_node_v = psi_v->k_head->next;
	//for( i=0; i<psi_v->key_n; i++)
    while(k_node_v !=NULL)
	{
		//IF_v->entry_v[ i].key = k_node_v->key;
		bst_node_v = ( bst_node_t*)malloc( sizeof( bst_node_t));
		memset( bst_node_v, 0, sizeof( bst_node_t));

		/*s*/
		stat_v.memory_v += sizeof( bst_node_t);
		/*s*/

		bst_node_v->key = k_node_v->key;
		bst_node_v->p_list_obj = alloc_obj_set( );
		
		obj_node_v = obj_set_v->head->next;
		while( obj_node_v != NULL)
		{
			if( has_key_obj( obj_node_v->obj_v, k_node_v->key))
				add_obj_set_entry( obj_node_v->obj_v, bst_node_v->p_list_obj);

			obj_node_v = obj_node_v->next;
		}

		bst_insert( IF_v, bst_node_v);

		k_node_v = k_node_v->next;
	}
	
	/*s*/
	if( stat_v.memory_v > stat_v.memory_max)
		stat_v.memory_max = stat_v.memory_v;
	/*s*/

	return IF_v;
}

/*
 *	Construct a obj_set_t structure containing 3 objects
 *	from a tri_t structure @triplet_v.
 */
// obj_set_t* const_obj_set( tri_t* triplet_v)
// {
// 	obj_set_t* obj_set_v;

// 	obj_set_v = alloc_obj_set( );
// 	obj_set_v->obj_n = 3;

// 	//Including the objects.
// 	add_obj_set_entry( triplet_v->o, obj_set_v);
// 	add_obj_set_entry( triplet_v->o_1, obj_set_v);
// 	add_obj_set_entry( triplet_v->o_2, obj_set_v);

// 	return obj_set_v;
// }

/*
 *	Check the distance constraint.
 */
bool check_dist_constraint( obj_set_t* obj_set_v, obj_t* obj_v, obj_t* o, B_KEY_TYPE d)
{
	obj_node_t* obj_node_iter;

	if( calc_dist_obj( o, obj_v) > d)
		return false;

	obj_node_iter = obj_set_v->head->next;
	while( obj_node_iter != NULL)
	{
		if( calc_dist_obj( obj_node_iter->obj_v, obj_v) > d)
			return false;

		obj_node_iter = obj_node_iter->next;
	}

	return true;
}

/*
 *	Update the IF structure @IF_v by removing the keywords 
 *	that have been covered by an object @obj_v.
 */
bst_node_list_t* update_IF_obj( bst_t* IF_v, obj_t* obj_v)
{
	k_node_t* k_node_v;
	bst_node_t* bst_node_v;
	bst_node_list_t* bst_node_list_v, *tmp;

	bst_node_list_v = ( bst_node_list_t*)malloc( sizeof( bst_node_list_t));
	memset( bst_node_list_v, 0, sizeof( bst_node_list_t));

	/*s*/
	stat_v.memory_v += sizeof( bst_node_list_t);
	/*s*/

		bst_node_v = bst_search( IF_v, obj_v->fea);

		if( bst_node_v != NULL)
		{
		

		bst_delete( IF_v, bst_node_v);
		//bug.
		bst_node_v->p = NULL;
		bst_node_v->left = NULL;
		bst_node_v->right = NULL;
		
		tmp = ( bst_node_list_t*)malloc( sizeof( bst_node_list_t));
		memset( tmp, 0, sizeof( bst_node_list_t));

		/*s*/
		stat_v.memory_v += sizeof( k_node_t);
		/*s*/

		tmp->bst_node_v = bst_node_v;
		tmp->next = bst_node_list_v->next;
		bst_node_list_v->next = tmp;
        }
//		k_node_v = k_node_v->next;
//	}

	/*s*/
	if( stat_v.memory_v > stat_v.memory_max)
		stat_v.memory_max = stat_v.memory_v;
	/*s*/

	return bst_node_list_v;
}

/*
 *	Release a bst_node_list_t structure.
 */
void release_bst_node_list( bst_node_list_t* bst_node_list_v)
{
	bst_node_list_t* tmp;
	
	while( bst_node_list_v != NULL)
	{
		tmp = bst_node_list_v->next;
		free( bst_node_list_v);
		bst_node_list_v = tmp;

		/*s*/
		stat_v.memory_v -= sizeof( bst_node_list_t);
		/*s*/
	}
}

/*
 *	Restore the IF_v structure @IF_v by re-including the bst_nodes of @bst_node_list_v.
 */
void restore_IF_bst_node_list( bst_t* IF_v, bst_node_list_t* bst_node_list_v)
{
	bst_node_list_t* bst_node_list_iter;

	bst_node_list_iter = bst_node_list_v->next;
	while( bst_node_list_iter != NULL)
	{
		bst_insert( IF_v, bst_node_list_iter->bst_node_v);

		bst_node_list_iter = bst_node_list_iter->next;
	}
}