/* ******************************** parser.c ********************************* */
/*  Authors: Marek Čupr (xcuprm01), Vojtěch Hrabovský (xhrabo18)               */
/*  Subject: IFJ/IAL - Project                                                 */
/*  Date: 12. 11. 2023                                                         */
/*  Functionality: Parse all the different parts of the program                */
/* *************************************************************************** */

#include "parser.h"   // header file
#include "exp_parser.h"
#include "code_gen.h"
#include "dynamic_str.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief Names of all the pre-defined functions.
 */
char *predef_functions[PREDEF_FUNCTIONS_CNT] = {
    "readString",
    "readInt",
    "readDouble",
    "write",
    "Int2Double",
    "Double2Int",
    "length",
    "substring",
    "ord",
    "chr"
};

/**
 * @brief Return types of all the pre-defined functions.
 */
enum Var_type predef_functions_ret_type[PREDEF_FUNCTIONS_CNT] = {
    STRING_NIL,     // String?
    INT_NIL,        // Int?
    DOUBLE_NIL,     // Double?
    VOID,           // Void
    DOUBLE,
    INT,
    INT,
    STRING_NIL,     // String?
    INT,
    STRING
};

/**
 * @brief The main parser structure.
 */
Parser_T parser;

/**
 * Gets a new token from the scanner and skips all the EOL tokens if needed. 
 *
 * @param t Token to load from the scanner
 * @returns The correct error return code (0 if success)
 */
int skip_EOL(Token_T *t){
    int result; // Variable that holds the return value
    if (parser.EOL_skip == true){ // Skipping the EOL tokens
        do {
            TOKENCHECKHELP(t)
        } while (t->token_type == TOKEN_EOL);
    } else { // Not skipping the EOL tokens
        TOKENCHECKHELP(t)
    }
    return 0;
}

/**
 * @brief Prints the terms on stdout (pre-defined function "write").
 * 
 * @returns The correct error return code (0 if success)
 */
int handle_write(){
    int result; // Variable that holds the return value
    while (parser.current_token.token_type != TOKEN_R_PAR){ // Reading the function arguments
        switch (parser.current_token.token_type) {
            case TOKEN_INT: 
                printf("WRITE int@%d\n", parser.current_token.token_value.num_integer); // Print the integer
                break;
            case TOKEN_FLOAT: 
                printf("WRITE float@%a\n", parser.current_token.token_value.num_decimal); // Print the float
                break;
            case TOKEN_STR:
                printf("WRITE string@");
                print_string_in_ifjcode_form(parser.current_token.token_value.dyn_str.dynamic_str); // Print the string
                printf("\n");
                break;
            case TOKEN_VAR_ID: ;            
                TNode *found_var = search_st_stack(parser.var_st_stack, parser.current_token.token_value.dyn_str.dynamic_str);
                if (found_var == NULL) // The passed variable is NOT defined
                    return SEMANTIC_ERR_C; 

                if (found_var->variable_data.init == false && found_var->variable_data.is_param == false){
                    return SEMANTIC_ERR_C; // The passed variable is NOT initialized
                }

                TNode *found_var_global = search_symbol(parser.global_var_symbtable->root, parser.current_token.token_value.dyn_str.dynamic_str);
                if (found_var == found_var_global){ // The passed variable is a global variable
                    printf("WRITE GF@__%s__\n", found_var_global->id); // Print the global variable
                } else { // The passed variable is a local variable
                    printf("WRITE LF@__%s__\n", found_var->id); // Print the local variable
                }
                break;
            case TOKEN_KEYWORD:
                if (parser.current_token.token_value.token_keyword == NIL){ // The passed variable is a special nil character
                    printf("WRITE nil@nil\n"); // Print the special nil characted
                    break;
                }
                else // Invalid function argument 
                    return SEMANTIC_ERR_B;
            case TOKEN_COMMA:
                break;
            default: // Invalid function argument 
                    return SYNTAX_ERR;
        }
        /* Get the next token */
        TOKENCHECK(&parser.current_token)
    }
    /* Get the next token */
    TOKENCHECK(&parser.current_token)
    return NO_ERR;
}

/**
 * @brief Checks if the passed arguments in function call are terms.
 * 
 * @returns The correct error return code, 0 if success
 */
int check_term(Arguments_Data_T *input_params_data, TNode *searched_node, int *loaded_paramas_cnt){
    switch(input_params_data[*loaded_paramas_cnt].term.token_type){
        case TOKEN_INT:
            if (searched_node->function_data.parameters[*loaded_paramas_cnt].type != INT && searched_node->function_data.parameters[*loaded_paramas_cnt].type != INT_KW)
                return SEMANTIC_ERR_B; // The function argument type doen't match the function parameter type
            break;
        case TOKEN_FLOAT:
            if (searched_node->function_data.parameters[*loaded_paramas_cnt].type != DOUBLE && searched_node->function_data.parameters[*loaded_paramas_cnt].type != DOUBLE_KW)
                return SEMANTIC_ERR_B; // The function argument type doen't match the function parameter type
            break;
        case TOKEN_STR:

            if (searched_node->function_data.parameters[*loaded_paramas_cnt].type != STRING && searched_node->function_data.parameters[*loaded_paramas_cnt].type != STRING_NIL)
                return SEMANTIC_ERR_B; // The function argument type doen't match the function parameter type
            break;
        case TOKEN_VAR_ID: ;
            TNode *found_var = search_st_stack(parser.var_st_stack, input_params_data[*loaded_paramas_cnt].term.token_value.dyn_str.dynamic_str);
            if (found_var == NULL) // The passed variable is NOT defined
                return SEMANTIC_ERR_B;
            if (found_var->variable_data.init == false) // The passed variable is NOT initialized
                return SEMANTIC_ERR_C;

            if (found_var->variable_data.type != searched_node->function_data.parameters[*loaded_paramas_cnt].type)
                return SEMANTIC_ERR_B; // The function argument type doen't match the function parameter type
            break;
        case TOKEN_KEYWORD:
            if (parser.current_token.token_value.token_keyword == NIL_KW && (searched_node->function_data.parameters[*loaded_paramas_cnt].type == INT_NIL || searched_node->function_data.parameters[*loaded_paramas_cnt].type != DOUBLE_NIL || searched_node->function_data.parameters[*loaded_paramas_cnt].type == STRING_NIL))
                return NO_ERR; // The special nil characted
            else
                return SEMANTIC_ERR_B; // The function argument type doen't match the function parameter type
        default: // Invalid function argument 
            return SEMANTIC_ERR_B;
    }
    return NO_ERR;
}

/**
 * @brief Creates the global symtables and inserts all the pre-defined functions into the global function symtable.
 *
 * @param parser Parser that holds the global symtables to be created
 * @returns The correct error return code (0 if success)
 */
int create_global_symtables(Parser_T *parser){
    int result; // Variable that holds the return value

    // Allocate memory for the global symtables
    parser->global_var_symbtable = (TTree *) malloc(sizeof(TTree)); // Global variable symtable 
    parser->global_func_symbtable = (TTree *) malloc(sizeof(TTree)); // Global function symtable
    if (parser->global_func_symbtable == NULL || parser->global_var_symbtable == NULL)
        return COMPILER_ERR_INTER; // Malloc failed

    // Initialize the global symtables
    init_symtable(parser->global_func_symbtable);
    init_symtable(parser->global_var_symbtable);

    // Create a variable stack to store the variable symtables 
    parser->var_st_stack = (Symtable_Stack_T *) malloc(sizeof(Symtable_Stack_T));
    if (parser->var_st_stack == NULL)
        return COMPILER_ERR_INTER; // Malloc failed

    // Push the empty global variable symtable to the variable symtable stack
    RETURNCHECK(st_stack_push(parser->var_st_stack, parser->global_var_symbtable))

    // Insert the pre-defined functions into the global function symtable
    return predef_functions_insert(parser);
}

/**
 * @brief Creates a new empty local symtable and pushes it to the variable symtable stack.
 *
 * @param local_symtable Local symtable to be created and pushed to the varialbe symtable stack
 * @param parser Parser that holds the variable symtable stack to push the local symtable into
 * @returns The correct error return code (0 if success)
 */
int create_local_symtable(Parser_T *parser, TTree *local_symtable){
    // Allocate memory for the new local symtable
    local_symtable = (TTree *) malloc(sizeof(TTree));
    if (local_symtable == NULL) // Malloc failed
        return COMPILER_ERR_INTER;
    
    // Initialize the local symtable
    init_symtable(local_symtable);
    return st_stack_push(parser->var_st_stack, local_symtable); // Push the local symtable to the variable symtable stack
}

/**
 * @brief Inserts all the pre-defined functions into the global symtable
 *
 * @param parser Parser that holds the global symtable to be inserted to
 * @returns The correct error return code (0 if success)
 */
int predef_functions_insert(Parser_T *parser){
    // Gradually insert all the pre-defined functions into the global symtable
    for (int i = 0; i < PREDEF_FUNCTIONS_CNT; i++){
        TData_var var_data; // The variable data will be empty (we're inserting a function)

        // Save the needed information about the pre-defined functions 
        TData_func func_data;
        func_data.defined = true;
        switch(i){ // Check which pre-defined function we're currently on 
            case 0: case 1: case 2: // readString() -> String?, readInt() -> Int?, readDouble() -> Double?
                func_data.parameters = (Function_param *) malloc(sizeof(Function_param)); // Allocate memory for the parameters array
                if (func_data.parameters == NULL) // Malloc failed
                    return COMPILER_ERR_INTER;

                // The pre-defined functions don't have any arguments
                func_data.parameters->id = NULL;    
                func_data.parameters->name = NULL;  
                func_data.parameters->type = UNDEFINED_TYPE;
                func_data.parameter_count = 0; 
                break;
            case 3: // write(term1 , term2, ..., termn)
                func_data.parameters = NULL;
                break;
            case 4: // Int2Double(_ term ∶ Int) -> Double 
                func_data.parameters = (Function_param *) malloc(sizeof(Function_param)); // Allocate memory for the parameters array
                if (func_data.parameters == NULL) // Malloc failed
                    return COMPILER_ERR_INTER;

                // Save the information about parameters
                func_data.parameters->name = "_";
                func_data.parameters->id = "term";
                func_data.parameters->type = INT;
                func_data.parameter_count = 1; 
                break;
            case 5: // Double2Int(_ term ∶ Double) -> Int
                func_data.parameters = (Function_param *) malloc(sizeof(Function_param)); // Allocate memory for the parameters array
                if (func_data.parameters == NULL) // Malloc failed
                    return COMPILER_ERR_INTER;

                // Save the information about parameters
                func_data.parameters->name = "_";
                func_data.parameters->id = "term";
                func_data.parameters->type = DOUBLE;
                func_data.parameter_count = 1; 
                break;
            case 6: // length(_ s : String) -> Int
                func_data.parameters = (Function_param *) malloc(sizeof(Function_param)); // Allocate memory for the parameters array
                if (func_data.parameters == NULL) // Malloc failed
                    return COMPILER_ERR_INTER;

                // Save the information about parameters
                func_data.parameters->name = "_";
                func_data.parameters->id = "s";
                func_data.parameters->type = STRING;
                func_data.parameter_count = 1; 
                break;
            case 7: // substring(of s : String, startingAt i : Int, endingBefore j : Int) -> String?
                func_data.parameters = (Function_param *) malloc(sizeof(Function_param) * 3); // Allocate memory for the parameters array
                if (func_data.parameters == NULL) // Malloc failed
                    return COMPILER_ERR_INTER;

                // Save the information about parameter 1
                func_data.parameters[0].name = "of"; 
                func_data.parameters[0].id = "s"; 
                func_data.parameters[0].type = STRING; 

                // Save the information about parameter 2
                func_data.parameters[1].name = "startingAt"; 
                func_data.parameters[1].id = "i"; 
                func_data.parameters[1].type = INT; 

                // Save the information about parameter 3
                func_data.parameters[2].name = "endingBefore"; 
                func_data.parameters[2].id = "j"; 
                func_data.parameters[2].type = INT; 

                func_data.parameter_count = 3; 
                break;
            case 8: // ord(_ c : String) -> Int
                func_data.parameters = (Function_param *) malloc(sizeof(Function_param)); // Allocate memory for the parameters array
                if (func_data.parameters == NULL) // Malloc failed
                    return COMPILER_ERR_INTER;

                // Save the information about parameters
                func_data.parameters->name = "_";
                func_data.parameters->id = "c";
                func_data.parameters->type = STRING;
                func_data.parameter_count = 1; 
                break;
            case 9: // chr(_ i : Int) -> String
                func_data.parameters = (Function_param *) malloc(sizeof(Function_param)); // Allocate memory for the parameters array
                if (func_data.parameters == NULL) // Malloc failed
                    return COMPILER_ERR_INTER;
                    
                // Save the information about parameters
                func_data.parameters->name = "_";
                func_data.parameters->id = "i";
                func_data.parameters->type = INT;
                func_data.parameter_count = 1; 
                break;
        }

        func_data.ret_type = predef_functions_ret_type[i]; // Save the correct return type
        // Insert the correct function into the global symtable
        if (insert_symbol(&parser->global_func_symbtable->root, predef_functions[i], FUNCTION, var_data, func_data, parser->global_func_symbtable) == 0) // Symbol insertion failed
            return COMPILER_ERR_INTER; // Malloc failed
    }
    return NO_ERR;
}

/**
 * @brief Checks if the function parameters have all different/unique names. 
 *
 * @param func_data Structure that contains all the current functions' parameter names
 * @returns The correct error return code (0 if success)
 */
int check_unique_name(TData_func *func_data){
    // Only 1 function parameter (the name's unique)
    if (func_data->parameter_count == 1)
        return NO_ERR;

    // More than 1 function parameter
    for (int i = 0; i < func_data->parameter_count - 1; i++){
        if (strcmp(func_data->parameters[i].name, parser.current_token.token_value.dyn_str.dynamic_str) == 0){
            return SEMANTIC_ERR_A; // The function parameter name is not unique
        }
    }
    return NO_ERR;
}


/**
 * @brief Checks if the function parameters have all different/unique IDs. 
 *
 * @param func_data Structure that contains all the current functions' parameter IDs
 * @returns The correct error return code (0 if success)
 */
int check_unique_id(TData_func *func_data){
    // Only 1 function parameter (the ID's unique)
    if (func_data->parameter_count == 1)
        return NO_ERR;

    // More than 1 function parameter
    for (int i = 0; i < func_data->parameter_count - 1; i++){
        if (strcmp(func_data->parameters[i].id, parser.current_token.token_value.dyn_str.dynamic_str) == 0){
            return SEMANTIC_ERR_A; // The function parameter ID is not unique
        }
    }
    return NO_ERR;
}

/**
 * @brief Allocates memory for all the function parameters. 
 *
 * @param func_data Structure that contains an array of parameters
 * @returns The correct error return code (0 if success)
 */
int func_param_init(TData_func *func_data){
    // Only 1 function parameter
    if (func_data->parameter_count == 1){ // Allocate memory for the first parameter
        func_data->parameters = (Function_param *) malloc(sizeof(Function_param));
    } else if (func_data->parameter_count > 1){ // Reallocate memory for more parameters
        func_data->parameters = (Function_param *) realloc(func_data->parameters, sizeof(Function_param) * func_data->parameter_count);
    } else { // Invalid parameter count (should not happen)
        return SYNTAX_ERR;
    }

    if (func_data->parameters == NULL) // Malloc failed
        return COMPILER_ERR_INTER;

    return NO_ERR;
}


/**
 * @brief Starts the return type parsing.
 *
 * Rule:  <return_type> ➔ → <type>
 * Rule:  <return_type> ➔ ε                                          
 * 
 * @param func_data Structure that contains all the information about the currently being loaded function
 * @returns The correct error code, 0 when success
 */
int parse_return_type(TData_func *func_data, TData_var *var_data) {
    int result; // Variable that holds the return value

    /* Get the next token */
    TOKENCHECK(&parser.current_token)

    if (parser.current_token.token_type == TOKEN_L_BRAC){ // The function return type is empty (void) 
        func_data->ret_type = VOID;
        parser.has_return = false;
        return NO_ERR;
    } else if (parser.current_token.token_type == TOKEN_ARROW){ // The function return type is NOT empty
        /* Get the next token */
        TOKENCHECK(&parser.current_token)

        /* Parse the type */
        parser.curr_type = 'r'; // Loading the return type
        RETURNCHECK(parse_type(func_data, var_data)); 

        /* Get the next token */
        TOKENCHECK(&parser.current_token)

        parser.has_return = true;
        return NO_ERR;
    } else { // Invalid function return type
        return SYNTAX_ERR;
    }
}

/**
 * @brief Starts the params list n parsing.
 *
 * Rule:  <params_list_n> ➔ , <param_name> param_ID : <type> <params_list_n>
 * Rule:  <params_list_n> ➔ ε                                        
 * 
 * @param func_data Structure that contains all the information about the currently being loaded function
 * @returns The correct error code, 0 when success
 */
int parse_params_list_n(TData_func *func_data, TData_var *var_data){
    int result; // Variable that holds the return value

    if (parser.current_token.token_type == TOKEN_R_PAR){  // The parameter list n is empty
        // Move parameters from TF to LF
        for(int i = 0; i < func_data->parameter_count; i++){
            printf("DEFVAR LF@__%s__\n", func_data->parameters[i].id);
            printf("MOVE LF@__%s__ TF@_p%d_\n", func_data->parameters[i].id, i);
        }
        return NO_ERR;
    } else if (parser.current_token.token_type == TOKEN_COMMA){ // The parameters list n is NOT empty
        // Realloc the parameters array for more parameters
        func_data->parameter_count++; // Increase the parameter count
        RETURNCHECK(func_param_init(func_data));

        /* Get the next token */
        TOKENCHECK(&parser.current_token)

        /* Parse the parameter name */
        RETURNCHECK(parse_param_name(func_data, var_data));

        /* Get the next token */
        TOKENCHECK(&parser.current_token)

         // Check if the token is a variable ID and if the parameter ID and the parametr name aren't identical
        if (parser.current_token.token_type != TOKEN_VAR_ID || strcmp(parser.current_token.token_value.dyn_str.dynamic_str, func_data->parameters[func_data->parameter_count - 1].name) == 0){
            return SEMANTIC_ERR_A;
        }

        // Check if parameter with this ID doesn't exist already
        RETURNCHECK(check_unique_id(func_data))

        // Save tha parameter ID
        func_data->parameters[func_data->parameter_count - 1].id = parser.current_token.token_value.dyn_str.dynamic_str; 

        /* Get the next token */
        TOKENCHECK(&parser.current_token)

        if (parser.current_token.token_type != TOKEN_COLON)
            return SYNTAX_ERR;

        /* Get the next token */
        TOKENCHECK(&parser.current_token)

        /* Parse the type */
        parser.curr_type = 'p'; // Loading the argument type
        RETURNCHECK(parse_type(func_data, var_data));

        /* Get the next token */
        TOKENCHECK(&parser.current_token)

        /* Parse the parameters list n */
        RETURNCHECK(parse_params_list_n(func_data, var_data));

        return NO_ERR;
    } else { // The parameters list n is invalid
        return SYNTAX_ERR;
    }
}

/**
 * @brief Starts the type parsing.
 *
 * Rule:  <type> ➔ String
 * Rule:  <type> ➔ String?
 * Rule:  <type> ➔ Int
 * Rule:  <type> ➔ Int?
 * Rule:  <type> ➔ Double
 * Rule:  <type> ➔ Double?
 * 
 * @param func_data Structure that contains all the information about the currently being loaded function
 * @returns The correct error code, 0 when success
 */
int parse_type(TData_func *func_data, TData_var *var_data) {
    if (parser.current_token.token_type != TOKEN_KEYWORD)  // The current token is NOT a keyword
        return SYNTAX_ERR;

    if (parser.curr_type == 'p'){   // We are loading the function parameter type
        switch(parser.current_token.token_value.token_keyword){
            case STRING_KW:
                if (parser.current_token.can_be_nil == true){
                    func_data->parameters[func_data->parameter_count - 1].type = STRING_NIL; // String?
                } else {
                    func_data->parameters[func_data->parameter_count - 1].type = STRING;
                }
                break;
            case INT_KW: 
                if (parser.current_token.can_be_nil == true){
                    func_data->parameters[func_data->parameter_count - 1].type = INT_NIL; // Int?
                } else {
                    func_data->parameters[func_data->parameter_count - 1].type = INT;
                }
                break;
            case DOUBLE_KW:
                if (parser.current_token.can_be_nil == true){
                    func_data->parameters[func_data->parameter_count - 1].type = DOUBLE_NIL; // Double?
                } else {
                    func_data->parameters[func_data->parameter_count - 1].type = DOUBLE;

                }
                break;
            default:  // Invalid keyword 
                return SYNTAX_ERR;
                break;
        }
    } else if (parser.curr_type == 'r'){ // We are loading the function return type
        switch(parser.current_token.token_value.token_keyword){
            case STRING_KW:
                if (parser.current_token.can_be_nil == true){
                    func_data->ret_type = STRING_NIL; // String?
                } else {
                    func_data->ret_type = STRING;
                }
                break;
            case INT_KW:
                if (parser.current_token.can_be_nil == true){
                    func_data->ret_type = INT_NIL; // Int?
                } else {
                    func_data->ret_type = INT;
                }
                break;
            case DOUBLE_KW:
                if (parser.current_token.can_be_nil == true){
                    func_data->ret_type = DOUBLE_NIL; // Double?
                } else {
                    func_data->ret_type = DOUBLE;
                }
                break;
            default: // Invalid keyword
                return SYNTAX_ERR;
                break;
        }
    } else if (parser.curr_type == 'v'){ // We are loading the variable type
        switch(parser.current_token.token_value.token_keyword){
            case STRING_KW:
            if (parser.current_token.can_be_nil == true){
                var_data->type = STRING_NIL; // String?
            } else {
                var_data->type = STRING;
            }
                break;
            case INT_KW:
                if (parser.current_token.can_be_nil == true){
                    var_data->type = INT_NIL;  // Int?
                } else {
                    var_data->type = INT;
                }
                break;
            case DOUBLE_KW:
                if (parser.current_token.can_be_nil == true){
                    var_data->type = DOUBLE_NIL; // Double?
                } else {
                    var_data->type = DOUBLE;
                }
                break;
            default: // Invalid keyword
                return SYNTAX_ERR;
                break;
        }
    }
    return NO_ERR;
}

/**
 * @brief Starts the param name parsing.
 *
 * Rule:  <param_name> ➔ param_NAME
 * Rule:  <param_name> ➔ _                                          
 * 
 * @param func_data Structure that contains all the information about the currently being loaded function
 * @returns The correct error code, 0 when success
 */
int parse_param_name(TData_func *func_data, TData_var *var_data){
    int result; // Variable that holds the return value
    
    /* Choose the corresponding rule */
    if (parser.current_token.token_type == TOKEN_VAR_ID){
        // The parameter name is an actual unique ID
        RETURNCHECK(check_unique_name(func_data)) // Check if a parameter with this name doesn't exist already
        // Save the parameter name
        func_data->parameters[func_data->parameter_count - 1].name = parser.current_token.token_value.dyn_str.dynamic_str;
    } else if (parser.current_token.token_type == TOKEN_UNDER_L){
        // The parameter name is a '_' symbol
        func_data->parameters[func_data->parameter_count - 1].name = "_";
    } else  {
        // The parameter name is invalid
        return SYNTAX_ERR;
    }
    return NO_ERR;
}

/**
 * @brief Starts the parameters list parsing.
 *
 * Rule:  <params_list> ➔ <param_name> param_ID : <type> <params_list_n>
 * Rule:  <params_list> ➔ ε                                           
 * 
 * @param func_data Structure that contains all the information about the currently being loaded function
 * @returns The correct error code, 0 when success
 */
int parse_params_list(TData_func *func_data, TData_var *var_data){
    int result; // Variable that holds the return value

    /* Get the next token */
    TOKENCHECK(&parser.current_token)

    if (parser.current_token.token_type == TOKEN_R_PAR)
        return NO_ERR; // The parameters list is empty

    func_data->parameter_count++; // The parameters list is NOT empty (at-least 1 parameter)

    // Allocate memory for the currently being loaded parameter
    RETURNCHECK(func_param_init(func_data));

    /* Parse the parameter name */
    RETURNCHECK(parse_param_name(func_data, var_data));

    /* Get the next token */
    TOKENCHECK(&parser.current_token)

    // Check if the token is a variable ID and if the parameter ID and parameter name aren't identical
    if (parser.current_token.token_type != TOKEN_VAR_ID || strcmp(parser.current_token.token_value.dyn_str.dynamic_str, func_data->parameters[func_data->parameter_count - 1].name) == 0)
        return SEMANTIC_ERR_OTHER;

    // Save the parameter ID
    func_data->parameters[func_data->parameter_count - 1].id = parser.current_token.token_value.dyn_str.dynamic_str; 

    /* Get the next token */
    TOKENCHECK(&parser.current_token)

    if (parser.current_token.token_type != TOKEN_COLON)
        return SYNTAX_ERR;

    /* Get the next token */
    TOKENCHECK(&parser.current_token)

    /* Parse the type */
    parser.curr_type = 'p'; // We are loading the 'parameter' types
    RETURNCHECK(parse_type(func_data, var_data))

    /* Get the next token */
    TOKENCHECK(&parser.current_token)

    /* Parse the parameters list n */
    RETURNCHECK(parse_params_list_n(func_data, var_data));

    return NO_ERR;
}

/**
 * @brief Starts the function definition parsing.
 *
 * Rule:  <function_definition> -> func function_ID ( <params_list> ) <return_type> { <statement_list> }
 * 
 * @returns The correct error code, 0 when success
 */
int parse_function_definition(){
    int result; // Variable that holds the return value

    /* Get the next token */
    TOKENCHECK(&parser.current_token)

    if (parser.current_token.token_type != TOKEN_FUNC_ID)
        return SYNTAX_ERR;

    // Check if function with the same name isn't defined already
    if (search_symbol(parser.global_func_symbtable->root, parser.current_token.token_value.dyn_str.dynamic_str) != NULL)
        return SEMANTIC_ERR_A; // Function redefinition
    
    // Save the function ID for later
    char *func_ID = parser.current_token.token_value.dyn_str.dynamic_str;

    // Skip the execution of this function
    // Execute this function only when called
    printf("JUMP $_%s_end_\nLABEL $_%s_\n", func_ID, func_ID);
    printf("PUSHS nil@nil\n");

    /* Get the next token */
    TOKENCHECK(&parser.current_token)

    if (parser.current_token.token_type != TOKEN_L_PAR)
        return SYNTAX_ERR;

    // Create a helping structure that holds all the function information (such as parameters count, return type, ....)
    TData_var var_data;            // The empty variable data
    TData_func func_data;          // The function data
    func_data.parameter_count = 0; // The function has 0 parameters by default

    /* Parse the parameters list */
    RETURNCHECK(parse_params_list(&func_data, &var_data))

    /* Parse the return type */
    RETURNCHECK(parse_return_type(&func_data, &var_data))

    // Insert the function into the global function symtable
    if (insert_symbol(&parser.global_func_symbtable->root, func_ID, FUNCTION, var_data, func_data, parser.global_func_symbtable) == 0)
        return COMPILER_ERR_INTER;

    if (parser.current_token.token_type != TOKEN_L_BRAC)
        return SYNTAX_ERR;

    parser.return_detected = false;

    // Create a new empty local symtable and push it to the top of the variable symtable stack
    TTree *local_symtable = (TTree *) malloc(sizeof(TTree));
    init_symtable(local_symtable);
    for (int i = 0; i < func_data.parameter_count; i++){
        TData_var var_data_arg;           // Function argument data
        TData_func fnc_data_tmp;          // The function data
        var_data_arg.constant = true;
        var_data_arg.type = func_data.parameters[i].type;
        var_data_arg.init = false;
        var_data_arg.is_param = true;
        if(insert_symbol(&local_symtable->root, func_data.parameters[i].id, VARIABLE, var_data_arg, fnc_data_tmp, local_symtable) == 0)
            return COMPILER_ERR_INTER;
    }
    RETURNCHECK(st_stack_push(parser.var_st_stack, local_symtable))

    /* Get the next token */
    TOKENCHECK(&parser.current_token)

    parser.inside_main = false; // We're inside a function
    parser.current_rule = FUNC_DEF;
    parser.current_func_name = func_ID; // Save the function ID for later

    /* Parse the statement list */
    RETURNCHECK(parse_statement_list())

    if (parser.current_token.token_type != TOKEN_R_BRAC)
        return SYNTAX_ERR;

    if (parser.has_return == true && parser.return_detected == false)
        return SYNTAX_ERR; // The function has a return type but did not return anything

    st_stack_pop(parser.var_st_stack);

    /* Get the next token */
    TOKENCHECK(&parser.current_token)

    parser.inside_main = true;  // We're inside the main (outside the function)
    parser.current_rule = OTHER;  // We're inside the main (outside the function)
    parser.current_func_name = NULL;

    // Exit function
    printf("POPFRAME\n");
    printf("RETURN\n");
    // End of function
    printf("LABEL $_%s_end_\n", func_ID);

    return NO_ERR;
}

/**
 * @brief Starts the variable assignment parsing.
 *
 * Rule:  <var_assign> ➔ = expression
 * Rule:  <var_assign> ➔ ε
 * 
 * @returns The correct error code, 0 when success
 */
int parse_var_assign(TData_func *func_data, TData_var *var_data){
    int result; // Variable that holds the return value

    if (parser.current_token.token_type == TOKEN_ASSIGN){
        if(var_data->init == false){
            // Variable doesnt exist in current scope -> declare 
            printf("DEFVAR ");
            if(parser.inside_main == 0){
                printf("LF");
            }else{printf("GF");}
            printf("@__%s__\n", parser.var_name);
        }

    // Call the expression parser to handle the expression
        parser.EOL_skip = false;
        parser.var_data = var_data; // Save the variable data for later
        parser.call_new_token = true;
        parser.current_rule = VAR_DEF;
        parser.lvalue = parser.current_token;
        RETURNCHECK(expression_parse(&parser))

        parser.EOL_skip = true;
        parser.call_new_token = true;
        var_data->init = true;

        if (parser.current_token.token_type == TOKEN_EOL || parser.current_token.token_type == TOKEN_R_PAR){
            /* Get the next token */
            TOKENCHECK(&parser.current_token)
        }
        else if (parser.current_token.token_type == TOKEN_FUNC_ID){
            /* Parse the function call */
            RETURNCHECK(parse_function_call())
            parser.function_count++;
        }

        if(parser.in_while == true){
            // If in while, use global frame, else choose based on parser.inside_main
            printf("POPS GF@__%s__\n", parser.lvalue.token_value.dyn_str.dynamic_str);
            }else{
            if(parser.inside_main == 0){
                printf("POPS LF@__%s__\n", parser.lvalue.token_value.dyn_str.dynamic_str);
            }else if(parser.inside_main == 1){
                printf("POPS GF@__%s__\n", parser.lvalue.token_value.dyn_str.dynamic_str);
            }
        }
        return NO_ERR;
    } else {
        if(parser.var_name != NULL){
            if(parser.inside_main == false){
                printf("DEFVAR LF@__%s__\n", parser.var_name); // Define a new local variable
            } else{
                printf("DEFVAR GF@__%s__\n", parser.var_name); // Define a new global variable
            }
        }
        if (var_data->type == UNDEFINED_TYPE){
            return SYNTAX_ERR;
        } else {
            var_data->init = false;
            return NO_ERR;
        }
    }
}

/**
 * @brief Starts the variable type parsing.
 *
 * Rule:  <var_type> ➔ : <type>
 * Rule:  <var_type> ➔ ε
 * 
 * @returns The correct error code, 0 when success
 */
int parse_var_type(TData_func *func_data, TData_var *var_data){
    int result; // Variable that holds the return value

    if (parser.current_token.token_type == TOKEN_COLON){
        /* Get the next token */
        TOKENCHECK(&parser.current_token)

        /* Parse the type */
        parser.curr_type = 'v'; // Variable type
        RETURNCHECK(parse_type(func_data, var_data))

        /* Get the next token */
        TOKENCHECK(&parser.current_token)

        return NO_ERR;
    } else {
        var_data->type = UNDEFINED_TYPE;
        return NO_ERR;
    }
}

/**
 * @brief Starts the variable definition type parsing.
 *
 * Rule:  <var_def_type> ➔ let
 * Rule:  <var_def_type> ➔ var
 * 
 * @returns The correct error code, 0 when success
 */
int parse_var_def_type(TData_func *func_data, TData_var *var_data){
    int result; // Variable that holds the return value

    // Check if the type is 'let' or 'var'
    var_data->constant = parser.current_token.token_value.token_keyword == LET_KW ? true : false;

    return NO_ERR;
}

/**
 * @brief Starts the variable definition parsing.
 *
 * Rule:  <variable_definition> ➔ <var_def_type> var_ID <var_type> <var_assign>
 * 
 * @returns The correct error code, 0 when success
 */
int parse_variable_definition(){
    int result; // Variable that holds the return value

    // Create a helping structure that holds all the variable information (such as variable name, variable type, ....)
    TData_func func_data; // The empty function data
    TData_var var_data;   // The variable data

    /* Parse the variable definition type */
    RETURNCHECK(parse_var_def_type(&func_data, &var_data))

    /* Get the next token */
    TOKENCHECK(&parser.current_token)

    if (parser.current_token.token_type != TOKEN_VAR_ID)
        return SYNTAX_ERR;

    if (search_symbol(parser.var_st_stack->stack_head->symtable_item->root, parser.current_token.token_value.dyn_str.dynamic_str) != NULL)
        return SEMANTIC_ERR_A; // Variable with the same ID exists already
    
    // Save the variable ID for later
    parser.var_name = parser.current_token.token_value.dyn_str.dynamic_str;

    /* Get the next token */
    TOKENCHECK(&parser.current_token)

    /* Parse the variable type */
    RETURNCHECK(parse_var_type(&func_data, &var_data))

    /* Parse the variable assign */
    RETURNCHECK(parse_var_assign(&func_data, &var_data))

    if (insert_symbol(&parser.var_st_stack->stack_head->symtable_item->root, parser.var_name, VARIABLE, var_data, func_data, parser.var_st_stack->stack_head->symtable_item) == 0)
        return COMPILER_ERR_INTER;

    return NO_ERR;
}

/**
 * @brief Starts the assignment parsing.
 *
 * Rule:  <assignment> ➔ var_ID = expression
 * Rule:  <assignment> ➔ var_ID = <function_call>
 * 
 * @returns The correct error code, 0 when success
 */
int parse_assignment(){
    int result; // Variable that holds the return value

    if (parser.current_token.token_type != TOKEN_VAR_ID)
        return SYNTAX_ERR;

    TNode *searched_node = search_st_stack(parser.var_st_stack, parser.current_token.token_value.dyn_str.dynamic_str); 
    if (searched_node == NULL) // The variable is not defined
        return SEMANTIC_ERR_C;

    /* Get the next token */
    TOKENCHECK(&parser.current_token)

    if (parser.current_token.token_type != TOKEN_ASSIGN)
        return SYNTAX_ERR;

    if (searched_node->variable_data.constant == true && searched_node->variable_data.init == true)
        return SEMANTIC_ERR_OTHER; // Assignment to a constant


    /* Get the next token */
    TOKENCHECK(&parser.current_token)

    if (parser.current_token.token_type == TOKEN_FUNC_ID){ // Assignment of a function
        TNode *searched_func = search_symbol(parser.global_func_symbtable->root, parser.current_token.token_value.dyn_str.dynamic_str);
        if (searched_func == NULL) // The function is NOT defined
            return SEMANTIC_ERR_A;

        if (searched_func->type != searched_node->type)
            return SEMANTIC_ERR_E; // The variable and function types are different

        /* Parse the function call */
        RETURNCHECK(parse_function_call())
    } else { // Assignment of a variable
        // Call the expression parser to handle the expression
        parser.call_new_token = false;
        parser.EOL_skip = false;
        parser.current_rule = ASSIGNMENT;
        parser.var_data = &(searched_node->variable_data);
        RETURNCHECK(expression_parse(&parser))

        parser.EOL_skip = true;
        parser.call_new_token = true;

        // Retrieve value of an assignment
        printf("POPS ");
        if(parser.in_while == true){
            printf("GF@__%s__\n", searched_node->id);
        }else{
            if(parser.inside_main == 0){
                printf("LF@__%s__", searched_node->id);
            }else{
                printf("GF@__%s__", searched_node->id);}
            printf("\n");
        }
        if (parser.current_token.token_type == TOKEN_EOL || parser.current_token.token_type == TOKEN_R_PAR)
            /* Get the next token */
            TOKENCHECK(&parser.current_token)
    }
    return NO_ERR;
}

/**
 * @brief Starts the while statement parsing.
 *
 * Rule:  <while_statement> -> while expression { <statement_list> }
 * 
 * @returns The correct error code, 0 when success
 */
int parse_while_statement(){
    int result; // Variable that holds the return value

    // Call the expression parser to handle the expression
    parser.EOL_skip = false;
    parser.current_rule = WHILE_STMNT;
    RETURNCHECK(expression_parse(&parser))

    /**
     * Generate beginning of while statement 
     * Condition check
     * Set in_while bool to true
    */
    parser.in_while = true;
    printf("JUMP while_end%d\nLABEL while_true%d\n", parser.while_count, parser.while_count);
    parser.inside_main = false; // We're inside the while statement
    parser.EOL_skip = true;

    if (parser.current_token.token_type == TOKEN_EOL || parser.current_token.token_type == TOKEN_R_PAR)
        /* Get the next token */
        TOKENCHECK(&parser.current_token)

    if (parser.current_token.token_type != TOKEN_L_BRAC)
        return SYNTAX_ERR;

    // Create a new empty local symtable and push it to the top of the variable symtable stack
    TTree *local_symtable = (TTree *) malloc(sizeof(TTree));
    init_symtable(local_symtable);
    RETURNCHECK(st_stack_push(parser.var_st_stack, local_symtable))

    // Create a new local codegen frame
    printf("CREATEFRAME\n");
    printf("PUSHFRAME\n");

    /* Get the next token */
    TOKENCHECK(&parser.current_token)

    /* Parse the statement list */
    RETURNCHECK(parse_statement_list())

    // End of while cycle, go back to condition check
    printf("JUMP while_check%d\nLABEL while_end%d\n", parser.while_count, parser.while_count);

    if (parser.current_token.token_type != TOKEN_R_BRAC)
        return SYNTAX_ERR;  

    // Pop the local symtable from the variable symtable stack
    st_stack_pop(parser.var_st_stack);
    // Pop the local codegen frame
    printf("POPFRAME\n");

    parser.inside_main = true; // We're inside the main again

    /* Get the next token */
    TOKENCHECK(&parser.current_token)

    // Code generation
    // Left while statement -> increment counter, set bool to false
    parser.in_while = false;
    parser.while_count++;
    return NO_ERR;
}

/**
 * @brief Starts the if statement parsing.
 *
 * Rule:  <if_statement> ➔ if expression { statement_list } else { statement_list }
 * 
 * @returns The correct error code, 0 when success
 */
int parse_if_statement(){
    int result; // Variable that holds the return value

    // Call the expression parser to handle the expression
    parser.EOL_skip = false;
    parser.current_rule = IF_STMNT;
    RETURNCHECK(expression_parse(&parser))

    parser.inside_main = false; // We're inside the if statement
    parser.EOL_skip = true;
    
    if (parser.current_token.token_type == TOKEN_EOL || parser.current_token.token_type == TOKEN_R_PAR)
        /* Get the next token */
        TOKENCHECK(&parser.current_token)

    if (parser.current_token.token_type != TOKEN_L_BRAC)
        return SYNTAX_ERR;

    // Create a new empty local symtable and push it to the top of the variable symtable stack
    TTree *local_symtable = (TTree *) malloc(sizeof(TTree));
    init_symtable(local_symtable);
    RETURNCHECK(st_stack_push(parser.var_st_stack, local_symtable))

    // Create a new local codegen frame
    printf("CREATEFRAME\n");
    printf("PUSHFRAME\n");

    /* Get the next token */
    TOKENCHECK(&parser.current_token)

    /* Parse the statement list */
    RETURNCHECK(parse_statement_list())

    if (parser.current_token.token_type != TOKEN_R_BRAC)
        return SYNTAX_ERR;    

    // Pop the local symtable from the variable symtable stack
    st_stack_pop(parser.var_st_stack);
    // Pop the local codegen frame
    printf("POPFRAME\n");

    /* Get the next token */
    TOKENCHECK(&parser.current_token)

    if (parser.current_token.token_type != TOKEN_KEYWORD && parser.current_token.token_value.token_keyword != ELSE_KW)
        return SYNTAX_ERR; // Missing else  

    /* Get the next token */
    TOKENCHECK(&parser.current_token)

    if (parser.current_token.token_type != TOKEN_L_BRAC)
        return SYNTAX_ERR;    

    // End of statement list 1, skip statement list 2 - go to end
    // Beginning of statement list 2
    printf("JUMP end%d\nLABEL if_not_passed%d\n", parser.if_count, parser.if_count);
    
    // Create a new empty local symtable and push it to the top of the variable symtable stack
    TTree *local_symtable2 = (TTree *) malloc(sizeof(TTree));
    init_symtable(local_symtable2);
    RETURNCHECK(st_stack_push(parser.var_st_stack, local_symtable2))

    // Create a new local codegen frame
    printf("CREATEFRAME\n");
    printf("PUSHFRAME\n");

    /* Get the next token */
    TOKENCHECK(&parser.current_token)

    /* Parse the statement list */
    RETURNCHECK(parse_statement_list())

    if (parser.current_token.token_type != TOKEN_R_BRAC)
        return SYNTAX_ERR;    

    // Pop the local symtable from the variable symtable stack
    st_stack_pop(parser.var_st_stack);
    // Pop the local codegen frame
    printf("POPFRAME\n");

    parser.inside_main = true; // We're inside the main again

    /* Get the next token */
    TOKENCHECK(&parser.current_token)

    // End of if statement
    printf("LABEL end%d\n", parser.if_count); 
    parser.if_count++;

    return NO_ERR;
}


/**
 * @brief Starts the input parameter name parsing.
 *
 * Rule:  <input_param_name> ➔ ε
 * Rule:  <input_param_name> ➔ param_NAME :
 * 
 * @returns The correct error code, 0 when success
 */
int parse_input_param_name(TNode *searched_node, int *loaded_paramas_cnt, Arguments_Data_T *input_params_data){
    int result; // Variable that holds the return value

    
    // Save the function argument name
    input_params_data[*loaded_paramas_cnt].param_name = parser.current_token.token_value.dyn_str.dynamic_str;  

    // The parametr has an actual name in the function definition (not "_")
    if (strcmp(searched_node->function_data.parameters[*loaded_paramas_cnt].name, "_") != 0){
        if (strcmp(input_params_data[*loaded_paramas_cnt].param_name, searched_node->function_data.parameters[*loaded_paramas_cnt].name) != 0){
            return SEMANTIC_ERR_B; // Wrong argument name
        }
        /* Get the next token */
        TOKENCHECK(&parser.current_token)

        if (parser.current_token.token_type != TOKEN_COLON)
            return SEMANTIC_ERR_B;

        /* Get the next token */
        TOKENCHECK(&parser.current_token)

        // Save the argument value
        input_params_data[*loaded_paramas_cnt].term = parser.current_token; 

        return NO_ERR;
    }

    // The parametr has a "_" as name in the function definition
    input_params_data[*loaded_paramas_cnt].term = parser.current_token; // Save the argument value  

    return NO_ERR;
}

/**
 * @brief Starts the input parameters list n parsing.
 *
 * Rule:  <input_params_list_n> ➔ , <input_param_name> term <input_params_list_n>
 * Rule:  <input_params_list_n> ➔ ε
 * 
 * @returns The correct error code, 0 when success
 */
int parse_input_params_list_n(TNode *searched_node, int *loaded_paramas_cnt, Arguments_Data_T *input_params_data){
    int result; // Variable that holds the return value

    if (*loaded_paramas_cnt >= searched_node->function_data.parameter_count) // Wrong amount of function arguments
        return SEMANTIC_ERR_B;

    /* Parse the input parameter name */
    RETURNCHECK(parse_input_param_name(searched_node, loaded_paramas_cnt, input_params_data))

    // Check if the passed argument is a term or not
    RETURNCHECK(check_term(input_params_data, searched_node, loaded_paramas_cnt))

    /* Get the next token */
    TOKENCHECK(&parser.current_token)

    (*loaded_paramas_cnt)++;

    if (parser.current_token.token_type == TOKEN_R_PAR){
        // Send parameters to function via TF
        if(searched_node->function_data.parameter_count > 0){
            printf("CREATEFRAME\n");
            printf("PUSHFRAME\n");
            printf("CREATEFRAME\n");
            // Define paramaters
            for(int i = 0; i < *loaded_paramas_cnt; i++){
                printf("DEFVAR TF@_p%d_\n", i);
                // If parameter is a variable
                if(input_params_data[i].param_id != NULL){
                    if(parser.inside_main == 0){
                        printf("MOVE TF@_p%d_ LF@__%s__\n",i, input_params_data[i].param_id);
                    }else{printf("MOVE TF@_p%d_ GF@__%s__\n",i, input_params_data[i].param_id);}
                }
                // If param is a literal
                else if(input_params_data[i].term.token_type == TOKEN_INT){
                    printf("MOVE TF@_p%d_ int@%d\n", i, input_params_data[i].term.token_value.num_integer);
                }else if(input_params_data[i].term.token_type == TOKEN_FLOAT){
                    printf("MOVE TF@_p%d_ float@%a\n", i,  input_params_data[i].term.token_value.num_decimal);
                }else if(input_params_data[i].term.token_type == TOKEN_STR || input_params_data[i].term.token_type == TOKEN_M_LINE_STR){
                    printf("MOVE TF@_p%d_ string@", i);
                    print_string_in_ifjcode_form(input_params_data[i].term.token_value.dyn_str.dynamic_str);
                    printf("\n");
                }
            }
        }

        // Call function
        // Built-in function write is handled separatelly
        if(strcmp(parser.current_token.token_value.dyn_str.dynamic_str, "write") != 0){
            printf("CALL $_%s_\n", searched_node->id);
        }
        return NO_ERR;
    } else if (parser.current_token.token_type == TOKEN_COMMA){ // Loading more arguments
        /* Get the next token */
        TOKENCHECK(&parser.current_token)

        /* Parse the input parameters list n */
        RETURNCHECK(parse_input_params_list_n(searched_node, loaded_paramas_cnt, input_params_data))
    } else {
        return SEMANTIC_ERR_B;
    }
}

/**
 * @brief Starts the input parameters list parsing.
 *
 * Rule:  <input_params_list> ➔ <input_param_name> term <input_params_list_n>
 * 
 * @returns The correct error code, 0 when success
 */
int parse_input_params_list(TNode *searched_node, int *loaded_params_cnt, Arguments_Data_T *input_params_data){
    int result; // Variable that holds the return value
 
    if (*loaded_params_cnt >= searched_node->function_data.parameter_count) // Wrong amount of function arguments
        return SEMANTIC_ERR_B;

    /* Parse the input parameter name */
    RETURNCHECK(parse_input_param_name(searched_node, loaded_params_cnt, input_params_data))

    // Check if the passed argument is a term or not
    RETURNCHECK(check_term(input_params_data, searched_node, loaded_params_cnt))

    /* Get the next token */
    TOKENCHECK(&parser.current_token)

    (*loaded_params_cnt)++;
    
    if (parser.current_token.token_type == TOKEN_R_PAR){
        // Handle built in functions with parammeters
        if(strcmp(searched_node->id, "Int2Double") == 0){
            printf("CREATEFRAME\nDEFVAR TF@$_builtin_return_%d\nINT2FLOAT TF@$_builtin_return_%d int@%d\nPUSHS TF@$_builtin_return_%d\n", 
            parser.builtin_function_count, parser.builtin_function_count, input_params_data[0].term.token_value.num_integer, parser.builtin_function_count);
            parser.builtin_function_count++;
        }else if(strcmp(searched_node->id, "Double2Int") == 0){
            printf("CREATEFRAME\nDEFVAR TF@$_builtin_return_%d\nFLOAT2INT TF@$_builtin_return_%d float@%a\nPUSHS TF@$_builtin_return_%d\n", 
            parser.builtin_function_count, parser.builtin_function_count, input_params_data[0].term.token_value.num_decimal, parser.builtin_function_count);
            parser.builtin_function_count++;
        }else if(strcmp(searched_node->id, "length") == 0){
            printf("CREATEFRAME\nDEFVAR TF@$_builtin_return_%d\nSTRLEN TF@$_builtin_return_%d ", parser.builtin_function_count, parser.builtin_function_count);
            get_frame(input_params_data[0].term, &parser);
            printf("\nPUSHS TF@$_builtin_return_%d\n", parser.builtin_function_count);
            parser.builtin_function_count++;
        }else if(strcmp(searched_node->id, "ord") == 0){
            printf("CREATEFRAME\nDEFVAR TF@$_builtin_return_%d\nDEFVAR GF@*tmp*\n", parser.builtin_function_count);
            printf("STRLEN GF@*tmp* ");
            get_frame(input_params_data[0].term, &parser);
            printf("\nJUMPIFEQ ord_label%d GF@*tmp* int@0\nGETCHAR TF@$_builtin_return_%d ", parser.builtin_function_count, parser.builtin_function_count);
            get_frame(input_params_data[0].term, &parser);
            printf("int@0\nSTRI2INT TF@$_builtin_return_%d TF@$_builtin_return_%d int@0\nPUSHS TF@$_builtin_return_%d\nJUMP ord_label%d_end\nLABEL ord_label%d\nMOVE TF@$_builtin_return_%d int@0\nPUSHS TF@$_builtin_return_%d\nLABEL ord_label%d_end\n",
            parser.builtin_function_count, parser.builtin_function_count, parser.builtin_function_count, parser.builtin_function_count, parser.builtin_function_count, parser.builtin_function_count, parser.builtin_function_count, parser.builtin_function_count);
            parser.builtin_function_count++;
        }else if(strcmp(searched_node->id, "chr") == 0){
            printf("CREATEFRAME\nDEFVAR TF@$_builtin_return_%d\n", parser.builtin_function_count);
            printf("INT2CHAR TF@$_builtin_return_%d ", parser.builtin_function_count);
            get_frame(input_params_data[0].term, &parser);
            printf("\nPUSHS TF@$_builtin_return_%d\n", parser.builtin_function_count);
            parser.builtin_function_count++;
        }else {
        // Handle user defined functions
            // Send parameters to function via TF
            if(searched_node->function_data.parameter_count > 0){
                printf("CREATEFRAME\n");
                printf("PUSHFRAME\n");
                printf("CREATEFRAME\n");
                // Define paramaters
                for(int i = 0; i < *loaded_params_cnt; i++){
                    printf("DEFVAR TF@_p%d_\n", i);
                    // else handle variable as a parameter
                    if(input_params_data[i].param_id != NULL){
                        if(parser.inside_main == 0){
                            printf("MOVE TF@_p%d_ LF@__%s__\n",i, input_params_data[i].param_id);
                        }else{printf("MOVE TF@_p%d_ GF@__%s__\n",i, input_params_data[i].param_id);}
                    }
                    // handle literal as a parameter
                    else if(input_params_data[i].term.token_type == TOKEN_INT){
                        printf("MOVE TF@_p%d_ int@%d\n", i, input_params_data[i].term.token_value.num_integer);
                    }else if(input_params_data[i].term.token_type == TOKEN_FLOAT){
                        printf("MOVE TF@_p%d_ float@%a\n", i,  input_params_data[i].term.token_value.num_decimal);
                    }else if(input_params_data[i].term.token_type == TOKEN_STR || input_params_data[i].term.token_type == TOKEN_M_LINE_STR){
                        printf("MOVE TF@_p%d_ string@", i);
                        print_string_in_ifjcode_form(input_params_data[i].term.token_value.dyn_str.dynamic_str);
                        printf("\n");
                    }
                }
            }

            // Call non-builtin function
            printf("CALL $_%s_\n", searched_node->id);
        }
        return NO_ERR;
    } else if (parser.current_token.token_type == TOKEN_COMMA){ // Loading more arguments
        /* Get the next token */
        TOKENCHECK(&parser.current_token)

        /* Parse the input parameters list n */
        RETURNCHECK(parse_input_params_list_n(searched_node, loaded_params_cnt, input_params_data))
    } else {
        return SEMANTIC_ERR_B;
    }
}

/**
 * @brief Starts the function call parsing.
 *
 * Rule:  <function_call> ➔ function_ID ( <input_params_list> )
 * 
 * @returns The correct error code, 0 when success
 */
int parse_function_call(){
    int result; // Variable that holds the return value

    // Search for the function
    TNode *searched_node = search_symbol(parser.global_func_symbtable->root, parser.current_token.token_value.dyn_str.dynamic_str);
    if (searched_node == NULL)
        return SEMANTIC_ERR_A; // The function is NOT defined

    
    if(searched_node->function_data.parameter_count == 0){
        // Call function with no params
        // Functions with params are called in parse_input_params_list()
        // Builtin functions without parameters are handeled separatelly
        if(strcmp(searched_node->id, "readString") == 0){ // readString()
            printf("CREATEFRAME\nDEFVAR TF@$_builtin_return_%d\n", parser.builtin_function_count);
            printf("READ TF@$_builtin_return_%d string\nPUSHS TF@$_builtin_return_%d\n", parser.builtin_function_count, parser.builtin_function_count);
            parser.builtin_function_count++;
        }else if(strcmp(searched_node->id, "readInt") == 0){ // readInt()
            printf("CREATEFRAME\nDEFVAR TF@$_builtin_return_%d\n", parser.builtin_function_count);
            printf("READ TF@$_builtin_return_%d int\nPUSHS TF@$_builtin_return_%d\n", parser.builtin_function_count, parser.builtin_function_count);
            parser.builtin_function_count++;
        }else if(strcmp(searched_node->id, "readDouble") == 0){ // readDouble()
            printf("CREATEFRAME\nDEFVAR TF@$_builtin_return_%d\n", parser.builtin_function_count);
            printf("READ TF@$_builtin_return_%d float\nPUSHS TF@$_builtin_return_%d\n", parser.builtin_function_count, parser.builtin_function_count);
            parser.builtin_function_count++;
        }else if(strcmp(searched_node->id, "write") != 0){
            printf("CREATEFRAME\nPUSHFRAME\nCREATEFRAME\n");
            printf("CALL $_%s_\n", searched_node->id);
        }
    }

    /* Get the next token */
    TOKENCHECK(&parser.current_token)

    if (parser.current_token.token_type != TOKEN_L_PAR)
        return SYNTAX_ERR;    

    /* Get the next token */
    TOKENCHECK(&parser.current_token)

    if (parser.current_token.token_type == TOKEN_R_PAR){
        if (searched_node->function_data.parameter_count != 0) // Wrong amount of function arguments
            return SEMANTIC_ERR_B;
        /* Get the next token */
        TOKENCHECK(&parser.current_token)
        return NO_ERR;
    }

    // Special function to handle the pre-defined function "write"
    if (strcmp(searched_node->id, "write") == 0){
        RETURNCHECK(handle_write())
        return NO_ERR;
    }

    // Load the function arguments
    Arguments_Data_T *input_params_data;      
    input_params_data = (Arguments_Data_T *) malloc(sizeof(Arguments_Data_T) * searched_node->function_data.parameter_count);
    int loaded_params_cnt = 0;

    /* Parse the parameters list */
    RETURNCHECK(parse_input_params_list(searched_node, &loaded_params_cnt, input_params_data))

    if (loaded_params_cnt != searched_node->function_data.parameter_count) // Wrong amount of function arguments
        return SEMANTIC_ERR_B;

    /* Get the next token */
    TOKENCHECK(&parser.current_token)

    return NO_ERR;
}

/**
 * @brief Starts the return option parsing.
 *
 * Rule:  <return_option> ➔ expression
 * Rule:  <return_option> ➔ ε
 * 
 * @returns The correct error code, 0 when success
 */
int parse_return_option(TNode *searched_node){
    int result; // Variable that holds the return value

    /* Get the next token */
    TOKENCHECK(&parser.current_token)

    if (parser.current_token.token_type == TOKEN_R_BRAC){ // Return (void)
        if (searched_node->function_data.ret_type != VOID){ // Function doesn't return anything but should
            return SYNTAX_ERR; 
        }
    } else { // Return expression
        if (searched_node->function_data.ret_type == VOID){ // Void function should NOT return anything
            return SEMANTIC_ERR_D;
        }

        // Call the expression parser to handle the expression
        parser.call_new_token = false;
        parser.EOL_skip = false;
        parser.current_rule = RETURN;
        RETURNCHECK(expression_parse(&parser))

        parser.EOL_skip = true;

    if (parser.current_token.token_type == TOKEN_EOL || parser.current_token.token_type == TOKEN_R_PAR)
        /* Get the next token */
        TOKENCHECK(&parser.current_token)
    }

    parser.call_new_token = true;
    return NO_ERR;
}

/**
 * @brief Starts the function return parsing.
 *
 * Rule:  <function_return> ➔ return <return_option>
 * Rule:  <function_return> ➔ ε
 * 
 * @returns The correct error code, 0 when success
 */
int parse_function_return(){
    int result; // Variable that holds the return value

    if (parser.inside_main == true) // Return outside of a function
        return SYNTAX_ERR;

    // Search for the function
    TNode *searched_node = search_symbol(parser.global_func_symbtable->root, parser.current_func_name);
    if (searched_node == NULL) // The function is NOT defined
        return SEMANTIC_ERR_A; 

    if (parser.current_token.token_type == TOKEN_KEYWORD && parser.current_token.token_value.token_keyword == RETURN_KW){
        parser.return_detected = true; // We've detected the return
        /* Parse the return option */
        RETURNCHECK(parse_return_option(searched_node))
    } else if (parser.current_token.token_type == TOKEN_R_BRAC){
        return NO_ERR; // We've NOT detected the return
    } else {
        return SYNTAX_ERR;
    }

    return NO_ERR;
}

/**
 * @brief Starts the statement parsing.
 *
 * Rule:  <statement> -> <variable_definition>
 * Rule:  <statement> -> <assignment>
 * Rule:  <statement> -> <if_statement>
 * Rule:  <statement> -> <while_statement>
 * Rule:  <statement> -> <function_call>
 * Rule:  <statement> -> <function_return>
 * 
 * @returns The correct error code, 0 when success
 */
int parse_statement(){
    int result; // Variable that holds the return value

    /* Choose the corresponding rule */
    if (parser.current_token.token_type == TOKEN_KEYWORD && parser.current_token.token_value.token_keyword == LET_KW ||
        parser.current_token.token_type == TOKEN_KEYWORD && parser.current_token.token_value.token_keyword == VAR_KW){
        /* Parse the variable definition */
        RETURNCHECK(parse_variable_definition())
    } else if (parser.current_token.token_type == TOKEN_VAR_ID){ 
        /* Parse the assignment */
        RETURNCHECK(parse_assignment())
    } else if (parser.current_token.token_type == TOKEN_KEYWORD && parser.current_token.token_value.token_keyword == IF_KW){
        /* Parse the if statement */
        RETURNCHECK(parse_if_statement())
    } else if (parser.current_token.token_type == TOKEN_KEYWORD && parser.current_token.token_value.token_keyword == WHILE_KW){
        /* Parse the while statement */
        RETURNCHECK(parse_while_statement())
    } else if (parser.current_token.token_type == TOKEN_FUNC_ID) {
        /* Parse the function call */
        RETURNCHECK(parse_function_call()) 
    } else {
        /* Parse the function return */
        RETURNCHECK(parse_function_return()) 
    }

    return NO_ERR;
}

/**
 * @brief Starts the statement list parsing.
 *
 * Rule:  <statement_list> -> <statement> <statement_list>
 * Rule:  <statement_list> -> <function_definition> <statement_list>
 * Rule:  <statement_list> -> ε 
 * 
 * @returns The correct error code, 0 when success
 */
int parse_statement_list(){
    int result; // Variable that holds the return value

    /* Choose the corresponding rule */
    if (parser.current_token.token_type == TOKEN_KEYWORD && parser.current_token.token_value.token_keyword == FUNC_KW){
        /* Parse the function definition */
        RETURNCHECK(parse_function_definition())
    } else if (parser.current_token.token_type == TOKEN_EOF || parser.current_token.token_type == TOKEN_R_BRAC){
        /* Parse the empty main body */
        return NO_ERR; 
    } else {
        /* Parse the statement */
        RETURNCHECK(parse_statement())
    }

    /* Parse the main body */
    RETURNCHECK(parse_statement_list()) 
}

/**
 * @brief Starts the end of the file parsing.
 *
 * Rule:  <program_eof> -> EOF
 * 
 * @returns The correct error code, 0 when success
 */
int parse_program_eof(){
    if (parser.current_token.token_type != TOKEN_EOF) // The current token is NOT the end of the file
        return SYNTAX_ERR;

    // Clean all the allocated memory
    free(parser.global_func_symbtable);
    free(parser.global_var_symbtable);
    parser.global_func_symbtable = NULL;
    parser.global_var_symbtable = NULL;
    return NO_ERR;
}

/**
 * @brief Starts the program parsing.
 *
 * Rule: <program> -> <statement_list> <program_eof>
 * 
 * @returns The correct error code, 0 when success
 */
int parse_program(){
    int result; // Variable that holds the return value

    /* Parse the main body */
    RETURNCHECK(parse_statement_list()) 

    /* Parse the end of the file */
    RETURNCHECK(parse_program_eof())
    
    return NO_ERR;
}

/**
 * @brief Starts the main program parsing.
 *
 * @returns The correct error code, 0 when success
 */
int parse(){
    int result; // Variable that holds the return value
    
    /* Get the first token that isn't EOL */
    parser.EOL_skip = true; // Indicates that we're skipping the EOL tokens
    TOKENCHECK(&parser.current_token)

    /* Create 2 global symtables for functions and variables, push the pre-defined functions to the global function symtable */
    RETURNCHECK(create_global_symtables(&parser))

    parser.call_new_token = true; // Indicates that the expression parser should call for a new token
    parser.inside_main = true; // Indicates that we're inside the main (outside of a function body)

    // Set code generation values to 0 by default
    parser.if_count = 0;
    parser.while_count = 0;
    parser.function_count = 0;
    parser.param_count = 0;
    parser.builtin_function_count = 0;
    parser.in_while = 0;
    // Print IFJcode23 header
    printf(".IFJcode23\n");
    
    /* Parse the main program */
    return parse_program();
}
/* End of parser.c */
