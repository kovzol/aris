/* The opening and saving functions.

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#if defined(__MACH__)
#include <stdlib.h>
#else
#include <malloc.h>
#endif

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
#define IF_FREE(p) if (p) free (p); p = NULL;
//#define PRINT_ALLOC() {struct mallinfo mal = mallinfo (); fprintf (stderr, "%i: blks == %i\n", __LINE__, mal.uordblks); }
#define PRINT_ALLOC()

#define IS_LINE(s) (!strcmp (CSTD_CAST (s),LINE_DATA) || !strcmp (CSTD_CAST (s),ALT_LINE_DATA))
#define IS_TEXT(s) (!strcmp (CSTD_CAST (s),TEXT_DATA) || !strcmp (CSTD_CAST (s),ALT_TEXT_DATA))
#define IS_RULE(s) (!strcmp (CSTD_CAST (s),RULE_DATA) || !strcmp (CSTD_CAST (s),ALT_RULE_DATA))
#define IS_REF(s) (!strcmp (CSTD_CAST (s),REF_DATA) || !strcmp (CSTD_CAST (s),ALT_REF_DATA))
#define IS_FILE(s) (!strcmp (CSTD_CAST (s),FILE_DATA) || !strcmp (CSTD_CAST (s),ALT_FILE_DATA))

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
    if (ret <= 0)
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
    if (ret <= 0)
        return NULL;

    *name = xmlTextReaderName (xml);
    if (!(*name))
        return NULL;

    buffer = xmlTextReaderValue (xml);
    return buffer;
}

///* Write the line number of a sentence object to an XML stream.
// *  input:
// *    xml - the XML stream to which to write.
// *    sd - the sentence data object that holds the information being written.
// *  output:
// *    the results of the write.
// */
int
aio_write_line_num (xmlTextWriter * xml, sen_data * sd)
{
    int ret;
    ret = xmlTextWriterWriteFormatAttribute (xml, XML_CAST(ALT_LINE_DATA),
                                            "%i", sd->line_num);
    if (ret < 0) XML_ERR (AEC_MEM);
    return ret;
}

///* Write the text of a sentence object to an XML stream.
// *  input:
// *    xml - the XML stream to which to write.
// *    sd - the sentence data object that holds the information being written.
// *  output:
// *    the results of the write.
// */
int
aio_write_text (xmlTextWriter * xml, sen_data * sd)
{
    int ret;
    ret = xmlTextWriterWriteFormatAttribute (xml, XML_CAST(ALT_TEXT_DATA),
                                            "%s", sd->text);
    if (ret < 0) XML_ERR (-1);
    return ret;
}

///* Write the rule of a sentence object to an XML stream.
// *  input:
// *    xml - the XML stream to which to write.
// *    sd - the sentence data object that holds the information being written.
// *  output:
// *    the results of the write.
// */
int
aio_write_rule (xmlTextWriter * xml, sen_data * sd)
{
    int ret;
    ret = xmlTextWriterWriteFormatAttribute (xml, XML_CAST(ALT_RULE_DATA),
                                            "%i", sd->rule);
    if (ret < 0) XML_ERR (-1);
    return ret;
}

///* Save a goal to an XML stream.
// *  input:
// *    xml - the XML stream to which to write.
// *    text - the goal text to be written.
// *  output:
// *    the results of the write.
// */
int
aio_save_goal (xmlTextWriter * xml, unsigned char * text)
{
    int ret;
    ret = xmlTextWriterStartElement (xml, XML_CAST(GOAL_ENTRY));
    if (ret < 0) XML_ERR (-1);

    ret = xmlTextWriterWriteFormatAttribute (xml, XML_CAST(ALT_TEXT_DATA),
                                            "%s", text);
    if (ret < 0) XML_ERR (-1);

    ret = xmlTextWriterEndElement (xml);
    if (ret < 0) XML_ERR (-1);

    return 0;
}

///* Opens a premise item from an XML stream.
// *  input:
// *    xml - the XML stream to read from.
// *  output:
// *    The new sentence data object, or NULL on error.
// */
sen_data *
aio_open_prem (xmlTextReader * xml)
{
    xmlChar * buffer, * name;
    int line_num = 0;
    unsigned char * text = NULL;
    sen_data * sd;
    int ret;

    buffer = aio_get_first_attribute (xml, &name);
    while (buffer && name)
    {
        if (IS_LINE(name))
        {
            IF_FREE (name);

            ret = sscanf (CSTD_CAST (buffer), "%i", &line_num);
            if (ret != 1) XML_ERR (NULL);

            free (buffer);
            buffer = aio_get_next_attribute (xml, &name);
            continue;
        }

        if (IS_TEXT(name))
        {
            IF_FREE (name);
            text = (unsigned char *) strdup (CSTD_CAST (buffer));

            free (buffer);
            buffer = aio_get_next_attribute (xml, &name);

            continue;
        }
    }

    sd = sen_data_init (line_num, -1, text, NULL, 1, NULL,
                       0, 0, NULL);
    if (!sd)
        return NULL;
    free (text);

    IF_FREE (buffer);
    return sd;
}

///* Opens a conclusion item from an XML stream.
// *  input:
// *    xml - the XML stream to read from.
// *  output:
// *    The new sentence data object, or NULL on error.
// */
sen_data *
aio_open_conc (xmlTextReader * xml)
{
    xmlChar * buffer, * name;
    int ret;
    int got_rule, got_refs, got_depth, got_file, got_text;
    sen_data * sd;
    int rule = -1, sen_depth = 0, line_num = 0;
    unsigned char * text, * file;
    text = file = NULL;
    short * refs = 0;
    int i;

    got_rule = got_refs = got_depth = got_file = got_text = 0;

    buffer = aio_get_first_attribute (xml, &name);

    while (buffer)
    {
        if (IS_LINE(name))
        {
            IF_FREE (name);

            ret = sscanf ((const char *) buffer, "%i", &line_num);
            if (ret != 1) XML_ERR (NULL);

            free (buffer);
            buffer = aio_get_next_attribute (xml, &name);
            continue;
        }

        if (IS_RULE (name))
        {
            IF_FREE (name);

            if (got_rule)
                XML_ERR (NULL);

            // check for current rule.

            ret = sscanf ((const char *) buffer, "%i", &rule);
            if (ret != 1)
                XML_ERR (NULL);

            got_rule = 1;
            free (buffer);
            buffer = aio_get_next_attribute (xml, &name);
            continue;
        }

        if (IS_REF (name))
        {
            IF_FREE (name);

            if (got_refs)
                XML_ERR (NULL);

            int num_refs;
            i = num_refs = 0;

            // strtok?

            for (; buffer[i]; i++)
                if (buffer[i] == ',')
                    num_refs ++;

            num_refs++;
            refs = (short *) calloc (num_refs + 1, sizeof (int));
            CHECK_ALLOC (refs, NULL);

            char * ref_str = strdup ((const char *) buffer);

            char * tok;
            tok = strtok (ref_str, ",");
            num_refs++;
            i = 0;

            while (tok)
            {
                short new_ref = (short) atoi(tok);
                refs[i++] = new_ref;
                tok = strtok (NULL, ",");
            }
            refs[i] = REF_END;

            free (ref_str);

            got_refs = 1;

            free (buffer);
            buffer = aio_get_next_attribute (xml, &name);
            continue;
        }

        if (!strcmp ((const char *) name, DEPTH_DATA))
        {
            IF_FREE (name);

            if (got_depth)
                XML_ERR (NULL);

            ret = sscanf ((const char *) buffer, "%i", &sen_depth);
            if (ret != 1)
                XML_ERR (NULL);

            got_depth = 1;

            free (buffer);
            buffer = aio_get_next_attribute (xml, &name);
            continue;
        }

        if (IS_FILE(name))
        {
            IF_FREE (name);
            if (got_file)
                XML_ERR (NULL);

            if (buffer[0] == '\0')
            {
                file = NULL;
            }
            else
            {
                file = (unsigned char *) strdup ((const char *) buffer);
                CHECK_ALLOC (file, NULL);
            }

            got_file = 1;

            free (buffer);
            buffer = aio_get_next_attribute (xml, &name);
            continue;
        }

        if (IS_TEXT(name))
        {
            IF_FREE (name);

            if (got_text)
                XML_ERR (NULL);

            text = (unsigned char *) strdup ((const char *)buffer);
            CHECK_ALLOC (text, NULL);

            got_text = 1;

            free (buffer);
            buffer = aio_get_next_attribute (xml, &name);
            continue;
        }
    }

    sd = sen_data_init (line_num, rule, text, refs, 0, file, 0, sen_depth, NULL);
    if (!sd)
        return NULL;

    IF_FREE (text);
    IF_FREE (refs);
    IF_FREE (file);

    return sd;
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
    if (!xml) XML_ERR (-1);

    ret = xmlTextWriterSetIndent (xml, 1);
    if (ret < 0) XML_ERR (-1);

    ret = xmlTextWriterStartDocument (xml, NULL, NULL, NULL);
    if (ret < 0) XML_ERR (-1);

    ret = xmlTextWriterStartElement (xml, XML_CAST(PROOF_TAG));
    if (ret < 0) XML_ERR (-1);

    char * mode;

    if (proof->boolean)
        mode = "boolean";
    else
        mode = "standard";

    ret = xmlTextWriterWriteFormatAttribute (xml, XML_CAST(MODE_DATA),
                                            "%s", mode);

    ret = xmlTextWriterWriteFormatAttribute (xml, XML_CAST (VER_DATA),
                                            "%1.1f", FILE_VER);

    ret = xmlTextWriterStartElement (xml, XML_CAST(GOAL_TAG));
    if (ret < 0) XML_ERR (-1);

    itr = proof->goals->head;

    for (itr = proof->goals->head; itr != NULL; itr = itr->next)
    {
        ret = aio_save_goal (xml, itr->value);
        if (ret == -1)
            return -1;
    }

    // End <goals> tag.
    ret = xmlTextWriterEndElement (xml);
    if (ret < 0) XML_ERR (-1);

    // Begin <premises> tag.
    ret = xmlTextWriterStartElement (xml, XML_CAST(PREMISE_TAG));
    if (ret < 0) XML_ERR (-1);

    for (itr = proof->everything->head; itr != NULL; itr = itr->next)
    {
        // Write each of the premises.
        sen_data * sd = itr->value;
        if (!sd->premise)
            break;

        ret = xmlTextWriterStartElement (xml, XML_CAST(SENTENCE_ENTRY));
        if (ret < 0) XML_ERR (-1);

        // Write the line number.
        ret = aio_write_line_num (xml, sd);
        if (ret == -1)
            return -1;

        ret = aio_write_text (xml, sd);
        if (ret == -1)
            return -1;

        ret = xmlTextWriterEndElement (xml);
        if (ret < 0) XML_ERR (-1);
    }

    ret = xmlTextWriterEndElement (xml);
    if (ret < 0)
    {
        XML_ERR (-1);
    }

    // Begin <conclusions> tag.
    ret = xmlTextWriterStartElement (xml, XML_CAST(CONCLUSION_TAG));
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

        ret = xmlTextWriterStartElement (xml, XML_CAST(SENTENCE_ENTRY));
        if (ret < 0) XML_ERR (-1);

        ret = aio_write_line_num (xml, sd);
        if (ret == -1)
            return -1;

        ret = aio_write_rule (xml, sd);
        if (ret == -1)
            return -1;

        if (sd->refs)
        {
            while (sd->refs[i] != REF_END)
            {
                max_line = (max_line > sd->refs[i]) ? max_line : sd->refs[i];
                num_refs++;
                i++;
            }
        }

        max_line = (max_line > 0) ? (int) log10 (max_line) + 1 : 0;

        refs = (char *) calloc (num_refs * (max_line + 1), sizeof (char));
        CHECK_ALLOC (refs, -1);

        i = 0;

        if (sd->refs)
        {
            while (sd->refs[i] != REF_END)
            {
                int ref_line = sd->refs[i];
                ref_off += sprintf (refs + ref_off, "%i", ref_line);

                if (sd->refs[i+1] != REF_END)
                    ref_off += sprintf (refs + ref_off, ",");
                i++;
            }
        }

        ret = xmlTextWriterWriteAttribute (xml, XML_CAST(ALT_REF_DATA),
                                          XML_CAST(refs));
        if (ret < 0) XML_ERR (-1);

        ret = xmlTextWriterWriteFormatAttribute (xml, XML_CAST(DEPTH_DATA),
                                                "%i", sd->depth);
        if (ret < 0) XML_ERR (-1);

        // If there is a file, write it.
        if (sd->file)
        {
            ret = xmlTextWriterWriteFormatAttribute (xml, XML_CAST(ALT_FILE_DATA),
                                                    "%s", sd->file);
            if (ret < 0) XML_ERR (-1);
        }

        ret = aio_write_text (xml, sd);
        if (ret == -1)
            return -1;

        ret = xmlTextWriterEndElement (xml);
        if (ret < 0) XML_ERR (-1);
    }

    // End <conclusions> tag.
    ret = xmlTextWriterEndElement (xml);
    if (ret < 0) XML_ERR (-1);

    // End <proof> tag.
    ret = xmlTextWriterEndElement (xml);
    if (ret < 0) XML_ERR (-1);

    // End the document.
    ret = xmlTextWriterEndDocument (xml);
    if (ret < 0) XML_ERR (-1);

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

//    proof_t * proof;
//    xmlTextReader * xml;

//    proof = proof_init ();
//    if (!proof)
//        return NULL;

//    xml = xmlReaderForFile (file_name, NULL, 0);
//    if (!xml) XML_ERR (NULL);

//    xmlChar * buffer, * name;
//    int ret;
//    int depth;

//    name = NULL;
//    ret = xmlTextReaderRead (xml);

//    if (ret < 0) XML_ERR (NULL);

//    buffer = xmlTextReaderName (xml);
//    if (!buffer) XML_ERR (NULL);

//    if (strcmp ((const char *) buffer, PROOF_TAG))
//        XML_ERR (NULL);

//    free (buffer);

//    buffer = aio_get_first_attribute (xml, &name);
//    if (buffer)
//    {
//        if (!strcmp (CSTD_CAST (name), MODE_DATA))
//        {
//            if (!strcmp ((const char *) buffer, "boolean"))
//                proof->boolean = 1;
//        }

//        if (!strcmp (CSTD_CAST (name), VER_DATA))
//        {
//            int ver_num;
//            sscanf (CSTD_CAST (buffer), "%i", &ver_num);
//            // Decide what to do with it.
//        }
//    }

//    IF_FREE (name);
//    IF_FREE (buffer);

//    // Get the <goals> tag.
//    ret = xmlTextReaderRead (xml);
//    if (ret < 0) XML_ERR (NULL);

//    ret = xmlTextReaderRead (xml);
//    if (ret < 0) XML_ERR (NULL);

//    buffer = xmlTextReaderName (xml);
//    if (!buffer) XML_ERR (NULL);

//    if (strcmp ((const char *) buffer, GOAL_TAG))
//        XML_ERR (NULL);

//    free (buffer);

//    ret = xmlTextReaderRead (xml);
//    if (ret < 0) XML_ERR (NULL);

//    depth = xmlTextReaderDepth (xml);
//    if (depth < 0) XML_ERR (NULL);

//    // Read the goals.

//    // May be able to use xmlTextReaderIsEmptyElement()
//    if (depth == 2)
//    {
//        while (ret == 1)
//        {
//            ret = xmlTextReaderRead (xml);
//            if (ret < 0) XML_ERR (NULL);

//            buffer = xmlTextReaderName (xml);
//            if (!buffer) XML_ERR (NULL);

//            if (!strcmp ((const char *) buffer, GOAL_ENTRY))
//            {
//                free (buffer);
//                buffer = aio_get_first_attribute (xml, &name);
//                if (!buffer || !IS_TEXT(name))
//                    XML_ERR (NULL);

//                IF_FREE (name);

//                item_t * ret_itm;
//                ret_itm = ls_push_obj (proof->goals, buffer);
//                if (!ret_itm)
//                    return NULL;
//            }
//            else if (!strcmp ((const char *) buffer, GOAL_TAG))
//            {
//                ret = xmlTextReaderRead (xml);
//                if (ret < 0) XML_ERR (NULL);
//                free (buffer);
//                break;
//            }
//            else
//                XML_ERR (NULL);

//            ret = xmlTextReaderRead (xml);
//            if (ret < 0) XML_ERR (NULL);
//        }
//    }

//    /*** Read the Premises. ***/

//    int line = 1;

//    ret = xmlTextReaderRead (xml);
//    if (ret < 0) XML_ERR (NULL);

//    buffer = xmlTextReaderName (xml);
//    if (!buffer) XML_ERR (NULL);

//    if (strcmp ((const char *) buffer, PREMISE_TAG))
//        XML_ERR (NULL);

//    free (buffer);

//    ret = xmlTextReaderRead (xml);
//    if (ret < 0) XML_ERR (NULL);

//    while (ret == 1)
//    {
//        ret = xmlTextReaderRead (xml);
//        if (ret < 0) XML_ERR (NULL);

//        buffer = xmlTextReaderName (xml);
//        if (!buffer) XML_ERR (NULL);

//        if (!strcmp ((const char *) buffer, SENTENCE_ENTRY))
//        {
//            free (buffer);
//            sen_data * sd;

//            sd = aio_open_prem (xml);
//            if (!sd)
//                return NULL;
//            sd->line_num = line++;

//            item_t * itm;
//            itm = ls_push_obj (proof->everything, sd);
//            if (!itm)
//                return NULL;
//        }
//        else if (!strcmp ((const char *) buffer, PREMISE_TAG))
//        {
//            ret = xmlTextReaderRead (xml);
//            if (ret < 0) XML_ERR (NULL);
//            free (buffer);
//            break;
//        }
//        else
//        {
//            XML_ERR (NULL);
//        }
//        ret = xmlTextReaderRead (xml);
//        if (ret < 0) XML_ERR (NULL);
//    }

//    ret = xmlTextReaderRead (xml);
//    if (ret < 0) XML_ERR (NULL);

//    buffer = xmlTextReaderName (xml);
//    if (!buffer) XML_ERR (NULL);

//    if (strcmp ((const char *) buffer, CONCLUSION_TAG))
//        XML_ERR (NULL);

//    free (buffer);

//    ret = xmlTextReaderRead (xml);
//    if (ret < 0) XML_ERR (NULL);

//    depth = xmlTextReaderDepth (xml);
//    if (depth < 0) XML_ERR (NULL);

//    if (depth == 2)
//    {
//        while (1)
//        {
//            ret = xmlTextReaderRead (xml);
//            if (ret < 0) XML_ERR (NULL);

//            buffer = xmlTextReaderName (xml);
//            if (!buffer) XML_ERR (NULL);

//            if (!strcmp ((const char *) buffer, CONCLUSION_TAG))
//            {
//                free (buffer);
//                break;
//            }

//            int str_cmp = strcmp ((const char *) buffer, SENTENCE_ENTRY);
//            free (buffer);

//            if (str_cmp)
//                continue;

//            sen_data * sd;

//            sd = aio_open_conc (xml);
//            if (!sd)
//                XML_ERR (NULL);

//            int sub = 0, old_depth;
//            old_depth = ((sen_data *) proof->everything->tail->value)->depth;
//            if (sd->depth > old_depth)
//                sub = 1;

//            sd->subproof = sub;
//            sd->line_num = line++;

//            item_t * itm;
//            itm = ls_push_obj (proof->everything, sd);
//            if (!itm)
//                return NULL;
//        }
//    }

//    xmlFreeTextReader (xml);

//    return proof;
}
