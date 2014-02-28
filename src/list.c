/*  Functions to handle the doubly-linked list structures.

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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "list.h"

/* Initializes a list structure.
 *  input:
 *    none.
 *  output:
 *    the newly initialized list, or NULL on error.
 */
list_t *
init_list ()
{
  list_t * ls;

  ls = (list_t *) calloc (1, sizeof (list_t));
  if (!ls)
    {
      perror (NULL);
      return NULL;
    }

  ls->num_stuff = 0;

  ls->head = ls->tail = NULL;

  return ls;
}

/* Copies a list from an old one.
 *  input:
 *    ls_old - the old doubly-linked list.
 *  output:
 *    the new doubly-linked list, or NULL on error.
 */
list_t *
ls_copy (list_t * ls_old)
{
  list_t * ls;
  ls = init_list ();
  if (!ls)
    return NULL;

  if (!ls_old)
    return ls;

  item_t * itr, * itm;
  itr = ls_old->head;

  for (itr = ls_old->head; itr; itr = itr->next)
    {
      itm = ls_ins_obj (ls, itr->value, ls->tail);
      if (!itm)
	return NULL;
    }

  return ls;
}

/* Destroys a list - DOES NOT FREE MEMORY OF ITEMS.
 *  input:
 *    ls - the list to destroy.
 *  output:
 *    none.
 */
void
destroy_list (list_t * ls)
{
  ls_clear (ls);
  free (ls);
}

/* Inserts a new item into a list.
 *  input:
 *    ls - the list to insert an item into.
 *    obj - the data for the new item.
 *    it - the iterator AFTER which the new item will be inserted.
 *  output:
 *    the newly inserted item's iterator, or NULL on error.
 */
item_t *
ls_ins_obj (list_t * ls, void * obj, item_t * it)
{
  item_t * ins_itm;

  ins_itm = (item_t *) calloc (1, sizeof (item_t));
  if (!ins_itm)
    {
      perror (NULL);
      return NULL;
    }
  ins_itm->prev = ins_itm->next = NULL;
  ins_itm->value = obj;

  if (!ls->head)
    {
      ls->head = ls->tail = ins_itm;
    }
  else if (it == ls->tail)
    {
      ins_itm->next = NULL;
      ins_itm->prev = ls->tail;
      ls->tail->next = ins_itm;
      ls->tail = ins_itm;
    }
  else
    {
      ins_itm->prev = it;
      ins_itm->next = it->next;
      it->next->prev = ins_itm;
      it->next = ins_itm;
    }

  ls->num_stuff += 1;
  return ins_itm;
}

/* Adds an object to the end of a list.
 *  input:
 *    ls - the list.
 *    obj - the object.
 *  output:
 *    same as ls_ins_obj.
 */
item_t *
ls_push_obj (list_t * ls, void * obj)
{
  return ls_ins_obj (ls, obj, ls->tail);
}

/* Removes an object from a list.
 *  input:
 *    ls - the list from which an item is being removed.
 *    it - the iterator of the item being removed.
 *  output:
 *    none.
 */
void
ls_rem_obj (list_t * ls, item_t * it)
{
  if (ls->num_stuff == 0)
    return;

  ls->num_stuff--;

  if (!ls->head)
    {
      ls->head = ls->tail = NULL;
    }
  else if (it == ls->head)
    {
      if (ls->head->next)
	ls->head->next->prev = NULL;

      ls->head = ls->head->next;
    }
  else if (it == ls->tail)
    {
      if (ls->tail->prev)
	ls->tail->prev->next = NULL;
      ls->tail = ls->tail->prev;
    }
  else
    {
      it->prev->next = it->next;
      it->next->prev = it->prev;
    }
}

/* Removes an item from a list by the item's value.
 *  input:
 *    ls - the list from which an item is being removed.
 *    obj - the value of the item being removed.
 *  output:
 *    none.
 */
void
ls_rem_obj_value (list_t * ls, void * obj)
{
  item_t * itr = ls_find (ls, obj);
  if (itr)
    ls_rem_obj (ls, itr);
}

/* Clears a list - DOES NOT FREE THE DATA OF THE ITEMS.
 *  input:
 *    ls - the list to clear.
 *  output:
 *    none.
 */
void
ls_clear (list_t * ls)
{
  item_t * itm, * n_itm;

  for (itm = ls->head; itm; itm = n_itm)
    {
      n_itm = itm->next;
      itm->next = itm->prev = NULL;
      free (itm);
    }

  ls->head = ls->tail = NULL;
}

/* Obtains an item in a list by the item's index.
 *  input:
 *    ls - the list to obtain an item from.
 *    n - the index of the item in ls.
 *  output:
 *    the item in index n, or NULL if n is greater than the list's size.
 */
item_t *
ls_nth (list_t * ls, int n)
{
  int i = 0;
  item_t * itm;
  for (itm = ls->head; itm; itm = itm->next, i++)
    {
      if (i == n)
	break;
    }

  return itm;
}

/* Finds an item in a list based on the item's value.
 *  input:
 *    ls - the list in which to find the item.
 *    val - the value of the item being found.
 *  output:
 *    the item in the list with the desired value, or NULL if no such item exists.
 */
item_t *
ls_find (list_t * ls, void * val)
{
  item_t * itm;
  for (itm = ls->head; itm; itm = itm->next)
    {
      if (itm->value == val)
	break;
    }

  return itm;
}

int
ls_empty (list_t * ls)
{
  int ret = (ls->num_stuff == 0) ? 1 : 0;
  return ret;
}
