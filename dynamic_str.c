/* ******************************** dynamic_str.c ********************************* */
/*  Author: Marek Čupr (xcuprm01)                                                   */
/*  Subject: IFJ/IAL - Project                                                      */
/*  Date: 20. 10. 2023                                                              */
/*  Functionality: Creates a dynamic string that stores the characters from stdin   */
/* ******************************************************************************** */

#include "dynamic_str.h"   // header file
#include <stdlib.h>        // calloc(), realloc(), free()

/**
 * Initializes a new dynamic string.
 *
 * @param str Dynamic string to be initialized 
 * @returns 0 ~ DYNAMIC_STR_OK   — No errors have occured
 *          1 ~ DYNAMIC_STR_ERR  — An error has occured
 */
int dynamic_str_init(Dynamic_Str_T *str){
    if (str == NULL) // The pointer to the dynamic string is invalid
        return DYNAMIC_STR_ERR;

    // Dynamically allocate memory for the string 
    str->dynamic_str = (char *) calloc(DEFAULT_STR_SIZE, sizeof(char)); // Allocate 8 bytes by default
    if (str->dynamic_str == NULL) // Malloc failed
        return DYNAMIC_STR_ERR;

    // Set the correct size values
    str->str_len = 0; // The string is empty by default
    str->bytes_alloc = DEFAULT_STR_SIZE; // The string has 8 bytes allocated by default
    return DYNAMIC_STR_OK;
}

/**
 * Appends a new character to the end of the string.
 *
 * @param str Dynamic tring to be appended to 
 * @param c The character to append to the end of the string 
 * @returns 0 ~ DYNAMIC_STR_OK   — No errors have occured
 *          1 ~ DYNAMIC_STR_ERR  — An error has occured
 */
int append_char_to_str(Dynamic_Str_T *str, char c){
    if (str == NULL || str->dynamic_str == NULL) // The dynamic string pointer is invalid
        return DYNAMIC_STR_ERR;

    // Allocate more memory if needed
    if (str->str_len + 1 == str->bytes_alloc){
        str->bytes_alloc++; // Add 1 more byte to the string

        // Dynamically allocate more memory for the string 
        str->dynamic_str = (char *) realloc(str->dynamic_str, str->bytes_alloc * sizeof(char));
        if (str->dynamic_str == NULL) // Realloc failed
            return DYNAMIC_STR_ERR;
    }

    // Append the character and set the correct size values
    str->dynamic_str[str->str_len] = c; // Append the character to the end of the string
    str->str_len++; // Increase the string length
    str->dynamic_str[str->str_len] = '\0'; // Add '\0' to the end of the string

    return DYNAMIC_STR_OK;
}

/**
 * Cleans all the dynamically allocated memory for the dynamic string. 
 *
 * @param str Dynamic string to be cleaned
 */
void dynamic_str_clean(Dynamic_Str_T *str){
    free(str->dynamic_str);
    str->dynamic_str = NULL;
    str->bytes_alloc = 0;
    str->str_len = 0;
}
/* End of dynamic_str.c */