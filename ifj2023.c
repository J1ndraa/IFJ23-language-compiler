/* ******************************* ifj2023.c ********************************* */
/*  Author: Marek Čupr (xcuprm01)                                              */
/*  Subject: IFJ/IAL - Project                                                 */
/*  Date: 18. 11. 2023                                                         */
/*  Functionality: Start the program                                           */
/* *************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scanner.h"
#include "parser.h"
#include "dynamic_str.h"
#include "utils.h"

int main(int argc, char *argv[]){
    // Set up the file
    set_file(stdin);
    // Run the parser
    return get_err_type(parse());
}
