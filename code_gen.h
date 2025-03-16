/* ****************************** code_gen.h ********************************* */
/*  Author: Vojtěch Hrabovský (xhrabo18)                                       */
/*  Subject: IFJ/IAL - Project                                                 */
/*  Date: 21. 11. 2023                                                         */
/*  Functionality: Header file for code_gen.c                                  */
/* *************************************************************************** */

#ifndef _CODE_GEN_H
#define _CODE_GEN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "symtable.h"
#include "scanner.h"
#include "exp_parser.h"
#include "dynamic_str.h"
#include "error.h"

int print_token_array(Token_T *token_array, int array_length, Parser_T *struct_parser, int type);
void get_frame(Token_T token, Parser_T *struct_parser);
void print_string_in_ifjcode_form(char *input);

#endif
