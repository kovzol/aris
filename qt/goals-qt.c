#include "goals-qt.h"
#include "process.h"
#include "sen-data.h"
#include "proof.h"
#include "list.h"
#include "vec.h"


int qtgoal_check_line(proof_t *proof, unsigned char *sen_text, vec_t *rets, int *ln, int *is_valid)
{
    // First, check for text errors.
    // Is that necessary?

    unsigned char * cmp_text;
    cmp_text = format_string (sen_text);
    if (!cmp_text)
        return AEC_MEM;

    int ret_check = check_text (cmp_text);
    if (ret_check < 0)
        return AEC_MEM;

    item_t * ev_itr;
    *is_valid = 1;
    *ln = -3;

    for (ev_itr = proof->everything->head; ev_itr != NULL;
         ev_itr = ev_itr->next)
    {
        sen_data * ev_sen = ev_itr->value;
        unsigned char * ev_cmp_text = format_string (ev_sen->text);
        unsigned char * ev_cur_ret = vec_str_nth(rets,ev_sen->line_num -1);

        if (!ev_cmp_text)
            return AEC_MEM;

        if (!strcmp (ev_cmp_text, cmp_text))
        {
            if (strcmp(ev_cur_ret, CORRECT))
                *is_valid = 0;

            *ln = ev_sen->line_num;

            //   char * sb_text = (char *) calloc (30, sizeof (char *));
            //   CHECK_ALLOC (sb_text, AEC_MEM);
            //   int offset = 0;
            //   offset += sprintf (sb_text, "The goal was met at line %i", ln);
            //   if (!is_valid)
            //     {
            //       offset += sprintf (sb_text + offset,
            // 			 ", however there are errors leading up to it.");
            //     }
            //   sen_parent_set_sb ((sen_parent *) goal, sb_text);
            //   free (sb_text);
            free (ev_cmp_text);
            free (cmp_text);
            return 0;
        }
        else
        {
            if (strcmp(ev_cur_ret, CORRECT))
                *is_valid = 0;
        }

        free (ev_cmp_text);
    }

    free (cmp_text);

    //   sentence_set_value (sen, VALUE_TYPE_FALSE);
    //   sen_parent_set_sb ((sen_parent *) goal, "This goal has not been met.");
    return 0;
}
