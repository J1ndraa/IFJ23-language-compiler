/* ******************************* symtable_stack.c ********************************* */
/*  Author: Marek Čupr (xcuprm01)                                                     */
/*  Subject: IFJ/IAL - Project                                                        */
/*  Date: 17. 11. 2023                                                                */
/*  Functionality: COntains different functions for the work with the symtable stack  */
/* ********************************************************************************** */

#include "symtable_stack.h"
#include "error.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * Initializes a new symtable stack.
 *
 * @param st_stack Symtable stack to be initialized
 */
void st_stack_init(Symtable_Stack_T *st_stack){
    st_stack->stack_head = NULL;
}

/**
 * Pushes an item to the top of the symtable stack.
 *
 * @param st_stack Symtable stack to push the item to
 * @param symtable_item Symtable item to be pushed on top of the stack
 * @returns The correct error return code (0 if success)
 */
int st_stack_push(Symtable_Stack_T *st_stack, TTree *symtable_item){
    Symtable_Stack_Item_T *tmp_item = st_stack->stack_head;
    // Allocate memory for the new item
    Symtable_Stack_Item_T *new_item = (Symtable_Stack_Item_T *) malloc(sizeof(Symtable_Stack_Item_T));
    if (new_item == NULL) // Malloc failed
        return COMPILER_ERR_INTER;
        
    // Put the new item to the top of the stack and connect it to the other items
    new_item->symtable_item = symtable_item;
    st_stack->stack_head = new_item;
    st_stack->stack_head->next_item = tmp_item;
    return NO_ERR;
}

/**
 * Searches for a symbol through all the symtables that are on the symtable stack.
 *
 * @param st_stack Symtable stack to be searched in
 * @param id Id of the searched item
 * @returns A pointer to the found item or NULL if the item does not exist
 */
TNode *search_st_stack(Symtable_Stack_T *st_stack, char *id){
    if (st_stack->stack_head == NULL) // The symtable stack is empty
        return NULL;

    Symtable_Stack_Item_T *tmp_item = st_stack->stack_head;
    TNode *result;
    while (tmp_item != NULL){
        result = search_symbol(tmp_item->symtable_item->root, id); // Search for the symbol in the current symtable
        if (result != NULL){ // We've found a matching symbol
            return result;
        }
        tmp_item = tmp_item->next_item;
    }
    return NULL; // We've NOT found a matching symbol
}

/**
 * Deletes the top item from the top of the stack.
 *
 * @param st_stack Symtable stack to be deleted in.
 */
void st_stack_pop(Symtable_Stack_T *st_stack){
    if (st_stack->stack_head == NULL)
        return;
    Symtable_Stack_Item_T *tmp_item = st_stack->stack_head;
    st_stack->stack_head = st_stack->stack_head->next_item; // Move to the next item in the symtable stack
    // Free the correct item in the symtable stack
    free(tmp_item);
    tmp_item = NULL;
}

/**
 * Clears the whole symtable stack.
 *
 * @param st_stack Symtable stack to be cleaned
 */
void st_stack_clean(Symtable_Stack_T *st_stack){
    if (st_stack->stack_head == NULL) // The symtable stack is empty
        return;

    Symtable_Stack_Item_T *tmp_item = st_stack->stack_head->next_item;
    while (tmp_item != NULL){
        // Free the correct item
        free(st_stack->stack_head);
        st_stack->stack_head = tmp_item;
        tmp_item = tmp_item->next_item;
    }
    // Free the last item as well
    free(st_stack->stack_head);
    st_stack->stack_head = NULL;
}
/* End of symtable_stack.c */
