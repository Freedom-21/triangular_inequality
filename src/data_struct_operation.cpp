
#include <stdio.h>
#include "data_struct_operation.h"

/*
 *	Allocate a fsi_t structure.
 */
fsi_t* alloc_fsi(int key_n)
{
    fsi_t* fsi_v;

    fsi_v = (fsi_t*)malloc(sizeof(fsi_t));
    memset(fsi_v, 0, sizeof(fsi_t));

    fsi_v->fea_n = key_n;

    fsi_v->feaset = (FEA_TYPE*)malloc(key_n * sizeof(FEA_TYPE));
    memset(fsi_v->feaset, 0, key_n * sizeof(FEA_TYPE));

    /*s*/
    stat_v.memory_v += sizeof(fsi_t) + key_n * sizeof(FEA_TYPE);
    if (stat_v.memory_v > stat_v.memory_max)
        stat_v.memory_max = stat_v.memory_v;
    /*s*/

    return fsi_v;
}

/*
 *	Allocate a fsi_set_t structure.
 */
fsi_set_t* alloc_fsi_set()
{
    fsi_set_t* fsi_set_v;

    fsi_set_v = (fsi_set_t*)malloc(sizeof(fsi_set_t));
    memset(fsi_set_v, 0, sizeof(fsi_set_t));

    fsi_set_v->head = (fsi_t*)malloc(sizeof(fsi_t));
    memset(fsi_set_v->head, 0, sizeof(fsi_t));

    /*s*/
    stat_v.memory_v += sizeof(fsi_set_t) + sizeof(fsi_t);
    if (stat_v.memory_v > stat_v.memory_max)
        stat_v.memory_max = stat_v.memory_v;
    /*s*/

    return fsi_set_v;
}

/*
 *	Release a fsi_t structure.
 */
void release_fsi(fsi_t* fsi_v)
{
    /*s*/
    stat_v.memory_v -= sizeof(fsi_t) + fsi_v->fea_n * sizeof(FEA_TYPE);
    /*s*/

    free(fsi_v->feaset);
    free(fsi_v);
}

/*
 *	Release the memory of an fsi_set_t structure.
 */
void release_fsi_set(fsi_set_t* fsi_set_v)
{
    fsi_t *fsi_v1, *fsi_v2;

    if (!fsi_set_v)
        return;

    /*s*/
    stat_v.memory_v -= sizeof(fsi_set_t) + (fsi_set_v->fsi_n + 1) * sizeof(fsi_t);
    /*s*/

    fsi_v1 = fsi_set_v->head;
    while (fsi_v1->next != NULL) {
        fsi_v2 = fsi_v1->next;
        release_fsi(fsi_v1);

        fsi_v1 = fsi_v2;
    }
    release_fsi(fsi_v1);

    free(fsi_set_v);
}

void add_fsi_set_entry(fsi_set_t* fsi_set_v, fsi_t* fsi_v)
{
    fsi_v->next = fsi_set_v->head->next;
    fsi_set_v->head->next = fsi_v;

    fsi_set_v->fsi_n++;
}

/*
 *  returning a new fsi = @fsi_v + fea
 */
fsi_t* add_fsi(fsi_t* fsi_v, FEA_TYPE fea)
{
    fsi_t* rtn;

    rtn = alloc_fsi(fsi_v->fea_n + 1);

    memcpy(rtn->feaset, fsi_v->feaset, fsi_v->fea_n * sizeof(FEA_TYPE));

    if (rtn->feaset[rtn->fea_n - 2] > fea) {
        rtn->feaset[rtn->fea_n - 1] = rtn->feaset[rtn->fea_n - 2];
        rtn->feaset[rtn->fea_n - 2] = fea;
    } else
        rtn->feaset[rtn->fea_n - 1] = fea;

    rtn->obj_set_list_v = new std::vector<obj_set_t*>();
    return rtn;
}

/*
 * constrcut an inverted file using obj_v->fea for the objects in @obj_set_v
 *
 */
bst_t* const_IF(obj_set_t* obj_set_v)
{
    bst_t* inverted_file;

    bst_node_t* bst_node_v;
    obj_node_t* obj_node_v;

    inverted_file = bst_ini();

    // Insert all the objects to construct the IF
    obj_node_v = obj_set_v->head->next;
    while (obj_node_v != NULL) {
        bst_node_v = bst_search(inverted_file, obj_node_v->obj_v->fea);

        if (bst_node_v != NULL) {
            // Update the posting list.
            add_obj_set_entry(obj_node_v->obj_v, bst_node_v->p_list_obj);
        } else // bst_node_v = NULL
        {
            // Insert a new keyword in the binary tree.
            bst_node_v = (bst_node_t*)malloc(sizeof(bst_node_t));
            memset(bst_node_v, 0, sizeof(bst_node_t));

            /*s*/
            stat_v.memory_v += sizeof(bst_node_t);
            if (stat_v.memory_v > stat_v.memory_max)
                stat_v.memory_max = stat_v.memory_v;
            /*s*/

            // Update the posting list.
            bst_node_v->key = obj_node_v->obj_v->fea;
            bst_node_v->p_list_obj = alloc_obj_set();

            add_obj_set_entry(obj_node_v->obj_v, bst_node_v->p_list_obj);
            bst_insert(inverted_file, bst_node_v);
        }
        obj_node_v = obj_node_v->next;
    }

    return inverted_file;
}

void release_IF_sub(bst_node_t* x)
{
    release_obj_set(x->p_list_obj);

    if (x->left != NULL)
        release_IF_sub(x->left);
    if (x->right != NULL)
        release_IF_sub(x->right);

    free(x);

    /*s*/
    stat_v.memory_v -= sizeof(bst_node_t);
    /*s*/
}

/*
 *	Release the IF @T.
 *  Different from bst_release, here we need to release p_list_obj
 */
void release_IF(bst_t* T)
{
    if (T != NULL) {
        if (T->root != NULL)
            release_IF_sub(T->root);
        free(T);

        /*s*/
        stat_v.memory_v -= sizeof(bst_t);
        /*s*/
    }
}
/*
 *	Print the @fsi in @o_fp.
 */
void print_fsi(fsi_t* fsi_v, FILE* o_fp)
{
    for (int i = 0; i < fsi_v->fea_n; i++) {
        fprintf(o_fp, "%d  ", fsi_v->feaset[i]);
    }

    fprintf(o_fp, "%lf  ", fsi_v->PI);

    fprintf(o_fp, "\n");
}

void print_fsi_set(fsi_set_t** result, int numOfFea, FILE* o_fp)
{
    for (int i = 0; i < numOfFea; i++) {
        if (result[i] != NULL) {
            fsi_t* fsi_v = result[i]->head->next;

            if (fsi_v != NULL)
                fprintf(o_fp, "Number of Colocation patterns on this size : %d\n", result[i]->fsi_n);

            while (fsi_v != NULL) {
                print_fsi(fsi_v, o_fp);

                fsi_v = fsi_v->next;
            }
            fprintf(o_fp, "\n");
        }
    }
}

/*
 *  convert fsi to psi
 *  @feaToExclude will not be included in the resulting psi
 *
 */
psi_t* fsi_to_psi(fsi_t* fsi_v, FEA_TYPE feaToExclude)
{
    psi_t* psi_v;
    psi_v = alloc_psi();

    for (int i = 0; i < fsi_v->fea_n; i++) {
        if (fsi_v->feaset[i] != feaToExclude)
            add_psi_entry(psi_v, fsi_v->feaset[i]);
    }

    return psi_v;
}

// Retrieve all the objects located at the sub-tree rooted at @node_v.
 
void retrieve_sub_tree(node_t* node_v, obj_set_t*& obj_set_v)
{
    int i;
    BIT_TYPE p_list;

    if (node_v->level == 0) {
        for (i = 0; i < node_v->num; i++) {
            add_obj_set_entry((obj_t*)(node_v->child[i]), obj_set_v);
        }
    } else {
        for (i = 0; i < node_v->num; i++) {
            retrieve_sub_tree((node_t*)(node_v->child[i]), obj_set_v);
        }
    }
}

void range_query_sub(node_t* node_v, disk_t* disk_v, obj_set_t*& obj_set_v)
{
    int i;
    BIT_TYPE p_list;
    range* MBR;

    if (node_v->parent == NULL)
        MBR = get_MBR_node(node_v, RTree_v.dim);
    else
        MBR = node_v->parent->MBRs[node_v->loc];

    // No overlapping.
    if (!is_overlap(MBR, disk_v))
        return;

    // Enclosed entrely.
    if (is_enclosed(MBR, disk_v)) {
        retrieve_sub_tree(node_v, obj_set_v);
        if (node_v->parent == NULL) {
            free(MBR);

            /*s*/
            stat_v.memory_v -= RTree_v.dim * sizeof(range);
            /*s*/
        }

        return;
    }

    // The remaining cases.
    if (node_v->level == 0) {
        // node_v is a leaf-node.
        for (i = 0; i < node_v->num; i++) {
            if (is_enclosed(((obj_t*)(node_v->child[i]))->MBR, disk_v))
                add_obj_set_entry((obj_t*)(node_v->child[i]), obj_set_v);
        }
    } else {
        // node_v is an inner-node.

        for (i = 0; i < node_v->num; i++) {
            range_query_sub((node_t*)(node_v->child[i]), disk_v, obj_set_v);
        }
    }

    if (node_v->parent == NULL) {
        free(MBR);
        /*s*/
        stat_v.memory_v -= RTree_v.dim * sizeof(range);
        /*s*/
    }
}

obj_set_t* range_query(disk_t* disk_v)
{
    obj_set_t* obj_set_v;

    obj_set_v = alloc_obj_set();
    range_query_sub(RTree_v.root, disk_v, obj_set_v);

    return obj_set_v;
}
