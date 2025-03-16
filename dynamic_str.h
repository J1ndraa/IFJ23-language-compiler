/* ****************************** dynamic_str.h ****************************** */
/*  Author: Marek Čupr (xcuprm01)                                              */
/*  Subject: IFJ/IAL - Project                                                 */
/*  Date: 20. 10. 2023                                                         */
/*  Functionality: Header file for dynamic_string.c                            */
/* *************************************************************************** */

#ifndef DYNAMIC_STR_H
#define DYNAMIC_STR_H

/* Default size for calloc in bytes */
#define DEFAULT_STR_SIZE 8  // Prevents issues with allocating memory chunks too small

/* Return values */
#define DYNAMIC_STR_OK 0    // No errors have occured
#define DYNAMIC_STR_ERR 1   // An error has occured

/*
 * / ****************************** Dynamic_Str_T *********************************** \  
 * / Structure that contains information about currently loaded characters from stdin \
*/
typedef struct Dynamic_Str{
    char *dynamic_str;           // String for storing the characters from stdin
    unsigned int str_len;        // Length of the currently loaded string
    unsigned int bytes_alloc;    // The size of a memory allocated for the string in bytes
} Dynamic_Str_T;

/*
 * / ************ dynamic_str_init() ************** \  
 * / Function that initializes a new dynamic string \
*/
int dynamic_str_init(Dynamic_Str_T *str);

/*
 * / ***************** append_char_to_str() ******************** \  
 * / Function that appends new character to the end of the string \
*/
int append_char_to_str(Dynamic_Str_T *str, char c);

/*
 * / ***************************** dynamic_str_clean() ********************************* \  
 * / Function that cleans all the dynamically allocated resources for the dynamic string \
*/
void dynamic_str_clean(Dynamic_Str_T *str);

#endif
/* End of dynamic_string.h */
