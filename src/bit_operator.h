#ifndef BIT_OPERATOR_H
#define BIT_OPERATOR_H

#include <math.h>

#define BIT_TYPE	unsigned int


int get_k_bit( BIT_TYPE v, int k);

void insert_k_bit( BIT_TYPE &v, int k);

void delete_k_bit( BIT_TYPE &v, int k);

void union_bit( BIT_TYPE &v1, BIT_TYPE v2);

#endif
