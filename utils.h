/* ********************************* utils.h ********************************* */
/*  Author: Marek Čupr (xcuprm01)                                              */
/*  Subject: IFJ/IAL - Project                                                 */
/*  Date: 13. 10. 2023                                                         */
/*  Functionality: Header file for utils.c                                     */
/* *************************************************************************** */

#ifndef UTILS_H
#define UTILS_H

/*
 * / ***************************** FSM_States_T ******************************* \  
 * / Enumeration that holds all the possible states of the Finite-state machine \
*/ 
typedef enum FSM_States {
    /* ***************************************************************************************** */
    /*      STATE       ||       SYMBOL      ||      FINAL STATE       ||          INDEX         */
    /* ***************************************************************************************** */
    FSM_START,          //                   ||                        ||            0           //
    FSM_PLUS,           //         +         ||          true          ||            1           //
    FSM_MINUS,          //         -         ||          true          ||            2           //
    FSM_ARROW,          //         ->        ||          true          ||            3           //
    FSM_MUL,            //         *         ||          true          ||            4           //
    FSM_DIV,            //         /         ||          true          ||            5           //
    FSM_BLOCK_COMM_S,   //         /*        ||          false         ||            6           //
    FSM_BLOCK_COMM_CL,  //        /**        ||          false         ||            7           //
    FSM_BLOCK_COMMENT,  //        /**/       ||          true          ||            8           //
    FSM_LINE_COMM_S,    //         //        ||          false         ||            9           //
    FSM_LINE_COMMENT,   //  // + {EOL/EOF}   ||          true          ||            10          //
    FSM_GREATER,        //         >         ||          true          ||            11          //
    FSM_GREATER_EQL,    //         >=        ||          true          ||            12          //
    FSM_LESS,           //         <         ||          true          ||            13          //
    FSM_LESS_EQL,       //         <=        ||          true          ||            14          //
    FSM_ASSIGN,         //         =         ||          true          ||            15          //
    FSM_EQLS,           //         ==        ||          true          ||            16          //
    FSM_EXCL_MARK,      //         !         ||          true          ||            17          //
    FSM_NOT_EQLS,       //         !=        ||          true          ||            18          //
    FSM_L_PAR,          //         (         ||          true          ||            19          //
    FSM_R_PAR,          //         )         ||          true          ||            20          //
    FSM_L_BRAC,         //         {         ||          true          ||            21          //
    FSM_R_BRAC,         //         }         ||          true          ||            22          //
    FSM_COLON,          //         :         ||          true          ||            23          //
    FSM_UNDER_L,        //         _         ||          true          ||            24          //
    FSM_COMMA,          //         ,         ||          true          ||            25          //
    FSM_QUESTION_MARK,  //         ?         ||          false         ||            26          //
    FSM_NILL_CMP,       //         ??        ||          true          ||            27          //
    FSM_INT,            //         15        ||          true          ||            28          //
    FSM_FLOAT_S,        //         15.       ||          false         ||            29          //
    FSM_FLOAT,          //        15.2       ||          true          ||            30          //
    FSM_EXP_S,          //        15.2e      ||          false         ||            31          //
    FSM_EXP_SIGN,       //       15.2e+      ||          false         ||            32          //
    FSM_EXP,            //      15.2e+4      ||          true          ||            33          //
    FSM_STRING_S,       //         "         ||          false         ||            34          //
    FSM_ESC_SEQ,        //         "/        ||          false         ||            35          //
    FSM_HEXA_SEQ_S,     //        "/u        ||          false         ||            36          //
    FSM_HEXA_SEQ,       //       "/u{A       ||          false         ||            37          //
    FSM_STRING,         //      "/u{A}"      ||          true          ||            38          //
    FSM_M_LINE_STR_S,   //     """Ah\noj     ||          false         ||            39          //
    FSM_M_LINE_STR,     //    """Ah\noj"""   ||          true          ||            40          //
    FSM_ID,             //       Var123_     ||          true          ||            41          //
    FSM_EOF,            //        EOF        ||          true          ||            42          //
    FSM_EOL,            //        EOL        ||          true          ||            43          //
    FSM_ERROR,          //        1.2e       ||          true          ||            44          //
} FSM_States_T;

/*
 * / ****************** Keywords_T ****************** \  
 * / Enumeration that holds all the possible keywords \
*/
typedef enum Keywords{
    /* CONDITIONS */ 
    WHILE_KW,
    IF_KW,
    ELSE_KW,
    /* FUNCTIONS */ 
    FUNC_KW,
    RETURN_KW,
    /* DATA TYPES */ 
    INT_KW,
    DOUBLE_KW,
    STRING_KW,
    /* VARIABLE TYPES */ 
    LET_KW,
    VAR_KW,
    /* SPECIAL */
    NIL_KW,
} Keywords_T;

/*
 * / *************** keyword_cmpr() ***************** \  
 * / Function that checks if the string is a keyword  \
*/
int keyword_cmpr(char *str_token);

/*
 * / ********************** hex_to_dec() *********************** \  
 * / Function that converts hexadecimal digits to decimal values \
*/
int hex_to_dec(char digit);

#endif
/* End of utils.h */