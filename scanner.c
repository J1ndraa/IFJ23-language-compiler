/* ******************************** scanner.c ******************************** */
/*  Authors: Marek Čupr (xcuprm01)                                             */
/*  Subject: IFJ/IAL - Project                                                 */
/*  Date: 19. 10. 2023                                                         */
/*  Functionality: Create tokens and send them to the parser when needed       */
/* *************************************************************************** */

#include "scanner.h"   // header file
#include <stdlib.h>    // strtoul(), strtod(), ...
#include <ctype.h>     // isdigit(), isalpha(), isspace(), ...

// Input file source
FILE *file_src;

/**
 * Sets the source file to read from.
 *
 * @param f Source of the input file 
 */
void set_file(FILE *f){
    file_src = f;
}

/**
 * Creates tokens for the parser.
 *
 * @param token Token to be created
 * @returns The correct error return code
 */
int get_token(Token_T *token){
    int c; // Character to read from stdin
    FSM_States_T current_state = FSM_START; // Default state
    token->block_comm_cnt = 0; // Default nested block comments count

    // Initialize a new dynamic string
    Dynamic_Str_T dyn_str;
    if (dynamic_str_init(&dyn_str)) // The dynamic string initialization failed
        return COMPILER_ERR_INTER;

    while(1){
        c = getc(file_src); // Load a new character from stdin
        switch(current_state){
            case FSM_START: // Default state
                if (c == '+'){
                    current_state = FSM_PLUS;
                } else if (c == '-'){
                    current_state = FSM_MINUS;
                } else if (c == '*'){
                    current_state = FSM_MUL;
                } else if (c == '/'){
                    current_state = FSM_DIV;
                } else if (c == '>'){
                    current_state = FSM_GREATER;
                } else if (c == '<'){
                    current_state = FSM_LESS;
                } else if (c == '='){
                    current_state = FSM_ASSIGN;
                } else if (c == '('){
                    current_state = FSM_L_PAR;
                } else if (c == ')'){
                    current_state = FSM_R_PAR;
                } else if (c == '{'){
                    current_state = FSM_L_BRAC;
                } else if (c == '}'){
                    current_state = FSM_R_BRAC;
                } else if (c == ':'){
                    current_state = FSM_COLON;
                } else if (c == ','){
                    current_state = FSM_COMMA;
                } else if (c == '?'){
                    current_state = FSM_QUESTION_MARK;
                } else if (c == '!'){
                    current_state = FSM_EXCL_MARK;
                } else if (c == '_'){ // Underline or ID
                    append_char_to_str(&dyn_str, c);
                    current_state = FSM_UNDER_L;
                } else if (isalpha(c)) { // ID
                    append_char_to_str(&dyn_str, c);
                    current_state = FSM_ID;
                } else if (isdigit(c)) { // Int or Float
                    append_char_to_str(&dyn_str, c);
                    current_state = FSM_INT;
                } else if (c == '"'){ // String or Multiline string
                    current_state = FSM_STRING_S;
                } else if (c == '\n'){
                    current_state = FSM_EOL;
                }else if (isspace(c) || c == '\t'){ // Skip white-spaces that are not needed
                    current_state = FSM_START;
                } else if (c == EOF) {
                    current_state = FSM_EOF;
                    ungetc((char) c,file_src);
                } else { // UNEXPECTED INPUT
                    current_state = FSM_ERROR;
                }
                break;

            case FSM_PLUS: // Final state
                token->token_type = TOKEN_PLUS; //  +
                ungetc((char) c,file_src);
                dynamic_str_clean(&dyn_str);
                return NO_ERR;

            case FSM_MINUS: // Final state
                if (c == '>') { //  ->
                    current_state = FSM_ARROW;
                    break;                    
                } else { //  -
                    token->token_type = TOKEN_MINUS;
                    ungetc((char) c,file_src);
                    dynamic_str_clean(&dyn_str);
                    return NO_ERR;
                }

            case FSM_ARROW: // Final state
                token->token_type = TOKEN_ARROW; //  ->
                ungetc((char) c,file_src);
                dynamic_str_clean(&dyn_str);
                return NO_ERR;

            case FSM_MUL: // Final state
                token->token_type = TOKEN_MUL; //  *
                ungetc((char) c,file_src);
                dynamic_str_clean(&dyn_str);
                return NO_ERR;

            case FSM_DIV: // Final state
                if (c == '/'){ //  //
                    current_state = FSM_LINE_COMM_S;
                } else if (c == '*'){ //  /*
                    current_state = FSM_BLOCK_COMM_S;
                } else { //  /
                    token->token_type = TOKEN_DIV;
                    ungetc((char) c,file_src);
                    dynamic_str_clean(&dyn_str);
                    return NO_ERR;
                }
                break;

            case FSM_BLOCK_COMM_S: // Inter state
                if (c == '*'){ //  /**
                    current_state = FSM_BLOCK_COMM_CL;
                } else if (c == '/' && getc(file_src) == '*'){ // Nested block comments
                    token->block_comm_cnt++;
                    current_state = FSM_BLOCK_COMM_S;
                } else if (c == EOF){ // Unexpected end of file
                    current_state = FSM_ERROR;
                    ungetc((char) c,file_src);
                }
                break;

            case FSM_BLOCK_COMM_CL: // Inter state
                if (c == '/'){ //  /**/
                    current_state = FSM_BLOCK_COMMENT;
                } else if (c == EOF){ // Unexpected end of file
                    current_state = FSM_ERROR;
                    ungetc((char) c,file_src);
                }
                break;

            case FSM_BLOCK_COMMENT: // Final state
                if (token->block_comm_cnt == 0){
                    current_state = FSM_START; // Ignore everything in the block comment
                    ungetc((char) c,file_src);
                    break;
                } else { // We're still nested in the block comment
                    current_state = FSM_BLOCK_COMM_S;
                    token->block_comm_cnt--;
                    ungetc((char) c,file_src);
                }
                break;

            case FSM_LINE_COMM_S: // Inter state
                if (c == '\n' || c == EOF){ // The end of the line comment
                    current_state = FSM_LINE_COMMENT;
                }
                break;

            case FSM_LINE_COMMENT: // Final state
                current_state = FSM_START; // Ignore everything in the line comment
                ungetc((char) c,file_src);
                break;

            case FSM_GREATER: // Final state
                if (c == '=') { //  >=
                    current_state = FSM_GREATER_EQL;
                    break;
                } else { //  >
                    token->token_type = TOKEN_GREATER;
                    ungetc((char) c,file_src);
                    dynamic_str_clean(&dyn_str);
                    return NO_ERR;
                }
                
            case FSM_GREATER_EQL:  // Final state
                token->token_type = TOKEN_GREATER_EQL; //  >=
                ungetc((char) c,file_src);
                dynamic_str_clean(&dyn_str);
                return NO_ERR;

            case FSM_LESS: // Final state
                if (c == '=') { //  <=
                    current_state = FSM_LESS_EQL;
                    break;
                } else { //  <
                    token->token_type = TOKEN_LESS;
                    ungetc((char) c,file_src);
                    dynamic_str_clean(&dyn_str);
                    return NO_ERR;
                }

            case FSM_LESS_EQL:  // Final state
                token->token_type = TOKEN_LESS_EQL; //  <=
                ungetc((char) c,file_src);
                dynamic_str_clean(&dyn_str);
                return NO_ERR;

            case FSM_ASSIGN: // Final state
                if (c == '=') { //  ==
                    current_state = FSM_EQLS;
                    break;
                } else { //  =
                    token->token_type = TOKEN_ASSIGN;
                    ungetc((char) c,file_src);
                    dynamic_str_clean(&dyn_str);
                    return NO_ERR;
                }

            case FSM_EQLS: // Final state
                token->token_type = TOKEN_EQLS; //  ==
                ungetc((char) c,file_src);
                dynamic_str_clean(&dyn_str);
                return NO_ERR;

            case FSM_L_PAR: // Final state
                token->token_type = TOKEN_L_PAR; //  (
                ungetc((char) c,file_src);
                dynamic_str_clean(&dyn_str);
                return NO_ERR;

            case FSM_R_PAR: // Final state
                token->token_type = TOKEN_R_PAR; //  )
                ungetc((char) c,file_src);
                dynamic_str_clean(&dyn_str);
                return NO_ERR;

            case FSM_L_BRAC: // Final state
                token->token_type = TOKEN_L_BRAC; //  {
                ungetc((char) c,file_src);
                dynamic_str_clean(&dyn_str);
                return NO_ERR;

            case FSM_R_BRAC: // Final state
                token->token_type = TOKEN_R_BRAC; //  }
                ungetc((char) c,file_src);
                dynamic_str_clean(&dyn_str);
                return NO_ERR;

            case FSM_COLON: // Final state
                token->token_type = TOKEN_COLON; //  :
                ungetc((char) c,file_src);
                dynamic_str_clean(&dyn_str);
                return NO_ERR;

            case FSM_UNDER_L: // Final state
                if (isalpha(c) || isdigit(c) || c == '_'){ //  ID
                    append_char_to_str(&dyn_str, c);
                    current_state = FSM_ID;
                    break;
                } else { //  _
                    token->token_type = TOKEN_UNDER_L;
                    ungetc((char) c,file_src);
                    dynamic_str_clean(&dyn_str);
                    return NO_ERR;
                }

            case FSM_COMMA: // Final state
                token->token_type = TOKEN_COMMA; //  ,
                ungetc((char) c,file_src);
                dynamic_str_clean(&dyn_str);
                return NO_ERR;

            case FSM_QUESTION_MARK: // Final state
                if (c == '?'){ //  ??
                    current_state = FSM_NILL_CMP;
                } else { //  ?
                    current_state = FSM_ERROR;
                    ungetc((char) c,file_src);
                }
                break;

            case FSM_NILL_CMP: // Final state
                token->token_type = TOKEN_NILL_CMP; //  ??
                ungetc((char) c,file_src);
                dynamic_str_clean(&dyn_str);
                return NO_ERR;

            case FSM_EXCL_MARK: // Final state
                if (c == '='){ //  !=
                    current_state = FSM_NOT_EQLS;
                    break;
                } else { //  !
                    token->token_type = TOKEN_EXCL_MARK;
                    ungetc((char) c,file_src);
                    dynamic_str_clean(&dyn_str);
                    return NO_ERR;
                }

            case FSM_NOT_EQLS: // Final state
                token->token_type = TOKEN_NOT_EQLS; //  !=
                ungetc((char) c,file_src);
                dynamic_str_clean(&dyn_str);
                return NO_ERR;

            case FSM_ID: // Final state
                if (isalpha(c) || isdigit(c) || c == '_'){ // Loading the ID
                    append_char_to_str(&dyn_str, c);
                    break;
                } else if (c == '?') { // Possible nil data type
                    int kw_check;
                    if ((kw_check = keyword_cmpr(dyn_str.dynamic_str)) != -1){ // Variable ID is a keyword
                        token->token_type = TOKEN_KEYWORD; // Final state
                        token->token_value.token_keyword = kw_check; // Set the correct keyword value
                        token->can_be_nil = true; // Data type can be nil
                        dynamic_str_clean(&dyn_str);
                        return NO_ERR;
                    } else { // Variable ID is NOT a keyword
                        ungetc((char) c,file_src);
                        token->token_type = TOKEN_VAR_ID;
                        token->token_value.dyn_str = dyn_str; // Save the dynamic string
                        return NO_ERR;
                    }
                } else if (c == '('){ // Function ID or a keyword
                    ungetc((char) c,file_src);
                    int kw_check;
                    if ((kw_check = keyword_cmpr(dyn_str.dynamic_str)) != -1){ // ID is a keyword
                        token->token_type = TOKEN_KEYWORD; // Final state
                        token->token_value.token_keyword = kw_check; // Set the correct keyword
                        dynamic_str_clean(&dyn_str);
                        return NO_ERR;
                    } else {  // ID is NOT a keyword
                        token->token_type = TOKEN_FUNC_ID;
                        token->token_value.dyn_str = dyn_str; // Save the dynamic string
                        return NO_ERR;
                    }
                } else { // Variable ID or a keyword
                    ungetc((char) c,file_src);
                    int kw_check;
                    if ((kw_check = keyword_cmpr(dyn_str.dynamic_str)) != -1){ // Variable ID is a keyword
                        token->token_type = TOKEN_KEYWORD; // Final state
                        token->token_value.token_keyword = kw_check; // Set the correct keyword
                        dynamic_str_clean(&dyn_str);
                        return NO_ERR;
                    } else { // Variable ID is NOT a keyword
                        token->token_type = TOKEN_VAR_ID;
                        token->token_value.dyn_str = dyn_str; // Save the dynamic string
                        return NO_ERR; 
                    }       
                }

            case FSM_INT: // Final state
                if (isdigit(c)){ // Loading the integer
                    append_char_to_str(&dyn_str, c);
                } else if (c == '.') { // Loading the float
                    append_char_to_str(&dyn_str, c);
                    current_state = FSM_FLOAT_S;
                } else if (c == 'e' || c == 'E'){ // Loading the exponent
                    token->negative_exp = false; // The exponent is positive (+) by default
                    append_char_to_str(&dyn_str, c);
                    current_state = FSM_EXP_S;
                } else { // The integer was loaded
                    ungetc((char) c,file_src);
                    unsigned long int int_value = strtoul(dyn_str.dynamic_str, NULL, 10); // Convert the string to an integer
                    token->token_type = TOKEN_INT;
                    token->token_value.num_integer = (int) int_value; // Save the integer value
                    dynamic_str_clean(&dyn_str);
                    return NO_ERR;
                }
                break;
            
                case FSM_FLOAT_S: // Inter state
                    if (isdigit(c)){ // Loading the float
                        append_char_to_str(&dyn_str, c);
                        current_state = FSM_FLOAT;
                    } else { 
                        current_state = FSM_ERROR;
                        ungetc((char) c,file_src);
                    }
                    break;

                case FSM_FLOAT: // Final state
                    if (isdigit(c)){ // Loading the float
                        append_char_to_str(&dyn_str, c);
                    } else if (c == 'e' || c == 'E') { // Loading the exponent
                        token->negative_exp = false; // The exponent is positive (+) by default
                        append_char_to_str(&dyn_str, c);
                        current_state = FSM_EXP_S;
                    } else { // The float was loaded
                        ungetc((char) c, file_src);
                        double double_value = strtod(dyn_str.dynamic_str, NULL); // Convert the string to a double
                        token->token_type = TOKEN_FLOAT;
                        token->token_value.num_decimal = (double) double_value; // Save the float value
                        dynamic_str_clean(&dyn_str);
                        return NO_ERR;
                    }
                    break;

                case FSM_EXP_S: // Inter state
                    if (isdigit(c)){ // Loading the exponent
                        append_char_to_str(&dyn_str, c);
                        current_state = FSM_EXP;
                    } else if (c == '+' || c == '-') { // Loading the exponent sign
                        if (c == '-') // The exponent is negative (-)
                            token->negative_exp = true;

                        current_state = FSM_EXP_SIGN;
                    } else {
                        current_state = FSM_ERROR;
                        ungetc((char) c,file_src);
                    }
                    break;

                case FSM_EXP_SIGN: // Inter state
                    if (isdigit(c)){ // Loading the exponent
                        append_char_to_str(&dyn_str, c);
                        current_state = FSM_EXP;
                    } else {
                        current_state = FSM_ERROR;
                        ungetc((char) c,file_src);
                    }
                    break;

                case FSM_EXP: // Final state
                    if (isdigit(c)){ // Loading the exponent
                        append_char_to_str(&dyn_str, c);
                    } else { // The exponent was loaded

                        // Exponent conversion logic
                        double value;
                        Dynamic_Str_T base;
                        Dynamic_Str_T exp;
                        dynamic_str_init(&base); // Tmp var to load the base number
                        dynamic_str_init(&exp);  // Tmp var to load the exponent number

                        bool e = false;
                        for (unsigned i = 0; i < (dyn_str.str_len); i++)
                        {
                            if (dyn_str.dynamic_str[i] == 'e' || dyn_str.dynamic_str[i] == 'E'){
                                e = true;
                                i++;
                            }
                            if (e == false){ // Loading the float part
                                append_char_to_str(&base,dyn_str.dynamic_str[i]);

                            } else if (e == true){ // Loading the exponent part
                                append_char_to_str(&exp,dyn_str.dynamic_str[i]);
                            }  
                        }
                        value = atof(base.dynamic_str); // Convert the string to a float
                        int i = 0;
                        while (i < atoi(exp.dynamic_str)){
                            if (token->negative_exp == true) // The exponent is negative (-)
                                value = value * 0.1;
                            else // The exponent is positive (+)
                                value = value * 10;
                            i++;
                        }
                        // The exponent was converted correctly
                        ungetc((char) c, file_src);
                        token->token_type = TOKEN_FLOAT;
                        token->token_value.num_decimal = value;
                        dynamic_str_clean(&dyn_str);
                        return NO_ERR;
                    }
                    break;
                
            case FSM_STRING_S: // Inter state
                if (c > 31 && c != 34 && c != 92) { // The symbol is not a '"" or a '\'
                    append_char_to_str(&dyn_str, c);
                } else if (c == 34){ // The string was loaded
                    current_state = FSM_STRING;
                } else if (c == 92){ // Loading the escape sequence
                    current_state = FSM_ESC_SEQ;
                } else {
                    current_state = FSM_ERROR;
                    ungetc((char) c,file_src);
                }
                break;

            case FSM_ESC_SEQ: // Inter state
                    if (c == 34){
                        append_char_to_str(&dyn_str, '\"');
                        current_state = FSM_STRING_S;
                    } else if (c == 92) {
                        append_char_to_str(&dyn_str, '\\');
                        current_state = FSM_STRING_S;
                    } else if (c == 'n') {
                        append_char_to_str(&dyn_str, '\n');
                        current_state = FSM_STRING_S;
                    } else if (c == 't') {
                        append_char_to_str(&dyn_str, '\t');
                        current_state = FSM_STRING_S;
                    } else if (c == 'r') {
                        append_char_to_str(&dyn_str, '\r');
                        current_state = FSM_STRING_S;
                    } else if (c == 'u'){
                        current_state = FSM_HEXA_SEQ_S; // Hexadecimal escape sequence
                    } else {
                        current_state = FSM_ERROR;
                        ungetc((char) c,file_src);
                    }
                    break;

            case FSM_HEXA_SEQ_S: // Inter state
                if (c == '{'){ //  \u{
                    current_state = FSM_HEXA_SEQ;
                } else {
                    current_state = FSM_ERROR;
                    ungetc((char) c,file_src);
                }
                break;

            case FSM_HEXA_SEQ:
                if (c == '}'){ //  \u{}
                    current_state = FSM_STRING_S;
                    break;
                }

                if (isdigit(c) || (c >= 65 && c <= 70) || (c >= 95 && c <= 102)){
                    // Loading the next char to check if the hexadecimal number has 2 digits
                    char tmp = getc(file_src);
                    // The value of a hexadecimal number 
                    char value;
                    // Hexadecimal number has 2 digits
                    if(isdigit(tmp) || (tmp >= 65 && tmp <= 70) || (tmp >= 95 && tmp <= 102)){
                        value = 16*hex_to_dec(c) + hex_to_dec(tmp); // Convert to decimal value
                        append_char_to_str(&dyn_str,value);
                        current_state = FSM_HEXA_SEQ;
                    } else if (tmp == '}'){ // The hexadecimal number has only 1 digit
                        value = hex_to_dec(c);
                        append_char_to_str(&dyn_str,value);
                        current_state = FSM_STRING_S;
                    } else {
                        current_state = FSM_ERROR;
                        ungetc((char) c,file_src);
                    }
                } else {
                    current_state = FSM_ERROR;
                    ungetc((char) c,file_src);
                }
                break;
            
            case FSM_STRING: // Final state
                if (c == 34 && dyn_str.str_len == 0) { //  """   
                    current_state = FSM_M_LINE_STR_S;
                } else { // The string was loaded
                    ungetc((char) c, file_src);
                    token->token_type = TOKEN_STR;
                    token->token_value.dyn_str = dyn_str; // Save the dynamic string
                    return NO_ERR;
                }
                break;

            case FSM_M_LINE_STR_S: // Inter state
                if (c > 31 && c != 34 && c != 92) { // The symbol is not a '"" or a '\'
                    append_char_to_str(&dyn_str, c);
                } else if (c == 34){ // Indicates the end of a multiline string
                    current_state = FSM_M_LINE_STR;
                } else if (c == 92){ // Loading the escape sequence
                    current_state = FSM_ESC_SEQ;
                } else if (c == '\n') {
                    append_char_to_str(&dyn_str, c);
                } else if (c == '\t'){
                    append_char_to_str(&dyn_str, '8');
                } else {
                    current_state = FSM_ERROR;
                    ungetc((char) c,file_src);
                }
                break;
            
            case FSM_M_LINE_STR: //Final state
                if (c == 34 && (getc(file_src)) == 34){ //  """"""
                    token->token_type = TOKEN_M_LINE_STR;
                    token->token_value.dyn_str = dyn_str; // Save the dynamic string
                    return NO_ERR;
                } else {
                    current_state = FSM_ERROR;
                    ungetc((char) c,file_src);    
                }    
                break;

            case FSM_EOL: // Final state
                token->token_type = TOKEN_EOL; //  \n
                ungetc((char) c, file_src);
                dynamic_str_clean(&dyn_str);
                return NO_ERR;     

            case FSM_EOF: // Final state
                token->token_type = TOKEN_EOF; //  EOF
                dynamic_str_clean(&dyn_str);
                return NO_ERR;

            case FSM_ERROR: // Error state
                dynamic_str_clean(&dyn_str);
                return LEX_ERR;
        }
    }
}
/* End of scanner.c */