/* ***************************** exp_parser.c ******************************** */
/*  Authors: Jindřich Halva (xhalva05)                                         */
/*           Vojtěch Hrabovský (xhrabo18)                                      */
/*  Subject: IFJ/IAL - Project                                                 */
/*  Date: 8. 11. 2023                                                          */
/*  Functionality: Handling expression                                         */
/* *************************************************************************** */

#include "exp_parser.h"
#include "exp_stack.h"
#include "scanner.h"
#include "parser.h"
#include "symtable_stack.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

Prec_Table_Action_T prec_table[PREC_TABLE_SIZE][PREC_TABLE_SIZE] = {
  /*     !   *   /   +   −   ==  !=  <   >   <=  >=  ??  (   )   id  $     */
        {Er, R,  R,  R,  R,  R,  R,  R,  R,  R,  R,  R,  S,  R,  S,  R},  // !
        {S,  R,  R,  R,  R,  R,  R,  R,  R,  R,  R,  R,  S,  R,  S,  R},  // *
        {S,  R,  R,  R,  R,  R,  R,  R,  R,  R,  R,  R,  S,  R,  S,  R},  // /
        {S,  S,  S,  R,  R,  R,  R,  R,  R,  R,  R,  R,  S,  R,  S,  R},  // +  
        {S,  S,  S,  R,  R,  R,  R,  R,  R,  R,  R,  R,  S,  R,  S,  R},  // −
        {S,  S,  S,  S,  S,  Er, Er, Er, Er, Er, Er, R,  S,  R,  S,  R},  // == rel
        {S,  S,  S,  S,  S,  Er, Er, Er, Er, Er, Er, R,  S,  R,  S,  R},  // != rel
        {S,  S,  S,  S,  S,  Er, Er, Er, Er, Er, Er, R,  S,  R,  S,  R},  // <  rel
        {S,  S,  S,  S,  S,  Er, Er, Er, Er, Er, Er, R,  S,  R,  S,  R},  // >  rel
        {S,  S,  S,  S,  S,  Er, Er, Er, Er, Er, Er, R,  S,  R,  S,  R},  // <= rel
        {S,  S,  S,  S,  S,  Er, Er, Er, Er, Er, Er, R,  S,  R,  S,  R},  // >= rel
        {S,  S,  S,  S,  S,  S,  S,  S,  S,  S,  S,  S,  S,  R,  S,  R},  // ??
        {S,  S,  S,  S,  S,  S,  S,  S,  S,  S,  S,  S,  S,  Eq, S,  Er}, // (
        {R,  R,  R,  R,  R,  R,  R,  R,  R,  R,  R,  R,  Er, R,  Er, R},  // )
        {R,  R,  R,  R,  R,  R,  R,  R,  R,  R,  R,  R,  Er, R,  Er, R},  // id
        {S,  S,  S,  S,  S,  S,  S,  S,  S,  S,  S,  S,  S,  Er, S,  Er}, // $
};

Prec_Table_Action_T which_relation(Prec_Table_Symbol_T symbol1, Prec_Table_Symbol_T symbol2){
    return prec_table[symbol1][symbol2];
}

Prec_Table_Symbol_T Token_to_Symbol(Token_T *token){
    switch (token->token_type) {
        case TOKEN_MUL:
            return P_TABLE_MUL;
        case TOKEN_DIV:
            return P_TABLE_DIV;
        case TOKEN_PLUS:
            return P_TABLE_PLUS;
        case TOKEN_MINUS:
            return P_TABLE_MINUS;
        case TOKEN_EQLS:
            return P_TABLE_EQLS;
        case TOKEN_NOT_EQLS:
            return P_TABLE_NOT_EQLS;
        case TOKEN_LESS:
            return P_TABLE_LESS;
        case TOKEN_GREATER:
            return P_TABLE_GREATER;
        case TOKEN_LESS_EQL:
            return P_TABLE_LESS_EQL;
        case TOKEN_GREATER_EQL:
            return P_TABLE_GREATER_EQL;
        case TOKEN_NILL_CMP:
            return P_TABLE_NILL_CMP;
        case TOKEN_L_PAR:
            return P_TABLE_L_PAR;
        case TOKEN_R_PAR:
            return P_TABLE_R_PAR;
        case TOKEN_EXCL_MARK:
            return P_TABLE_EXCL_MARK;
        case TOKEN_STR:
        case TOKEN_INT:
        case TOKEN_FLOAT:
        case TOKEN_VAR_ID:
            return P_TABLE_ID;
        default:
            return P_TABLE_DOLLAR;
    }
}

//operand 1 is the lowest in stack (lowest from those 3 operands)
Prec_rules_T which_rule(int num_of_operands, Stack_Item_T *operand1, Stack_Item_T *operand2, Stack_Item_T *operand3){
    if(num_of_operands == 1){
    	if(operand1->pt_symbol == P_TABLE_ID){
    	    return RULE_ID;
    	}
        return RULE_UNDEFINED;
    }
    else if(num_of_operands == 2){
        if((operand1->pt_symbol == P_TABLE_NON_TERMINAL) && (operand2->pt_symbol == P_TABLE_EXCL_MARK)){
            return RULE_NOT_NIL;
        }
        return RULE_UNDEFINED;
    }
    else if(num_of_operands == 3){
        // E -> E * E
        if((operand1->pt_symbol == P_TABLE_NON_TERMINAL) && (operand2->pt_symbol == P_TABLE_MUL) && (operand3->pt_symbol == P_TABLE_NON_TERMINAL)) {
            return RULE_MUL;
        }
        // E -> E / E
        if((operand1->pt_symbol == P_TABLE_NON_TERMINAL) && (operand2->pt_symbol == P_TABLE_DIV) && (operand3->pt_symbol == P_TABLE_NON_TERMINAL)) {
            return RULE_DIV;
        }
        // E -> E + E
        if((operand1->pt_symbol == P_TABLE_NON_TERMINAL) && (operand2->pt_symbol == P_TABLE_PLUS) && (operand3->pt_symbol == P_TABLE_NON_TERMINAL)) {
            return RULE_PLUS;
        }
        // E -> E - E
        if((operand1->pt_symbol == P_TABLE_NON_TERMINAL) && (operand2->pt_symbol == P_TABLE_MINUS) && (operand3->pt_symbol == P_TABLE_NON_TERMINAL)) {
            return RULE_MINUS;
        }
        // E -> E == E
        if((operand1->pt_symbol == P_TABLE_NON_TERMINAL) && (operand2->pt_symbol == P_TABLE_EQLS) && (operand3->pt_symbol == P_TABLE_NON_TERMINAL)) {
            return RULE_EQ;
        }
        // E -> E != E
        if((operand1->pt_symbol == P_TABLE_NON_TERMINAL) && (operand2->pt_symbol == P_TABLE_NOT_EQLS) && (operand3->pt_symbol == P_TABLE_NON_TERMINAL)) {
            return RULE_NOT_EQ;
        }
        // E -> E < E
        if((operand1->pt_symbol == P_TABLE_NON_TERMINAL) && (operand2->pt_symbol == P_TABLE_LESS) && (operand3->pt_symbol == P_TABLE_NON_TERMINAL)) {
            return RULE_LESS;
        }
        // E -> E > E
        if((operand1->pt_symbol == P_TABLE_NON_TERMINAL) && (operand2->pt_symbol == P_TABLE_GREATER) && (operand3->pt_symbol == P_TABLE_NON_TERMINAL)) {
            return RULE_GREATER;
        }
        // E -> E <= E
        if((operand1->pt_symbol == P_TABLE_NON_TERMINAL) && (operand2->pt_symbol == P_TABLE_LESS_EQL) && (operand3->pt_symbol == P_TABLE_NON_TERMINAL)) {
            return RULE_LESS_EQ;
        }
        // E -> E >= E
        if((operand1->pt_symbol == P_TABLE_NON_TERMINAL) && (operand2->pt_symbol == P_TABLE_GREATER_EQL) && (operand3->pt_symbol == P_TABLE_NON_TERMINAL)) {
            return RULE_GREATER_EQ;
        }
        // E -> E ?? E
        if((operand1->pt_symbol == P_TABLE_NON_TERMINAL) && (operand2->pt_symbol == P_TABLE_NILL_CMP) && (operand3->pt_symbol == P_TABLE_NON_TERMINAL)) {
            return RULE_NILL_CMP;
        }
        // E -> (E)
        if((operand1->pt_symbol == P_TABLE_L_PAR) && (operand2->pt_symbol == P_TABLE_NON_TERMINAL) && (operand3->pt_symbol == P_TABLE_R_PAR)) {
            return RULE_PARS;
        }
        return RULE_UNDEFINED;
    }
    else{
    	return RULE_UNDEFINED;
    }
}
enum Var_type which_data_type(Token_T *token, Parser_T *struct_parser){
    switch (token->token_type) {
        case TOKEN_INT:
            return INT;
        case TOKEN_FLOAT:
            return DOUBLE;
        case TOKEN_STR:
            return STRING;
        case TOKEN_VAR_ID: ;
            TNode *var_node;
            var_node = search_st_stack(struct_parser->var_st_stack,token->token_value.dyn_str.dynamic_str);
            if(var_node == NULL){
                return UNDEFINED_TYPE;
            }
            return var_node->variable_data.type;
        default:
            return UNDEFINED_TYPE;
    }
}

int semantic_analysis(Prec_rules_T rule, Stack_Item_T *operand1, Stack_Item_T *operand2, Stack_Item_T *operand3, enum Var_type *final_type){

    if(rule != RULE_ID && rule != RULE_PARS && rule != RULE_NOT_NIL) {
        if((operand1->data_type == UNDEFINED_TYPE) || (operand3->data_type == UNDEFINED_TYPE)){
            return SEMANTIC_ERR_C;}

        if((operand1->data_type == BOOL) || (operand3->data_type == BOOL)){
            return SEMANTIC_ERR_E;}
    }

    switch (rule) {
        case RULE_NOT_NIL:
            if(operand1->data_type == UNDEFINED_TYPE){
                return SEMANTIC_ERR_C;}
            if(operand1->token == NULL){
                return SEMANTIC_ERR_E;
            }
            if(operand1->token->token_value.token_keyword == NIL_KW) {
                return SEMANTIC_ERR_E;}
            break;

        case RULE_MUL:
        case RULE_PLUS:
        case RULE_MINUS:
            if(operand1->data_type == INT && operand3->data_type == INT ){
                *final_type = INT;}
            else if(operand1->data_type == DOUBLE && operand3->data_type == DOUBLE ){
                *final_type = DOUBLE;}
            else if(operand1->data_type == DOUBLE && operand3->data_type == INT ){
                if(operand3->token == NULL){
                    return SEMANTIC_ERR_E;
                }
                if(operand3->token->token_type == TOKEN_INT){
                    //IntToDouble(operand3)
                    *final_type = DOUBLE;
                    break;
                }
                return SEMANTIC_ERR_E;}
            else if(operand1->data_type == INT && operand3->data_type == DOUBLE ){
                if(operand1->token == NULL){
                    return SEMANTIC_ERR_E;
                }
                if(operand1->token->token_type == TOKEN_INT){
                    //IntToDouble(operand1)
                    *final_type = DOUBLE;
                    break;
                }
                return SEMANTIC_ERR_E;}
            //concatenate
            else if (operand1->data_type == STRING && operand2->pt_symbol == P_TABLE_PLUS && operand3->data_type == STRING){
                *final_type = STRING;
            }
            else if(operand1->data_type != operand3->data_type){
                return SEMANTIC_ERR_E;}
            break;

        case RULE_DIV:
            if(operand1->data_type == INT && operand3->data_type == INT ){
                *final_type = INT;}
            else if(operand1->data_type == DOUBLE && operand3->data_type == DOUBLE ){
                *final_type = DOUBLE;}
            break;

        //rel. operators
        case RULE_EQ:
        case RULE_NOT_EQ:
            *final_type = BOOL;
            if(operand1->data_type == INT && operand3->data_type == DOUBLE){
                //IntToDouble(operand1)
                break;
            }
            else if(operand1->data_type == INT && operand3->data_type == DOUBLE){
                //IntToDouble(operand3)
                break;
            }
            else if(operand1->data_type != operand3->data_type){
                return SEMANTIC_ERR_E;}
            break;

        case RULE_LESS:
        case RULE_GREATER:
        case RULE_LESS_EQ:
        case RULE_GREATER_EQ:
            *final_type = BOOL;
            if(operand1->data_type != operand3->data_type){
                return SEMANTIC_ERR_E;}
            break;

        case RULE_NILL_CMP:
            if(operand1->data_type == UNDEFINED_TYPE || operand3->data_type == UNDEFINED_TYPE){
                return SEMANTIC_ERR_E;
            }
            if(operand1->token == NULL){
                return SEMANTIC_ERR_E;
            }
            if(operand1->token->token_value.token_keyword == NIL_KW){
                if((operand3->data_type == INT && operand1->data_type == INT_NIL) ||
                   (operand3->data_type == DOUBLE && operand1->data_type == DOUBLE_NIL) ||
                   (operand3->data_type == STRING && operand1->data_type ==STRING_NIL))
                    break;
                else{
                    return SEMANTIC_ERR_E;
                }
            }
            break;

        case RULE_PARS:
            if(operand2->data_type == UNDEFINED_TYPE){
                return SEMANTIC_ERR_C;}

            *final_type = operand2->data_type;
            break;

        case RULE_ID:
            if(operand1->data_type == UNDEFINED_TYPE){
                return SEMANTIC_ERR_C;}
            if(operand1->data_type == BOOL){
                return SEMANTIC_ERR_E;}
            *final_type = operand1->data_type;
            break;

        default:
            break;
    }
    return NO_ERR;
}

int shift_to_stack(Stack_T *stack, Token_T *token, Parser_T *struct_parser){
    //first push the stop_reduce ("<") symbol
    if(!stack_push_item_after_top_terminal(stack,P_TABLE_STOP_REDUCING,UNDEFINED_TYPE,NULL)){
        return 99;
    }

    Prec_Table_Symbol_T token_symbol =  Token_to_Symbol(token);
    if(!stack_push_item(stack, token_symbol, which_data_type(token,struct_parser),token)){
        return 99;
    }
    
    // Push values on stack
    if (token->token_type == TOKEN_INT){
            printf("PUSHS int@%d\n", token->token_value.num_integer);
    } else if (token->token_type == TOKEN_FLOAT){
            printf("PUSHS float@%a\n", token->token_value.num_decimal);
    } else if (token->token_type == TOKEN_STR){
            printf("PUSHS string@");
            print_string_in_ifjcode_form(token->token_value.dyn_str.dynamic_str);
            printf("\n");
    } else if (token->token_type == TOKEN_VAR_ID){
        if (struct_parser->inside_main == true || struct_parser->in_while == true)
            printf("PUSHS GF@__%s__\n", token->token_value.dyn_str.dynamic_str);
        else
            printf("PUSHS LF@__%s__\n", token->token_value.dyn_str.dynamic_str);
    }   
    
    return NO_ERR;
}

int reduce(Stack_T *stack){
    Stack_Item_T *operand1 = NULL;
    Stack_Item_T *operand2 = NULL;
    Stack_Item_T *operand3 = NULL;
    Prec_rules_T rule;
    int error;
    enum Var_type final_type;
    int after_stop_reduce = stack_items_after_stop_reducing(stack);
    switch (after_stop_reduce) {
        case 1:
            //one operand to reduce
            operand1 = stack->stack_head;
            if((rule = which_rule(1, operand1, operand2, operand3)) == RULE_UNDEFINED){
                return SYNTAX_ERR;}

            //pop two items, bcs I need to remove the stop_reduce symbol too
            //semantic analysis, if error, return to main parser function and end program
            if((error = semantic_analysis(rule, operand1, operand2, operand3, &final_type)) != NO_ERR){
                return error;
            }
            stack_pop_item_multi(stack,2);
            //Operand is literal, so I will pass the token to non_terminal that represents this number
            //this will be usefull for semantic checks in reduce_function
            if(operand1->token->token_type == TOKEN_INT || operand1->token->token_type == TOKEN_FLOAT){
                stack_push_item(stack, P_TABLE_NON_TERMINAL, final_type, operand1->token);
                break;}

            stack_push_item(stack, P_TABLE_NON_TERMINAL, final_type, NULL);
            break;
        case 2:
            //two operands to reduce
            operand1 = stack->stack_head->next_item;
            operand2 = stack->stack_head;
            if((rule = which_rule(2, operand1, operand2, operand3)) == RULE_UNDEFINED){
                return SYNTAX_ERR;}

            //semantic analysis, if error, return to main parser function and end program
            if((error = semantic_analysis(rule, operand1, operand2, operand3, &final_type)) != NO_ERR){
                return error;}

            stack_pop_item_multi(stack,3);
            stack_push_item(stack, P_TABLE_NON_TERMINAL, final_type, NULL);
            break;
        case 3:
            //three operands to reduce
            operand1 = stack->stack_head->next_item->next_item;
            operand2 = stack->stack_head->next_item;
            operand3 = stack->stack_head;
            if((rule = which_rule(3, operand1, operand2, operand3)) == RULE_UNDEFINED){
                return SYNTAX_ERR;}
            //semantic analysis, if error, return to main parser function and end program
            if((error = semantic_analysis(rule, operand1, operand2, operand3, &final_type)) != NO_ERR){
                return error;}
            // Apply arithmetic operations to values on stack
            if (rule == RULE_MUL){
                printf("MULS\n");
            }
            else if (rule == RULE_DIV){
                if (operand3->token->token_type == TOKEN_FLOAT){
                    printf("DIVS\n");
                }
                else if (operand3->token->token_type == TOKEN_INT){
                    printf("IDIVS\n");
                }
            }
            else if (rule == RULE_PLUS){
                printf("ADDS\n");
            }
            else if (rule == RULE_MINUS){
                printf("SUBS\n");
            }

            stack_pop_item_multi(stack,4);

            stack_push_item(stack, P_TABLE_NON_TERMINAL, final_type, NULL);
            break;
        default:
            return SYNTAX_ERR;
    }
    return NO_ERR;
}

int expression_parse(Parser_T *struct_parser) {
    int result;
    Stack_T *stack = stack_init();
    if(stack == NULL){ return 99; }
    //at beginning, push dollar on bottom of stack
    if ((stack_push_item(stack, P_TABLE_DOLLAR, UNDEFINED_TYPE, NULL) == false)) {
        stack_clean(stack); return 99;
    }
    int error;
    Token_T *token = &struct_parser->current_token;
    //if we are in assignment rule, it is possible that current token sent from parser struct,
    //is already a part of an expression
    if(struct_parser->call_new_token == true) {
        TOKEN_OR_STACKCLEAN(token, stack)
    }

    //special part for "nil" in expression
    if(token->token_value.token_keyword == NIL_KW) {
        if(struct_parser->current_rule == VAR_DEF){
            if(struct_parser->var_data->type == UNDEFINED_TYPE){
                return SEMANTIC_ERR_F;
            }
            if(struct_parser->var_data->type == NIL ||
            struct_parser->var_data->type == INT_NIL ||
            struct_parser->var_data->type == STRING_NIL ||
            struct_parser->var_data->type == DOUBLE_NIL){
                TOKEN_OR_STACKCLEAN(token,stack)
                struct_parser->current_token = *token;
                printf("PUSHS nil@nil\n");
                return NO_ERR;
            }
            return SEMANTIC_ERR_E;
        }
        return SYNTAX_ERR;
    }
    //special part for: if let a
    if(struct_parser->current_rule == IF_STMNT) {
        if (token->token_value.token_keyword == LET_KW) {
            TOKEN_OR_STACKCLEAN(token, stack)
            /**
             * Code generation
             * if var == nil
             * Get its type as a string
             * If string begins with "n" ("nil") -> jump to second statement list
            */
            printf("DEFVAR GF@$_tmp_%d\nDEFVAR GF@$_tmp2_%d\n", struct_parser->if_count, struct_parser->if_count);
            printf("TYPE GF@$_tmp_%d ", struct_parser->if_count);
            get_frame(*token, struct_parser);
            printf("\nSTRLEN GF@$_tmp2_%d GF@$_tmp_%d\nJUMPIFEQ if_not_passed%d GF@$_tmp2_%d int@0\n", struct_parser->if_count, struct_parser->if_count, struct_parser->if_count, struct_parser->if_count);
            printf("GETCHAR GF@$_tmp_%d GF@$_tmp_%d int@0\n", struct_parser->if_count, struct_parser->if_count);
            printf("JUMPIFNEQ if_passed%d GF@$_tmp_%d string@n\n", struct_parser->if_count, struct_parser->if_count);
            printf("JUMP if_not_passed%d\nLABEL if_passed%d\n", struct_parser->if_count, struct_parser->if_count);

            Prec_Table_Symbol_T symbol = Token_to_Symbol(token);
            if (symbol == P_TABLE_ID) {
                TNode *var_node = search_st_stack(struct_parser->var_st_stack,token->token_value.dyn_str.dynamic_str);
                if(var_node->variable_data.init == true){}
                if(var_node->variable_data.constant == false){
                    return SEMANTIC_ERR_OTHER;
                }
                TOKEN_OR_STACKCLEAN(token, stack)
                if ((token->token_type == TOKEN_EOL) || (token->token_type == TOKEN_L_BRAC)) {
                    struct_parser->current_token = *token;
                    return NO_ERR;
                }
                return SYNTAX_ERR;
            } else {
                return SYNTAX_ERR;}}
    }
    Stack_Item_T *top_terminal = NULL;
    bool stop_while = false;
    int num_shift = 0;

    /*
    Code-gen
    */
    int token_count = 0;
    Token_T *token_array;
    if(struct_parser->current_rule == IF_STMNT || struct_parser->current_rule == WHILE_STMNT){
        token_array = (Token_T *)malloc(sizeof(Token_T)*5);
        if(token_array == NULL){return COMPILER_ERR_INTER;}
    }
    char while_counter[5];
    sprintf(while_counter, "%d", struct_parser->while_count);

    while(!stop_while){
        /*
          Code-gen
          Get token array for if expression in correct order
        */
        if(struct_parser->current_rule == IF_STMNT){
            if(token->token_type != TOKEN_L_PAR){
                
                if(token_count == 0){
                    
                    token_array[1] = *token;
                }
                if(token_count == 1){
                    token_array[0] = *token;
                }
                if(token_count == 3){
                    token_array[2] = *token;
                    print_token_array(token_array, 5, struct_parser, 1);
                }
                token_count++;
            }
        }
        /*
          Code-gen
          Get token array for while expression in correct order
        */
        if(struct_parser->current_rule == WHILE_STMNT){
            if(token->token_type != TOKEN_L_PAR){
                
                if(token_count == 0){
                    token_array[1] = *token;
                }
                if(token_count == 1){
                    token_array[0] = *token;
                }
                if(token_count == 3){
                    token_array[2] = *token;
                    print_token_array(token_array, 5, struct_parser, 2);
                }
                token_count++;
            }
        }

        top_terminal = stack_first_terminal(stack);
        Prec_Table_Symbol_T token_symbol = Token_to_Symbol(token);

        switch (which_relation(top_terminal->pt_symbol,token_symbol)) {

            case S: ;
                //shift item to stack and insert stop_reduce
                Token_T tmp = *token;
                if((error = shift_to_stack(stack, &tmp, struct_parser)) != NO_ERR){
                    stack_clean(stack); return error;
                }
                TOKEN_OR_STACKCLEAN(token,stack)
                num_shift++;
                break;

            case R:
                //reduce expressions with one of the rules
                if((error = reduce(stack)) != NO_ERR){
                    stack_clean(stack); return error;
                }
                break;

            case Eq: ;
                //Pars are complet/closed, just push item to stack
                Token_T tmp2 = *token;
                if(!stack_push_item(stack, token_symbol, which_data_type(&tmp2, struct_parser), &tmp2)){
                    stack_clean(stack); return 99;}
                TOKEN_OR_STACKCLEAN(token,stack)
                break;

            case Er:
                //successful end of expression parse, or error
                if(struct_parser->current_token.token_type == TOKEN_FUNC_ID && struct_parser->current_rule == VAR_DEF){
                    stack_clean(stack);
                    return NO_ERR;
                }
                if((top_terminal->pt_symbol == P_TABLE_DOLLAR ) && (token_symbol == P_TABLE_DOLLAR)){
                    stop_while = true;
                    break;}
                stack_clean(stack);
                return SYNTAX_ERR;
        }
        //special part for: if(let a)
        if(struct_parser->current_rule == IF_STMNT && num_shift == 1 && token_symbol == P_TABLE_L_PAR){
            if(token->token_value.token_keyword == LET_KW){
                TOKEN_OR_STACKCLEAN(token,stack)
                Prec_Table_Symbol_T symbol = Token_to_Symbol(token);
                if (symbol == P_TABLE_ID){
                    TNode *var_node = search_st_stack(struct_parser->var_st_stack,token->token_value.dyn_str.dynamic_str);
                    if(var_node->variable_data.constant == false){
                        return SEMANTIC_ERR_OTHER;
                    }
                    TOKEN_OR_STACKCLEAN(token,stack)
                    if(token->token_type == TOKEN_R_PAR){
                        struct_parser->current_token = *token;
                        return NO_ERR;}
                    return SYNTAX_ERR;}
                else{
                    return SYNTAX_ERR;}}}
    }
    if(stack->stack_head->pt_symbol != P_TABLE_NON_TERMINAL){
        stack_clean(stack);
        return SYNTAX_ERR;
    }
    enum Var_type return_type = stack->stack_head->data_type;
    if(struct_parser->current_rule == VAR_DEF) {
        if (struct_parser->var_data->type == UNDEFINED_TYPE) {
            struct_parser->var_data->type = return_type;
            struct_parser->var_data->init = true;
        }
        if((struct_parser->var_data->type == INT_NIL && return_type == INT) ||
           (struct_parser->var_data->type == DOUBLE_NIL && return_type == DOUBLE) ||
           (struct_parser->var_data->type == STRING_NIL && return_type == STRING)){;
            struct_parser->var_data->init = true;
            struct_parser->current_token = *token;
            stack_clean(stack);
            return NO_ERR;
        }
        if (struct_parser->var_data->type != return_type) {
            return SEMANTIC_ERR_E;
        }
    }
    if(struct_parser->current_rule == IF_STMNT) {
        if (return_type != BOOL) {
            stack_clean(stack);
            return SEMANTIC_ERR_E;}
    }

    //check of return type
    if(struct_parser->current_rule == RETURN) {
        printf("CREATEFRAME\n");
        TNode *found = search_symbol(struct_parser->global_func_symbtable->root, struct_parser->current_func_name);
        enum Var_type wanted_return = found->function_data.ret_type;
        switch (wanted_return) {
            case INT:
                if (stack->stack_head->data_type != INT) {
                    return SEMANTIC_ERR_B;}
                break;
            case DOUBLE:
                if (stack->stack_head->data_type != DOUBLE) {
                    return SEMANTIC_ERR_B;}
                break;
            case STRING:
                if (stack->stack_head->data_type != STRING) {
                    return SEMANTIC_ERR_B;}
                break;
            default:
                return SEMANTIC_ERR_D;
        }
    }
    
    //send current token to parser and clean stack
    struct_parser->current_token = *token;
    stack_clean(stack);
    return NO_ERR;
}

/* End of exp_parser.c */