indexdir = ${kde_datadir}/gideon/docindices
index_DATA = libc.index

htmldir = $(kde_htmldir)/en/gideon/libc

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(htmldir) ; \
	for file in *.html ; do $(INSTALL_DATA) $$file $(DESTDIR)$(htmldir)/$$file ; done

dist-hook:
	mkdir -p $(distdir) ; \
	for file in $(srcdir)/*.html ; do cp -p $$file $(distdir) ; done

html:
	texi2html -split_node libc.texinfo

index:
	perl ../tools/index-texi --title="libc" --base=$(htmldir) \
	  --map "Concept Index"=conceptindex \
	  --map "Type Index"=identindex \
	  --map "Function and Macro Index"=identindex \
	  --map "Variable and Constant Macro Index"=identindex \
	  --map "Program and File Index"=fileindex \
	  *.html > libc.index

