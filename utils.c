/* ******************************* utils.c *********************************** */
/*  Author: Marek Čupr (xcuprm01)                                              */
/*  Subject: IFJ/IAL - Project                                                 */
/*  Date: 19. 10. 2023                                                         */
/*  Functionality: Contains helping functions and utilities for the scanner    */
/* *************************************************************************** */

#include "utils.h"    // header file
#include <string.h>   // strcmp()
#include <ctype.h>    // isdigit()

/**
 * Checks if string is a keyword or not.
 *
 * @param string_token String to be compared with the keywords
 * @returns Index of the matched keyword, -1 if the string is not a keyword
 */
int keyword_cmpr(char *str_token){
    if (!strcmp(str_token, "while")){
        return 0;
    } else if (!strcmp(str_token, "if")){
        return 1;
    } else if (!strcmp(str_token, "else")){
        return 2;
    } else if (!strcmp(str_token, "func")){
        return 3;
    } else if (!strcmp(str_token, "return")){
        return 4;
    } else if (!strcmp(str_token, "Int")){
        return 5;
    } else if (!strcmp(str_token, "Double")){
        return 6;
    } else if (!strcmp(str_token, "String")){
        return 7;
    } else if (!strcmp(str_token, "let")){
        return 8;
    } else if (!strcmp(str_token, "var")){
        return 9;
    } else if (!strcmp(str_token, "nil")){
        return 10;
    } else {
        return -1; // The string is not a keyword
    }
}
    
/**
 * Converts hexadecimal digits to decimal values.
 *
 * @param digit Hexadecimal digits to be converted
 * @returns Decimal value of digit, -1 if digit is not a hexadecimal digit
 */
int hex_to_dec(char digit){
    int value;
    if (isdigit(digit)){
        value = digit - '0';
    } else if (digit >= 'A' && digit <= 'Z'){
        value = 10 + (digit - 'A');
    } else if (digit >= 'a' && digit <= 'z'){
        value = 10 + (digit - 'a');
    }
    else{
        return -1; // The digit is not a hexadecimal value
    }
}
/* End of utils.c */