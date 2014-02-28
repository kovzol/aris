/* Functions and tags for opening and saving files.

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

#ifndef ARIS_A_IO_H
#define ARIS_A_IO_H

#include <stdlib.h>
#include "typedef.h"

// Tags for the xml files.
#define HEADER "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
#define PROOF_TAG "proof"
#define GOAL_TAG "goals"
#define PREMISE_TAG "premises"
#define CONCLUSION_TAG "conclusions"
#define GOAL_ENTRY "goal"
#define SENTENCE_ENTRY "entry"
#define LINE_DATA "line"
#define TEXT_DATA "text"
#define RULE_DATA "rule"
#define REF_DATA "ref"
#define FILE_DATA "file"
#define DEPTH_DATA "d"
#define MODE_DATA "mode"

#define ALT_LINE_DATA "n"
#define ALT_TEXT_DATA "t"
#define ALT_RULE_DATA "l"
#define ALT_REF_DATA "r"
#define ALT_FILE_DATA "f"

// Convienence type casts

#define CSTD_CAST (const char *)
#define CXML_CAST (const xmlChar *)
#define CUNS_CAST (const unsigned char *)
#define STD_CAST (char *)
#define XML_CAST(o) ((xmlChar *)o)
#define UNS_CAST (unsigned char *)

int aio_save (proof_t * proof, const char * file_name);
proof_t * aio_open (const char * file_name);

#endif /* ARIS_A_IO_H */
