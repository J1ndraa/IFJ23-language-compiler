/* ********************************  symtable.h  ********************************** */
/*  Author: Vojtěch Hrabovský (xhrabo18)                                            */
/*  Subject: IFJ/IAL - Project                                                      */
/*  Date: 20. 10. 2023                                                              */
/*  Functionality: Header file for symtable.h                                       */
/* ******************************************************************************** */

#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <stdbool.h>

enum Var_type {
    NIL,
    INT,
    BOOL,
    DOUBLE,
    STRING,
    UNDEFINED_TYPE,
    INT_NIL,
    DOUBLE_NIL,
    STRING_NIL,
    VOID
};

enum Type {
    FUNCTION,
    VARIABLE,
    UNDEFINED
};

typedef struct Tdata_var {
    enum Var_type type;      //  Variable type - int, float, string, bool, nil
    bool init;          //  Is initialized?
    char *value;        //  Value 
    bool constant;      //  Is constant (defined by "let")?
    bool is_param;      //  Is constant (defined by "let")?
} TData_var;

typedef struct function_data {
    char *name;         //  Parameter name
    char *id;        //  Paremeter  id
    enum Var_type type;      //  Data type of parameter
} Function_param;

typedef struct Tdata_func {
    enum Var_type ret_type;          //  Return type
    Function_param *parameters; //  Parameter count
    bool declared;              //  Is declared?
    bool defined;               //  Is defined?
    struct TTree *local_table;         //  Local sym. table
    int parameter_count;
} TData_func;

typedef struct Tnode {
	char            *id;            //  Id
    enum Type            type;           //  Function / variable
    TData_var       variable_data;  //  Data of var.
    TData_func      function_data;  //  Function data
    struct Tnode           *left_node;     //  Left subtree
    struct Tnode           *right_node;    //  Right subtree
} TNode;                // Node of a tree

typedef struct Ttree {
    struct Tnode *root;
} TTree;

/**
 * / ***************** init_symtable() *********************** \
 *    @brief initializes sym. table
 */
void init_symtable(TTree *tree);

/**
 * / ***************** insert_symbol() *********************** \
 *    @brief recursivelly inserts new node or edits node with specified id
 *    @param root pointer to the tree root node
 *    @param id id that will be inserted or edited
 *    @param type type of the node (variable / function)
 *    @param var_data data of varible
 *    @param func_data data of function
 *    @param tree  pointer to the tree structure
 *    @return 1 on success, otherwise 0
 */
int insert_symbol(TNode **root, char *id, enum Type type, TData_var var_data, TData_func func_data, TTree *tree);


/**
 * / ***************** search_symbol() *********************** \
 *    @brief searches for node specified by id
 *    @param root pointer to the tree
 *    @param id   specifies which node to return
 *    @return pointer to node, NULL if id is not found
 */
TNode *search_symbol(TNode *root, char *id);

/**
 * / ***************** delete_symbol() *********************** \
 *    @brief deletes node specified by id
 *    @param root pointer to the tree
 *    @param id   specifies which node to delete
 *    @return 1 on success, otherwise 0
 */
int delete_symbol(TTree *root, char *id);

/** 
 * / ***************** dispose_sym_table() ********************* \
 * @brief disposes the tree
 * @param tree pointer to the tree that will be disposed
 */
void dispose_symtable(TTree *tree);

/**
 * / ***************** my_strdup() ********************* \
 * @brief helper function that coppies string
 * @param input_string will be coppied
 * @return coppied string
 */
char *my_strdup(const char *input_string);


#endif
