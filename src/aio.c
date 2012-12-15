/* The opening and saving functions.

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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlreader.h>

#include "aio.h"
#include "var.h"
#include "sen-data.h"
#include "proof.h"
#include "list.h"
#include "rules.h"
#include "process.h"

#define XML_ERR(r) {fprintf (stderr, "XML Error\n"); return r;}
#define PRINT_LINE() {printf ("%i\n", __LINE__);}

/* Gets the first attribute from an xml stream.
 *  input:
 *    xml - the xml stream to get the attribute from.
 *    name - receives the name of the attribute.
 *  output:
 *    the attribute data.
 */
static xmlChar *
aio_get_first_attribute (xmlTextReader * xml, xmlChar ** name)
{
  int ret;
  xmlChar * buffer;

  ret = xmlTextReaderMoveToFirstAttribute (xml);
  if (ret < 0)
    return NULL;

  *name = xmlTextReaderName (xml);
  if (!(*name))
    return NULL;

  buffer = xmlTextReaderValue (xml);
  return buffer;
}

/* Gets the next attribute from an xml stream.
 *  input:
 *    xml - the xml stream to get the attribute from.
 *    data - the data to compare the tag against.
 *  output:
 *    the attribute data.
 */
static xmlChar *
aio_get_next_attribute (xmlTextReader * xml, xmlChar ** name)
{
  int ret;
  xmlChar * buffer;

  ret = xmlTextReaderMoveToNextAttribute (xml);
  if (ret < 0)
    return NULL;

  *name = xmlTextReaderName (xml);
  if (!(*name))
    return NULL;

  buffer = xmlTextReaderValue (xml);
  return buffer;
}

/* Saves a proof to a file.
 *  input:
 *    proof - the proof to save.
 *    file_name - the name of the file to save to.
 *  output:
 *    0 on success, -1 on error.
 */
int
aio_save (proof_t * proof, const char * file_name)
{
  xmlTextWriter * xml;
  int ret;
  item_t * itr;

  xml = xmlNewTextWriterFilename (file_name, 0);
  if (!xml)
    {
      XML_ERR (-1);
    }

  ret = xmlTextWriterSetIndent (xml, 1);
  if (ret < 0)
    {
      XML_ERR (-1);
    }

  ret = xmlTextWriterStartDocument (xml, NULL, NULL, NULL);
  if (ret < 0)
    {
      XML_ERR (-1);
    }

  ret = xmlTextWriterStartElement (xml, XML_CAST PROOF_TAG);
  if (ret < 0)
    {
      XML_ERR (-1);
    }

  char * mode;

  if (proof->boolean)
    mode = "boolean";
  else
    mode = "standard";

  ret = xmlTextWriterWriteFormatAttribute (xml, XML_CAST MODE_DATA,
					   "%s", mode);

  ret = xmlTextWriterStartElement (xml, XML_CAST GOAL_TAG);
  if (ret < 0)
    {
      XML_ERR (-1);
    }

  itr = proof->goals->head;

  for (itr = proof->goals->head; itr != NULL; itr = itr->next)
    {
      unsigned char * text = itr->value;
      ret = xmlTextWriterStartElement (xml, XML_CAST GOAL_ENTRY);
      if (ret < 0)
	{
	  XML_ERR (-1);
	}

      ret = xmlTextWriterWriteFormatAttribute (xml, XML_CAST TEXT_DATA,
					       "%s", text);
      if (ret < 0)
	{
	  XML_ERR (-1);
	}

      ret = xmlTextWriterEndElement (xml);
      if (ret < 0)
	{
	  XML_ERR (-1);
	}
    }

  // End <variables> tag.
  ret = xmlTextWriterEndElement (xml);
  if (ret < 0)
    {
      XML_ERR (-1);
    }

  // Begin <premises> tag.
  ret = xmlTextWriterStartElement (xml, XML_CAST PREMISE_TAG);
  if (ret < 0)
    {
      XML_ERR (-1);
    }

  for (itr = proof->everything->head; itr != NULL; itr = itr->next)
    {
      // Write each of the premises.
      sen_data * sd = itr->value;
      if (!sd->premise)
	break;

      ret = xmlTextWriterStartElement (xml, XML_CAST SENTENCE_ENTRY);
      if (ret < 0)
	{
	  XML_ERR (-1);
	}

      // Write the line number.
      ret = xmlTextWriterWriteFormatAttribute (xml, XML_CAST LINE_DATA,
					       "%i", sd->line_num);
      if (ret < 0)
	{
	  XML_ERR (-1);
	}

      ret = xmlTextWriterWriteFormatAttribute (xml, XML_CAST TEXT_DATA,
					       "%s", sd->text);
      if (ret < 0)
	{
	  XML_ERR (-1);
	}

      
      ret = xmlTextWriterEndElement (xml);
      if (ret < 0)
	{
	  XML_ERR (-1);
	}
    }

  ret = xmlTextWriterEndElement (xml);
  if (ret < 0)
    {
      XML_ERR (-1);
    }

  // Begin <conclusions> tag.
  ret = xmlTextWriterStartElement (xml, XML_CAST CONCLUSION_TAG);
  if (ret < 0)
    {
      XML_ERR (-1);
    }

  for (; itr != NULL; itr = itr->next)
    {
      // Wriet each of the conclusions.
      sen_data * sd = itr->value;
      char * refs;
      int i = 0, num_refs = 0, ref_off = 0, max_line = 0;

      ret = xmlTextWriterStartElement (xml, XML_CAST SENTENCE_ENTRY);
      if (ret < 0)
	{
	  XML_ERR (-1);
	}
      ret = xmlTextWriterWriteFormatAttribute (xml, XML_CAST LINE_DATA,
					       "%i", sd->line_num);
      if (ret < 0)
	{
	  XML_ERR (-1);
	}
      ret = xmlTextWriterWriteFormatAttribute (xml, XML_CAST RULE_DATA,
					       "%i", sd->rule);
      if (ret < 0)
	{
	  XML_ERR (-1);
	}

      while (sd->refs[i] != -1)
	{
	  max_line = (max_line > sd->refs[i]) ? max_line : sd->refs[i];
	  num_refs++;
	  i++;
	}

      max_line = (int) log10 (max_line) + 1;

      refs = (char *) calloc (num_refs * (max_line + 1), sizeof (char));
      if (!refs)
	{
	  REPORT ();
	  perror (NULL);
	  return -1;
	}

      i = 0;

      while (sd->refs[i] != -1)
	{
	  int ref_line = sd->refs[i];
	  ref_off += sprintf (refs + ref_off, "%i", ref_line);

	  if (sd->refs[i+1] != -1)
	    ref_off += sprintf (refs + ref_off, ",");
	  i++;
	}

      ret = xmlTextWriterWriteAttribute (xml, XML_CAST REF_DATA,
					 XML_CAST refs);
      if (ret < 0)
	{
	  XML_ERR (-1);
	}

      ret = xmlTextWriterWriteFormatAttribute (xml, XML_CAST DEPTH_DATA,
					       "%i", sd->depth);
      if (ret < 0)
	{
	  XML_ERR (-1);
	}

      // If there is a file, write it.
      if (sd->file)
	{
	  ret = xmlTextWriterWriteFormatAttribute (xml, XML_CAST FILE_DATA,
					     "%s", sd->file);
	  if (ret < 0)
	    {
	      XML_ERR (-1);
	    }
	}

      ret = xmlTextWriterWriteFormatAttribute (xml, XML_CAST TEXT_DATA,
					       "%s", sd->text);
      if (ret < 0)
	{
	  XML_ERR (-1);
	}

      ret = xmlTextWriterEndElement (xml);
      if (ret < 0)
	{
	  XML_ERR (-1);
	}
    }

  // End <conclusions> tag.
  ret = xmlTextWriterEndElement (xml);
  if (ret < 0)
    {
      XML_ERR (-1);
    }

  // End <proof> tag.
  ret = xmlTextWriterEndElement (xml);
  if (ret < 0)
    {
      XML_ERR (-1);
    }

  // End the document.
  ret = xmlTextWriterEndDocument (xml);
  if (ret < 0)
    {
      XML_ERR (-1);
    }

  xmlFreeTextWriter (xml);

  return 0;
}

/* Opens a proof.
 *  input:
 *    file_name - the name of the file to open.
 *  output:
 *    the opened proof, or NULL on error.
 */
proof_t *
aio_open (const char * file_name)
{
  if (file_name == NULL)
    return NULL;

  proof_t * proof;
  xmlTextReader * xml;

  proof = proof_init ();
  if (!proof)
    return NULL;

  xml = xmlReaderForFile (file_name, NULL, 0);
  if (!xml)
    {
      XML_ERR (NULL);
    }

  xmlChar * buffer, * name;
  int ret;
  int depth;

  name = NULL;
  ret = xmlTextReaderRead (xml);

  if (ret < 0)
    {
      XML_ERR (NULL);
    }

  buffer = xmlTextReaderName (xml);
  if (!buffer)
    {
      XML_ERR (NULL);
    }

  if (strcmp ((const char *) buffer, PROOF_TAG))
    {
      XML_ERR (NULL);
    }

  free (buffer);

  buffer = aio_get_first_attribute (xml, &name);
  if (buffer)
    {
      if (!strcmp (name, MODE_DATA))
	{
	  if (!strcmp (buffer, "boolean"))
	    proof->boolean = 1;
	}
    }

  if (name)  free (name);

  // Get the <goals> tag.
  ret = xmlTextReaderRead (xml);
  if (ret < 0)
    {
      XML_ERR (NULL);
    }

  ret = xmlTextReaderRead (xml);
  if (ret < 0)
    {
      XML_ERR (NULL);
    }

  buffer = xmlTextReaderName (xml);
  if (!buffer)
    {
      XML_ERR (NULL);
    }

  if (strcmp ((const char *) buffer, GOAL_TAG))
    {
      XML_ERR (NULL);
    }

  free (buffer);

  ret = xmlTextReaderRead (xml);
  if (ret < 0)
    {
      XML_ERR (NULL);
    }

  depth = xmlTextReaderDepth (xml);
  if (depth < 0)
    {
      XML_ERR (NULL);
    }

  // Read the goals.

  // May be able to use xmlTextReaderIsEmptyElement()
  if (depth == 2)
    {
      while (ret == 1)
	{
	  ret = xmlTextReaderRead (xml);
	  if (ret < 0)
	    {
	      XML_ERR (NULL);
	    }

	  buffer = xmlTextReaderName (xml);
	  if (!buffer)
	    {
	      XML_ERR (NULL);
	    }

	  if (!strcmp ((const char *) buffer, GOAL_ENTRY))
	    {
	      buffer = aio_get_first_attribute (xml, &name);
	      if (!buffer
		  || (strcmp (name, TEXT_DATA) && strcmp (name, ALT_TEXT_DATA)))
		{
		  XML_ERR (NULL);
		}

	      if (name)  free (name);

	      item_t * ret_itm;
	      ret_itm = ls_push_obj (proof->goals, buffer);
	      if (!ret_itm)
		return NULL;
	    }
	  else if (!strcmp ((const char *) buffer, GOAL_TAG))
	    {
	      ret = xmlTextReaderRead (xml);
	      if (ret < 0)
		{
		  XML_ERR (NULL);
		}
	      break;
	    }
	  else
	    {
	      XML_ERR (NULL);
	    }

	  ret = xmlTextReaderRead (xml);
	  if (ret < 0)
	    {
	      XML_ERR (NULL);
	    }
	}
    }

  int line = 1;

  ret = xmlTextReaderRead (xml);
  if (ret < 0)
    {
      XML_ERR (NULL);
    }

  buffer = xmlTextReaderName (xml);
  if (!buffer)
    {
      XML_ERR (NULL);
    }

  if (strcmp ((const char *) buffer, PREMISE_TAG))
    {
      XML_ERR (NULL);
    }

  free (buffer);

  ret = xmlTextReaderRead (xml);
  if (ret < 0)
    {
      XML_ERR (NULL);
    }

  while (ret == 1)
    {
      ret = xmlTextReaderRead (xml);
      if (ret < 0)
	{
	  XML_ERR (NULL);
	}

      buffer = xmlTextReaderName (xml);
      if (!buffer)
	{
	  XML_ERR (NULL);
	}

      if (!strcmp ((const char *) buffer, SENTENCE_ENTRY))
	{
	  sen_data * sd;
	  int line_num;

	  buffer = aio_get_first_attribute (xml, &name);
	  if (!buffer
	      || (strcmp (name, LINE_DATA) && strcmp (name, ALT_LINE_DATA)))
	    {
	      XML_ERR (NULL);
	    }

	  if (name)  free (name);

	  ret = sscanf ((const char *) buffer, "%i", &line_num);
	  if (ret != 1)
	    {
	      XML_ERR (NULL);
	    }

	  free (buffer);

	  buffer = aio_get_next_attribute (xml, &name);
	  if (!buffer
	      || (strcmp (name, TEXT_DATA) && strcmp (name, ALT_TEXT_DATA)))
	    {
	      XML_ERR (NULL);
	    }

	  if (name)  free (name);

	  sd = sen_data_init (line++, -1, buffer, NULL, 1, NULL,
			      0, 0, NULL);
	  if (!sd)
	    return NULL;

	  item_t * itm;
	  itm = ls_push_obj (proof->everything, sd);
	  if (!itm)
	    return NULL;
	}
      else if (!strcmp ((const char *) buffer, PREMISE_TAG))
	{
	  ret = xmlTextReaderRead (xml);
	  if (ret < 0)
	    {
	      XML_ERR (NULL);
	    }
	  break;
	}
      else
	{
	  XML_ERR (NULL);
	}
      ret = xmlTextReaderRead (xml);
      if (ret < 0)
	{
	  XML_ERR (NULL);
	}
    }

  ret = xmlTextReaderRead (xml);
  if (ret < 0)
    {
      XML_ERR (NULL);
    }

  buffer = xmlTextReaderName (xml);
  if (!buffer)
    {
      XML_ERR (NULL);
    }

  if (strcmp ((const char *) buffer, CONCLUSION_TAG))
    {
      XML_ERR (NULL);
    }

  free (buffer);

  ret = xmlTextReaderRead (xml);
  if (ret < 0)
    {
      XML_ERR (NULL);
    }

  depth = xmlTextReaderDepth (xml);
  if (depth < 0)
    {
      XML_ERR (NULL);
    }

  if (depth == 2)
    {
      while (1)
	{
	  ret = xmlTextReaderRead (xml);
	  if (ret < 0)
	    {
	      XML_ERR (NULL);
	    }

	  buffer = xmlTextReaderName (xml);
	  if (!buffer)
	    {
	      XML_ERR (NULL);
	    }

	  if (!strcmp ((const char *) buffer, CONCLUSION_TAG))
	    {
	      //printf ("Got Closing conclusion tag\n");
	      break;
	    }

	  if (!strcmp ((const char *) buffer, SENTENCE_ENTRY))
	    {
	      int attrs;
	      sen_data * sd;
	      int line_num, rule, * refs, i, ref_len, num_refs, sen_depth;
	      unsigned char * file, * text;

	      rule = -1;
	      sen_depth = 0;
	      file = text = NULL;

	      attrs = xmlTextReaderAttributeCount (xml);
	      if (attrs < 0)
		{
		  XML_ERR (NULL);
		}

	      // Get the line number.

	      buffer = aio_get_first_attribute (xml, &name);
	      if (!buffer
		  || (strcmp (name, LINE_DATA) && strcmp (name, ALT_LINE_DATA)))
		{
		  XML_ERR (NULL);
		}

	      if (name)  free (name);
	      name = NULL;

	      ret = sscanf ((const char *) buffer, "%i", &line_num);
	      if (ret != 1)
		{
		  XML_ERR (NULL);
		}

	      // Get the rule index.

	      int j;
	      int got_rule, got_text, got_refs, got_file, got_depth;

	      got_rule = got_text = got_refs = got_file = got_depth = 0;

	      while (xmlTextReaderMoveToNextAttribute (xml))
		{
		  if (name)  free (name);
		  name = NULL;

		  name = xmlTextReaderName (xml);
		  if (!name)
		    XML_ERR (NULL);

		  buffer = xmlTextReaderValue (xml);
		  if (!buffer)
		    XML_ERR (NULL);

		  if (!strcmp (name, RULE_DATA) || !strcmp (name, ALT_RULE_DATA))
		    {
		      if (got_rule)
			XML_ERR (NULL);

		      ret = sscanf ((const char *) buffer, "%i", &rule);
		      if (ret != 1)
			XML_ERR (NULL);

		      got_rule = 1;
		      continue;
		    }

		  if (!strcmp (name, REF_DATA) || !strcmp (name, ALT_REF_DATA))
		    {
		      if (got_refs)
			XML_ERR (NULL);

		      i = num_refs = 0;
		      ref_len = strlen ((const char *) buffer);

		      for (; i < ref_len; i++)
			if (buffer[i] == ',')
			  num_refs ++;

		      num_refs++;
		      refs = (int *) calloc (num_refs, sizeof (int));
		      CHECK_ALLOC (refs, NULL);

		      int old_i = -1, k = 0;

		      for (i = 0; i < ref_len; i++)
			{
			  if (buffer[i] == ',')
			    {
			      ret = sscanf ((const char *) buffer + old_i + 1, "%i",
					    &(refs[k]));
			      if (ret != 1)
				{
				  XML_ERR (NULL);
				}

			      k++;
			      old_i = i;
			    }
			}

		      ret = sscanf ((const char *) buffer + old_i + 1,
				    "%i", &(refs[k]));
		      if (ret < 0 && ret != EOF)
			{
			  XML_ERR (NULL);
			}

		      refs[k+1] = -1;

		      got_refs = 1;
		      continue;
		    }

		  if (!strcmp (name, DEPTH_DATA))
		    {
		      if (got_depth)
			XML_ERR (NULL);

		      ret = sscanf (buffer, "%i", &sen_depth);
		      if (ret != 1)
			XML_ERR (NULL);

		      got_depth = 1;
		      continue;
		    }

		  if (!strcmp (name, FILE_DATA) || !strcmp (name, ALT_FILE_DATA))
		    {
		      if (got_file)
			XML_ERR (NULL);

		      if (buffer[0] == '\0')
			file = NULL;
		      else
			file = buffer;

		      got_file = 1;
		      continue;
		    }

		  if (!strcmp (name, TEXT_DATA) || !strcmp (name, ALT_TEXT_DATA))
		    {
		      if (got_text)
			XML_ERR (NULL);

		      text = buffer;

		      got_text = 1;
		      continue;
		    }
		}

	      if (name)  free (name);
	      name = NULL;

	      int sub = 0, old_depth;
	      old_depth = ((sen_data *) proof->everything->tail->value)->depth;
	      if (sen_depth > old_depth)
		sub = 1;

	      sd = sen_data_init (line++, rule, text, refs, 0,
				  file, sub, sen_depth, NULL);
	      if (!sd)
		XML_ERR (NULL);

	      item_t * itm;
	      itm = ls_push_obj (proof->everything, sd);
	      if (!itm)
		return NULL;
	    }
	}
    }

  xmlFreeTextReader (xml);

  return proof;
}
