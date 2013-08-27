/* Typedefs.

   Copyright (C) 2012 Ian Dunn.

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

#ifndef ARIS_TYPE_DEF_H
#define ARIS_TYPE_DEF_H

typedef struct proof proof_t;
typedef struct list list_t;
typedef struct item item_t;
typedef struct vector vec_t;
typedef struct sen_data sen_data;
typedef struct sen_parent sen_parent;
typedef struct sentence sentence;
typedef struct goal goal_t;
typedef struct aris_proof aris_proof;
typedef struct variable variable;
typedef struct input_type in_type;
typedef struct key_function key_func;
typedef struct aris_app aris_app;
typedef struct rules_table rules_table;
typedef struct sen_id sen_id;
typedef struct rules_group rules_group;
typedef struct conf_object conf_obj;
typedef struct menu_item_data mid_t;

typedef void * (* conf_obj_value_func) (conf_obj * obj, int get);

#ifndef WIN32
#include <libintl.h>
#define _(String) gettext (String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)
#else
#include <windows.h>
#define _(String) String
#define N_(String) String
#endif

#define REPORT() fprintf (stderr, "%s:%i reporting!\n", __FILE__, __LINE__);
#define CHECK_ALLOC(o,r) if (!o) {perror (NULL); return r; }

#endif /*  ARIS_TYPE_DEF_H  */
