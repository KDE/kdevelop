#!/usr/bin/perl -w
require ("flush.pl");
use File::Copy;

$homedirectory = $ENV{HOME};
my $entriesfilename= shift || "entries";
printflush (STDOUT,"Starting with configuration\n");

#open file "entries" for reading the parameters from kAppWizard and put it in a hash
open (PROCESSLIST,$homedirectory . "/.kde/share/apps/kdevelop/" . $entriesfilename)
  || die "can not open file \"$entriesfilename\" in your local app-data-dir:\n  $!";
while ( defined ($name = <PROCESSLIST> )) {
  chomp ($name);
  chomp ($process = <PROCESSLIST>);
  $processes{$name} = $process;
}
close (PROCESSLIST);

$nameLittle = $processes{NAME};
$nameLittle =~ tr/A-Z/a-z/;
$nameBig = $processes{NAME};
$nameBig =~ tr/a-z/A-Z/;
$name =  $processes{NAME};

$overDirectory = $processes{DIRECTORY} . "/";
$underDirectory = $overDirectory . $nameLittle;

if ($processes{APPLICATION} eq "customproj") {}
else {
  my $confargs = $processes{CONFIGARG} || "";
  #start make -f Makefile.dist
  printflush (STDOUT,"create configuration files...\n");
  printflush (STDOUT,">make -f Makefile.dist\n");
  chdir ($overDirectory);
  system ("make -f Makefile.dist");
  
  #start configure
  printflush (STDOUT,"make configure...\n");
  printflush (STDOUT,">LDFLAGS=\" \" CFLAGS=\"-O0 -g3 -Wall\" CXXFLAGS=\"-O0 -g3 -Wall\"\n>./configure $confargs\n");
  chdir ($overDirectory);
  system ("LDFLAGS=\" \" CFLAGS=\"-O0 -g3 -Wall\" CXXFLAGS=\"-O0 -g3 -Wall\" ./configure " . $confargs);
	
}
#if User-Documentation was chosen in kAppWizard
if ($processes{USER} eq "yes") {
  
  #create the html-files
  printflush (STDOUT,"configure files...\n");
  chdir ($underDirectory . "/docs/en");

  if ( 	$processes{APPLICATION} eq "kde2normal" || $processes{APPLICATION} eq "kde2mini" ||
       	$processes{APPLICATION} eq "kde2mdi")
  {
     # FIXME: there should be the proper call for docbooks
  }
  else
  {
    if (-e "index.nif")
    {
      printflush (STDOUT,">ksgml2html index.sgml en\n");
      system ("ksgml2html index.sgml en");
    }
    else
    {
      printflush (STDOUT,">sgml2html index.sgml\n");
      system ("sgml2html index.sgml");
    }
  }
}

chdir ($underDirectory);
printflush (STDOUT,"READY\n");

system("rm -f ". $homedirectory . "/.kde/share/apps/kdevelop/" . $entriesfilename);
exit;
