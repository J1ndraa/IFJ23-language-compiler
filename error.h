/* ********************************* error.h ********************************* */
/*  Author: Marek Čupr (xcuprm01)                                              */
/*  Subject: IFJ/IAL - Project                                                 */
/*  Date: 13. 10. 2023                                                         */
/*  Functionality: Header file for error.c                                     */
/* *************************************************************************** */

#ifndef ERR_H
#define ERR_H

/*
 * / ******************************** Errors_T ********************************* \  
 * / Enumeration that holds all the possible error types and their return values \
*/ 
typedef enum {                /* ERROR TYPE               | ERROR DESCRIPTION                                                                      | RETURN VALUE */
    NO_ERR,                   /* No Error                 | Everything is fine                                                                     | return 0     */
    LEX_ERR,                  /* Lexical Analysis         | Incorrect structure of the current lexem                                               | return 1     */
    SYNTAX_ERR,               /* Syntactic Analysis       | Invalid program syntax, missing header file, etc..                                     | return 2     */
    SEMANTIC_ERR_A,           /* Semantic Analysis        | Undefined function, variable redefinition                                              | return 3     */
    SEMANTIC_ERR_B,           /* Semantic Analysis        | Incorrect parameter count/type, wrong return type                                      | return 4     */
    SEMANTIC_ERR_C,           /* Semantic Analysis        | Usage of undefined/uninitialized variable                                              | return 5     */
    SEMANTIC_ERR_D,           /* Semantic Analysis        | Missing/extra expression at the function return command                                | return 6     */
    SEMANTIC_ERR_E,           /* Semantic Analysis        | Type incompability at aritmetic/string/relational expressions                          | return 7     */
    SEMANTIC_ERR_F,           /* Semantic Analysis        | Variable/parameter type isn't mentioned and can't be derived from the used expression  | return 8     */
    SEMANTIC_ERR_OTHER,       /* Semantic Analysis        | Other semantic error                                                                   | return 9     */
    COMPILER_ERR_INTER = 99,  /* Internal compiler error  | Failed at dynamical memory alocation, etc...                                           | return 99    */
} Errors_T;                 

/*
 * / ***************** get_err_type() ******************** \  
 * / Function that prints the correct error code on stderr \
*/
int get_err_type(Errors_T err);

#endif
/* End of error.h */