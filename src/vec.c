/* Functions for the vector data type.

   Copyright (C) 2012, 2013 Ian Dunn.

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

#include "vec.h"

/* Initializes a vector object.
 *  input:
 *    stuff_size - the size of the desired objects.
 *  output:
 *    the newly created vector, or NULL on error.
 */
vec_t *
init_vec (const unsigned int stuff_size)
{
  vec_t * v;

  v = (vec_t *) calloc (1, sizeof (vec_t));
  if (!v)
    {
      perror (NULL);
      return NULL;
    }

  v->num_stuff = 0;
  v->size_stuff = stuff_size;
  v->alloc_space = 1;

  v->stuff = calloc (1, stuff_size);
  if (!v->stuff)
    {
      perror (NULL);
      return NULL;
    }

  return v;
}

/* Destroys a vector.
 *  input:
 *    v - the vector to destroy.
 *  output:
 *    none.
 */
void
destroy_vec (vec_t * v)
{
  if (v->stuff)
    free (v->stuff);

  v->num_stuff = 0;
  v->alloc_space = 0;
  v->size_stuff = 0;
  free (v);
}

/* Destroys a string vector.
 *  input:
 *    v - the vector to destroy.
 *  output:
 *    none.
 */
void
destroy_str_vec (vec_t * v)
{
  if (v->stuff)
    {
      int i;
      for (i = 0; i < v->num_stuff; i++)
	{
	  unsigned char * cur_str;
	  cur_str = vec_str_nth (v, i);

	  if (cur_str)
	    free (cur_str);
	  cur_str = NULL;
	}

      free (v->stuff);
    }

  v->num_stuff = 0;
  v->alloc_space = 0;
  v->size_stuff = 0;
  free (v);
}

/* Adds an object to a vector.
 *  input:
 *    v - the vector to which to add an object.
 *    more - the object data to add.
 *  output:
 *    0 on success, -1 on error.
 */
int
vec_add_obj (vec_t * v, const void * more)
{
  assert (more != NULL);

  v->num_stuff++; 

  if (v->alloc_space == v->num_stuff)
    {
      v->alloc_space *= 2;

      v->stuff = realloc (v->stuff, v->alloc_space * v->size_stuff);
      if (!v->stuff)
	{
	  perror (NULL);
	  return -1;
	}
    }

  memcpy (v->stuff + ((v->num_stuff - 1) * v->size_stuff),
	  more, v->size_stuff);

  return 0;
}

/* Adds a new string to a string vector.
 *  input:
 *    v - the string vector.
 *    more - the new string.
 *  output:
 *    0 on success, -1 on memory error.
 */
int
vec_str_add_obj (vec_t * v, unsigned char * more)
{
  v->num_stuff++;

  if (v->alloc_space == v->num_stuff)
    {
      v->alloc_space *= 2;

      v->stuff = realloc (v->stuff, v->alloc_space * sizeof (char *));
      if (!v->stuff)
	{
	  perror (NULL);
	  return -1;
	}
    }

  unsigned char * obj;
  obj = (unsigned char *) calloc (strlen (more) + 1, sizeof (char));
  if (!obj)
    {
      perror (NULL);
      return -1;
    }

  strcpy (obj, more);
  memcpy (v->stuff + ((v->num_stuff - 1) * sizeof (char *)),
	  &obj, sizeof (char *));

  unsigned char * new_obj;
  new_obj = vec_str_nth (v, v->num_stuff - 1);

  return 0;
}

/* Removes an object from the end of a vector.
 *  input:
 *    v - the vector from which to remove the end element.
 *  output:
 *    none.
 */
void
vec_pop_obj (vec_t * v)
{
  if (v->num_stuff == 0)
    return;

  v->num_stuff--;
}

/* Clears a vector.
 *  input:
 *    vec - the vector to clear.
 *  output:
 *    0 on success, -1 on error.
 */
int
vec_clear (vec_t * vec)
{
  free (vec->stuff);
  vec->num_stuff = 0;
  vec->alloc_space = 1;
  vec->stuff = calloc (1, vec->size_stuff);
  if (!vec->stuff)
    {
      perror (NULL);
      return -1;
    }
  return 0;
}

/* Gets the nth element of a vector.
 *  input:
 *    vec - the vector.
 *    n - the index to obtain.
 *  output:
 *    the nth object of the vector, or NULL if n is too large.
 */
void *
vec_nth (vec_t * vec, int n)
{
  if (n >= vec->num_stuff)
    return NULL;

  return (vec->stuff + (vec->size_stuff * n));
}

/* Gets the nth string from a string vector.
 *  input:
 *    vec - the string vector.
 *    n - the index of the string.
 *  output:
 *    The nth string.
 */
unsigned char *
vec_str_nth (vec_t * vec, int n)
{
  unsigned char ** tmp = vec_nth (vec, n);
  if (!tmp)
    return NULL;
  return *tmp;
}

/* Finds an object in a vector.
 *  input:
 *    vec - the vector in which to find an object.
 *    obj - the object's value.
 *  output:
 *    the index of the object, or -1 if no such object exists in the vector.
 */
int
vec_find (vec_t * vec, void * obj)
{
  int i;
  
  for (i = 0; i < vec->num_stuff; i++)
    {
      void * cur_obj;
      cur_obj = vec_nth (vec, i);
      if (cur_obj == obj)
	break;
    }

  i = (i == vec->num_stuff) ? -1 : i;
  return i;
}


/* Compares two string vectors, ignoring positioning.
 *  input:
 *    vec_0, vec_1 - the string vectors.
 *  output:
 *    0 - they are the same.
 *    -1 - memory error.
 *    -2 - An element from vec_0 doesn't match one from vec_1
 *    -3 - An element from vec_1 doesn't match one from vec_0
 */
int
vec_str_cmp (vec_t * vec_0, vec_t * vec_1)
{
  int i, j;
  short * check;

  check = (short *) calloc (vec_1->num_stuff, sizeof (short));
  if (!check)
    {
      perror (NULL);
      return -1;
    }

  for (i = 0; i < vec_0->num_stuff; i++)
    {
      unsigned char * cur_0;
      cur_0 = vec_str_nth (vec_0, i);

      for (j = 0; j < vec_1->num_stuff; j++)
	{
	  if (check[j])
	    continue;

	  unsigned char * cur_1;
	  cur_1 = vec_str_nth (vec_1, j);

	  if (!strcmp (cur_0, cur_1))
	    {
	      check[j] = 1;
	      break;
	    }
	}

      if (j == vec_1->num_stuff)
	{
	  free (check);
	  return -2;
	}
    }

  // vec_0 is a subsequence of vec_1

  for (i = 0; i < vec_1->num_stuff; i++)
    {
      if (!check[i])
	{
	  free (check);
	  return -3;
	}
    }

  free (check);

  return 0;
}

/* Checks if a vector is a subsequence of another vector.
 *  input:
 *    vec_0 - the smaller vector.
 *    vec_1 - the larger vector.
 *  output:
 *    0 - vec_0 is a subsequence of vec_1
 *    -1 - memory error.
 *    -2 - an element from vec_1 doesn't match any from vec_0
 *    -3 - an element from vec_0 doesn't match any from vec_1
 */
int
vec_str_sub (vec_t * vec_0, vec_t * vec_1)
{
  // vec_0 is the smaller vector,
  // vec_1 is the larger one.
  int ret;
  
  int i, j;
  short * check;

  check = (short *) calloc (vec_0->num_stuff, sizeof (short));
  if (!check)
    {
      perror (NULL);
      return -1;
    }

  for (i = 0; i < vec_1->num_stuff; i++)
    {
      unsigned char * cur_1;
      cur_1 = vec_str_nth (vec_1, i);

      for (j = 0; j < vec_0->num_stuff; j++)
	{
	  unsigned char * cur_0;
	  cur_0 = vec_str_nth (vec_0, j);

	  if (!strcmp (cur_0, cur_1))
	    {
	      check[j] = 1;
	      break;
	    }
	}

      if (j == vec_0->num_stuff)
	{
	  free (check);
	  return -2;
	}
    }

  for (i = 0; i < vec_0->num_stuff; i++)
    {
      if (!check[i])
	{
	  free (check);
	  return -3;
	}
    }

  free (check);

  return 0;
}
