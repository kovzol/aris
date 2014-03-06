# This is part of Aris.

#  Copyright (C) 2012 Ian Dunn.

#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.

#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.

#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.


bin_PROGRAMS = aris

AM_CFLAGS = $(CFLAGS) $(GTK_CFLAGS) $(XML_CFLAGS)
AM_CFLAGS += -DARIS_GUI
aris_LDADD = $(LDFLAGS) $(GTK_LIBS) $(XML_LIBS)
aris_LDADD += -lm


if WIN32
AM_CFLAGS += \
        -mms-bitfields

aris_LDFLAGS = \
        -Wl,-subsystem,windows
endif

XPM_SOURCES = \
	and-conn.xpm	\
	bic-conn.xpm	\
	con-conn.xpm	\
	ctr-conn.xpm	\
	elm-conn.xpm	\
	exl-conn.xpm	\
	icon.xpm	\
	nil-conn.xpm	\
	not-conn.xpm	\
	or-conn.xpm	\
	tau-conn.xpm	\
	unv-conn.xpm

aris_SOURCES = $(XPM_SOURCES)

aris_SOURCES +=			\
	aio.h			\
	aio.c			\
	app.h			\
	app.c			\
	aris-proof.h		\
	aris-proof.c		\
	callbacks.h		\
	callbacks.c		\
	conf-file.h		\
	conf-file.c		\
	goal.h			\
	goal.c			\
	interop-isar.h		\
	interop-isar.c		\
	list.h 			\
	list.c			\
	menu.h			\
	menu.c			\
	pound.h			\
	process.h		\
	process.c		\
	process-main.c		\
	proof.h			\
	proof.c			\
	rules.h			\
	rules-table.h		\
	rules-table.c		\
	sen-data.h		\
	sen-data.c		\
	sen-parent.h		\
	sen-parent.c		\
	sentence.h		\
	sentence.c		\
	sexpr-process.h		\
	sexpr-process.c		\
	sexpr-process-bool.c  	\
	sexpr-process-equiv.c	\
	sexpr-process-infer.c	\
	sexpr-process-misc.c  	\
	sexpr-process-quant.c	\
	typedef.h		\
	undo.c			\
	undo.h			\
	var.h 			\
	var.c			\
	vec.h			\
	vec.c

aris_SOURCES += aris.c
