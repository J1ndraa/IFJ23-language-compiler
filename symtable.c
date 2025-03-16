/* ********************************  symtable.c  ********************************** */
/*  Author: Vojtěch Hrabovský (xhrabo18)                                            */
/*  Subject: IFJ/IAL - Project                                                      */
/*  Date: 20. 10. 2023                                                              */
/*  Functionality: Symbol table implementation using height balanced BST            */
/* ******************************************************************************** */

#include "symtable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/**
 * Helper function for printig tree structure, DELETE BEFORE SUBMITTING
 * 
 */

#define COUNT 6

/**
 * @brief helper function to calculate heights of subtrees, this function will be called automatically
 * @param root 
 */
int get_height(TNode *root){
    if (root == NULL) {
        return 0;
    }

    // Get heights of left and right subtree
    int left_height = get_height(root->left_node);
    int right_height = get_height(root->right_node);

    // Return the bigger height + 1 (for this node)
    if (left_height > right_height) {
        return 1 + left_height;
    } else {
        return 1 + right_height;
    }
}

/**
 * @brief helper function for balancing tree, this function will be called automatically
 * @param critical node that has to be balanced 
 */
int balance_tree_helper(TNode *critical) {
    // check if the node is null
    if (critical == NULL) {
        return 0;
    }

    // Get height of critical node's subtrees
    int left = get_height(critical->left_node);
    int right = get_height(critical->right_node);

    // If critical node is imbalanced on the left
    if (left > right) {
        // check for null pointers
        if (critical->left_node == NULL) {
            // error
            return 0;  
        }

        left = get_height(critical->left_node->left_node);
        right = get_height(critical->left_node->right_node);

        // If critical's left node is imbalanced on the left
        // LL
        if (left > right) {
            return 1;
        }
        // If critical's left node is imbalanced on the right
        // LR
        else if (left < right) {
            return 2;
        }
    }
    // If critical node is imbalanced on the right
    else if (left < right) {
        // check for null pointers
        if (critical->right_node == NULL) {
            // error
            return 0;  
        }

        left = get_height(critical->right_node->left_node);
        right = get_height(critical->right_node->right_node);

        // If critical's right node is imbalanced on the left
        // RL
        if (left > right) {
            return 3;
        }
        // If critical's right node is imbalanced on the right
        // RR
        else if (left < right) {
            return 4;
        }
    }

    return 0;
}
/**
 * @brief helper function that finds the parent of a node
 * @param root root node of a tree
 * @param child node whose parent will be searched for
 * @return parent node if found, null if parent and child are root nodes
 */
TNode *find_parent(TNode *root, TNode *child){

    TNode *parent = root;	

    if(parent == NULL){return NULL;}

    if(parent == child){return NULL;}

    while(parent->left_node != child && parent->right_node != child){
        if(strcmp(parent->id, child->id) > 0){
            parent = parent->left_node;
        }
        else if(strcmp(child->id, parent->id) > 0){
            parent = parent->right_node;
        }
    }

    return parent;
}

/**
 * @brief balances critical nodes, this function will be called automatically
 * @param critical node that will be balanaced
 */
int balance_tree(TTree *tree, TNode *critical) {
    int type = balance_tree_helper(critical);

    TNode *parent;
    TNode *ll;
    TNode *left;
    TNode *left_right;
    TNode *rr;
    TNode *right;
    TNode *right_left;

    switch (type) {
        case 1: ;
            if (critical != NULL && critical->left_node != NULL) {
                ll = critical->left_node;
                critical->left_node = ll->right_node;
                ll->right_node = critical;

            // update parent
                parent = find_parent(tree->root, critical);  
                if (parent != NULL) {
                    if (parent->left_node == critical) {
                        parent->left_node = ll;
                    } else if (parent->right_node == critical) {
                        parent->right_node = ll;
                    }
                } else {
                    tree->root = ll;  // Update the root if it critical was the root
                }
            }
            break;
        case 2: ;
            left = critical->left_node;
            left_right = left->right_node;

            left->right_node = left_right->left_node;
            left_right->left_node = left;
            critical->left_node = left_right->right_node;
            left_right->right_node = critical;

            // update parent
            parent = find_parent(tree->root, critical);  
            if (parent != NULL) {
                if (parent->left_node == critical) {
                    parent->left_node = left_right;
                } else if (parent->right_node == critical) {
                    parent->right_node = left_right;
                }
            } else {
                tree->root = left_right;   // Update the root if it critical was the root
            }
            break;
        case 3: ;
            // Right-Left (RL) imbalance
            
            right = critical->right_node;
            right_left = right->left_node;

            right->left_node = right_left->right_node;
            right_left->right_node = right;
            critical->right_node = right_left->left_node;
            right_left->left_node = critical;

            // update parent
            parent = find_parent(tree->root, critical);  
            if (parent != NULL) {
                if (parent->left_node == critical) {
                    parent->left_node = right_left;
                } else if (parent->right_node == critical) {
                    parent->right_node = right_left;
                }
            } else {
                tree->root = right_left;  // Update the root if it critical was the root
            }
            
            break;
        case 4: ;
            // Right-Right (RR) imbalance
            
            rr = critical->right_node;
            critical->right_node = rr->left_node;
            rr->left_node = critical;
            
            // update parent
            parent = find_parent(tree->root, critical);
            if (parent != NULL) {
                if (parent->left_node == critical) {
                    parent->left_node = rr;
                } else if (parent->right_node == critical) {
                    parent->right_node = rr;
                }
            } else {
                tree->root = rr;    // Update the root if it critical was the root
            }

            break;
        case 0: ;
            return 0;
            break;
    }
    return 1;
}

/**
 * @brief check whether nodes are height balanced, this function will be called automatically
 * @param root specifies which subtree to check
 * @param tree will be passed to balance_tree function
 * @return 1 if subtree is balanced, 0 if balancing failed
 */
int check_balance(TNode *root, TTree *tree){
    if(root == NULL){return 1;}

    // check subtrees
    check_balance(root->left_node, tree);
    check_balance(root->right_node, tree);
    
    // get heights
    int l = get_height(root->left_node)+1;
    int r = get_height(root->right_node)+1;
    
    // is balancing needed?
    if(get_height(root->left_node) > get_height(root->right_node) + 1){
        return balance_tree(tree, root);
    }else
    if(get_height(root->left_node) < get_height(root->right_node) - 1){
        return balance_tree(tree, root);
    }else{
        return 1;
    }
}


/** 
 * / ***************** free_node() ********************* \
 * @brief Frees allocated memmory
 * @param root pointer to the node that will be freed
 */
void free_node(TNode *to_delete) {
    if (to_delete == NULL) {
        return;
    }

    if(to_delete->id != NULL)
        free(to_delete->id);

    free(to_delete);
}




/**
 * @brief delete_symbol helper function that finds suitable node
 * @param node specifies in which subtree to look
 * @return suitable node
 */
TNode *minValueNode(TNode *node)
{
    TNode *current = node;
 
    // Find leftmost node in subtree
    while (current->left_node != NULL){
        current = current->left_node;
    }

    return current;
}




/**
 * @brief helper function
 */
char *my_strdup(const char *input_string) {
    size_t len = strlen(input_string) + 1;
    char *output_string = malloc(len);
    if (output_string != NULL) {
        memcpy(output_string, input_string, len);
    }
    return output_string;
}

int delete_symbol(TTree *tree, char *id){
    // get node to delete
    TNode *to_delete = search_symbol(tree->root, id);
    if(to_delete == NULL){return 0;}

    // get its parent 
    TNode *parent = tree->root;
    int which_child = 0;

    if(parent == to_delete){goto skip;}
    while(parent->left_node != to_delete && parent->right_node != to_delete){
        if(strcmp(parent->id, to_delete->id) > 0){
            parent = parent->left_node;
        }
        else if(strcmp(to_delete->id, parent->id) > 0){
            parent = parent->right_node;
        }
    }
    
    // which child is to_delete
    if(parent->left_node == to_delete){which_child = -1;}else if(parent->right_node == to_delete){which_child = 1;}

    skip:
        /*
            which_child = -1 -> to_delete je levy uzel 
            which_child = 0 -> to_delete = parent (pouze v pripade, ze je to koren stromu)
            which_child = 1 -> to_delete je pravy uzel
        */
        // no subtrees
        if(to_delete->left_node == NULL & to_delete->right_node == NULL){
            //printf("simple delete, *to_delete = null \n");
            if(which_child == -1){free_node(to_delete);parent->left_node = NULL;return 1;}
            if(which_child == 0){free_node(to_delete);init_symtable(tree);return 1;}
            if(which_child == 1){free_node(to_delete);parent->right_node = NULL; return 1;}
            return check_balance(tree->root, tree);
        }else // only left subtree
        if(to_delete->left_node != NULL & to_delete->right_node == NULL){
            //printf("only left subtree -> parent gets it \n");
            TNode *tmp = to_delete->left_node;
            if(which_child == -1){free_node(to_delete);parent->left_node = tmp;return 1;}
            if(which_child == 0){free_node(to_delete);tree->root = tmp;return 1;}
            if(which_child == 1){free_node(to_delete);parent->right_node = tmp; return 1;}
            return check_balance(tree->root, tree);
        }else // only right subtree
        if(to_delete->left_node == NULL & to_delete->right_node != NULL){
            //printf("only right subtree -> parent gets it \n");
            TNode *tmp = to_delete->right_node;
            if(which_child == -1){free_node(to_delete);parent->left_node = tmp;return 1;}
            if(which_child == 0){free_node(to_delete);tree->root = tmp;return 1;}
            if(which_child == 1){free_node(to_delete);parent->right_node = tmp; return 1;}
            return check_balance(tree->root, tree);
        }else // both subtrees
        if(to_delete->left_node != NULL & to_delete->right_node != NULL){
            TNode *to_replace_with = minValueNode(to_delete->right_node);
            //printf("both subtrees, replace node with node: %s \n", to_replace_with->id);
            
            // replace values
            free(to_delete->id); // Free memory associated with the old id
            to_delete->id = malloc(strlen(to_replace_with->id) + 1); // Allocate memory for the new id
            strcpy(to_delete->id, to_replace_with->id);

            to_delete->variable_data = to_replace_with->variable_data;
            to_delete->function_data = to_replace_with->function_data;
            to_delete->type = to_replace_with->type;
            
            // right subtree has only replacement node, delete it
            if(to_delete->right_node == to_replace_with){
                free_node(to_replace_with);
                to_delete->right_node = NULL;
                return check_balance(tree->root, tree);
            }else // if there is more than replacement node in right subtree
            if(to_delete->right_node != to_replace_with){

                // find parent
                TNode *replace_parent = to_delete->right_node;
                while(replace_parent->left_node != to_replace_with)
                {
                    replace_parent = replace_parent->left_node;
                }

                // if replacement node has right child, parent gets is
                if(to_replace_with->right_node != NULL){replace_parent->left_node = to_replace_with->right_node; return 1;}
                
                // parents left child (replacement node) will be deleted
                replace_parent->left_node = NULL;
                
                free_node(to_replace_with);
                return check_balance(tree->root, tree);
            }
        }

    return 0;
}


/**
 * / ***************** init_symtable() *********************** \
 *    @brief initializes sym. table
 *    @param pointer to tree structure
 */
void init_symtable(TTree *tree){
    tree->root = NULL;
}

/**
 * / ***************** insert_symbol() *********************** \
 *    @brief inserts node   
 *    @param root pointer to pointer to root node
 *    @param id name of the variable / function
 *    @param type type of the node (variable / function)
 *    @param var_data data of varible
 *    @param func_data data of function
 *    @return 1 on success, otherwise 0
 */
int insert_symbol(TNode **root, char *id, enum Type type, TData_var var_data, TData_func func_data, TTree *tree) {
    // If tree is empty, allocate memmory for node and insert it
    if (*root == NULL) {
        *root = (TNode *)malloc(sizeof(TNode));
        if (*root == NULL) {
            return 0; // Handle memory allocation error
        }
        // Assign data
        (*root)->id = my_strdup(id);
        (*root)->type = type;
        (*root)->variable_data = var_data; 
        (*root)->function_data = func_data; 
        (*root)->left_node = NULL;
        (*root)->right_node = NULL;
        return check_balance(tree->root, tree);
    }

    // If tree is not empty, insert node at correct position
    int cmp_result = strcmp(id, (*root)->id);

    if (cmp_result < 0) {
        return insert_symbol(&((*root)->left_node), id, type, var_data, func_data, tree);
    } else if (cmp_result > 0) {
        return insert_symbol(&((*root)->right_node), id, type, var_data, func_data, tree);
    } else {

        // Duplicate identifiers -> overwrite
        (*root)->type = type;
        (*root)->variable_data = var_data; 
        (*root)->function_data = func_data; 
        return 1;
    }

    return 0;
}

/**
 * / ***************** search_symbol() *********************** \
 *    @brief searches for node specified by id
 *    @param root pointer to the root node
 *    @param id   specifies which node to return
 *    @return pointer to node, NULL if id is not found
 */
TNode *search_symbol(TNode *root, char *id) {
    if (id == NULL || id[0] == '\0') {
        // Invalid id, return NULL to indicate failure.
        
        return NULL;
    }

    // If tree is empty, return NULL
    if (root == NULL) {
        
        return NULL;
    }
    

    // Compare ids
    int cmp_result = strcmp(id, root->id);

    // If root has the id that is being searched, return the root node
    if (cmp_result == 0) {
        return root;
    }

    // If root id is bigger, search left subtree
    if (cmp_result < 0) {
        return search_symbol(root->left_node, id);
    }

    // If root id is smaller, search right subtree
    else if (cmp_result > 0) {
        return search_symbol(root->right_node, id);
    }

    return NULL;
}

/**
 * @brief dispose helper function
 */
void dispose_helper(TNode *root){
    if (root != NULL) {
        // Recursively dispose of the left and right subtrees
        dispose_helper(root->left_node);
        dispose_helper(root->right_node);

        // Deallocate the node
        free(root->id);
        free(root);
    }
}

/** 
 * / ***************** dispose_sym_table() ********************* \
 * @brief disposes the tree
 * @param tree pointer to the tree that will be disposed
 */
void dispose_symtable(TTree *tree) {
    dispose_helper(tree->root);
    init_symtable(tree);
}
