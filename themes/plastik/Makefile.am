include $(top_srcdir)/Makefile.base

EXTRA_DIST = themerc plastikxp

themedir = $(METATHEMEDIR)/themes/plastik
theme_DATA = themerc
theme_LTLIBRARIES = libmt_plastik.la

libmt_plastik_la_SOURCES = plastik.c
libmt_plastik_la_LDFLAGS = -avoid-version -module

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(METATHEMEDIR)/themes/plastikxp
	$(INSTALL_DATA) plastikxp $(DESTDIR)$(METATHEMEDIR)/themes/plastikxp/themerc

uninstall-local:
	rm -f $(DESTDIR)$(METATHEMEDIR)/themes/plastikxp/themerc
	-rmdir $(DESTDIR)$(METATHEMEDIR)/themes/plastikxp
