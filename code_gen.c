/* ****************************** code_gen.h ********************************* */
/*  Author: Vojtěch Hrabovský (xhrabo18)                                       */
/*  Subject: IFJ/IAL - Project                                                 */
/*  Date: 21. 11. 2023                                                         */
/*  Functionality: Code generation                                             */ 
/* *************************************************************************** */

#include "code_gen.h"

/**
 * 0 - OK
 * 1 - Error
*/

/**
 * Prints string in IFJcode form
 *
 * @param intput string to be printed
 * 
 */
void print_string_in_ifjcode_form(char *input) {
    size_t lenght = strlen(input);
    Dynamic_Str_T result;
    dynamic_str_init(&result);

    for (size_t i = 0; i < lenght; i++)
    {
        if (input[i] >= 0 && input[i] <= 32){
            
            int a = input[i]/10; //first digit
            int b = input[i]%10; //second digit

            append_char_to_str(&result,'\\');
            append_char_to_str(&result, '0');
            append_char_to_str(&result, (char)(a + '0'));
            append_char_to_str(&result, (char)(b + '0'));
        }else if (input[i] == '\\'){
            append_char_to_str(&result,'\\');
            append_char_to_str(&result, '0');
            append_char_to_str(&result, '9');
            append_char_to_str(&result, '2');
        }else if (input[i] == '#'){
            append_char_to_str(&result,'\\');
            append_char_to_str(&result, '0');
            append_char_to_str(&result, '3');
            append_char_to_str(&result, '5');
        }else{
            append_char_to_str(&result, input[i]);
        }   
    }
    printf("%s", result.dynamic_str);
}

// Prints literal value or variable with corresponding frame
void get_frame(Token_T token, Parser_T *struct_parser){
    if(token.token_type == TOKEN_VAR_ID){
        if(struct_parser->inside_main == true){
            printf("GF@__");}else{printf("LF@__");}
            printf("%s__", token.token_value.dyn_str.dynamic_str);
            printf(" ");}
    else if(token.token_type == TOKEN_INT){
            char num[50];
            sprintf(num, "%d", token.token_value.num_integer);
            printf("int@");
            printf("%s",num);
            printf(" ");
        }else if(token.token_type == TOKEN_FLOAT){
            char num[50];
            sprintf(num, "%a", token.token_value.num_decimal);
            printf("float@");
            printf("%s",num);
            printf(" ");
        }else if(token.token_type == TOKEN_STR || token.token_type == TOKEN_M_LINE_STR){
            printf("string@");
            print_string_in_ifjcode_form(token.token_value.dyn_str.dynamic_str);
            printf(" ");
        }
}

// Prints if and while statement or token array
int print_token_array(Token_T *token_array, int array_length, Parser_T *struct_parser, int type){
    int i = 0;
    int allowed = 1;
    char if_counter[5];
    sprintf(if_counter, "%d", struct_parser->if_count);
    char while_counter[5];
    sprintf(while_counter, "%d", struct_parser->while_count);
    while(i < array_length){
        /**
         * TYPE 1 = IF
         * TYPE 2 = WHILE
        */
        if(type == 1){
            /**
             * For simple == and != expressions, only one conditional jump is needed
             * For more complicated expressions, values have to be compared
             * 
            */
            if(token_array[i].token_type == TOKEN_EQLS){
                allowed = 0;
                printf("JUMPIFEQ if_passed%s ", if_counter);
                get_frame(token_array[1], struct_parser);
                get_frame(token_array[2], struct_parser);
            }else if(token_array[i].token_type == TOKEN_NOT_EQLS){
                allowed = 0;
                printf("JUMPIFNEQ if_passed%s ", if_counter);
                get_frame(token_array[1], struct_parser);
                get_frame(token_array[2], struct_parser);
            }else if(token_array[i].token_type == TOKEN_GREATER){
                allowed = 0;
                printf("DEFVAR GF@___IF_RESULT___%s \n", if_counter);
                printf("GT GF@___IF_RESULT___%s ", if_counter);
                get_frame(token_array[1], struct_parser);
                get_frame(token_array[2], struct_parser);
                printf("\nJUMPIFNEQ if_passed%s GF@___IF_RESULT___%s bool@false", if_counter, if_counter);     
            }else if(token_array[i].token_type == TOKEN_LESS){
                allowed = 0;
                printf("DEFVAR GF@___IF_RESULT___%s \n", if_counter);
                printf("LT GF@___IF_RESULT___%s ", if_counter);
                get_frame(token_array[1], struct_parser);
                get_frame(token_array[2], struct_parser);
                printf("\nJUMPIFNEQ if_passed%s GF@___IF_RESULT___%s bool@false", if_counter, if_counter);
            }else if(token_array[i].token_type == TOKEN_GREATER_EQL){
                allowed = 0;
                printf("DEFVAR GF@___GT_RESULT___%s \n", if_counter);
                printf("DEFVAR GF@___EQ_RESULT___%s \n", if_counter);
                printf("DEFVAR GF@___IF_RESULT___%s \n", if_counter);
                printf("GT GF@___GT_RESULT___%s ", if_counter);
                get_frame(token_array[1], struct_parser);
                get_frame(token_array[2], struct_parser);
                printf("\n");
                printf("EQ GF@___EQ_RESULT___%s ", if_counter);
                get_frame(token_array[1], struct_parser);
                get_frame(token_array[2], struct_parser);
                printf("\n");
                printf("OR GF@___IF_RESULT___%s GF@___EQ_RESULT___%s GF@___GT_RESULT___%s", if_counter, if_counter, if_counter);
                printf("\nJUMPIFNEQ if_passed%s GF@___IF_RESULT___%s bool@false", if_counter, if_counter);
            }else if(token_array[i].token_type == TOKEN_LESS_EQL){
                allowed = 0;
                printf("DEFVAR GF@___LT_RESULT___%s\n", if_counter);
                printf("DEFVAR GF@___EQ_RESULT___%s \n", if_counter);
                printf("DEFVAR GF@___IF_RESULT___%s \n", if_counter);
                printf("LT GF@___LT_RESULT___%s ", if_counter);
                get_frame(token_array[1], struct_parser);
                get_frame(token_array[2], struct_parser);
                printf("\n");
                printf("EQ GF@___EQ_RESULT___%s ", if_counter);
                get_frame(token_array[1], struct_parser);
                get_frame(token_array[2], struct_parser);
                printf("\n");
                printf("OR GF@___IF_RESULT___%s GF@___EQ_RESULT___%s GF@___LT_RESULT___%s", if_counter, if_counter, if_counter);
                printf("\nJUMPIFNEQ if_passed%s GF@___IF_RESULT___%s bool@false", if_counter, if_counter);
            }else if(token_array[i].token_type == TOKEN_VAR_ID && allowed == 1){
                get_frame(token_array[i], struct_parser);
            }
        }
        else if(type == 2){
            /**
             * For simple == and != expressions, only one conditional jump is needed
             * For more complicated expressions, values have to be compared
             * 
            */
            if(token_array[i].token_type == TOKEN_EQLS){
                allowed = 0;
                printf("LABEL while_check%s\n", while_counter);
                printf("JUMPIFEQ while_true%s ", while_counter);
                get_frame(token_array[1], struct_parser);
                get_frame(token_array[2], struct_parser);
            }else if(token_array[i].token_type == TOKEN_NOT_EQLS){
                allowed = 0;
                printf("LABEL while_check%s\n", while_counter);
                printf("JUMPIFNEQ while_true%s ", while_counter);
                get_frame(token_array[1], struct_parser);
                get_frame(token_array[2], struct_parser);
            }else if(token_array[i].token_type == TOKEN_GREATER){
                allowed = 0;
                printf("DEFVAR GF@___WHILE_RESULT___%s \n", while_counter);
                printf("LABEL while_check%s\n", while_counter);
                printf("GT GF@___WHILE_RESULT___%s ", while_counter);
                get_frame(token_array[1], struct_parser);
                get_frame(token_array[2], struct_parser);
                printf("\nJUMPIFNEQ while_true%s GF@___WHILE_RESULT___%s bool@false\n", while_counter, while_counter);     
            }else if(token_array[i].token_type == TOKEN_LESS){
                allowed = 0;
                printf("DEFVAR GF@___WHILE_RESULT___%s \n", while_counter);
                printf("LABEL while_check%s\n", while_counter);
                printf("LT GF@___WHILE_RESULT___%s ", while_counter);
                get_frame(token_array[1], struct_parser);
                get_frame(token_array[2], struct_parser);
                printf("\nJUMPIFNEQ while_true%s GF@___WHILE_RESULT___%s bool@false\n", while_counter, while_counter);
            }else if(token_array[i].token_type == TOKEN_GREATER_EQL){
                allowed = 0;
                printf("DEFVAR GF@___WHILE_GT_RESULT___%s \n", while_counter);
                printf("DEFVAR GF@___WHILE_EQ_RESULT___%s \n", while_counter);
                printf("DEFVAR GF@___WHILE_RESULT___%s \n", while_counter);
                printf("LABEL while_check%s\n", while_counter);
                printf("GT GF@___WHILE_GT_RESULT___%s ", while_counter);
                get_frame(token_array[1], struct_parser);
                get_frame(token_array[2], struct_parser);
                printf("\n");
                printf("EQ GF@___WHILE_EQ_RESULT___%s ", while_counter);
                get_frame(token_array[1], struct_parser);
                get_frame(token_array[2], struct_parser);
                printf("\n");
                printf("OR GF@___WHILE_RESULT___%s GF@___WHILE_EQ_RESULT___%s GF@___WHILE_GT_RESULT___%s", while_counter, while_counter, while_counter);
                printf("\nJUMPIFNEQ while_true%s GF@___WHILE_RESULT___%s bool@false\n", while_counter, while_counter);
            }else if(token_array[i].token_type == TOKEN_LESS_EQL){
                allowed = 0;
                printf("DEFVAR GF@___WHILE_LT_RESULT___%s\n", while_counter);
                printf("DEFVAR GF@___WHILE_EQ_RESULT___%s \n", while_counter);
                printf("DEFVAR GF@___WHILE_RESULT___%s \n", while_counter);
                printf("LABEL while_check%s\n", while_counter);
                printf("LT GF@___WHILE_LT_RESULT___%s ", while_counter);
                get_frame(token_array[1], struct_parser);
                get_frame(token_array[2], struct_parser);
                printf("\n");
                printf("EQ GF@___WHILE_EQ_RESULT___%s ", while_counter);
                get_frame(token_array[1], struct_parser);
                get_frame(token_array[2], struct_parser);
                printf("\n");
                printf("OR GF@___WHILE_RESULT___%s GF@___WHILE_EQ_RESULT___%s GF@___WHILE_LT_RESULT___%s", while_counter, while_counter, while_counter);
                printf("\nJUMPIFNEQ while_true%s GF@___WHILE_RESULT___%s bool@false\n", while_counter, while_counter);
            }else if(token_array[i].token_type == TOKEN_VAR_ID && allowed == 1){
                get_frame(token_array[i], struct_parser);
            }
        }
        i++;
    }
    if(type == 1){printf("\nJUMP if_not_passed%s\nLABEL if_passed%s\n", if_counter, if_counter);}
    return 0;
}