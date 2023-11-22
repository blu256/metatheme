include $(top_srcdir)/Makefile.base

INCLUDES += $(GTK2_CFLAGS) $(CAIRO_CFLAGS)

noinst_HEADERS = mt_gtk2.h mt_gtk2_patches.h mt_gtk2_style.h

toolkitdir = $(METATHEMEDIR)/toolkits
toolkit_LTLIBRARIES = libmt_gtk2.la

libmt_gtk2_la_SOURCES = mt_gtk2.c mt_gtk2_style.c mt_gtk2_draw.c mt_gtk2_patches.c
libmt_gtk2_la_LDFLAGS = -avoid-version -module $(GTK2_CFLAGS)
libmt_gtk2_la_LIBADD = $(GTK2_LIBS) $(CAIRO_LIBS) -L$(top_srcdir)/metatheme -lmetatheme

gtkrcdir = $(GTK2_PREFIX)/share/themes/MetaTheme/gtk-2.0
gtkrc_DATA = gtkrc

CLEANFILES = gtkrc

gtkrc:
	@( \
		echo "style \"metatheme\""; \
		echo "{"; \
		echo "	engine \"$(libdir)/metatheme/toolkits/libmt_gtk2.so\" {}"; \
		echo "}"; \
		echo "widget_class \"*\" style \"metatheme\"" \
	) > $(srcdir)/gtkrc
