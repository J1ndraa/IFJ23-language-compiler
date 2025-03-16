/* ****************************** exp_stack.c ******************************** */
/*  Author: Jindřich Halva (xhalva05)                                          */
/*  Subject: IFJ/IAL - Project                                                 */
/*  Date: 12. 11. 2023                                                          */
/*  Functionality: Stack for exp_parser.c                                      */
/* *************************************************************************** */

#include "exp_stack.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

Stack_T* stack_init() {
    Stack_T *stack = malloc(sizeof (Stack_T));
    if(stack == NULL){
        return NULL;
    }
    stack->stack_head = NULL;
    return stack;
}

bool stack_push_item(Stack_T *stack, Prec_Table_Symbol_T pt_symbol, enum Var_type data_type, Token_T *token){
    Stack_Item_T *new_item = malloc(sizeof(Stack_Item_T));
    if(new_item == NULL){
        return false;
    }
    //insert data about new_item push
    new_item->next_item = stack->stack_head;
    new_item->data_type = data_type;
    new_item->pt_symbol = pt_symbol;
    new_item->token = token;
    stack->stack_head = new_item;
    return true;
}

void stack_pop_item(Stack_T *stack){
    //if stack is not empty...
    if(stack->stack_head != NULL){
        Stack_Item_T *tmp_del_item = stack->stack_head;
        stack->stack_head = stack->stack_head->next_item;
        free(tmp_del_item);
    }
}

void stack_clean(Stack_T *stack) {
    //deleting items until stack_head is NULL
    while(true) {
        if (stack->stack_head != NULL) {
            stack_pop_item(stack);
        }
        else{
            break;
        }
    }
    free(stack);
}

void stack_pop_item_multi(Stack_T *stack, int how_many){
    while(how_many != 0) {
        if (stack->stack_head != NULL) {
            Stack_Item_T *tmp_del_item = stack->stack_head;
            stack->stack_head = stack->stack_head->next_item;
            free(tmp_del_item);
        }
        how_many--;
    }
}

Stack_Item_T* stack_first_terminal(Stack_T *stack){
    Stack_Item_T *current_item = stack->stack_head;
    while(current_item != NULL) {
        if ((current_item->pt_symbol != P_TABLE_NON_TERMINAL)&&(current_item->pt_symbol != P_TABLE_STOP_REDUCING)){
            return current_item;
        }
        else{
            current_item = current_item->next_item;
        }
    }
    return NULL;
}

bool stack_push_item_after_top_terminal(Stack_T *stack, Prec_Table_Symbol_T pt_symbol, enum Var_type data_type, Token_T *token){
    Stack_Item_T *prev_item = NULL;
    Stack_Item_T *current_item = stack->stack_head;
    while(current_item != NULL){
        if ((current_item->pt_symbol != P_TABLE_NON_TERMINAL)&&(current_item->pt_symbol != P_TABLE_STOP_REDUCING)){
            //NON_terminal found
            //no upper items in stack
            if(prev_item == NULL){
                stack_push_item(stack, pt_symbol, data_type, token);
                return true;
            }
            //not top_stack item
            else{
                Stack_Item_T *new_item = malloc(sizeof(Stack_Item_T));
                new_item->pt_symbol = pt_symbol;
                new_item->data_type = data_type;
                new_item->token = token;
                //insert new item between Terminal and another item
                prev_item->next_item = new_item;
                new_item->next_item = current_item;
                return true;
            }
        }
        prev_item = current_item;
        current_item = current_item->next_item;
    }
    return false;
}

int stack_items_after_stop_reducing(Stack_T *stack){
    Stack_Item_T *current_item = stack->stack_head;
    int count = 0;
    while(current_item != NULL){
        if(current_item->pt_symbol == P_TABLE_STOP_REDUCING){
            return count;
        }
        count++;
        current_item = current_item->next_item;
    }
    return count;
}

void print_stack(Stack_T *stack){
    Stack_Item_T *item = stack->stack_head;
    int i=0;
    printf("stacktop->");
    do {
        printf("%d:%d ", i, item->pt_symbol);
        i++;
        item = item->next_item;
    }while(item != NULL);
}

