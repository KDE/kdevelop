
tools_DATA =  processesend.pl processes.pl.in

EXTRA_DIST = $(tools_DATA)

toolsdir = $(kde_datadir)/kdevelop/tools/


processes.pl: processes.pl.in
	cp processes.pl.in processes.pl
	perl -npi -e 's%^\$$templatedir = _;$$%\$$templatedir = \"'${kde_datadir}'/kdevelop/templates\";%g;' \
	    processes.pl
	perl -npi -e 's%^\$$kde_icondir = _;$$%\$$kde_icondir = \"'${kde_icondir}'\";%g;' \
	    processes.pl


