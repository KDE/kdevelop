
tools_DATA =  processesend.pl processes.pl

EXTRA_DIST = processes.pl.in processesend.pl kimport

toolsdir = $(kde_datadir)/kdevelop/tools/


processes.pl: processes.pl.in
	cp $(srcdir)/processes.pl.in processes.pl
	perl -npi -e 's%^\$$templatedir = _;$$%\$$templatedir = \"'${kde_datadir}'/kdevelop/templates\";%g;' \
	    processes.pl
	perl -npi -e 's%^\$$kde_icondir = _;$$%\$$kde_icondir = \"'${kde_icondir}'\";%g;' \
	    processes.pl

install-exec-local:
	$(mkinstalldirs) $(KDEDIR)/bin
	$(INSTALL_SCRIPT) kimport $(KDEDIR)/bin
