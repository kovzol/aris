/* Processing functions used by all main process functions.

   Copyright (C) 2012, 2013, 2014 Ian Dunn.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "process.h"
#include "vec.h"
#include "var.h"
#include "list.h"
#include "sen-data.h"
#include "rules.h"

#include <ctype.h>
#include <math.h>


struct connectives_list main_conns;

struct connectives_list main_conns = { 0 };

/* Eliminates a negation from a string.
 *  input:
 *    not_str - the string from which to eliminate the negation.
 *  output:
 *    the string without the negation, or NULL on error.
 */
unsigned char *
elim_not (const unsigned char * not_str)
{
    unsigned char * ret;
    unsigned int not_len = strlen ((const char *) not_str);

    ret = (unsigned char *) calloc (not_len, sizeof (char));
    CHECK_ALLOC (ret, NULL);
    strncpy (ret, not_str + NL, not_len - NL);
    ret[not_len - NL] = '\0';

    return ret;
}

/* Eliminates parentheses from a string.
 *  input:
 *    par_str - the string from which to eliminate parentheses.
 *  output:
 *    the string without parentheses, or NULL on error.
 */
unsigned char *
elim_par (const unsigned char * par_str)
{
    unsigned char * ret;
    unsigned int par_len = strlen ((const char *) par_str);

    if (par_len < 2)
        return NULL;

    if (par_str[0] != '(' || par_str[par_len - 1] != ')')
        return NULL;

    ret = (unsigned char *) calloc (par_len, sizeof (char));
    CHECK_ALLOC (ret, NULL);
    strncpy (ret, par_str + 1, par_len - 2);
    ret[par_len - 2] = '\0';

    return ret;
}

/* Removes the whitespaces from the input string.
 *  input:
 *    in_str - string from which to remove spaces.
 *  output:
 *    the string without whitespaces, or NULL on error.
 */
unsigned char *
die_spaces_die (unsigned char * in_str)
{
    size_t in_str_len;
    unsigned char* ret_str;

    //Get the length of str, and allocate enough memory for ret_str.
    in_str_len = strlen ((const char *) in_str);
    ret_str = (unsigned char *) calloc (in_str_len + 1, sizeof(char));
    CHECK_ALLOC (ret_str, NULL);

    //Iterate through the string.
    int i, j = 0;
    for (i = 0; i < in_str_len; i++)
    {
        //If the character in the input string is not a space,
        //then add it to ret_str.
        if (!isspace (in_str[i]))
            ret_str[j++] = in_str[i];
    }

    //Append a terminating null onto ret_str, and return it.
    ret_str[j] = '\0';

    return ret_str;
}

/* Removes a comment from the end of a string.
 *  input:
 *    in_str - the string from which to remove a comment.
 *  output:
 *    The string without the comment, or NULL on memory error.
 */
unsigned char *
remove_comment (unsigned char * in_str)
{
    size_t in_str_len, alloc_size;
    unsigned char * out_str, * ret_str;
    in_str_len = strlen (in_str);

    out_str = strchr (in_str, ';');

    if (!out_str)
        out_str = in_str + in_str_len;

    alloc_size = out_str - in_str;
    ret_str = (unsigned char *) calloc (alloc_size + 1, sizeof (char));
    CHECK_ALLOC (ret_str, NULL);

    strncpy (ret_str, in_str, alloc_size);
    ret_str[alloc_size] = '\0';

    return ret_str;
}

/* Strips a comment, and removes the spaces from a string.
 *  input:
 *    in_str - the string to format.
 *  output:
 *    The formatted string, or NULL on memory error.
 */
unsigned char *
format_string (unsigned char * in_str)
{
    unsigned char * tmp_str, * ret_str;
    tmp_str = remove_comment (in_str);
    if (!tmp_str)
        return NULL;

    ret_str = die_spaces_die (tmp_str);
    if (!ret_str)
        return NULL;
    free (tmp_str);

    return ret_str;
}

/* Finds a difference between two strings.
 *  input:
 *    sen_0 - the first string.
 *    sen_1 - the second string.
 *  output:
 *    the position in the two strings of the first difference, or -1 if none exists.
 */
int
find_difference (unsigned char * sen_0, unsigned char * sen_1)
{
    int i = 0;
    unsigned int sen0_len, sen1_len, act_len;

    sen0_len = strlen (sen_0);
    sen1_len = strlen (sen_1);

    act_len = (sen0_len < sen1_len) ? sen0_len : sen1_len;

    for (i = 0; i < act_len; i++)
        if (sen_0[i] != sen_1[i])
            break;

    i = (i == act_len) ? -1 : i;
    return i;
}

/* Parses specific tags on a string.
 *  input:
 *    in_str - the string to parse.
 *    init_pos - the position in in_str at which to begin parsing.
 *    out_str - a pointer to a string in which to store the result.
 *    o_tag - the opening tag.
 *    c_tag - the closing tag.
 */
int
parse_tags (const unsigned char * in_str, const int init_pos,
           unsigned char ** out_str, const char * o_tag,
           const char * c_tag)
{
    if (!in_str)
        return -2;

    //The lengths of the tags will be needed.
    int o_len, c_len;

    o_len = strlen (o_tag);
    c_len = strlen (c_tag);

    if (strncmp (in_str + init_pos, o_tag, o_len))
    {
        if (out_str)
            *out_str = NULL;
        return -2;
    }

    //The opening and closing parentheses count.
    int o_tag_count, c_tag_count;

    //The offset of the last tag.
    int tag_pos;

    //Temporary strings that take the return from strchr.
    unsigned char * o_str, * c_str;

    o_tag_count = 1;
    c_tag_count = 0;

    tag_pos = init_pos + 1;

    while (o_tag_count != c_tag_count)
    {
        //Find the next opening and closing parentheses.
        o_str = (unsigned char*) strstr ((const char *) in_str + tag_pos, o_tag);
        c_str = (unsigned char*) strstr ((const char *) in_str + tag_pos, c_tag);

        //If the offset of c_str from in_str,
        // is less than the offset of o_str from in_str,
        //then this is a closing parentheses.
        if (c_str != NULL
            && (o_str == NULL || (c_str - in_str) < (o_str - in_str)))
        {
            c_tag_count++;
            tag_pos = c_str - in_str + c_len;
        }
        else if (o_str != NULL)
        {
            o_tag_count++;
            tag_pos = o_str - in_str + o_len;
        }
        else
        {
            //If both strings are NULL, then return -2.
            return -2;
        }
    }

    tag_pos--;

    if (out_str)
    {
        // Allocate enough room for out_str,
        // and copy the parentheses construct from in_str.
        *out_str = (unsigned char*) calloc (tag_pos - init_pos + 2, sizeof (char));
        CHECK_ALLOC (*out_str, AEC_MEM);
        strncpy (*out_str, in_str + init_pos, tag_pos - init_pos + 1);
        (*out_str)[tag_pos - init_pos + 1] = '\0';
    }

    //Return tag_pos.
    return tag_pos;
}

/* Parses parentheses on an input string.
 *  input:
 *    in_str - the string to parse.
 *    init_pos - the position in the input string to begin parsing.
 *    out_str - receives the parentheses construct, including parentheses.
 *  output:
 *    on success - the position of the closing parentheses in the input string.
 *    on error - -2.
 *    on memory error - -1.
 */
int
parse_parens (const unsigned char * in_str, const int init_pos,
             unsigned char ** out_str)
{
    return parse_tags (in_str, init_pos, out_str, "(", ")");
}

/* Reverses the paren parsing process, and starts from a closing parentheses.
 *  input:
 *    in_str - the string to parse.
 *    init_pos - the position in the input string to begin parsing.
 *    out_str - receives the parentheses construct, including parentheses.
 *  output:
 *    on success - the position of the opening parentheses in the input string.
 *    on error - -2.
 *    on memory error - -1.
 */
int
reverse_parse_parens (const unsigned char * in_str, const int init_pos, unsigned char ** out_str)
{
    //If the character at the given position is not a closing parenthesis,
    //return -1 and set out_str to NULL.
    if (out_str)
        *out_str = NULL;
    if (in_str[init_pos] != ')')
        return -2;

    //The opening and closing parentheses count.
    int o_paren, c_paren;

    //The offset of the last parentheses.
    int paren_pos;

    //Temporary strings that take the return from strchr.
    unsigned char * o_str, * c_str;

    //Since strrchr can't start at a specific position,
    //a new string must be used.
    unsigned int in_len;
    unsigned char * tmp_str;

    //Allocate enough memory, then copy the memory into tmp_str.
    in_len = strlen ((const char *) in_str);
    tmp_str = (unsigned char *) calloc (in_len, sizeof (char));
    CHECK_ALLOC (tmp_str, AEC_MEM);
    strncpy (tmp_str, in_str, init_pos);
    tmp_str[init_pos] = '\0';

    //Initialize the counters.
    o_paren = 0;
    c_paren = 1;

    paren_pos = init_pos;

    while (o_paren != c_paren)
    {
        //Find the next opening and closing parentheses.
        o_str = (unsigned char*) strrchr ((const char *) tmp_str, '(');
        c_str = (unsigned char*) strrchr ((const char *) tmp_str, ')');

        //If the offset of c_str from in_str is greater than the offset of o_str from in_str,
        //then this is a closing parentheses.
        if (c_str != NULL
            && (o_str == NULL || (c_str - tmp_str) > (o_str - tmp_str)))
        {
            c_paren++;

            //Set paren_pos to the offset from in_str.
            paren_pos = c_str - tmp_str;

            c_str[0] = '\0';
        }
        else if (o_str != NULL)
        {
            o_paren++;

            //Set paren_pos to the offset from in_str.
            paren_pos = o_str - tmp_str;

            o_str[0] = '\0';
        }
        else
        {
            //If both strings are NULL, then return -1.
            free (tmp_str);
            return -2;
        }
    }

    //Free the memory used by tmp_str.
    free (tmp_str);

    if (out_str)
    {
        //Allocate space for out_str.
        *out_str = (unsigned char *) calloc (init_pos - paren_pos + 2, sizeof (char));
        CHECK_ALLOC (*out_str, AEC_MEM);

        //When this is all finished, o_str will point to the string that is needed.
        strncpy (*out_str, in_str + paren_pos, init_pos - paren_pos + 1);
        (*out_str)[init_pos - paren_pos + 2] = '\0';
    }

    return paren_pos;
}

/* Checks that each opening parentheses has a corresponding closing parentheses.
 *  input:
 *    chk_str - the string to be checked.
 *  output:
 *    1 if all parentheses are matched, 0 otherwise.
 */
int
check_parens (const unsigned char *chk_str)
{
    if (chk_str[0] == '\0')
        return 0;

    //Temporary strings for strstr.
    unsigned char * o_str = (unsigned char *) !0;
    unsigned char * c_str = (unsigned char *) !0;

    //Position indicators and helpers.
    unsigned int o_pos, c_pos, chk_pos;

    //Gets the string length of chk_str.
    //Used to determine if o_str or c_str are finished.
    unsigned int chk_len;

    //Count the amount of parentheses.
    unsigned int c_paren, o_paren;

    //Initialize the variables.
    o_pos = 0;
    c_pos = 0;
    chk_pos = 0;

    c_paren = 0;
    o_paren = 0;

    //Set chk_len to the length of chk_str.
    chk_len = strlen ((const char *) chk_str);

    while (o_str != NULL || c_str != NULL)
    {
        //If o_pos is alright, then get o_str.
        if (o_pos != chk_len)
            o_str = (unsigned char *) strchr ((const char *) chk_str + chk_pos, '(');

        //If c_pos is alright, then get c_str.
        if (c_pos != chk_len)
            c_str = (unsigned char *) strchr ((const char *) chk_str + chk_pos, ')');

        //Set o_pos and c_pos.
        o_pos = (o_str != NULL) ? o_str - chk_str : chk_len;
        c_pos = (c_str != NULL) ? c_str - chk_str : chk_len;

        //Increment the apropriate counter.
        if ( o_pos < c_pos && o_str != NULL)
            o_paren++;
        else if (c_str != NULL)
            c_paren++;

        //Set chk_pos.
        chk_pos = (o_pos < c_pos) ? (o_pos + 1) : (c_pos + 1);
    }

    //If there are an equal amount of opening and closing parentheses,
    //then return true.
    if (o_paren == c_paren)
        return 1;

    return 0;
}

/* Checks that each connective is not beside another connective.
 *  input:
 *    chk_str - the string to check.
 *    init_pos - the position of a connective.
 *  output:
 *    1 if the connective checks out, 0 otherwise.
 */
int
check_sides (const unsigned char * chk_str, const unsigned int init_pos)
{

    //Confirm that the connective is not at the beginning of
    //the original string.
    if (init_pos == 0)
        return 0;

    if (!isalnum (chk_str[init_pos - 1]) && chk_str[init_pos - 1] != ')'
        && (init_pos < CL || (strncmp (chk_str + init_pos - CL, NIL, CL)
                              && strncmp (chk_str + init_pos - CL, CTR, CL)
                              && strncmp (chk_str + init_pos - CL, TAU, CL))))
        return 0;

    //This connective must not be the last character.
    if (chk_str[init_pos + CL] == '\0')
        return 0;

    if (!ISGOOD (chk_str + init_pos + CL) && !isalnum (chk_str[init_pos + CL]))
        return 0;

    return 1;
}

/* Checks each position in the input string for a connective,
 *  then runs check_sides on it.
 *  input:
 *    chk_str - the string to check.
 *  output:
 *    1 if every connective checks out, 0 otherwise.
 */
int
check_conns (const unsigned char * chk_str)
{
    //Length of chk_str.
    int chk_len;

    // TODO Fix CONNECTIVES Later
//    main_conns = cli_conns;
//    main_conns = gui_conns;
    //Get the length of chk_str.
    chk_len = strlen ((const char *) chk_str);
    int chk_sides, j;

    for (j = 0; j < chk_len - CL + 1; j++)
    {
        if (IS_BIN_CONN (chk_str + j))
        {
            chk_sides = check_sides (chk_str, j);

            if (!chk_sides)
                return 0;
        }

        if (!strncmp (chk_str + j, NOT, NL))
        {
            // Left side.
            if (!(j == 0
                  || chk_str[j-1] == '('
                  || (j >= NL && !strncmp (chk_str + j - NL, NOT, NL))
                  || (j >= CL && IS_BIN_CONN (chk_str + j - CL))))

                return 0;

            // Right side
            if (j == chk_len - NL)
                return 0;
        }
    }

    return 1;
}

/* Gets a variable from a quantifier.
 *  input:
 *    quant_str - a string that points to the position just after a quantifier.
 *    quant_var - a pointer to a string that will receive the variable.
 *  output:
 *    The length of the variable on success.
 */
int
get_quant_var (const unsigned char * quant_str, unsigned char ** quant_var)
{
    int i, len = 0;
    for (i = 0; quant_str[i]; i++)
    {
        if (quant_str[i] == '('
            || !strncmp (quant_str + i, EXL, CL)
            || !strncmp (quant_str + i, UNV, CL)
            || !strncmp (quant_str + i, NOT, NL))
            break;

        if (i == 0 && !islower (quant_str[i]))
            return -2;

        if (!islower (quant_str[i]) && !isdigit (quant_str[i]))
            return -2;
    }

    len = i;
    (*quant_var) = (unsigned char *) calloc (len + 1, sizeof (char));
    CHECK_ALLOC (*quant_var, AEC_MEM);

    strncpy (*quant_var, quant_str, len);

    return len;
}

/* Checks that a quantifier is next to a bound variable and parenthesis.
 *  input:
 *    chk_str - the string to check.
 *    init_pos - the position of a quantifier.
 *  output:
 *    1 if the quantifier checks out, 0 otherwise.
 */
int
check_sides_quant (const unsigned char * chk_str, const unsigned int init_pos)
{
    //Get the length of the input string.
    unsigned int chk_len;
    chk_len = strlen ((const char *) chk_str);

    //Allocate memory for a test string.
    unsigned char tmp_str[CL + 1];

    //If this is not at the beginning of the string,
    //then process the connective (if there is one) before it.
    if (init_pos >= 0)
    {
        //Copy enough memory for a connective.
        strncpy (tmp_str, chk_str - CL, CL);

        //If this is a universal or existential, exit.
        if (!strncmp (tmp_str, UNV, CL) || !strncmp (tmp_str, EXL, CL))
            return 0;

        //Zero out the test string.
        memset (tmp_str, 0, CL);
    }

    //If there is no more data in the input string,
    //then exit.
    if (CL >= chk_len)
        return 0;

    /*
   * Determine the next quantifier or o-paren
   * Check that the variable is valid (should match [:lower:]([:lower:]|[:digit:])*)
   * Then proceed to check what ever is next to it.
   */

    int i;
    for (i = 0; chk_str[i]; i++)
    {
        if (chk_str[i] == '('
            || !strncmp (chk_str + i, EXL, CL)
            || !strncmp (chk_str + i, UNV, CL)
            || !strncmp (chk_str + i, NOT, NL))
            break;

        if (i == 0 && !islower (chk_str[i]))
            return 0;

        if (!islower (chk_str[i]) && !isdigit (chk_str[i]))
            return 0;
    }

    if (!chk_str[i])
        return 0;

    return 1;
}

/* Checks each quantifier of a string by running check_sides_quant.
 *  input:
 *    chk_str - the string to be checked.
 *  output:
 *    1 if every quantifier checks out, 0 otherwise.
 */
int
check_quants (const unsigned char * chk_str)
{

    //The length of the input string.
    int chk_len;

    //Get the length of the input string.
    chk_len = strlen ((const char *) chk_str);

    int chk_sides, j;
    vec_t * var_vec;
    var_vec = init_vec (sizeof (char *));
    if (!var_vec)
        return AEC_MEM;

    for (j = 0; j < chk_len - CL + 1; j++)
    {
        if (!strncmp (chk_str + j, UNV, CL) || !strncmp (chk_str + j, EXL, CL))
        {
            chk_sides = check_sides_quant (chk_str + j, j);
            if (chk_sides < 0)
                return AEC_MEM;

            if (!chk_sides)
            {
                destroy_str_vec (var_vec);
                return 0;
            }

            unsigned char * tmp_var;
            chk_sides = get_quant_var (chk_str + j + CL, &tmp_var);
            if (chk_sides == AEC_MEM)
                return AEC_MEM;

            chk_sides = vec_str_add_obj (var_vec, tmp_var);
            if (chk_sides < 0)
                return AEC_MEM;

            free (tmp_var);
        }
    }

    for (j = 0; j < var_vec->num_stuff; j++)
    {
        int k;
        for (k = 0; k < var_vec->num_stuff; k++)
        {
            if (j == k)
                continue;
            if (!strcmp (vec_str_nth (var_vec, j), vec_str_nth (var_vec, k)))
                break;
        }
        if (k != var_vec->num_stuff)
            break;
    }

    chk_sides = (j == var_vec->num_stuff);
    destroy_str_vec (var_vec);

    if (!chk_sides)
        return 0;

    //No errors, so return true.
    return 1;
}

/* Gets a single generality.
 *  input:
 *    in_str - the string from which to get a generality.
 *    in_pos - the initial position.
 *    out_str - a string pointer that receives the generality.
 *  output:
 *    The end of the generality on success, -1 on memory error.
 */
int
get_gen (unsigned char * in_str, int in_pos, unsigned char ** out_str)
{
    int i;

    CHECK_ALLOC (in_str, -1);

    for (i = in_pos; in_str[i] != '\0'; i++)
    {
        if (in_str[i] == '(')
        {
            i = parse_parens (in_str, i, NULL);
            continue;
        }

        if (!strncmp (in_str + i, AND, CL) || !strncmp (in_str + i, OR, CL)
            || !strncmp (in_str + i, CON, CL) || !strncmp (in_str + i, BIC, CL))
            break;
    }

    *out_str = (unsigned char *) calloc (i - in_pos + 1, sizeof (char));
    CHECK_ALLOC (*out_str, AEC_MEM);

    strncpy (*out_str, in_str + in_pos, i - in_pos);
    (*out_str)[i - in_pos] = '\0';
    return i;
}

/* Gets the generalities of a string with a given connective.
 *  input:
 *    chk_str - string to get the generalities from.
 *    conn - the connective to use, or if none given, takes the connective found.
 *    vec - receives the generalities.
 *  output:
 *    on success - the number of generalities.
 *    on error - -3
 *    if wrong connective given - -2
 *    memory error - -1
 */
int
get_generalities (unsigned char * chk_str, unsigned char * conn, vec_t * vec)
{
    int ret;

    unsigned char * lsen;

    int conn_pos, pos;
    unsigned int c_len;

    c_len = strlen (chk_str);
    lsen = NULL;

    conn_pos = 0;
    pos = get_gen (chk_str, conn_pos, &lsen);
    if (pos == AEC_MEM)
        return AEC_MEM;

    // Confirm that the first part wasn't the entire sentence.

    if (pos >= c_len - 1)
    {
        free (lsen);
        return -3;
    }

    // Check for the connective.

    int conn_len;

    if (conn[0] != '\0')
    {
        conn_len = strlen (conn);

        if (strncmp (chk_str + pos, conn, conn_len))
        {
            ret = vec_str_add_obj (vec, chk_str);
            if (ret < 0)
                return AEC_MEM;

            return -3;
        }
    }
    else
    {
        // Set conn to be the connective.
        strncpy (conn, chk_str + pos, CL);
        conn[CL] = '\0';

        conn_len = strlen (conn);
    }

    // Add the first sentence.

    ret = vec_str_add_obj (vec, lsen);
    if (ret < 0)
        return AEC_MEM;
    free (lsen);
    lsen = NULL;

    // While the string has more parts, continue to get them.

    unsigned char * rsen = NULL;
    while (chk_str[pos] != '\0')
    {
        if (rsen)
            free (rsen);
        rsen = NULL;

        if (strncmp (chk_str + pos, conn, conn_len))
        {
            destroy_str_vec (vec);
            return -2;
        }

        conn_pos = pos + conn_len;

        pos = get_gen (chk_str, conn_pos, &rsen);
        if (pos == AEC_MEM)
            return AEC_MEM;

        ret = vec_str_add_obj (vec, rsen);
        if (ret < 0)
            return AEC_MEM;
    }

    return (int) vec->num_stuff;
}

/* Runs text checking on a string to confirm that it follows FOL syntax.
 *  input:
 *    text - the string to check.
 *  output:
 *    0  - Success
 *    -1 - Memory Error
 *    -2 - Parenthesis Error
 *    -3 - Connective Error
 *    -4 - Quantifier Error
 */
int
check_text (unsigned char * text)
{
    if (text[0] == '\0')
        return -2;

    unsigned char * eval_text = strdup (text);
    CHECK_ALLOC (eval_text, AEC_MEM);

    int paren_check = 0, conn_check = 0, quant_check = 0;
    paren_check = check_parens (eval_text);
    if (!paren_check)
    {
        free (eval_text);
        return -2;
    }

    conn_check = check_conns (eval_text);
    if (!conn_check)
    {
        free (eval_text);
        return -3;
    }

    quant_check = check_quants (eval_text);
    if (!quant_check)
    {
        free (eval_text);
        return -4;
    }

    int gen = check_generalities (eval_text);
    if (gen != 0)
    {
        free (eval_text);
        return gen;
    }

    free (eval_text);
    return 0;
}

/* Checks the generalities of a string.
 *  input:
 *    text - the string to check, and recursively check generalities of.
 *  output:
 *    0  - Success
 *    -1 - Memory Error
 *    -2 - Parenthesis Error
 *    -3 - Connective Error
 *    -4 - Quantifier Error
 *    -5 - Construction Error
 */
int
check_generalities (unsigned char * text)
{
    // 1. Check for negation, or quantifier, and move until a parentheses is found.
    // 1a. If no opening parentheses is found, confirm that the string is alnum, and return success.
    // 2. Parse the parenthesis, and check their scope.
    // 2a. If the new string is empty, return an error.
    // 3. Run get_generalities on the new string.
    // 4. If get_generalities returns an error, return an error.
    // 5. Check the connective get_generalities returned, and confirm that it fits.
    // 6. Run check_text() on all generalities, returning an error if one arises.

    int pos = 0;

    unsigned char * tmp_str;
    int tmp_pos, ret_chk;

    if (text[0] == '\0')
        return -5;

    if (!ISGOOD (text)
        && !(strstr (text, AND) || strstr (text, OR)
             || strstr (text, CON) || strstr (text, BIC)))
    {
        if (strstr (text, ELM) || strchr (text, '=') || strchr (text, '<'))
        {
            ret_chk = check_infix (text, 1);
            if (ret_chk == AEC_MEM)
                return AEC_MEM;

            if (ret_chk == 0)
                return 0;
        }

        return -5;
    }

    tmp_pos = get_gen (text, 0, &tmp_str);
    if (tmp_pos == AEC_MEM)
        return AEC_MEM;

    if (!strcmp (tmp_str, text))
    {
        // This is the entire scope, so strip away the preceding symbol,
        //  if one exists.

        if (!strncmp (text, NOT, NL))
        {
            if (text[NL] == '\0')
                return -3;

            return check_generalities (text + NL);
        }

        if (!strncmp (text, UNV, CL) || !strncmp (text, EXL, CL))
        {
            pos = CL;
            while ((islower(text[pos]) || isdigit(text[pos]))
                   && text[pos] != '\0')
                pos++;
            if (text[pos] == '\0')
                return -4;

            if (strncmp (text + pos, UNV, CL) && strncmp (text + pos, EXL, CL)
                && strncmp (text + pos, NOT, NL) && text[pos] != '(')
                return -4;

            return check_generalities (text + pos);
        }

        if (text[0] == '(')
        {
            tmp_str = elim_par (text);
            if (!tmp_str)
                return AEC_MEM;

            ret_chk = check_generalities (tmp_str);
            if (ret_chk == AEC_MEM)
                return AEC_MEM;
            free (tmp_str);
            return ret_chk;
        }

        if (text[0] != '(')
        {
            ret_chk = check_symbols (text, 1);
            if (ret_chk == AEC_MEM)
                return AEC_MEM;

            if (ret_chk == -2)
                return -5;

            return 0;
        }
    }

    // Get generalities across the parenthesis construct.

    int gg;
    unsigned char conn[CL + 1];
    vec_t * gens;

    gens = init_vec (sizeof (char *));
    if (!gens)
        return AEC_MEM;

    conn[0] = '\0';
    gg = get_generalities (text, conn, gens);
    if (gg == AEC_MEM)
        return AEC_MEM;

    if (gens->num_stuff == 1 || gens->num_stuff == 0)
    {
        destroy_str_vec (gens);

        int ret_chk = check_symbols (text, 1);
        if (ret_chk == AEC_MEM)
            return AEC_MEM;

        if (ret_chk == -2)
            return -5;

        return 0;
    }

    if (gg == -2)
    {
        // Connective Error.
        return -3;
    }

    if (gens->num_stuff >= 3)
    {
        if (!strcmp (conn, CON) || !strcmp (conn, BIC))
        {
            // Connective Error.
            destroy_str_vec (gens);
            return -3;
        }
    }

    int i;
    unsigned char * cur_gen;
    for (i = 0; i < gens->num_stuff; i++)
    {
        cur_gen = vec_str_nth (gens, i);
        ret_chk = check_generalities (cur_gen);
        if (ret_chk == AEC_MEM)
            return AEC_MEM;

        if (ret_chk != 0)
        {
            destroy_str_vec (gens);
            return ret_chk;
        }
    }

    destroy_str_vec (gens);

    return 0;
}

/* Checks a predicate / function symbol.
 *  input:
 *    in_str - the string to check.
 *    pred - 1 if a predicate, 0 if a function symbol.
 *  output:
 *    0 - success.
 *    -1 - Memory error
 *    -2 - Error
 */
int
check_symbols (unsigned char * in_str, int pred)
{
    // Check for infix predicates / function symbols.

    int ret_chk;

    if (!strcmp (in_str, NIL))
        return 0;

    ret_chk = check_infix (in_str, pred);
    if (ret_chk != -3)
        return ret_chk;

    if (pred)
    {
        if (!strcmp (in_str, TAU) || !strcmp (in_str, CTR))
            return 0;

        if (!isupper (in_str[0]))
            return -2;
    }
    else
    {
        if (!islower (in_str[0]) && !isdigit (in_str[0]))
            return -2;
    }

    int pos = 1;

    while (in_str[pos] != '(' && in_str[pos] != '\0')
    {
        if (!ISLEGIT (in_str[pos]))
            return -2;

        pos++;
    }

    if (in_str[pos] == '\0')
        return 0;

    pos++;

    if (in_str[pos] == ')')
        return -2;

    int cur_pos;

    while (in_str[pos] != ')')
    {
        if (!strncmp (in_str + pos, NIL, CL))
        {
            cur_pos = pos + CL;
        }
        else
        {
            cur_pos = pos;

            while (ISLEGIT (in_str[cur_pos]))
                cur_pos++;
        }

        if (!ISSEP (in_str[cur_pos]))
            return -2;

        int done = 0;

        while (1)
        {
            int tmp_pos;
            unsigned char * tmp_str;

            switch (in_str[cur_pos])
            {
            case '(':
                tmp_pos = parse_parens (in_str, cur_pos, NULL);
                cur_pos = tmp_pos + 1;
                break;

            case '+':
            case '*':
                cur_pos++;
                break;

            case ')':
            case ',':
                tmp_str = (unsigned char *) calloc (cur_pos - pos + 1, sizeof (char));
                CHECK_ALLOC (tmp_str, AEC_MEM);

                strncpy (tmp_str, in_str + pos, cur_pos - pos);
                tmp_str[cur_pos - pos] = '\0';

                ret_chk = check_symbols (tmp_str, 0);
                if (ret_chk == AEC_MEM)
                    return AEC_MEM;

                free (tmp_str);
                if (ret_chk == -2)
                    return -2;
                done = 1;
                if (in_str[cur_pos] == ',')
                    cur_pos++;
                break;

            case '\0':
                return -2;
            }

            if (ISSEP (in_str[cur_pos]) && in_str[cur_pos] != ')')
            {
                if (in_str[cur_pos - 1] != ')')
                    return -2;

                if (in_str[cur_pos] == '(')
                    return -2;
            }

            if (done)
                break;
        }

        pos = cur_pos;
    }

    if (in_str[pos + 1] != '\0')
        return -2;

    return 0;
}


/* Checks a predicate / function infix symbol.
 *  input:
 *    in_str - the string to check.
 *    pred - 1 if a predicate, 0 if a function symbol.
 *  output:
 *    0 - success.
 *    -1 - Memory error
 *    -2 - Error
 *    -3 - No infix symbols found
 */
int
check_infix (unsigned char * in_str, int pred)
{
    // Check for an infix predicate.

    unsigned char * lsen, * rsen;

    if (pred)
    {
        // Determine which predicate symbol is being used, if any.

        unsigned char * sym;

        char * ss_elm, * ss_id, * ss_lt;

        ss_elm = strstr (in_str, ELM);
        ss_id = strchr (in_str, '=');
        ss_lt = strchr (in_str, '<');

        if (!ss_elm && !ss_id && !ss_lt)
            return -3;

        if (ss_elm)
        {
            if (ss_id || ss_lt)
                return -2;

            sym = ELM;
        }

        if (ss_id)
        {
            if (ss_lt)
                return -2;

            sym = "=";
        }

        if (ss_lt)
            sym = "<";

        unsigned char * sym_str;
        sym_str = (unsigned char *) strstr (in_str, sym);

        lsen = (unsigned char *) calloc (sym_str - in_str + 1, sizeof (char));
        CHECK_ALLOC (lsen, AEC_MEM);

        strncpy (lsen, in_str, sym_str - in_str);
        lsen[sym_str - in_str] = '\0';

        rsen = sym_str + strlen (sym);
        if (strstr (rsen, sym))
        {
            free (lsen);
            return -2;
        }

        int ret_chk;

        ret_chk = check_symbols (lsen, 0);
        if (ret_chk == AEC_MEM)
            return AEC_MEM;

        free (lsen);
        if (ret_chk == -2)
            return -2;

        ret_chk = check_symbols (rsen, 0);
        if (ret_chk == AEC_MEM)
            return AEC_MEM;

        if (ret_chk == -2)
            return -2;
    }
    else
    {
        char * ss_add, * ss_mul;

        ss_add = strchr (in_str, '+');
        ss_mul = strchr (in_str, '*');

        if (!ss_add && !ss_mul)
            return -3;
    }

    return 0;
}

/* Determines the sexpr connective of a standard connective.
 *  input:
 *    conn - the standard connective.
 *  output:
 *    The sexpr connective that corresponds to the connective.
 */
unsigned char *
conn_to_sexpr (unsigned char * conn)
{
    unsigned char * ret_conn = NULL;

    if (!strcmp (conn, AND))
        ret_conn = sexpr_conns.And;

    if (!strcmp (conn, OR))
        ret_conn = sexpr_conns.Or;

    if (!strcmp (conn, NOT))
        ret_conn = sexpr_conns.Not;

    if (!strcmp (conn, CON))
        ret_conn = sexpr_conns.con;

    if (!strcmp (conn, BIC))
        ret_conn = sexpr_conns.bic;

    if (!strcmp (conn, UNV))
        ret_conn = sexpr_conns.unv;

    if (!strcmp (conn, EXL))
        ret_conn = sexpr_conns.exl;

    if (!strcmp (conn, TAU))
        ret_conn = sexpr_conns.tau;

    if (!strcmp (conn, CTR))
        ret_conn = sexpr_conns.ctr;

    if (!strcmp (conn, ELM))
        ret_conn = sexpr_conns.elm;

    if (!strcmp (conn, NIL))
        ret_conn = sexpr_conns.nil;

    return ret_conn;
}

/* Converts a string to a sexpr string.
 *  input:
 *    in_str - the string to convert.
 *  output:
 *    The sexpr form of the input string.
 */
unsigned char *
convert_sexpr (unsigned char * in_str)
{
    unsigned char * out_str;
    int out_pos = 0;
    int in_len;

    in_len = strlen (in_str);

    unsigned char conn[CL + 1], * sexpr_conn;
    vec_t * gg_vec;
    int gg;

    gg_vec = init_vec (sizeof (char *));
    if (!gg_vec)
        return NULL;

    conn[0] = '\0';

    gg = get_generalities (in_str, conn, gg_vec);
    if (gg == AEC_MEM)
        return NULL;

    int i;

    if (gg < 0)
    {
        destroy_str_vec (gg_vec);

        // Only one generality.
        // Check for a negation.
        if (!strncmp (in_str, NOT, NL))
        {
            unsigned char * neg_in_str, * sexpr_str;

            neg_in_str = elim_not (in_str);
            if (!neg_in_str)
                return NULL;

            sexpr_str = convert_sexpr (neg_in_str);
            if (!sexpr_str)
                return NULL;
            free (neg_in_str);

            out_str = (unsigned char *) calloc (strlen (sexpr_str) + sexpr_conns.nl + 4,
                                               sizeof (char));
            CHECK_ALLOC (out_str, NULL);
            out_pos += sprintf (out_str + out_pos, "(%s %s)",
                               sexpr_conns.Not, sexpr_str);
            free (sexpr_str);
            return out_str;
        }

        // Check for an opening parenthesis.
        if (in_str[0] == '(')
        {
            unsigned char * par_in_str, * sexpr_str;

            par_in_str = elim_par (in_str);
            if (!par_in_str)
                return NULL;

            sexpr_str = convert_sexpr (par_in_str);
            if (!sexpr_str)
                return NULL;
            free (par_in_str);

            out_str = (unsigned char *) calloc (strlen (sexpr_str) + 1, sizeof (char));
            CHECK_ALLOC (out_str, NULL);
            strcpy (out_str, sexpr_str);
            free (sexpr_str);
            return out_str;
        }

        // Check for a quantifier.
        if (!strncmp (in_str, UNV, CL) || !strncmp (in_str, EXL, CL))
        {
            unsigned char * sexpr_quant, * quant_var, quant[CL + 1];

            strncpy (quant, in_str, CL);
            quant[CL] = '\0';

            sexpr_quant = conn_to_sexpr (quant);

            for (i = CL; in_str[i] != '\0'; i++)
            {
                if (!strncmp (in_str + i, EXL, CL) || !strncmp (in_str + i, UNV, CL)
                    || in_str[i] == '(' || !strncmp (in_str + i, NOT, NL))
                    break;
            }

            quant_var = (unsigned char *) calloc (i - CL + 1, sizeof (char));
            CHECK_ALLOC (quant_var, NULL);

            strncpy (quant_var, in_str + CL, i - CL);
            quant_var[i - CL] = '\0';

            unsigned char * quant_in_str, * sexpr_str;

            quant_in_str = (unsigned char *) calloc (in_len - i + 1, sizeof (char));
            CHECK_ALLOC (quant_in_str, NULL);

            strcpy (quant_in_str, in_str + i);
            sexpr_str = convert_sexpr (quant_in_str);
            if (!sexpr_str)
                return NULL;
            free (quant_in_str);

            int alloc_size;

            alloc_size = strlen (quant_var) + sexpr_conns.cl + strlen (sexpr_str) + 7;
            out_str = (unsigned char *) calloc (alloc_size, sizeof (char));
            CHECK_ALLOC (out_str, NULL);
            out_pos += sprintf (out_str + out_pos, "((%s %s) %s)",
                               sexpr_quant, quant_var, sexpr_str);
            free (sexpr_str);
            free (quant_var);

            return out_str;
        }

        unsigned char * tmp_str;
        tmp_str = infix_to_prefix (in_str);
        if (!tmp_str)
            return NULL;

        int tmp_len;
        tmp_len = strlen (tmp_str);

        // Nothing was found, so check for a function or predicate.
        for (i = 0; i < tmp_len; i++)
        {
            if (tmp_str[i] == '(')
                break;
        }

        if (i == tmp_len)
        {
            // No parentheses, return in_str.
            if (!strcmp (tmp_str, TAU) || !strcmp (tmp_str, CTR))
            {
                out_str = (unsigned char *) calloc (sexpr_conns.cl + 1, sizeof (char));
                CHECK_ALLOC (out_str, NULL);

                if (!strcmp (tmp_str, TAU))
                    strcpy (out_str, sexpr_conns.tau);
                else
                    strcpy (out_str, sexpr_conns.ctr);
            }
            else if (!strcmp (tmp_str, NIL))
            {
                out_str = (unsigned char *) calloc (sexpr_conns.cl + 1, sizeof (char));
                CHECK_ALLOC (out_str, NULL);
                strcpy (out_str, sexpr_conns.nil);
            }
            else
            {
                out_str = (unsigned char *) calloc (tmp_len + 1, sizeof (char));
                CHECK_ALLOC (out_str, NULL);
                strcpy (out_str, tmp_str);
            }
        }
        else
        {
            unsigned char * pred_in_str, * sexpr_str;

            vec_t * arg_vec, * sexpr_args;
            arg_vec = init_vec (sizeof (char *));
            if (!arg_vec)
                return NULL;

            int ret_check, alloc_size;
            ret_check = get_pred_func_args (tmp_str, 0, &pred_in_str, arg_vec);
            if (ret_check < 0)
                return NULL;

            sexpr_args = init_vec (sizeof (char *));
            if (!sexpr_args)
                return NULL;

            alloc_size = strlen (pred_in_str) + 1;
            for (i = 0; i < arg_vec->num_stuff; i++)
            {
                unsigned char * cur_arg;
                cur_arg = vec_str_nth (arg_vec, i);

                sexpr_str = convert_sexpr (cur_arg);
                if (!sexpr_str)
                    return NULL;

                ret_check = vec_str_add_obj (sexpr_args, sexpr_str);
                if (ret_check < 0)
                    return NULL;

                alloc_size += strlen (sexpr_str) + 1;
                free (sexpr_str);
            }

            destroy_str_vec (arg_vec);

            out_str = (unsigned char *) calloc (alloc_size + 2, sizeof (char));
            CHECK_ALLOC (out_str, NULL);
            out_pos += sprintf (out_str + out_pos, "(%s", pred_in_str);
            free (pred_in_str);

            for (i = 0; i < sexpr_args->num_stuff; i++)
            {
                unsigned char * sexpr_str;
                sexpr_str = vec_str_nth (sexpr_args, i);
                out_pos += sprintf (out_str + out_pos, " %s", sexpr_str);
            }

            destroy_str_vec (sexpr_args);
            out_pos += sprintf (out_str + out_pos, ")");
        }

        free (tmp_str);
        return out_str;
    }

    sexpr_conn = conn_to_sexpr (conn);

    int alloc_size;
    vec_t * sexpr_gens;

    sexpr_gens = init_vec (sizeof (char *));
    if (!sexpr_gens)
        return NULL;

    alloc_size = sexpr_conns.cl + 1;

    for (i = 0; i < gg_vec->num_stuff; i++)
    {
        int ret_chk;
        unsigned char * cur_gen, * sexpr_gen;

        cur_gen = vec_str_nth (gg_vec, i);

        sexpr_gen = convert_sexpr (cur_gen);
        if (!sexpr_gen)
            return NULL;

        ret_chk = vec_str_add_obj (sexpr_gens, sexpr_gen);
        if (ret_chk < 0)
            return NULL;

        alloc_size += strlen (sexpr_gen) + 1;
        free (sexpr_gen);
    }

    destroy_str_vec (gg_vec);
    out_str = (unsigned char *) calloc (alloc_size + 2, sizeof (char));
    CHECK_ALLOC (out_str, NULL);

    out_pos += sprintf (out_str, "(%s", sexpr_conn);

    for (i = 0; i < sexpr_gens->num_stuff; i++)
    {
        unsigned char * sexpr_gen;
        sexpr_gen = vec_str_nth (sexpr_gens, i);
        out_pos += sprintf (out_str + out_pos, " %s", sexpr_gen);
    }

    out_pos += sprintf (out_str + out_pos, ")");

    destroy_str_vec (sexpr_gens);
    return out_str;
}

/* Converts a string from infix to prefix.
 *  input:
 *    in_str - the string to convert.
 *  output:
 *    The string in prefix form.
 */
unsigned char *
infix_to_prefix (unsigned char * in_str)
{
    unsigned char * out_str;

    int i, pos, in_len, got_nil = 0;
    unsigned char * lsen, * rsen;

    in_len = strlen (in_str);
    i = 0;
    pos = 0;

    if (!strcmp (in_str, NIL) && in_len == CL)
    {
        out_str = (unsigned char *) calloc (in_len + 1, sizeof (char));
        CHECK_ALLOC (out_str, NULL);

        strcpy (out_str, in_str);
        return out_str;
    }

    while (i < in_len)
    {
        if (!strncmp (in_str + i, NIL, CL))
        {
            i += CL;
            got_nil = 1;
            continue;
        }

        while (islower (in_str[i]) || isdigit (in_str[i]))
            i++;

        if (in_str[i] == '(')
        {
            int tmp_pos;
            unsigned char * tmp_str;

            tmp_pos = parse_parens (in_str, i, &tmp_str);
            if (tmp_pos == AEC_MEM)
                return NULL;

            free (tmp_str);
            i = tmp_pos + 1;
            continue;
        }

        if (in_str[i] == '=' || in_str[i] == '<' || !strncmp (in_str + i, ELM, CL))
        {
            lsen = (unsigned char *) calloc (i + 1, sizeof (char));
            CHECK_ALLOC (lsen, NULL);
            strncpy (lsen, in_str + pos, i);
            lsen[i] = '\0';

            if (!strncmp (in_str + i, ELM, CL))
                pos = i + CL;
            else
                pos = i+1;

            break;
        }

        i++;
    }

    if (i >= in_len)
    {
        out_str = infix_to_prefix_func (in_str);
        return out_str;
    }

    rsen = (unsigned char *) calloc (in_len - pos + 1, sizeof (char));
    CHECK_ALLOC (rsen, NULL);
    strcpy (rsen, in_str + pos);

    unsigned char * c_lsen, * c_rsen;

    c_lsen = infix_to_prefix_func (lsen);
    if (!c_lsen)
        return NULL;

    c_rsen = infix_to_prefix_func (rsen);
    if (!c_rsen)
        return NULL;

    free (lsen);
    free (rsen);

    out_str = (unsigned char *) calloc (strlen (c_lsen) + strlen (c_rsen) + 8,
                                       sizeof (char));
    CHECK_ALLOC (out_str, NULL);

    int out_pos;

    if (!strncmp (in_str + i, ELM, CL))
    {
        strncpy (out_str, sexpr_conns.elm, sexpr_conns.cl);
        out_pos = sexpr_conns.cl;
    }
    else
    {
        out_str[0] = in_str[i];
        out_pos = 1;
    }

    sprintf (out_str + out_pos, "(%s,%s)", c_lsen, c_rsen);
    free (c_lsen);
    free (c_rsen);
    return out_str;
}

/* Converts a function from infix to prefix.
 *  input:
 *    in_str - the string to convert.
 *  output:
 *    The converted string.
 */
unsigned char *
infix_to_prefix_func (unsigned char * in_str)
{
    unsigned char * out_str;

    int i, pos, in_len;
    unsigned char * lsen, * rsen, * sym;

    in_len = strlen (in_str);
    i = 0;
    pos = 0;

    if (!strcmp (in_str, NIL))
    {
        out_str = (unsigned char *) calloc (in_len + 1, sizeof (char));
        CHECK_ALLOC (out_str, NULL);

        strcpy (out_str, in_str);
        return out_str;
    }

    while (i < in_len)
    {
        while (islower (in_str[i]) || isdigit (in_str[i]))
            i++;

        if (in_str[i] == '(')
        {
            int tmp_pos;
            unsigned char * tmp_str;

            tmp_pos = parse_parens (in_str, i, &tmp_str);
            if (tmp_pos == AEC_MEM)
                return NULL;

            free (tmp_str);
            i = tmp_pos + 1;
            continue;
        }

        if (i != 0 && (in_str[i] == '+' || in_str[i] == '*'))
        {
            lsen = (unsigned char *) calloc (i + 1, sizeof (char));
            CHECK_ALLOC (lsen, NULL);

            strncpy (lsen, in_str + pos, i);
            lsen[i] = '\0';

            pos = i+1;

            break;
        }

        i++;
    }

    if (i >= in_len)
    {
        out_str = (unsigned char *) calloc (in_len + 1, sizeof (char));
        CHECK_ALLOC (out_str, NULL);

        strcpy (out_str, in_str);
        return out_str;
    }

    rsen = (unsigned char *) calloc (in_len - pos + 1, sizeof (char));
    CHECK_ALLOC (rsen, NULL);
    strcpy (rsen, in_str + pos);

    unsigned char * c_lsen, * c_rsen;

    c_lsen = infix_to_prefix_func (lsen);
    if (!c_lsen)
        return NULL;

    c_rsen = infix_to_prefix_func (rsen);
    if (!c_rsen)
        return NULL;

    free (lsen);
    free (rsen);

    out_str = (unsigned char *) calloc (strlen (c_lsen) + strlen (c_rsen) + 5,
                                       sizeof (char));
    CHECK_ALLOC (out_str, NULL);

    sprintf (out_str, "%c(%s,%s)", in_str[i], c_lsen, c_rsen);
    free (c_lsen);
    free (c_rsen);
    return out_str;
}

/* Gets the arguments from a predicate or function.
 *  input:
 *    in_str - the string to check.
 *    in_pos - the initial position.
 *    sym - a pointer to a string that receives the symbol.
 *    args - a string vector that receives the arguments.
 *  output:
 *    0 on success, -1 on memory error.
 */
int
get_pred_func_args (unsigned char * in_str, int in_pos,
                   unsigned char ** sym, vec_t * args)
{
    int i;

    i = in_pos;
    while (in_str[i] != '(')
        i++;

    *sym = (unsigned char *) calloc (i + 1, sizeof (char));
    CHECK_ALLOC (*sym, AEC_MEM);
    strncpy (*sym, in_str, i);
    (*sym)[i] = '\0';

    int pos;
    unsigned char * tmp_str;
    int tmp_pos;

    tmp_pos = parse_parens (in_str, i, &tmp_str);
    if (tmp_pos == AEC_MEM)
        return AEC_MEM;

    unsigned char * elim_str;
    elim_str = elim_par (tmp_str);
    if (!elim_str)
        return AEC_MEM;
    free (tmp_str);

    i = 0;
    pos = 0;
    while (1)
    {
        int ret_check;

        while (islower (elim_str[i]) || isdigit (elim_str[i]))
            i++;

        if (elim_str[i] == '(')
        {
            tmp_pos = parse_parens (elim_str, i, &tmp_str);
            if (tmp_pos == AEC_MEM)
                return AEC_MEM;

            free (tmp_str);
            i = tmp_pos + 1;
            continue;
        }

        if (elim_str[i] == ',' || elim_str[i] == '\0')
        {
            unsigned char * new_arg;

            new_arg = (unsigned char *) calloc (i - pos + 1, sizeof (char));
            CHECK_ALLOC (new_arg, AEC_MEM);

            strncpy (new_arg, elim_str + pos, i - pos);
            new_arg[i - pos] = '\0';

            ret_check = vec_str_add_obj (args, new_arg);
            if (ret_check < 0)
                return AEC_MEM;

            free (new_arg);
            if (elim_str[i] == '\0')
                break;

            pos = ++i;
            continue;
        }

        i++;
    }

    return 0;
}

/* Creates a constant definition.
 *  input:
 *    in_str - a string that contains one number.
 *  output:
 *    A string containing a definition of the number in seqlog.
 */
unsigned char *
new_const (char * in_str)
{
    int n, i;
    unsigned char * out_str;
    int out_pos = 0;
    int alloc_size;

    sscanf (in_str, "%i", &n);

    alloc_size = CL + 14 + (3 * n) + (log (n) + 1);
    out_str = (unsigned char *) calloc (alloc_size + 1, sizeof (char));
    CHECK_ALLOC (out_str, NULL);

    out_pos += sprintf (out_str + out_pos, "%s(v(n,", UNV);
    for (i = 0; i < n; i++)
        out_pos += sprintf (out_str + out_pos, "s(");

    out_pos += sprintf (out_str + out_pos, "z(x)");
    for (i = 0; i < n; i++)
        out_pos += sprintf (out_str + out_pos, ")");
    out_pos += sprintf (out_str + out_pos, ") = %i)", n);

    return out_str;
}
