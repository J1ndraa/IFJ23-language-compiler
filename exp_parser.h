/* ***************************** exp_parser.h ******************************** */
/*  Author: Jindřich Halva (xhalva 05), Marek Čupr (xcuprm01)                  */
/*  Subject: IFJ/IAL - Project                                                 */
/*  Date: 8. 11. 2023                                                          */
/*  Functionality: Header file for exp_parser.c                                */
/* *************************************************************************** */

#ifndef EXP_PARSER
#define EXP_PARSER

#include "scanner.h"
#include "parser.h"
#include "code_gen.h"
#include "error.h"

#define PREC_TABLE_SIZE 16 // Size of the precedence table (column x row ~ 16 x 16)

#define TOKEN_OR_STACKCLEAN(token,stack) \
    result = skip_EOL(token);            \
    if (result != NO_ERR){               \
        stack_clean(stack);              \
        return result;}

/*
 * / ****************** Prec_Table_Symbol_T ****************** \  
 * / Enum that holds all the possible precedence table symbols \
*/
typedef enum Prec_Table_Symbol {
    /* ************************************************************* */
    /*        TYPE         ||       SYMBOL      ||       INDEX       */
    /* ************************************************************* */
    P_TABLE_EXCL_MARK,     //         !         ||         0         ||
    P_TABLE_MUL,           //         *         ||         1         ||
    P_TABLE_DIV,           //         /         ||         2         ||
    P_TABLE_PLUS,          //         +         ||         3         ||
    P_TABLE_MINUS,         //         −         ||         4         ||
    P_TABLE_EQLS,          //         ==        ||         5         ||
    P_TABLE_NOT_EQLS,      //         !=        ||         6         ||
    P_TABLE_LESS,          //         <         ||         7         ||
    P_TABLE_GREATER,       //         >         ||         8         ||
    P_TABLE_LESS_EQL,      //         <=        ||         9         ||
    P_TABLE_GREATER_EQL,   //         >=        ||         10        ||
    P_TABLE_NILL_CMP,      //         ??        ||         11        ||
    P_TABLE_L_PAR,         //         (         ||         12        ||
    P_TABLE_R_PAR,         //         )         ||         13        ||
    P_TABLE_ID,            //         ID        ||         14        ||
    P_TABLE_DOLLAR,        //         $         ||         15        ||
    //********************************************************************************\\
    //Symbols below are not in precedence table, but it is important to have them here\\
    //"Expression" that can be stored in a Stack...
    P_TABLE_NON_TERMINAL,  //         E         ||         16        ||
    //stop reducing has same function as "shift" in "Prec_Table_Action_T"
    //but its only fot searching in stack... where i should stop reducing
    P_TABLE_STOP_REDUCING, //    STOP_REDUCE    ||         17        ||
} Prec_Table_Symbol_T;

/*
 * / ****************** Prec_Table_Action_T ****************** \  
 * / Enum that holds all the possible precedence table actions \
*/
typedef enum Prec_Table_Action {
    /* *********************************** */
    /*   TYPE   ||   ACTION   ||   INDEX   */
    /* *********************************** */
    S,          //   Shift    ||     0     */ 
    R,          //   Reduce   ||     1     */
    Eq,         //   Equals   ||     2     */
    Er          //   Error    ||     3     */
} Prec_Table_Action_T;


/*
 * / ********************** Prec_rules_T ********************* \  
 * / Enum that holds all the possible rules of precedence table\
*/
typedef enum Prec_rules {
    /* ************************************************* */
    /*      NAME       ||     RULE      ||      INDEX    */
    /* ************************************************* */
    RULE_NOT_NIL,      //   E -> E!      ||      0       */
    RULE_MUL,          //   E -> E * E   ||      1       */
    RULE_DIV,          //   E -> E / E   ||      2       */
    RULE_PLUS,         //   E -> E + E   ||      3       */
    RULE_MINUS,        //   E -> E - E   ||      4       */
    RULE_EQ,           //   E -> E == E  ||      5       */
    RULE_NOT_EQ,       //   E -> E != E  ||      6       */
    RULE_LESS,         //   E -> E < E   ||      7       */
    RULE_GREATER,      //   E -> E > E   ||      8       */
    RULE_LESS_EQ,      //   E -> E <= E  ||      9       */
    RULE_GREATER_EQ,   //   E -> E >= E  ||      10      */
    RULE_NILL_CMP,     //   E -> E ?? E  ||      11      */
    RULE_PARS,         //   E -> (E)     ||      12      */
    RULE_ID,           //   E -> id      ||      13      */
    RULE_UNDEFINED     //                ||      14      */
}Prec_rules_T;


/*
 * / ****************** Token_to_Symbol() ******************* \  
 * / Function that converts token in to the Prec. table symbol\
*/
Prec_Table_Symbol_T Token_to_Symbol(Token_T *token);

/*
 * / ****************** which_data_type() ******************* \
 * /         Function that returns data_type of token         \
*/
enum Var_type which_data_type(Token_T *token, Parser_T *struct_parser);

/*
 * / ******************** expression_parse() ********************** \
 * / Main function of exp_parser.c, it will be called from parser.c \
*/
int expression_parse(Parser_T *struct_parser);


#endif
/* End of exp_parser.h */