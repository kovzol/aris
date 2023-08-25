/* Function to check a goal line (platform independent but named with qt as it's required only for the qt version).
   Inspired from goals.h.

   Copyright (C) 2023 Saksham Attri.

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
#include "goals-qt.h"
#include "../src/process.h"
#include "../src/sen-data.h"
#include "../src/proof.h"
#include "../src/list.h"
#include "../src/vec.h"


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
    return 0;
}
