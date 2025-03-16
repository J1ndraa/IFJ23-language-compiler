/* ******************************* error.c *********************************** */
/*  Author: Marek Čupr (xcuprm01)                                              */
/*  Subject: IFJ/IAL - Project                                                 */
/*  Date: 13. 10. 2023                                                         */
/*  Functionality: Handle errors and print warnings on stderr                  */
/* *************************************************************************** */

#include "error.h"   // header file
#include <stdio.h>   // fprintf()

/**
 * Prints the correct error on stderr.
 *
 * @param err Type of error to be printed out
 * @returns Code of the specific error
 */
int get_err_type(Errors_T err) {
    switch (err){
        case NO_ERR:
            break; 
        case LEX_ERR:
            fprintf(stderr, "!! WARNING: AN ERROR HAS OCCURED !!\n\n- ERROR CLASIFICATION: Lexical Analysis\n- ERROR DESCRIPTION: Incorrect structure of the current lexem\n- RETURN VALUE: The program has ended with a return value of %d\n", err);
            break;
        case SYNTAX_ERR:
            fprintf(stderr, "!! WARNING: AN ERROR HAS OCCURED !!\n\n- ERROR CLASIFICATION: Syntactic Analysis\n- ERROR DESCRIPTION: Invalid program syntax, missing header file, etc..\n- RETURN VALUE: The program has ended with a return value of %d\n", err);
            break;
        case SEMANTIC_ERR_A:
            fprintf(stderr, "!! WARNING: AN ERROR HAS OCCURED !!\n\n- ERROR CLASIFICATION: Semantic Analysis\n- ERROR DESCRIPTION: Undefined function / variable redefinition\n- RETURN VALUE: The program has ended with a return value of %d\n", err);
            break;
        case SEMANTIC_ERR_B:
            fprintf(stderr, "!! WARNING: AN ERROR HAS OCCURED !!\n\n- ERROR CLASIFICATION: Semantic Analysis\n- ERROR DESCRIPTION: Incorrect parameter count or type / wrong return type\n- RETURN VALUE: The program has ended with a return value of %d\n", err);
            break;
        case SEMANTIC_ERR_C:
            fprintf(stderr, "!! WARNING: AN ERROR HAS OCCURED !!\n\n- ERROR CLASIFICATION: Semantic Analysis\n- ERROR DESCRIPTION: Usage of undefined / uninitialized variable\n- RETURN VALUE: The program has ended with a return value of %d\n", err);
            break;
        case SEMANTIC_ERR_D:
            fprintf(stderr, "!! WARNING: AN ERROR HAS OCCURED !!\n\n- ERROR CLASIFICATION: Semantic Analysis\n- ERROR DESCRIPTION: Missing / extra expression at function return command\n- RETURN VALUE: The program has ended with a return value of %d\n", err);
            break;
        case SEMANTIC_ERR_E:
            fprintf(stderr, "!! WARNING: AN ERROR HAS OCCURED !!\n\n- ERROR CLASIFICATION: Semantic Analysis\n- ERROR DESCRIPTION: Type incompability at aritmetic / string / relational expressions\n- RETURN VALUE: The program has ended with a return value of %d\n", err);
            break;
        case SEMANTIC_ERR_F:
            fprintf(stderr, "!! WARNING: AN ERROR HAS OCCURED !!\n\n- ERROR CLASIFICATION: Semantic Analysis\n- ERROR DESCRIPTION: Variable / parameter type isn't mentioned and can't be derived from the used expression\n- RETURN VALUE: The program has ended with a return value of %d\n", err);
            break;
        case SEMANTIC_ERR_OTHER:
            fprintf(stderr, "!! WARNING: AN ERROR HAS OCCURED !!\n\n- ERROR CLASIFICATION: Semantic Analysis\n- ERROR DESCRIPTION: Other semantic error\n- RETURN VALUE: The program has ended with a return value of %d\n", err);
            break;
        case COMPILER_ERR_INTER:
            fprintf(stderr, "!! WARNING: AN ERROR HAS OCCURED !!\n\n- ERROR CLASIFICATION: Internal Compiler Error\n- ERROR DESCRIPTION: Failed at dynamical memory allocation, etc..\n- RETURN VALUE: The program has ended with a return value of %d\n", err);
            break;
        default:
            return -1; 
    }
    return err;
}
/* End of error.c */