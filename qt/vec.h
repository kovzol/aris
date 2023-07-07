/* The vector data type.

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


#ifndef VEC_H
#define VEC_H

#ifdef __cplusplus
extern "C" {
#endif
#include "typedef.h"

// The vector data structure.

struct vector
{
    unsigned int num_stuff;    //The amount of stuff.
    unsigned int size_stuff;   //The size of each stuff.

    unsigned int alloc_space;  //The allocated space.

    void * stuff;              //The stuff.
};


vec_t * init_vec (const unsigned int stuff_size);
void destroy_vec (vec_t * v);
void destroy_str_vec (vec_t * v);
int vec_add_obj (vec_t * v , const void * more);
int vec_str_add_obj (vec_t * v, unsigned char * more);
void vec_pop_obj (vec_t * v);
int vec_clear (vec_t * vec);
int vec_str_clear (vec_t * vec);
void * vec_nth (vec_t * vec, int n);
unsigned char * vec_str_nth (vec_t * vec, int n);
int vec_find (vec_t * vec, void * obj);
int vec_str_cmp (vec_t * vec_0, vec_t * vec_1);
int vec_str_sub (vec_t * vec_0, vec_t * vec_1);
#ifdef __cplusplus
}
#endif

#endif // VEC_H
