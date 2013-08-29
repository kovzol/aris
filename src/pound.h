/* Several convenient pound defines.

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

#ifndef ARIS_POUND_H
#define ARIS_POUND_H

// Most of these are fairly self explanatory.

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#ifdef gtk_widget_override_font
#define LABEL_SET_FONT(l,f) gtk_widget_override_font (l, f);
#define ENTRY_SET_FONT(e,f) gtk_widget_override_font (e, f);
#else
#define LABEL_SET_FONT(l,f) gtk_widget_modify_font (l, f);
#define ENTRY_SET_FONT(e,f) gtk_widget_modify_font (e, f);
#endif

// Fonts
#define FONT_TYPE PangoFontDescription *
#define INIT_FONT(f,s) f = pango_font_description_new ();  \
  pango_font_description_set_family (f, "Arial"); \
  pango_font_description_set_variant (f, PANGO_VARIANT_NORMAL); \
  pango_font_description_set_style (f, PANGO_STYLE_NORMAL);  \
  pango_font_description_set_size (f, s * PANGO_SCALE);
#define FONT_GET_SIZE(f,s) s = pango_font_description_get_size (f) / PANGO_SCALE;

#ifdef GdkRGBA
#define COLOR_TYPE GdkRGBA *
#define INIT_COLOR(c,r,g,b) c = (GdkRGBA *) calloc (1, sizeof (GdkRGBA)); \
  c->red = (double) r / 255.0; \
  c->green = (double) g / 255.0; \
  c->blue = (double) b / 255.0; \
  c->alpha = (double) 1.0;
#define INVERT(c,n) n = (GdkColor *) calloc (1, sizeof (GdkRGBA));\
  n->red = (double) 1.0 - c->red; \
  n->green = (double) 1.0 - c->green; \
  n->blue = (dobule) 1.0 - c->blue; \
  n->alpha = (double) 1.0;

#else
#define COLOR_TYPE GdkColor *
#define INIT_COLOR(c,r,g,b) c = (GdkColor *) calloc (1, sizeof (GdkColor)); \
  c->red = r * r; \
  c->green = g * g; \
  c->blue = b * b;
#define INVERT(c,n) n = (GdkColor *) calloc (1, sizeof (GdkColor)); \
  n->red = c->red ^ 0xffff; \
  n->green = c->green ^ 0xffff; \
  n->blue = c->blue ^ 0xffff;
#endif

#endif  /*  ARIS_POUND_H  */
