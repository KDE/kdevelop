htmldir = $(kde_htmldir)/en/kdevdoctreeview/libc
indexdir = ${kde_datadir}/kdevdoctreeview/indices
index_DATA = libc.index

libc.index: $(srcdir)/libc.index.in
	echo "<!DOCTYPE gideonindex>" > libc.index ; \
	echo "<gideonindex>" >> libc.index
	echo "<title>libc</title>" >> libc.index ; \
	echo "<base href=\"$(htmldir)\"/>" >> libc.index ; \
	cat $(srcdir)/libc.index.in >> libc.index ; \
	echo "</gideonindex>" >> libc.index ; \
	echo "" >> libc.index

DISTCLEANFILES = libc.index
EXTRA_DIST = libc.index.in

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(htmldir) ; \
	for file in *.html ; do $(INSTALL_DATA) $$file $(DESTDIR)$(htmldir)/$$file ; done

#dist-hook:
#	mkdir -p $(distdir) ; \
#	for file in $(srcdir)/*.html ; do cp -p $$file $(distdir) ; done

html:
	texi2html -split_node libc.texinfo

index:
	perl ../tools/index-texi \
	  --map "Concept Index"=conceptindex \
	  --map "Type Index"=identindex \
	  --map "Function and Macro Index"=identindex \
	  --map "Variable and Constant Macro Index"=identindex \
	  --map "Program and File Index"=fileindex \
	  *.html > libc.index.in
