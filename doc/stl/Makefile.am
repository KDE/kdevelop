htmldir = $(kde_htmldir)/en/kdevdoctreeview/stl

tocdir = ${kde_datadir}/kdevdoctreeview/tocs
toc_DATA = stl.toc

indexdir = ${kde_datadir}/kdevdoctreeview/indices
index_DATA = stl.index

stl.toc: $(srcdir)/stl.toc.in
	echo "<!DOCTYPE gideontoc>" > stl.toc ; \
	echo "<gideontoc>" >> stl.toc ; \
	echo "<title>STL</title>" >> stl.toc ; \
	echo "<base href=\"$(htmldir)\"/>" >> stl.toc ; \
	cat $(srcdir)/stl.toc.in >> stl.toc
	echo "</gideontoc>" >> stl.toc ; \
	echo "" >> stl.toc

stl.index: $(srcdir)/stl.index.in
	echo "<!DOCTYPE gideonindex>" > stl.index ; \
	echo "<gideonindex>" >> stl.index ; \
	echo "<title>STL</title>" >> stl.index ; \
	echo "<base href=\"$(htmldir)\"/>" >> stl.index ; \
	cat $(srcdir)/stl.index.in >> stl.index ; \
	echo "</gideonindex>" >> stl.index

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(htmldir) ;
#	$(INSTALL_DATA) $(srcdir)/*.gif $(DESTDIR)$(htmldir)
# 	$(INSTALL_DATA) $(srcdir)/*.html $(DESTDIR)$(htmldir) ;

dist-hook:
	mkdir -p $(distdir) ; \
	for file in $(srcdir)/*.html $(srcdir)/*.gif ; do cp -p $$file $(distdir) ; done
