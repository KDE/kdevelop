htmldir = $(kde_htmldir)/en/gideon/kde2book
indexdir = ${kde_datadir}/gideon/docindices
index_DATA = kde2book.index

kde2book.index:
	echo "<!DOCTYPE gideonindex>" > kde2book.index ; \
	echo "<gideonindex>" >> kde2book.index ; \
	echo "<title>KDE2 Development Book</title>" >> kde2book.index ; \
	echo "<base href=\"$(htmldir)\"/>" >> kde2book.index ; \
	echo "<conceptindex>" >> kde2book.index ; \
	sort < kde2book.index.in | uniq >> kde2book.index ; \
	echo "</conceptindex>" >> kde2book.index ; \
	echo "</gideonindex>" >> kde2book.index ; \
	echo "" >> kde2book.index

EXTRA_DIST = kde2book.index.in

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(htmldir) ; \
	for file in *.html ; do $(INSTALL_DATA) $$file $(DESTDIR)$(htmldir)/$$file ; done ; \
	$(mkinstalldirs) $(DESTDIR)$(htmldir)/graphics ; \
	for file in graphics/*.gif ; do $(INSTALL_DATA) $$file $(DESTDIR)$(htmldir)/$$file ; done ;

dist-hook:
	mkdir -p $(distdir) ; \
	for file in $(srcdir)/*.html ; do cp -p $$file $(distdir) ; done

index:
	rm -f indextmp.xml kde2book.index ; \
	xsltproc ../tools/index.xsl main.xml > kde2book.index.in

