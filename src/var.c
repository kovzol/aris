#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "var.h"

/* Initializes a variable object.
 *  input:
 *    text - the text to set to the variable.
 *    arbitrary - whether or not the variable is arbitrary.
 *  output:
 *    the newly initialized variable, or NULL on error.
 */
variable *
variable_init (unsigned char * text, int arbitrary)
{
    variable * var;
    int text_len;

    var = (variable *) calloc (1, sizeof (variable));
    if (!var)
    {
        PERROR (NULL);
        return NULL;
    }

    var->text = NULL;
    if (text)
    {
        text_len = strlen ((const char *) text);
        var->text = (unsigned char *) calloc (text_len + 1, sizeof (char));
        if (!var->text)
        {
            PERROR (NULL);
            free (var);
            return NULL;
        }

        strcpy (var->text, text);
    }

    var->arbitrary = arbitrary;

    return var;
}
