/* ********************************* parser.h ******************************** */
/*  Author: Marek Čupr (xcuprm01)                                              */
/*  Subject: IFJ/IAL - Project                                                 */
/*  Date: 7. 11. 2023                                                          */
/*  Functionality: Header file for parser.c                                    */
/* *************************************************************************** */

#ifndef PARSER_H
#define PARSER_H

#include "scanner.h"  
#include "symtable.h"  
#include "symtable_stack.h"  
#include <stdbool.h>  

// Pre-defined functions
#define PREDEF_FUNCTIONS_CNT 10

/*
 * / ****************************** TOKENCHECKHELP ******************************** \  
 * / Helping macro that checks if the token loaded from the scanner is valid or not \
*/
#define TOKENCHECKHELP(token)            \
    result = get_token(token);           \
    if (result != NO_ERR)                \
        return result;                       

/*
 * / ********************************************** TOKENCHECK ************************************************* \  
 * / Macro that checks if the token loaded from the scanner is valid or not and skips the EOL tokens when needed \
*/
#define TOKENCHECK(token)                \
    result = skip_EOL(token);            \
    if (result != NO_ERR)                \
        return result;                       

/*
 * / ************************* RETURNCHECK *************************** \  
 * / Macro that checks if the called function returned an error or not \
*/
#define RETURNCHECK(func)                         \
    result = func;                                \
    if (result != NO_ERR)                         \
        return result;                            

/*
 * / ********************* Rule_Type_T ********************** \  
 * / Enumeration that holds all the possible LL grammar rules \
*/ 
typedef enum Rule_Type {
    IF_STMNT,
    WHILE_STMNT,
    FUNC_DEF,
    VAR_DEF,
    ASSIGNMENT,
    RETURN,
    OTHER
} Rule_Type_T;

/*
 * / ************************* Arguments_Data_T ************************* \  
 * / Structure that contains all the information about function arguments \
*/
typedef struct Arguments_Data {
    Token_T term;               //  Term value - int, float, string, bool, nil
    char *param_id;             //  Parameter id 
    char *param_name;           //  Parameter name
} Arguments_Data_T;

/*
 * / ********************** Parser_T ********************** \  
 * / Struct that holds all the information about the parser \
*/
typedef struct Parser {
    Token_T current_token;          // The current token from the scanner
    TTree *global_var_symbtable;    // The global symtable for storing global variables
    TTree *global_func_symbtable;   // The global symtable for storing global variables
    Symtable_Stack_T *var_st_stack; // The symtable stack
    Rule_Type_T current_rule;       // The rule we're currently on
    TData_var *var_data;

    bool inside_main; // Indicates if we're inside of a function or not
    bool EOL_skip; // Indicates if we're skipping the token for EOL or not
    bool has_return; // Indicates if function should have a return or not
    bool return_detected; // Indicates if a function return was detected or not
    bool call_new_token; // Indicates if the expression parser should call for a new token or not

    char curr_type; // Indicates if we're parsing the function/variable type
    char *current_func_name;
    char *var_name;

    int function_count; // Function counter for correct label generation
    int param_count;    // Parameter counter for passign parameters to a function correctly
    Token_T lvalue;     // Lvalue of an assignement
    int if_count;       // If counter for correct label generation
    int while_count;    // While counter for correct label generation
    int builtin_function_count; // Builtin function counter for correct label generation
    bool in_while;      // Boolean used to indicate which frame to use
} Parser_T;


/*
 * / ***************** skip_EOL() ***************** \
 * / Function that skips the EOL tokens when needed \
*/
int skip_EOL(Token_T *t);

/*
 * / ******************* create_global_symtable() ******************* \
 * / Function that creates the global variable and function symtables \
*/
int create_global_symtables(Parser_T *parser);

/*
 * / ***************************** predef_functions_insert() ***************************** \
 * / Function that inserts all the pre-defined functions into the global function symtable \
*/
int predef_functions_insert(Parser_T *parser);

/*
 * / *********** create_local_symtable() ************ \
 * / Function that creates a new empty local symtable \
*/
int create_local_symtable(Parser_T *parser, TTree *local_symtable);

/*
 * / ************************ check_term() ************************* \
 * / Function that checks if the function parameter is a term or not \
*/
int check_term(Arguments_Data_T *input_params_data, TNode *searched_node, int *loaded_paramas_cnt);

/*
 * / ****** handle_write() ******** \
 * / Function that prints the terms \
*/
int handle_write();

/*
 * / **************** func_param_init() ****************** \
 * / Function that allocates memory for all the parameters \
*/
int func_param_init(TData_func *func_data);

/*
 * / ******************* check_unique_id() ********************** \
 * / Function that checks if a function parameter has a unique ID \
*/
int check_unique_id(TData_func *func_data);

/*
 * / ******************* check_unique_name() ********************** \
 * / Function that checks if a function parameter has a unique name \
*/
int check_unique_name(TData_func *func_data);

/* *********************************************************** */
/*  The following functions are based on the LL Grammar rules  */
/* *********************************************************** */

/*  Main parser function  */
int parse();

/*  Rule 1: <program> ➔ <statement_list> <program_eof>  */
int parse_program();

/*  Rule 2: <statement_list> ➔ <function_definition> <statement_list> */
/*  Rule 3: <statement_list> ➔ <statement> <statement_list>           */
/*  Rule 4: <statement_list> ➔ ε                                      */
int parse_statement_list();

/*  Rule 5: <function_definition> ➔ func function_ID ( <params_list> ) <return_type> { <statement_list> }  */
int parse_function_definition();

/*  Rule 6: <params_list> ➔ <param_name> param_ID : <type> <params_list_n>  */
/*  Rule 7: <params_list> ➔ ε                                               */
int parse_params_list(TData_func *func_data, TData_var *var_data);

/*  Rule 8: <param_name> ➔ param_NAME  */
/*  Rule 9: <param_name> ➔ _           */
int parse_param_name(TData_func *func_data, TData_var *var_data);

/*  Rule 10: <type> ➔ String   */
/*  Rule 11: <type> ➔ String?  */
/*  Rule 12: <type> ➔ Int      */
/*  Rule 13: <type> ➔ Int?     */
/*  Rule 14: <type> ➔ Double   */
/*  Rule 15: <type> ➔ Double?  */
int parse_type(TData_func *func_data, TData_var *var_data);

/*  Rule 16: <params_list_n> ➔ , <param_name> param_ID : <type> <params_list_n>  */
/*  Rule 17: <params_list_n> ➔ ε                                                 */
int parse_params_list_n(TData_func *func_data, TData_var *var_data);

/*  Rule 18: <return_type> ➔ → <type>  */
/*  Rule 19: <return_type> ➔ ε         */
int parse_return_type(TData_func *func_data, TData_var *var_data);

/*  Rule 20: <statement> ➔ <variable_definition>  */
/*  Rule 21: <statement> ➔ <assignment>           */
/*  Rule 22: <statement> ➔ <if_statement>         */
/*  Rule 23: <statement> ➔ <while_statement>      */
/*  Rule 24: <statement> ➔ <function_call>        */
/*  Rule 25: <statement> ➔ <function_return>      */
int parse_statement();

/*  Rule 26: <variable_definition> ➔ <var_def_type> var_ID <var_type> <var_assign>  */
int parse_variable_definition();

/*  Rule 27: <var_def_type> ➔ let  */
/*  Rule 28: <var_def_type> ➔ var  */
int parse_var_def_type();

/*  Rule 29: <var_type> ➔ : <type>  */
/*  Rule 30: <var_type> ➔ ε         */
int parse_var_type();

/*  Rule 31: <var_assign> ➔ = expression  */
/*  Rule 32: <var_assign> ➔ ε             */
int parse_var_assign();

/*  Rule 33: <assignment> ➔ var_ID = expression       */
/*  Rule 34: <assignment> ➔ var_ID = <function_call>  */
int parse_assignment();

/*  Rule 35: <function_call> ➔ function_ID ( <input_params_list> )  */
int parse_function_call();

/*  Rule 36: <input_params_list> ➔ <input_param_name> term <input_params_list_n>  */
/*  Rule 37: <input_params_list> ➔ ε                                              */
int parse_input_params_list();

/*  Rule 38: <input_param_name> ➔  param_NAME :  */
/*  Rule 39: <input_param_name> ➔  ε             */
int parse_input_param_name();

/*  Rule 40: <input_params_list_n> ➔ , <input_param_name> term <input_params_list_n>  */
/*  Rule 41: <input_params_list_n> ➔ ε                                                */
int parse_input_params_list_n();

/*  Rule 42: <if_statement> ➔ if expression { statement_list } else { statement_list }  */
int parse_if_statement();

/*  Rule 43: <while_statement> ➔ while expression { statement_list }  */
int parse_while_statement();

/*  Rule 44: <function_return> ➔ return <return_option>  */
/*  Rule 45: <function_return> ➔ ε                       */
int parse_function_return();

/*  Rule 46: <return_option> ➔ expression  */
/*  Rule 47: <return_option> ➔ ε           */
int parse_return_option();

/*  Rule 48: <program_eof> ➔ EOF  */
int parse_program_eof();

#endif
/* End of parser.h */

