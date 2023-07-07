/*  The doubly-linked list structure.

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

#ifndef LIST_H
#define LIST_H

#ifdef __cplusplus
extern "C" {
#endif
#include "typedef.h"

// The item structure of the doubly-linked list.

struct item
{
    struct item * prev;
    struct item * next;

    void * value;
};

// The doubly-linked list structure itself.

struct list
{
    unsigned int num_stuff;

    item_t * head, * tail;
};

list_t * init_list ();
list_t * ls_copy (list_t * ls_old);
void destroy_list (list_t * ls);
item_t * ls_ins_obj (list_t * ls, void * obj, item_t * it);
item_t * ls_push_obj (list_t * ls, void * obj);
void ls_rem_obj (list_t * ls, item_t * it);
void ls_rem_obj_value (list_t * ls, void * obj);
void ls_clear (list_t * ls);
item_t * ls_nth (list_t * ls, int n);
item_t * ls_find (list_t * ls, void * val);
int ls_empty (list_t * ls);
#ifdef __cplusplus
}
#endif

#endif // LIST_H
