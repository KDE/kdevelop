KDE_LANG = en
KDE_DOCS = AUTO

htmldir = $(kde_htmldir)/en/kdevdoctreeview/kde2book

tocdir = ${kde_datadir}/kdevdoctreeview/tocs
toc_DATA = kde2book.toc

indexdir = ${kde_datadir}/kdevdoctreeview/indices
index_DATA = kde2book.index

EXTRA_DIST = kde2book.toc.in kde2book.index.in
DISTCLEANFILES = kde2book.toc kde2book.index

kde2book.toc: $(srcdir)/kde2book.toc.in
	echo "<!DOCTYPE kdeveloptoc>" > kde2book.toc ; \
	echo "<kdeveloptoc>" >> kde2book.toc ; \
	echo "<title>KDE2 Development Book</title>" >> kde2book.toc ; \
	echo "<base href=\"$(htmldir)\"/>" >> kde2book.toc ; \
	cat $(srcdir)/kde2book.toc.in >> kde2book.toc
	echo "</kdeveloptoc>" >> kde2book.toc ; \
	echo "" >> kde2book.toc

kde2book.index: $(srcdir)/kde2book.index.in
	echo "<!DOCTYPE kdevelopindex>" > kde2book.index ; \
	echo "<kdevelopindex>" >> kde2book.index ; \
	echo "<title>KDE2 Development Book</title>" >> kde2book.index ; \
	echo "<base href=\"$(htmldir)\"/>" >> kde2book.index ; \
	echo "<conceptindex>" >> kde2book.index ; \
	sort < $(srcdir)/kde2book.index.in | uniq >> kde2book.index ; \
	echo "</conceptindex>" >> kde2book.index ; \
	echo "</kdevelopindex>" >> kde2book.index ; \
	echo "" >> kde2book.index



#install-data-local:
#	$(mkinstalldirs) $(DESTDIR)$(htmldir) ; \
#	for file in *.html ; do $(INSTALL_DATA) $$file $(DESTDIR)$(htmldir)/$$file ; done ; \
#	$(mkinstalldirs) $(DESTDIR)$(htmldir)/graphics ; \
#	for file in graphics/*.gif ; do $(INSTALL_DATA) $$file $(DESTDIR)$(htmldir)/$$file ; done
#
#dist-hook:
#	mkdir -p $(distdir) ; \
#	for file in $(srcdir)/*.html ; do cp -p $$file $(distdir) ; done

index:
	rm -f indextmp.xml kde2book.index ; \
	xsltproc ../tools/index.xsl main.xml > kde2book.index.in

toc:
	perl ../tools/toc-docbook index.html > kde2book.toc.in
