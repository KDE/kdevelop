####### kdevelop will overwrite this part!!! (begin)##########

SUBDIRS = cref kde_libref manual tutorial welcome 

EXTRA_DIST = tip.database 

####### kdevelop will overwrite this part!!! (end)############
EXTRA_DIST = tip.database

SUBDIRS = manual welcome cref tutorial kde_libref
		
install-data-local:
	$(mkinstalldirs) $(kde_htmldir)/de/kdevelop/
	$(INSTALL_DATA) $(srcdir)/tip.database $(kde_htmldir)/de/kdevelop/tip.database

uninstall-local:
	-rm -f $(kde_htmldir)/de/kdevelop/tip.database


