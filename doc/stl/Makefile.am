indexdir = ${kde_datadir}/gideon/docindices
index_DATA = stl.index

htmldir = $(kde_htmldir)/en/gideon/stl

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(htmldir) ; \
	for file in *.html *.gif; do $(INSTALL_DATA) $$file $(DESTDIR)$(htmldir)/$$file ; done

dist-hook:
	mkdir -p $(distdir) ; \
	for file in $(srcdir)/*.html $(srcdir)/*.gif ; do cp -p $$file $(distdir) ; done

stl.index: stl.index.in
	@rm -f stl.index ; \
	echo "<!DOCTYPE gideonindex>" > stl.index ; \
	echo "<gideonindex>" >> stl.index ; \
	echo "<title>STL</title>" >> stl.index ; \
	echo "<base href=\"$(htmldir)\"/>" >> stl.index ; \
	cat stl.index.in >> stl.index ; \
	echo "</gideonindex>" >> stl.index

