/* ***************************** symtable_stack.h **************************** */
/*  Author: Marek Čupr (xcuprm01)                                              */
/*  Subject: IFJ/IAL - Project                                                 */
/*  Date: 17. 11. 2023                                                         */
/*  Functionality: Header file for symtable_stack.c                            */
/* *************************************************************************** */

#ifndef ST_STACK_H
#define ST_STACK_H

#include "symtable.h"

/*
 * / ************** Symtable_Stack_Item_T *************** \  
 * / Struct that holds the information about a stack item \
*/
typedef struct Stack_Item {
    struct Stack_Item *next_item;  // Item in the stack
    TTree *symtable_item;          // Pointer to a symtable
} Symtable_Stack_Item_T;

/*
 * / ************************ Symtable_Stack_T ************************ \  
 * / Struct that holds the pointer to the top of the stack of symtables \
*/
typedef struct Symtable_Stack {
    Symtable_Stack_Item_T *stack_head;  // Top of the stack
} Symtable_Stack_T;

/*
 * / ************** st_stack_init *************** \
 * / Function that initializes the symtable stack \
*/
void st_stack_init(Symtable_Stack_T *st_stack);

/*
 * / ******************* st_stack_push ********************* \
 * / Function that pushes a symtable to the top of the stack \
*/
int st_stack_push(Symtable_Stack_T *st_stack, TTree *symtable_item);

/*
 * / ************************ search_st_stack *************************** \
 * / Function that searches through all the symtables for a specific item \
*/
TNode *search_st_stack(Symtable_Stack_T *st_stack, char *id);

/*
 * / ******************** st_stack_pop ********************** \
 * / Function that deletes the item from the top of the stack \
*/
void st_stack_pop(Symtable_Stack_T *st_stack);

/*
 * / *************** st_stack_pop **************** \
 * / Function that cleans the whole symtable stack \
*/
void st_stack_clean(Symtable_Stack_T *st_stack);

#endif
/* End of symtable_stack.c */
