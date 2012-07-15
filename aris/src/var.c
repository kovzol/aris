/* Functions for handling variables.

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
      perror (NULL);
      return NULL;
    }

  var->text = NULL;
  if (text)
    {
      text_len = strlen ((const char *) text);
      var->text = (unsigned char *) calloc (text_len + 1, sizeof (char));
      if (!var->text)
	{
	  perror (NULL);
	  return NULL;
	}

      strcpy (var->text, text);
    }

  var->arbitrary = arbitrary;

  return var;
}
