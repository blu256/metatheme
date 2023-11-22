include $(top_srcdir)/Makefile.base

INCLUDES += $(GTK2_CFLAGS)

noinst_HEADERS = about.h color_scheme.h colors.h fonts.h util.h main.h preview.h themes.h settings.h

bin_PROGRAMS = mt-config

mt_config_SOURCES = about.c color_scheme.c colors.c fonts.c util.c main.c preview.c themes.c settings.c
mt_config_LDFLAGS = $(GTK2_CFLAGS)
mt_config_LDADD = $(GTK2_LIBS) $(CAIRO_LIBS) -L$(top_srcdir)/metatheme -lmetatheme
