/* ****************************** exp_stack.h ******************************** */
/*  Author: Jindřich Halva (xhalva05)                                          */
/*  Subject: IFJ/IAL - Project                                                 */
/*  Date: 8. 11. 2023                                                          */
/*  Functionality: Header file for exp_stack.c                                 */
/* *************************************************************************** */


#ifndef EXP_RSTACK
#define EXP_RSTACK

#include "exp_parser.h"
#include "scanner.h"
#include "symtable.h"


typedef struct StackItem {
    Prec_Table_Symbol_T pt_symbol;
    enum Var_type data_type;
    struct StackItem *next_item;
    Token_T *token;
} Stack_Item_T;

typedef struct Stack {
    Stack_Item_T *stack_head;
} Stack_T;

/*
 * / ****************** stack_init() ******************* \
 * /Initialization of stack, returns pointer on new stack\
*/
Stack_T* stack_init();

/*
 * / *************** stack_push_item() **************** \
 * /        Insert one item on top of stack             \
 * /Returns true if allocating a new item was successful\
*/
bool stack_push_item(Stack_T *stack, Prec_Table_Symbol_T pt_symbol, enum Var_type data_type, Token_T *token);

/*
 * / *************** stack_pop_item() **************** \
 * /        Deletes the top item in the stack          \
*/
void stack_pop_item(Stack_T *stack);

/*
 * / ************ stack_clean() ************** \
 * /         Deletes the whole stack           \
*/
void stack_clean(Stack_T *stack);

/*
 * / *************** stack_pop_item_multi() **************** \
 * /       Can delete more than one item from stack          \
*/
void stack_pop_item_multi(Stack_T *stack, int how_many);

/*
 * / **************** stack_first_terminal() ***************** \
 * /Returns pointer on first terminal item in the stack or NULL\
 */
Stack_Item_T* stack_first_terminal(Stack_T *stack);

/*                                                                                    |STACK_TOP|
 * / **************** stack_push_item_after_top_terminal() ***************** \        |    E    | ___ Insert here
 * / Inserts an item after a Terminal, located closest to the Top of the stack\       |    $    |
 */
bool stack_push_item_after_top_terminal(Stack_T *stack, Prec_Table_Symbol_T pt_symbol, enum Var_type data_type, Token_T *token);

/*
 * / **************** stack_items_after_stop_reducing() ****************** \
 * / Returns count of items located in stack higher than stop_reduce symbol\
 */
int stack_items_after_stop_reducing(Stack_T *stack);

/*
 * / ******************** shift_to_stack() *********************** \
 * / Function that converts tokens to stack items, and shifts them \
*/
int shift_to_stack(Stack_T *stack, Token_T *token, Parser_T *struct_parser);

void print_stack(Stack_T *stack);

#endif
/* End of exp_stack.h */