#include "bst.h"

extern colocation_stat_t stat_v;


bst_t* bst_ini( )
{
	bst_t* T;

	T = ( bst_t*)malloc( sizeof( bst_t));
	memset( T, 0, sizeof( bst_t));

	//Problem specific.
	T->max = - INT_MAX;
	T->min = INT_MAX;

	/*s*/
	stat_v.memory_v += sizeof( bst_t);
	if( stat_v.memory_v > stat_v.memory_max)
		stat_v.memory_max = stat_v.memory_v;
	/*s*/

	return T;
}

void bst_release_sub( bst_node_t* x)
{
	if( x->left != NULL)
		bst_release_sub( x->left);
	if( x->right != NULL)
		bst_release_sub( x->right);
    
	free( x);

	/*s*/
	stat_v.memory_v -= sizeof( bst_node_t);
	/*s*/
}

/*
 *	Release the binary search tree T.
 */
void bst_release( bst_t* T)
{
	if( T != NULL)
	{
		if( T->root != NULL)
			bst_release_sub( T->root);
		free( T);

		/*s*/
		stat_v.memory_v -= sizeof( bst_t);
		/*s*/
	}
}

void bst_insert( bst_t* T, bst_node_t* z)
{
	bst_node_t* y, *x;

	y = NULL;
	x = T->root;

	while( x != NULL)
	{
		y = x;
		if( z->key < x->key)
			x = x->left;
		else
			x = x->right;
	}

	z->p = y;
	if( y == NULL)
		T->root = z;	//T was empty.
	else if( z->key < y->key)
		y->left = z;	
	else
		y->right = z;

	//
	T->node_n ++;
	if( T->max < z->key)
		T->max = z->key;
	if( T->min > z->key)
		T->min = z->key;
}

void bst_transplant( bst_t* T, bst_node_t* u, bst_node_t* v)
{
	if( u->p == NULL)
		T->root = v;
	else if( u == u->p->left)
		u->p->left = v;
	else
		u->p->right = v;
	if( v != NULL)
		v->p = u->p;
}


bst_node_t* bst_get_min( bst_node_t* x)
{
	if( x == NULL)
		return NULL;

	while( x->left != NULL)
		x = x->left;
	return x;
}

bst_node_t* bst_get_max( bst_node_t* x)
{
	if( x == NULL)
		return NULL;

	while( x->right != NULL)
		x = x->right;
	return x;
}

void bst_delete( bst_t* T, bst_node_t* x)
{
	bst_node_t* y;

	if( x->left == NULL)
		bst_transplant( T, x, x->right);
	else if( x->right == NULL)
		bst_transplant( T, x, x->left);
	else
	{
		y = bst_get_min( x->right);
		if( y->p != x)
		{
			bst_transplant( T, y, y->right);
			y->right = x->right;
			y->right->p = y;
		}

		bst_transplant( T, x, y);
		y->left = x->left;
		y->left->p = y;
	}

	//Problem specific.
	T->node_n --;

	if( T->node_n == 0)
	{
		T->max = -DBL_MAX;
		T->min =  DBL_MAX;

		return;
	}

	if( x->key == T->max)
		T->max = bst_get_max( T->root)->key;
	if( x->key == T->min)
		T->min = bst_get_min( T->root)->key;
}

void bst_update( bst_t* T, bst_node_t* x)
{
	bst_delete( T, x);
	x->left = NULL;
	x->right = NULL;		//bug.
	bst_insert( T, x);

}

bst_node_t*  bst_successor( bst_node_t* x)
{
	bst_node_t* y;

	if( x->right != NULL)
		return bst_get_min( x->right);

	y = x->p;
	while( y != NULL && x == y->right)
	{
		x = y;
		y = y->p;
	}

	return y;
}

bool in_order_sub( bst_node_t* &x, int &tag)
{
	if( x->right != NULL)
	{
		x = x->right;
		tag = 0;
	}
	else
	{
		if( x->p != NULL)
		{
			if( x == x->p->left)
				tag = -1;
			else
				tag = 1;

			x = x->p;
		}
		else
			return false;
	}

	return true;
}

//  Locate the "next" bst_node wrt the "in-order" from the current one @x.
bool get_next_in_order( bst_node_t* &x, int &tag)
{
	//tag should be set to 0 when x is the root.
	while( true)
	{
		if( tag == 0)
		{
			while( x->left != NULL)
				x = x->left;
			
			//printf( "%i  ", x->key);
			return true;
		}
		else if( tag == -1)
		{

			return true;
		}
		else	//tag ==1
		{
			if( x->p != NULL)
			{
				if( x == x->p->left)
					tag = -1;
				else
					tag = 1;
				x = x->p;
			}
			else 
				return false;
		}
	}
}

/*
 *	Search the bst_node that contans a specific key @key.
 */
bst_node_t* bst_search( bst_t* bst_v, KEY_TYPE key)
{
	bst_node_t* x;

	x = bst_v->root;
	while( true)
	{
		if( x == NULL)
			return x;
		
		if( x->key == key)
			return x;
		else if( key < x->key)
			x = x->left;
		else //key > x->key
			x = x->right;
	}

	return NULL;
}