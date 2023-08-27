/* The variable data type.

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

#ifndef VAR_H
#define VAR_H

#ifdef __cplusplus
extern "C" {
#endif
#include "typedef.h"

// The variable structure.

struct variable {
    unsigned char * text;  // The text of the variable.
    int arbitrary : 1;     // Whether or not the variable is arbitrary.
};


variable * variable_init (unsigned char * text,
                        int arbitrary);

#ifdef __cplusplus
}
#endif

#endif // VAR_H
