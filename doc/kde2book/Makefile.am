indexdir = ${kde_datadir}/gideon/docindices
index_DATA = kde2book.index

htmldir = $(kde_htmldir)/en/gideon/kde2book

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(htmldir) ; \
	for file in *.html ; do $(INSTALL_DATA) $$file $(DESTDIR)$(htmldir)/$$file ; done ; \
	$(mkinstalldirs) $(DESTDIR)$(htmldir)/graphics ; \
	for file in graphics/*.gif ; do $(INSTALL_DATA) $$file $(DESTDIR)$(htmldir)/$$file ; done ;

dist-hook:
	mkdir -p $(distdir) ; \
	for file in $(srcdir)/*.html ; do cp -p $$file $(distdir) ; done

index:
	perl ../tools/index-db --title="KDE2 Development Book" --base=$(htmldir) *.xml > kde2book.index

