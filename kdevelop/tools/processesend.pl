#!/usr/bin/perl -w
require ("flush.pl");
use File::Copy;

$homedirectory = $ENV{HOME};
printflush (STDOUT,"Starting with installation\n");

#open file "entries" for reading the parameters from kAppWizard and put it in a hash
open (PROCESSLIST,$homedirectory . "/.kde/share/apps/kdevelop/entries") || die "kann Datei nicht öffnen: $!";
while ( defined ($name = <PROCESSLIST> )) {
    chomp ($name);
    $process = <PROCESSLIST>;
    chomp ($process);
    $processes{$name} = $process;
}
close (PROCESSLIST);

$nameLittle = $processes{NAME};
$nameLittle =~ tr/A-Z/a-z/;
$nameBig = $processes{NAME};
$nameBig =~ tr/a-z/A-Z/;
$name =  $processes{NAME};

$overDirectory = $processes{DIRECTORY} . $nameLittle;
$underDirectory = $processes{DIRECTORY} . $nameLittle . "/" . $nameLittle;


#start make -f Makefile.dist
printflush (STDOUT,"create configurationfiles...\n");
chdir ($overDirectory);
system ("make -f Makefile.dist");

#start configure
printflush (STDOUT,"make configure...\n");
chdir ($overDirectory);
system ("./configure");

#if User-Documentation was chosen in kAppWizard
if ($processes{USER} eq "yes") {
  
  #create the html-files
  printflush (STDOUT,"configure files...\n");
  chdir ($underDirectory . "/docs/en");
  system ("sgml2html index.sgml");
}


chdir ($underDirectory);

printflush (STDOUT,"READY\n");

exit;

