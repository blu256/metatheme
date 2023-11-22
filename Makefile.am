EXTRA_DIST = CREDITS Makefile.base autogen.sh metathemerc-sample \
             docs/reference.xml 

SUBDIRS = metatheme themes toolkits colors @MAIN_SUBDIRS@

metathemedir = $(METATHEMEDIR)
metatheme_DATA = metathemerc-sample

bin_SCRIPTS = metatheme-install

dist-hook:
	cp -pr $(srcdir)/docs/html $(srcdir)/docs/images $(distdir)/docs
	mkdir $(distdir)/kde
	cp -p $(srcdir)/kde/*.kcsrc $(distdir)/kde
