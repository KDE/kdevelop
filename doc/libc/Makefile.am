htmldir = $(kde_htmldir)/en/libc

tocdir = ${kde_datadir}/kdevdoctreeview/tocs
toc_DATA = libc.toc

indexdir = ${kde_datadir}/kdevdoctreeview/indices
index_DATA = libc.index

EXTRA_DIST = libc.toc.in libc.index.in
DISTCLEANFILES = libc.toc libc.index

libc.toc: $(srcdir)/libc.toc.in
	echo "<!DOCTYPE kdeveloptoc>" > libc.toc ; \
	echo "<kdeveloptoc>" >> libc.toc ; \
	echo "<title>libc</title>" >> libc.toc ; \
	echo "<base href=\"$(htmldir)\"/>" >> libc.toc ; \
	cat $(srcdir)/libc.toc.in >> libc.toc
	echo "</kdeveloptoc>" >> libc.toc ; \
	echo "" >> libc.toc

libc.index: $(srcdir)/libc.index.in
	echo "<!DOCTYPE kdevelopindex>" > libc.index ; \
	echo "<kdevelopindex>" >> libc.index
	echo "<title>libc</title>" >> libc.index ; \
	echo "<base href=\"$(htmldir)\"/>" >> libc.index ; \
	cat $(srcdir)/libc.index.in >> libc.index ; \
	echo "</kdevelopindex>" >> libc.index ; \
	echo "" >> libc.index

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(htmldir) ; \
	$(INSTALL_DATA) $(srcdir)/*.html $(DESTDIR)$(htmldir)

#dist-hook:
#	mkdir -p $(distdir) ; \
#	for file in $(srcdir)/*.html ; do cp -p $$file $(distdir) ; done

html:
	texi2html -split_node libc.texinfo

toc:
	perl ../tools/toc-texi libc_toc.html > libc.toc.in

index:
	perl ../tools/index-texi \
	  --map "Concept Index"=conceptindex \
	  --map "Type Index"=identindex \
	  --map "Function and Macro Index"=identindex \
	  --map "Variable and Constant Macro Index"=identindex \
	  --map "Program and File Index"=fileindex \
	  *.html > libc.index.in
