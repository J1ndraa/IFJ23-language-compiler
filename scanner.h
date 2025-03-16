/* ****************************** scanner.h ********************************** */
/*  Author: Marek Čupr (xcuprm01)                                              */
/*  Subject: IFJ/IAL - Project                                                 */
/*  Date: 28. 10. 2023                                                         */
/*  Functionality: Header file for scanner.c                                   */
/* *************************************************************************** */

#ifndef SCANNER_H
#define SCANNER_H

#include "utils.h"    
#include "error.h"        
#include "dynamic_str.h"  
#include <stdio.h>        // FILE, fopen(), getc(), ...
#include <stdbool.h> 

/*
 * / ****************** Token_Type_T ******************* \  
 * / Enumeration that holds all the possible token types \
*/
typedef enum Token_Type{
    /* ********************************************************* */
    /*       TYPE       ||       SYMBOL      ||       INDEX      */
    /* ********************************************************* */
    TOKEN_DIV,          //         /         ||         0        //
    TOKEN_MUL,          //         *         ||         1        //
    TOKEN_PLUS,         //         +         ||         2        //
    TOKEN_MINUS,        //         -         ||         3        //
    TOKEN_ARROW,        //         ->        ||         4        //
    TOKEN_GREATER,      //         >         ||         5        //
    TOKEN_GREATER_EQL,  //         >=        ||         6        //
    TOKEN_LESS,         //         <         ||         7        //
    TOKEN_LESS_EQL,     //         <=        ||         8        //
    TOKEN_EXCL_MARK,    //         !         ||         9        //
    TOKEN_ASSIGN,       //         =         ||         10       //
    TOKEN_EQLS,         //         ==        ||         11       //
    TOKEN_NOT_EQLS,     //         !=        ||         12       //
    TOKEN_INT,          //        462        ||         13       //
    TOKEN_FLOAT,        //       11.25       ||         14       //
    TOKEN_STR,          //      "hello"      ||         15       //
    TOKEN_M_LINE_STR,   //   """he\nlo"""    ||         16       //
    TOKEN_L_PAR,        //         (         ||         17       //
    TOKEN_R_PAR,        //         )         ||         18       //
    TOKEN_L_BRAC,       //         {         ||         19       //
    TOKEN_R_BRAC,       //         }         ||         20       //
    TOKEN_COLON,        //         :         ||         21       //
    TOKEN_UNDER_L,      //         _         ||         22       //
    TOKEN_COMMA,        //         ,         ||         23       //
    TOKEN_NILL_CMP,     //         ??        ||         24       //
    TOKEN_FUNC_ID,      //      funcName     ||         25       //
    TOKEN_VAR_ID,       //      varName      ||         26       //
    TOKEN_KEYWORD,      //         if        ||         27       //
    TOKEN_EOL,          //         \n        ||         28       //
    TOKEN_EOF,          //         EOF       ||         29       //
} Token_Type_T;

/*
 * / ******************** Token_Value_T *********************** \  
 * / Structure that contains all the possible values of a token \
*/
typedef struct Token_Value{
    Keywords_T token_keyword; // Keyword value
    Dynamic_Str_T dyn_str;    // String value
    int num_integer;          // Integer value
    double num_decimal;       // Double value
} Token_Value_T;

/*
 * / *********************** Token_T *********************** \  
 * / Structure that contains all the information about token \
*/
typedef struct Token{
    Token_Type_T token_type;    // Type of a token
    Token_Value_T token_value;  // Value of a token
    bool can_be_nil;            // Indicates if a data type can be nil or not
    int block_comm_cnt;         // Counter for nested block comments
    bool negative_exp;          // Indicates if the float exponent is negative (-) or not (+)
} Token_T;

/*
 * / ****************** set_file() ******************* \  
 * / Function that sets up the input file to read from \
*/
void set_file(FILE *f);

/*
 * / ***************** get_token() ****************** \  
 * / Function that creates a new token for the parser \
*/
int get_token(Token_T *token);

#endif
/* End of scanner.h */