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

$overDirectory = $processes{DIRECTORY};
$underDirectory = $processes{DIRECTORY} . $nameLittle;

#create the projectdirectory 
printflush (STDOUT, "change directory...\n");
chdir ($processes{DIRECTORY});
$kdedirectory = $ENV{KDEDIR};
$date = `date`;
@time = localtime();
$year = 1900 + $time[5];
printflush (STDOUT,"chance directory...\n");
chdir ($overDirectory);

if ($processes{APPLICATION} eq "standard") {
  changeNormalApp();  
}
elsif ($processes{APPLICATION} eq "mini") {
  changeMiniApp();
}
elsif ($processes{APPLICATION} eq "terminal") {
  changeTerminalApp();
}
elsif ($processes{APPLICATION} eq "qt") {
  changeQtApp();
}

exit;

#the subroutine for replacing words in files
sub replaceOldFile {
  $where = 0;
  $counter = 0;
  $wordlength = length($word);
  $newfile = "newfile.txt";
  open (INPUT,$oldfile) || die "kann Datei nicht öffnen: $!";
  open (OUTPUT,">$newfile");
  while ( defined ($line = <INPUT> )) {
    while ($where != -1) {
      $where = index($line,$word,$where);
      if ($where == -1) {
	++$counter;
	print OUTPUT $line;
      }
      else {
	substr($line,$where,$wordlength) = $replace;
	++$where;
      }
    }
    $where = 0;
  }
  close (INPUT);
  close (OUTPUT);
  unlink ($oldfile);
  rename ("newfile.txt",$oldfile);
  unlink ("newfile.txt");
}

sub changeNormalApp() {
  #copy the file in the projectdirectory and unpacked it
  chdir ($overDirectory);
  printflush (STDOUT,"unzip file...\n");
  system ("gunzip normal.tar.gz");
  printflush (STDOUT,"untar file...\n");
  system ("tar -xf normal.tar");
  unlink "normal.tar";

  #renamed the directory
  printflush (STDOUT,"change files...\n");
  chdir ($overDirectory);
  rename ("skel", $nameLittle);

  chdir ($underDirectory);
  rename ("kbase.cpp","skel.cpp");
  rename ("kbase.h","skel.h");
  rename ("kbasedoc.cpp",$nameLittle . "doc.cpp");
  rename ("kbasedoc.h",$nameLittle . "doc.h");
  rename ("kbaseview.cpp",$nameLittle . "view.cpp");
  rename ("kbaseview.h",$nameLittle . "view.h");

  #create the templatedirectory
  chdir ($underDirectory);
  mkdir ("templates",0777);

  #copying the templates in the templatedirectoy
  if ($processes{CPP} eq "no" and $processes{HEADER} eq "no") {}
  else {
    if ($processes{HEADER} eq "yes") {
      $directory = $homedirectory . "/.kde/share/apps/kdevelop/header";
      $targetdirectory = $underDirectory . "/templates";
      copy ($directory, $targetdirectory);
      chdir ($targetdirectory);
      rename ("header","header_template");
    }
    
    if ($processes{CPP} eq "yes") {
      $directory = $homedirectory . "/.kde/share/apps/kdevelop/cpp";
      $targetdirectory = $underDirectory . "/templates";
      copy ($directory, $targetdirectory);
      chdir ($targetdirectory);
      rename ("cpp","cpp_template");
    }
  }

  #replaced skel with the projectname in different files
  chdir ($overDirectory);
  $word = "skel";
  $oldfile = "Makefile.am";
  $replace = $nameLittle;
  replaceOldFile($word,$replace,$oldfile);
  $oldfile = "configure.in";
  replaceOldFile($word,$replace,$oldfile);
  $word = "VERSION";
  $replace = $processes{VERSION};
  replaceOldFile($word,$replace,$oldfile);
  
  chdir ($underDirectory);
  $word = "skel";
  $oldfile = "Makefile.am";
  $replace = $nameLittle;
  replaceOldFile($word,$replace,$oldfile);
  
  $word = "kbase";
  $oldfile = "skel.h";
  replaceOldFile($word,$replace,$oldfile);
  
  $oldfile = "main.cpp";
  replaceOldFile($word,$replace,$oldfile);

  $oldfile = "skel.cpp";
  replaceOldFile($word,$replace,$oldfile);

  #replaced Skel with the projectname in different files
  $word = "KBase";
  $oldfile = "main.cpp";
  $replace = $name;
  replaceOldFile($word,$replace,$oldfile);
  $oldfile = "skel.cpp";
  replaceOldFile($word,$replace,$oldfile);
  $oldfile = "skel.h";
  replaceOldFile($word,$replace,$oldfile);
  
  #replaced AUTHOR with the authorname in skel.cpp
  $word = "AUTHOR";
  $oldfile = "skel.cpp";
  $replace = $processes{AUTHOR};
  replaceOldFile($word,$replace,$oldfile);
  
  #replaced SKEL with the projectname in skel.h
  $word = "KBASE";
  $oldfile = "skel.h";
  $replace = $nameBig;
  replaceOldFile($word,$replace,$oldfile);
  
  #renamed skel with the projectname
  rename ("skel.cpp", $nameLittle . ".cpp");
  rename ("skel.h", $nameLittle . ".h");

  chdir ($underDirectory);
  $word = "KBase";
  $oldfile = $nameLittle . "doc.cpp";
  $replace = $name;
  replaceOldFile($word,$replace,$oldfile);
  $oldfile = $nameLittle . "doc.h";
  replaceOldFile($word,$replace,$oldfile);
  $oldfile = $nameLittle . "view.cpp";
  replaceOldFile($word,$replace,$oldfile);
  $oldfile = $nameLittle . "view.h";
  replaceOldFile($word,$replace,$oldfile);
  $oldfile = "resource.h";
  replaceOldFile($word,$replace,$oldfile);
  $word = "KBASE";
  $replace = $nameBig;
  $oldfile = $nameLittle . "doc.h";
  replaceOldFile($word,$replace,$oldfile);
  $oldfile = $nameLittle . "view.h";
  replaceOldFile($word,$replace,$oldfile);
  $word = "kbase";
  $oldfile = $nameLittle . "doc.cpp";
  $replace = $nameLittle;
  replaceOldFile($word,$replace,$oldfile);
  $oldfile = $nameLittle . "view.cpp";
  replaceOldFile($word,$replace,$oldfile);
  $oldfile = $nameLittle . "view.h";
  replaceOldFile($word,$replace,$oldfile);
  
  chdir ($underDirectory . "/docs/en");
  $word = "skel";
  $oldfile = "Makefile.am";
  $replace = $nameLittle;
  replaceOldFile($word,$replace,$oldfile);

  #if GNU-Files was chosen in kAppWizard
  if ($processes{GNU} eq "yes") {
    
    #copying the GNU-Files and renamed these
    chdir ($kdedirectory);
    copy ("share/apps/kdevelop/templates/AUTHORS_template", $overDirectory);
    copy ("share/apps/kdevelop/templates/COPYING_template", $overDirectory);
    copy ("share/apps/kdevelop/templates/ChangeLog_template", $overDirectory);
    copy ("share/apps/kdevelop/templates/INSTALL_template", $overDirectory);
    copy ("share/apps/kdevelop/templates/README_template", $overDirectory);
    copy ("share/apps/kdevelop/templates/TODO_template", $overDirectory);
    chdir ($overDirectory);
    rename ("AUTHORS_template", "AUTHORS");
    rename ("COPYING_template", "COPYING");
    rename ("ChangeLog_template", "ChangeLog");
    rename ("INSTALL_template", "INSTALL");
    rename ("README_template", "README");
    rename ("TODO_template", "TODO");
    
    # replaced AUTHOR and EMAIL
    $word = "AUTHOR";
    $replace = $processes{AUTHOR};
    $oldfile = "AUTHORS";
    replaceOldFile($word,$replace,$oldfile);
    $word = "EMAIL";
    $replace = $processes{EMAIL};
    $oldfile = "AUTHORS";
    replaceOldFile($word,$replace,$oldfile);
  }
  #if LSM-Files was chosen in kAppWizard
  if ($processes{LSM} eq "yes") {
    
    #copying, rename and replace in the lsm-template
    chdir ($kdedirectory);
    copy ("share/apps/kdevelop/templates/lsm_template", $overDirectory);
    chdir ($overDirectory);
    rename ("lsm_template", $nameLittle . ".lsm");
    $oldfile = $nameLittle . ".lsm";
    $word = "PROJECT_NAME";
    $replace = $name;
    replaceOldFile($word,$replace,$oldfile);
    $word = "AUTHOR";
    $replace = $processes{AUTHOR};
    replaceOldFile($word,$replace,$oldfile);
    $word = "EMAIL";
    $replace = $processes{EMAIL};
    replaceOldFile($word,$replace,$oldfile);
    $word = "VERSION";
    $replace = $processes{VERSION};
    replaceOldFile($word,$replace,$oldfile);
    
  }

  #if USER-Docs was chosen in kAppWizard
  if ($processes{USER} eq "yes") {
    
    #copying, rename and replace in the handbook-en-template
    chdir ($kdedirectory);
    $targetdirectory = $underDirectory . "/docs/en";
    copy ("share/apps/kdevelop/templates/handbook_en_template", $targetdirectory);
    chdir ($targetdirectory);
    rename ("handbook_en_template", "index.sgml");
    $oldfile = "index.sgml";
    $word = "PROJECT_NAME";
    $replace = $name;
    replaceOldFile($word,$replace,$oldfile);
    $word = "AUTHOR";
    $replace = $processes{AUTHOR};
    replaceOldFile($word,$replace,$oldfile);
    $word = "EMAIL";
    $replace = $processes{EMAIL};
    replaceOldFile($word,$replace,$oldfile);
    $word = "VERSION";
    $replace = $processes{VERSION};
    replaceOldFile($word,$replace,$oldfile);
    $word = "DATE";
    $replace = $date;
    replaceOldFile($word,$replace,$oldfile);
    $word = "YEAR";
    $replace = $year;
    replaceOldFile($word,$replace,$oldfile);
    
  }
  
  #if USER-Docs was not chosen in kAppWizard
  if ($processes{USER} eq "no") {
    chdir ($underDirectory);
    $word = "SUBDIRS = docs";
    $replace = ""; 
    $oldfile = "Makefile.am";
    replaceOldFile($word,$replace,$oldfile);
    chdir ($overDirectory);
    $word = $nameLittle . "/docs/Makefile \\";
    $replace = ""; 
    $oldfile = "configure.in";
    replaceOldFile(  $word,$replace,$oldfile);
    $word = $nameLittle . "/docs/en/Makefile \\";
    replaceOldFile($word,$replace ,$oldfile);
  }
  
  #if cpp-template was chosen in kAppWizard
  if ($processes{CPP} eq "yes") {
    chdir ($homedirectory);
    copy (".kde/share/apps/kdevelop/cpp", $underDirectory);
    chdir ($underDirectory);
    $oldfile = "cpp";
    $word = "FILENAME";
    $replace = "main.cpp";
    replaceOldFile($word,$replace,$oldfile);
    $word = "AUTHOR";
    $replace = $processes{AUTHOR};
    replaceOldFile($word,$replace,$oldfile);
    $word = "EMAIL";
    $replace = $processes{EMAIL};
    replaceOldFile($word,$replace,$oldfile);
    $word = "DATE";
    $replace = $date;
    replaceOldFile($word,$replace,$oldfile);
    $word = "YEAR";
    $replace = $year;
    replaceOldFile($word,$replace,$oldfile);
    $file = "main.cpp";
    open (INPUT,"$file") || die "kann Datei nicht öffnen: $!";
    open (OUTPUT,">>cpp");
    while ( defined ($line = <INPUT> )) {
      print OUTPUT $line;
    }
    close (INPUT);
    close (OUTPUT);
    rename ("cpp" , $file);
    unlink ("cpp");
    #copying, rename and replace in the cpp-file
    chdir ($homedirectory);
    copy (".kde/share/apps/kdevelop/cpp", $underDirectory);
    chdir ($underDirectory);
    $oldfile = "cpp";
    $word = "FILENAME";
    $replace = $nameLittle . ".cpp";
    replaceOldFile($word,$replace,$oldfile);
    $word = "AUTHOR";
    $replace = $processes{AUTHOR};
    replaceOldFile($word,$replace,$oldfile);
    $word = "EMAIL";
    $replace = $processes{EMAIL};
    replaceOldFile($word,$replace,$oldfile);
    $word = "DATE";
    $replace = $date;
    replaceOldFile($word,$replace,$oldfile);
    $word = "YEAR";
    $replace = $year;
    replaceOldFile($word,$replace,$oldfile);
    # summarized the files cpp and project.cpp
    $file = $nameLittle . ".cpp";
    open (INPUT,"$file") || die "kann Datei nicht öffnen: $!";
    open (OUTPUT,">>cpp");
    while ( defined ($line = <INPUT> )) {
      print OUTPUT $line;
    }
    close (INPUT);
    close (OUTPUT);
    rename ("cpp" , $file);
    unlink ("cpp");
    
    chdir ($homedirectory);
    copy (".kde/share/apps/kdevelop/cpp", $underDirectory);
    chdir ($underDirectory);
    $oldfile = "cpp";
    $word = "FILENAME";
    $replace = $nameLittle . "view.cpp";
    replaceOldFile($word,$replace,$oldfile);
    $word = "AUTHOR";
    $replace = $processes{AUTHOR};
    replaceOldFile($word,$replace,$oldfile);
    $word = "EMAIL";
    $replace = $processes{EMAIL};
    replaceOldFile($word,$replace,$oldfile);
    $word = "DATE";
    $replace = $date;
    replaceOldFile($word,$replace,$oldfile);
    $word = "YEAR";
    $replace = $year;
    replaceOldFile($word,$replace,$oldfile);
    
    # summarized the files cpp and projectview.cpp
    $file = $nameLittle . "view.cpp";
    open (INPUT,"$file") || die "kann Datei nicht öffnen: $!";
    open (OUTPUT,">>cpp");
    while ( defined ($line = <INPUT> )) {
      print OUTPUT $line;
    }
    close (INPUT);
    close (OUTPUT);
    rename ("cpp" , $file);
    unlink ("cpp");
    
    chdir ($homedirectory);
    copy (".kde/share/apps/kdevelop/cpp", $underDirectory);
    chdir ($underDirectory);
    $oldfile = "cpp";
    $word = "FILENAME";
    $replace = $nameLittle . "doc.cpp";
    replaceOldFile($word,$replace,$oldfile);
    $word = "AUTHOR";
    $replace = $processes{AUTHOR};
    replaceOldFile($word,$replace,$oldfile);
    $word = "EMAIL";
    $replace = $processes{EMAIL};
    replaceOldFile($word,$replace,$oldfile);
    $word = "DATE";
    $replace = $date;
    replaceOldFile($word,$replace,$oldfile);
    $word = "YEAR";
    $replace = $year;
    replaceOldFile($word,$replace,$oldfile);
    
    # summarized the files cpp and projectdoc.cpp
    $file = $nameLittle . "doc.cpp";
    open (INPUT,"$file") || die "kann Datei nicht öffnen: $!";
    open (OUTPUT,">>cpp");
    while ( defined ($line = <INPUT> )) {
      print OUTPUT $line;
    }
    close (INPUT);
    close (OUTPUT);
    rename ("cpp" , $file);
    unlink ("cpp"); 
  }

  #if header-template was chosen in kAppWizard
  if ($processes{HEADER} eq "yes") {
    #copying, rename and replace in the header-file
    chdir ($homedirectory);
    copy (".kde/share/apps/kdevelop/header", $underDirectory);
    chdir ($underDirectory);
    $oldfile = "header";
    $word = "FILENAME";
    $replace = $nameLittle . ".h";
    replaceOldFile($word,$replace,$oldfile);
    $word = "AUTHOR";
    $replace = $processes{AUTHOR};
    replaceOldFile($word,$replace,$oldfile);
    $word = "EMAIL";
    $replace = $processes{EMAIL};
    replaceOldFile($word,$replace,$oldfile);
    $word = "DATE";
    $replace = $date;
    replaceOldFile($word,$replace,$oldfile);
    $word = "YEAR";
    $replace = $year;
    replaceOldFile($word,$replace,$oldfile);
    # summarized the files header and project.h
    $file = $nameLittle . ".h";
    open (INPUT,"$file") || die "kann Datei nicht öffnen: $!";
    open (OUTPUT,">>header");
    while ( defined ($line = <INPUT> )) {
      print OUTPUT $line;
    }
    close (INPUT);
    close (OUTPUT);
    rename ("header" , $file);
    unlink ("header");
    
    chdir ($homedirectory);
    copy (".kde/share/apps/kdevelop/header",$underDirectory);
    chdir ($underDirectory);
    $oldfile = "header";
    $word = "FILENAME";
    $replace = $nameLittle . "view.h";
    replaceOldFile($word,$replace,$oldfile);
    $word = "AUTHOR";
    $replace = $processes{AUTHOR};
    replaceOldFile($word,$replace,$oldfile);
    $word = "EMAIL";
    $replace = $processes{EMAIL};
    replaceOldFile($word,$replace,$oldfile);
    $word = "DATE";
    $replace = $date;
    replaceOldFile($word,$replace,$oldfile);
    $word = "YEAR";
    $replace = $year;
    replaceOldFile($word,$replace,$oldfile);
    
    # summarized the files header and project.h
    $file = $nameLittle . "view.h";
    open (INPUT,"$file") || die "kann Datei nicht öffnen: $!";
    open (OUTPUT,">>header");
    while ( defined ($line = <INPUT> )) {
      print OUTPUT $line;
    }
    close (INPUT);
    close (OUTPUT);
    rename ("header" , $file);
    unlink ("header");
    chdir ($homedirectory);
    copy (".kde/share/apps/kdevelop/header", $underDirectory);
    chdir ($underDirectory);
    $oldfile = "header";
    $word = "FILENAME";
    $replace = $nameLittle . "doc.h";
    replaceOldFile($word,$replace,$oldfile);
    $word = "AUTHOR";
    $replace = $processes{AUTHOR};
    replaceOldFile($word,$replace,$oldfile);
    $word = "EMAIL";
    $replace = $processes{EMAIL};
    replaceOldFile($word,$replace,$oldfile);
    $word = "DATE";
    $replace = $date;
    replaceOldFile($word,$replace,$oldfile);
    $word = "YEAR";
    $replace = $year;
    replaceOldFile($word,$replace,$oldfile);
    
    # summarized the files header and project.h
    $file = $nameLittle . "doc.h";
    open (INPUT,"$file") || die "kann Datei nicht öffnen: $!";
    open (OUTPUT,">>header");
    while ( defined ($line = <INPUT> )) {
      print OUTPUT $line;
    }
    close (INPUT);
    close (OUTPUT);
    rename ("header" , $file);
    unlink ("header");
    chdir ($homedirectory);
    copy (".kde/share/apps/kdevelop/header", $underDirectory);
    chdir ($underDirectory);
    $oldfile = "header";
    $word = "FILENAME";
    $replace = "resource.h";
    replaceOldFile($word,$replace,$oldfile);
    $word = "AUTHOR";
    $replace = $processes{AUTHOR};
    replaceOldFile($word,$replace,$oldfile);
    $word = "EMAIL";
    $replace = $processes{EMAIL};
    replaceOldFile($word,$replace,$oldfile);
    $word = "DATE";
    $replace = $date;
    replaceOldFile($word,$replace,$oldfile);
    $word = "YEAR";
    $replace = $year;
    replaceOldFile($word,$replace,$oldfile);
    
    # summarized the files header and project.h
    $file = "resource.h";
    open (INPUT,"$file") || die "kann Datei nicht öffnen: $!";
    open (OUTPUT,">>header");
    while ( defined ($line = <INPUT> )) {
      print OUTPUT $line;
    }
    close (INPUT);
    close (OUTPUT);
    rename ("header" , $file);
    unlink ("header");
  }

  #if .kdelnk-file was chosen in kAppWizard
  if ($processes{KDELNK} eq "yes") {
  
    #copying, rename and replace in the kdelnk-file
    chdir ($kdedirectory);
    copy ("share/apps/kdevelop/templates/kdelnk_template", $underDirectory);
    chdir ($underDirectory);
    rename ("kdelnk_template", $nameLittle . ".kdelnk");
    $word = PROJECT_NAME;
    $oldfile = $nameLittle . ".kdelnk";
    $replace = $nameLittle;
    replaceOldFile($word,$replace,$oldfile);
  }

  #if no ProgIcon was chosen in kAppWizard
  if ($processes{PROGICON} eq "no") {} 

  #if the default ProgIcon was chosen in kAppWizard
  elsif ($processes{PROGICON} eq "(null)") {
    chdir ($kdedirectory);
    copy ("share/icons/edit.xpm", $underDirectory);
    chdir ($underDirectory);
    chmod (0666,"edit.xpm");
    rename ("edit.xpm", $nameLittle . ".xpm");
  }

  #if a new ProgIcon was chosen in kAppWizard
  else {
    $icon = $kdedirectory . "/share/icons/" . $processes{PROGICON};
    $copying = copy ($icon, $underDirectory);
    if ($copying)  {
      $icon = $homedirectory . "/.kde/share/icons/" . $processes{PROGICON};
      copy ($icon, $underDirectory);
    }
    chmod (0666,$processes{PROGICON});
    rename ($processes{PROGICON}, $nameLittle . ".xpm");
  }

  #if no MiniIcon was chosen in kAppWizard
  if ($processes{MINIICON} eq "no") {}
  
  #if the default MiniIcon was chosen in kAppWizard
  elsif ($processes{MINIICON} eq "(null)") {
    chdir ($kdedirectory);
    copy ("share/icons/mini/application_settings.xpm", $underDirectory);
    chdir ($underDirectory);
    chmod (0666,"application_settings.xpm");
    rename ("application_settings.xpm","mini-" . $nameLittle . ".xpm");
  }

  #if a new MiniIcon was chosen in kAppWizard
  else {
    $icon = $kdedirectory . "/share/icons/mini/" . $processes{MINIICON};
    $copying = copy ($icon, $underDirectory);
    if ($copying)  {
      $icon = $homedirectory . "/.kde/share/icons/mini/" . $processes{MINIICON};
      copy ($icon, $underDirectory);
    }
    chmod (0666,$processes{MINIICON});
    rename ($processes{MINIICON}, "mini-" . $nameLittle . ".xpm");
  }

  #if API-Files was chosen in kAppWizard
  if ($processes{API} eq "yes") {
    
    #create the API-documentation
    chdir ($underDirectory);
    mkdir ("api",0777);
    system ("kdoc -d ./api/ $name *.h");
  }
}

sub changeMiniApp() {
  #copy the file in the projectdirectory and unpacked it
  chdir ($overDirectory);
  system ("gunzip mini.tar.gz");
  printflush (STDOUT,"untar file...\n");
  system ("tar -xf mini.tar");
  unlink "mini.tar";

  #renamed the directory
  printflush (STDOUT,"change files...\n");
  chdir ($overDirectory);
  rename ("skel", $nameLittle);

  #create the templatedirectory
  chdir ($underDirectory);
  mkdir ("templates",0777);
  
  #copying the templates in the templatedirectoy
  if ($processes{CPP} eq "no" and $processes{HEADER} eq "no") {}
  else {
    if ($processes{HEADER} eq "yes") {
      $directory = $homedirectory . "/.kde/share/apps/kdevelop/header";
      $targetdirectory = $underDirectory . "/templates";
      copy ($directory, $targetdirectory);
      chdir ($targetdirectory);
      rename ("header","header_template");
    }
    
    if ($processes{CPP} eq "yes") {
      $directory = $homedirectory . "/.kde/share/apps/kdevelop/cpp";
      $targetdirectory = $underDirectory . "/templates";
      copy ($directory, $targetdirectory);
      chdir ($targetdirectory);
      rename ("cpp","cpp_template");
    }
  }
  
  #replaced skel with the projectname in different files
  chdir ($overDirectory);
  $word = "skel";
  $oldfile = "Makefile.am";
  $replace = $nameLittle;
  replaceOldFile($word,$replace,$oldfile);
  $oldfile = "configure.in";
  replaceOldFile($word,$replace,$oldfile);
  $word = "VERSION";
  $replace = $processes{VERSION};
  replaceOldFile($word,$replace,$oldfile);
  
  chdir ($underDirectory);
  $word = "skel";
  $oldfile = "Makefile.am";
  $replace = $nameLittle;
  replaceOldFile($word,$replace,$oldfile);

  $oldfile = "main.cpp";
  replaceOldFile($word,$replace,$oldfile);

  $oldfile = "skel.cpp";
  replaceOldFile($word,$replace,$oldfile);

  #replaced Skel with the projectname in different files
  $word = "Skel";
  $oldfile = "main.cpp";
  $replace = $name;
  replaceOldFile($word,$replace,$oldfile);
  $oldfile = "skel.cpp";
  replaceOldFile($word,$replace,$oldfile);
  $oldfile = "skel.h";
  replaceOldFile($word,$replace,$oldfile);
  
  #replaced AUTHOR with the authorname in skel.cpp
  $word = "AUTHOR";
  $oldfile = "skel.cpp";
  $replace = $processes{AUTHOR};
  replaceOldFile($word,$replace,$oldfile);
  
  #replaced SKEL with the projectname in skel.h
  $word = "SKEL";
  $oldfile = "skel.h";
  $replace = $nameBig;
  replaceOldFile($word,$replace,$oldfile);
  
  #renamed skel with the projectname
  rename ("skel.cpp", $nameLittle . ".cpp");
  rename ("skel.h", $nameLittle . ".h");

  chdir ($underDirectory . "/docs/en");
  $word = "skel";
  $oldfile = "Makefile.am";
  $replace = $nameLittle;
  replaceOldFile($word,$replace,$oldfile);

  #if GNU-Files was chosen in kAppWizard
  if ($processes{GNU} eq "yes") {
    
    #copying the GNU-Files and renamed these
    chdir ($kdedirectory);
    copy ("share/apps/kdevelop/templates/AUTHORS_template", $overDirectory);
    copy ("share/apps/kdevelop/templates/COPYING_template", $overDirectory);
    copy ("share/apps/kdevelop/templates/ChangeLog_template", $overDirectory);
    copy ("share/apps/kdevelop/templates/INSTALL_template", $overDirectory);
    copy ("share/apps/kdevelop/templates/README_template", $overDirectory);
    copy ("share/apps/kdevelop/templates/TODO_template", $overDirectory);
    chdir ($overDirectory);
    rename ("AUTHORS_template", "AUTHORS");
    rename ("COPYING_template", "COPYING");
    rename ("ChangeLog_template", "ChangeLog");
    rename ("INSTALL_template", "INSTALL");
    rename ("README_template", "README");
    rename ("TODO_template", "TODO");
    
    # replaced AUTHOR and EMAIL
    $word = "AUTHOR";
    $replace = $processes{AUTHOR};
    $oldfile = "AUTHORS";
    replaceOldFile($word,$replace,$oldfile);
    $word = "EMAIL";
    $replace = $processes{EMAIL};
    $oldfile = "AUTHORS";
    replaceOldFile($word,$replace,$oldfile);
  }
  #if LSM-Files was chosen in kAppWizard
  if ($processes{LSM} eq "yes") {
    
    #copying, rename and replace in the lsm-template
    chdir ($kdedirectory);
    copy ("share/apps/kdevelop/templates/lsm_template", $overDirectory);
    chdir ($overDirectory);
    rename ("lsm_template", $nameLittle . ".lsm");
    $oldfile = $nameLittle . ".lsm";
    $word = "PROJECT_NAME";
    $replace = $name;
    replaceOldFile($word,$replace,$oldfile);
    $word = "AUTHOR";
    $replace = $processes{AUTHOR};
    replaceOldFile($word,$replace,$oldfile);
    $word = "EMAIL";
    $replace = $processes{EMAIL};
    replaceOldFile($word,$replace,$oldfile);
    $word = "VERSION";
    $replace = $processes{VERSION};
    replaceOldFile($word,$replace,$oldfile); 
  }

  #if USER-Docs was chosen in kAppWizard
  if ($processes{USER} eq "yes") {
    
    #copying, rename and replace in the handbook-en-template
    chdir ($kdedirectory);
    $targetdirectory = $underDirectory . "/docs/en";
    copy ("share/apps/kdevelop/templates/handbook_en_template", $targetdirectory);
    chdir ($targetdirectory);
    rename ("handbook_en_template", "index.sgml");
    $oldfile = "index.sgml";
    $word = "PROJECT_NAME";
    $replace = $name;
    replaceOldFile($word,$replace,$oldfile);
    $word = "AUTHOR";
    $replace = $processes{AUTHOR};
    replaceOldFile($word,$replace,$oldfile);
    $word = "EMAIL";
    $replace = $processes{EMAIL};
    replaceOldFile($word,$replace,$oldfile);
    $word = "VERSION";
    $replace = $processes{VERSION};
    replaceOldFile($word,$replace,$oldfile);
    $word = "DATE";
    $replace = $date;
    replaceOldFile($word,$replace,$oldfile);
    $word = "YEAR";
    $replace = $year;
    replaceOldFile($word,$replace,$oldfile); 
  }
  
  #if USER-Docs was not chosen in kAppWizard
  if ($processes{USER} eq "no") {
    chdir ($underDirectory);
    $word = "SUBDIRS = docs";
    $replace = ""; 
    $oldfile = "Makefile.am";
    replaceOldFile($word,$replace,$oldfile);
    chdir ($overDirectory);
    $word = $nameLittle . "/docs/Makefile \\";
    $replace = ""; 
    $oldfile = "configure.in";
    replaceOldFile(  $word,$replace,$oldfile);
    $word = $nameLittle . "/docs/en/Makefile \\";
    replaceOldFile($word,$replace ,$oldfile);
  }
  
  #if cpp-template was chosen in kAppWizard
  if ($processes{CPP} eq "yes") {
    chdir ($homedirectory);
    copy (".kde/share/apps/kdevelop/cpp", $underDirectory);
    chdir ($underDirectory);
    $oldfile = "cpp";
    $word = "FILENAME";
    $replace = "main.cpp";
    replaceOldFile($word,$replace,$oldfile);
    $word = "AUTHOR";
    $replace = $processes{AUTHOR};
    replaceOldFile($word,$replace,$oldfile);
    $word = "EMAIL";
    $replace = $processes{EMAIL};
    replaceOldFile($word,$replace,$oldfile);
    $word = "DATE";
    $replace = $date;
    replaceOldFile($word,$replace,$oldfile);
    $word = "YEAR";
    $replace = $year;
    replaceOldFile($word,$replace,$oldfile);
    $file = "main.cpp";
    open (INPUT,"$file") || die "kann Datei nicht öffnen: $!";
    open (OUTPUT,">>cpp");
    while ( defined ($line = <INPUT> )) {
      print OUTPUT $line;
    }
    close (INPUT);
    close (OUTPUT);
    rename ("cpp" , $file);
    unlink ("cpp");
    #copying, rename and replace in the cpp-file
    chdir ($homedirectory);
    copy (".kde/share/apps/kdevelop/cpp", $underDirectory);
    chdir ($underDirectory);
    $oldfile = "cpp";
    $word = "FILENAME";
    $replace = $nameLittle . ".cpp";
    replaceOldFile($word,$replace,$oldfile);
    $word = "AUTHOR";
    $replace = $processes{AUTHOR};
    replaceOldFile($word,$replace,$oldfile);
    $word = "EMAIL";
    $replace = $processes{EMAIL};
    replaceOldFile($word,$replace,$oldfile);
    $word = "DATE";
    $replace = $date;
    replaceOldFile($word,$replace,$oldfile);
    $word = "YEAR";
    $replace = $year;
    replaceOldFile($word,$replace,$oldfile);
    # summarized the files cpp and project.cpp
    $file = $nameLittle . ".cpp";
    open (INPUT,"$file") || die "kann Datei nicht öffnen: $!";
    open (OUTPUT,">>cpp");
    while ( defined ($line = <INPUT> )) {
      print OUTPUT $line;
    }
    close (INPUT);
    close (OUTPUT);
    rename ("cpp" , $file);
    unlink ("cpp");  
  }

  #if header-template was chosen in kAppWizard
  if ($processes{HEADER} eq "yes") {
    #copying, rename and replace in the header-file
    chdir ($homedirectory);
    copy (".kde/share/apps/kdevelop/header", $underDirectory);
    chdir ($underDirectory);
    $oldfile = "header";
    $word = "FILENAME";
    $replace = $nameLittle . ".h";
    replaceOldFile($word,$replace,$oldfile);
    $word = "AUTHOR";
    $replace = $processes{AUTHOR};
    replaceOldFile($word,$replace,$oldfile);
    $word = "EMAIL";
    $replace = $processes{EMAIL};
    replaceOldFile($word,$replace,$oldfile);
    $word = "DATE";
    $replace = $date;
    replaceOldFile($word,$replace,$oldfile);
    $word = "YEAR";
    $replace = $year;
    replaceOldFile($word,$replace,$oldfile);
    # summarized the files header and project.h
    $file = $nameLittle . ".h";
    open (INPUT,"$file") || die "kann Datei nicht öffnen: $!";
    open (OUTPUT,">>header");
    while ( defined ($line = <INPUT> )) {
      print OUTPUT $line;
    }
    close (INPUT);
    close (OUTPUT);
    rename ("header" , $file);
    unlink ("header");  
}

  #if .kdelnk-file was chosen in kAppWizard
  if ($processes{KDELNK} eq "yes") {
    
    #copying, rename and replace in the kdelnk-file
    chdir ($kdedirectory);
    copy ("share/apps/kdevelop/templates/kdelnk_template", $underDirectory);
    chdir ($underDirectory);
    rename ("kdelnk_template", $nameLittle . ".kdelnk");
    $word = PROJECT_NAME;
    $oldfile = $nameLittle . ".kdelnk";
    $replace = $nameLittle;
    replaceOldFile($word,$replace,$oldfile);
  }
  
  #if no ProgIcon was chosen in kAppWizard
  if ($processes{PROGICON} eq "no") {} 
  
  #if the default ProgIcon was chosen in kAppWizard
  elsif ($processes{PROGICON} eq "(null)") {
    chdir ($kdedirectory);
    copy ("share/icons/edit.xpm", $underDirectory);
    chdir ($underDirectory);
    chmod (0666,"edit.xpm");
    rename ("edit.xpm", $nameLittle . ".xpm");
  }
  
  #if a new ProgIcon was chosen in kAppWizard
  else {
    $icon = $kdedirectory . "/share/icons/" . $processes{PROGICON};
    $copying = copy ($icon, $underDirectory);
    if ($copying)  {
      $icon = $homedirectory . "/.kde/share/icons/" . $processes{PROGICON};
      copy ($icon, $underDirectory);
    }
    chmod (0666,$processes{PROGICON});
    rename ($processes{PROGICON}, $nameLittle . ".xpm");
  }
  
  #if no MiniIcon was chosen in kAppWizard
  if ($processes{MINIICON} eq "no") {}
  
  #if the default MiniIcon was chosen in kAppWizard
  elsif ($processes{MINIICON} eq "(null)") {
    chdir ($kdedirectory);
    copy ("share/icons/mini/application_settings.xpm", $underDirectory);
    chdir ($underDirectory);
    chmod (0666,"application_settings.xpm");
    rename ("application_settings.xpm","mini-" . $nameLittle . ".xpm");
  }
  
  #if a new MiniIcon was chosen in kAppWizard
  else {
    $icon = $kdedirectory . "/share/icons/mini/" . $processes{MINIICON};
    $copying = copy ($icon, $underDirectory);
    if ($copying)  {
      $icon = $homedirectory . "/.kde/share/icons/mini/" . $processes{MINIICON};
      copy ($icon, $underDirectory);
    }
    chmod (0666,$processes{MINIICON});
    rename ($processes{MINIICON}, "mini-" . $nameLittle . ".xpm");
  }
  
  #if API-Files was chosen in kAppWizard
  if ($processes{API} eq "yes") {
    
    #create the API-documentation
    chdir ($underDirectory);
    mkdir ("api",0777);
    system ("kdoc -d ./api/ $name *.h");
  }
}

sub changeTerminalApp() {
  #copy the file in the projectdirectory and unpacked it
  chdir ($overDirectory);
  printflush (STDOUT,"unzip file...\n");
  system ("gunzip cpp.tar.gz");
  printflush (STDOUT,"untar file...\n");
  system ("tar -xf cpp.tar");
  unlink "cpp.tar";

  #renamed the directory
  printflush (STDOUT,"change files...\n");
  chdir ($overDirectory);
  rename ("skel", $nameLittle);

  #create the templatedirectory
  chdir ($underDirectory);
  mkdir ("templates",0777);

  #copying the templates in the templatedirectoy
  if ($processes{CPP} eq "no" and $processes{HEADER} eq "no") {}
  else {
    if ($processes{HEADER} eq "yes") {
      $directory = $homedirectory . "/.kde/share/apps/kdevelop/header";
      $targetdirectory = $underDirectory . "/templates";
      copy ($directory, $targetdirectory);
      chdir ($targetdirectory);
      rename ("header","header_template");
    }
    
    if ($processes{CPP} eq "yes") {
      $directory = $homedirectory . "/.kde/share/apps/kdevelop/cpp";
      $targetdirectory = $underDirectory . "/templates";
      copy ($directory, $targetdirectory);
      chdir ($targetdirectory);
      rename ("cpp","cpp_template");
    }
  }

  #replaced skel with the projectname in different files
  chdir ($overDirectory);
  $word = "skel";
  $oldfile = "Makefile.am";
  $replace = $nameLittle;
  replaceOldFile($word,$replace,$oldfile);
  $oldfile = "configure.in";
  replaceOldFile($word,$replace,$oldfile);
  $word = "VERSION";
  $replace = $processes{VERSION};
  replaceOldFile($word,$replace,$oldfile);
  
  chdir ($underDirectory);
  $word = "skel";
  $oldfile = "Makefile.am";
  $replace = $nameLittle;
  replaceOldFile($word,$replace,$oldfile);

  $oldfile = "main.cpp";
  replaceOldFile($word,$replace,$oldfile);

  #replaced Skel with the projectname in different files
  $word = "Skel";
  $oldfile = "main.cpp";
  $replace = $name;
  replaceOldFile($word,$replace,$oldfile);

  chdir ($underDirectory . "/docs/en");
  $word = "skel";
  $oldfile = "Makefile.am";
  $replace = $nameLittle;
  replaceOldFile($word,$replace,$oldfile);
  
  #if GNU-Files was chosen in kAppWizard
  if ($processes{GNU} eq "yes") {
    
    #copying the GNU-Files and renamed these
    chdir ($kdedirectory);
    copy ("share/apps/kdevelop/templates/AUTHORS_template", $overDirectory);
    copy ("share/apps/kdevelop/templates/COPYING_template", $overDirectory);
    copy ("share/apps/kdevelop/templates/ChangeLog_template", $overDirectory);
    copy ("share/apps/kdevelop/templates/INSTALL_template", $overDirectory);
    copy ("share/apps/kdevelop/templates/README_template", $overDirectory);
    copy ("share/apps/kdevelop/templates/TODO_template", $overDirectory);
    chdir ($overDirectory);
    rename ("AUTHORS_template", "AUTHORS");
    rename ("COPYING_template", "COPYING");
    rename ("ChangeLog_template", "ChangeLog");
    rename ("INSTALL_template", "INSTALL");
    rename ("README_template", "README");
    rename ("TODO_template", "TODO");
    
    # replaced AUTHOR and EMAIL
    $word = "AUTHOR";
    $replace = $processes{AUTHOR};
    $oldfile = "AUTHORS";
    replaceOldFile($word,$replace,$oldfile);
    $word = "EMAIL";
    $replace = $processes{EMAIL};
    $oldfile = "AUTHORS";
    replaceOldFile($word,$replace,$oldfile);
  }
  #if LSM-Files was chosen in kAppWizard
  if ($processes{LSM} eq "yes") {
    
    #copying, rename and replace in the lsm-template
    chdir ($kdedirectory);
    copy ("share/apps/kdevelop/templates/lsm_template", $overDirectory);
    chdir ($overDirectory);
    rename ("lsm_template", $nameLittle . ".lsm");
    $oldfile = $nameLittle . ".lsm";
    $word = "PROJECT_NAME";
    $replace = $name;
    replaceOldFile($word,$replace,$oldfile);
    $word = "AUTHOR";
    $replace = $processes{AUTHOR};
    replaceOldFile($word,$replace,$oldfile);
    $word = "EMAIL";
    $replace = $processes{EMAIL};
    replaceOldFile($word,$replace,$oldfile);
    $word = "VERSION";
    $replace = $processes{VERSION};
    replaceOldFile($word,$replace,$oldfile); 
  }

  #if USER-Docs was chosen in kAppWizard
  if ($processes{USER} eq "yes") {
    
    #copying, rename and replace in the handbook-en-template
    chdir ($kdedirectory);
    $targetdirectory = $underDirectory . "/docs/en";
    copy ("share/apps/kdevelop/templates/handbook_en_template", $targetdirectory);
    chdir ($targetdirectory);
    rename ("handbook_en_template", "index.sgml");
    $oldfile = "index.sgml";
    $word = "PROJECT_NAME";
    $replace = $name;
    replaceOldFile($word,$replace,$oldfile);
    $word = "AUTHOR";
    $replace = $processes{AUTHOR};
    replaceOldFile($word,$replace,$oldfile);
    $word = "EMAIL";
    $replace = $processes{EMAIL};
    replaceOldFile($word,$replace,$oldfile);
    $word = "VERSION";
    $replace = $processes{VERSION};
    replaceOldFile($word,$replace,$oldfile);
    $word = "DATE";
    $replace = $date;
    replaceOldFile($word,$replace,$oldfile);
    $word = "YEAR";
    $replace = $year;
    replaceOldFile($word,$replace,$oldfile);
  }

  #if USER-Docs was not chosen in kAppWizard
  if ($processes{USER} eq "no") {
    chdir ($underDirectory);
    $word = "SUBDIRS = docs";
    $replace = ""; 
    $oldfile = "Makefile.am";
    replaceOldFile($word,$replace,$oldfile);
    chdir ($overDirectory);
    $word = $nameLittle . "/docs/Makefile \\";
    $replace = ""; 
    $oldfile = "configure.in";
    replaceOldFile(  $word,$replace,$oldfile);
    $word = $nameLittle . "/docs/en/Makefile \\";
    replaceOldFile($word,$replace ,$oldfile);
  }
  
  #if cpp-template was chosen in kAppWizard
  if ($processes{CPP} eq "yes") {
    chdir ($homedirectory);
    copy (".kde/share/apps/kdevelop/cpp", $underDirectory);
    chdir ($underDirectory);
    $oldfile = "cpp";
    $word = "FILENAME";
    $replace = "main.cpp";
    replaceOldFile($word,$replace,$oldfile);
    $word = "AUTHOR";
    $replace = $processes{AUTHOR};
    replaceOldFile($word,$replace,$oldfile);
    $word = "EMAIL";
    $replace = $processes{EMAIL};
    replaceOldFile($word,$replace,$oldfile);
    $word = "DATE";
    $replace = $date;
    replaceOldFile($word,$replace,$oldfile);
    $word = "YEAR";
    $replace = $year;
    replaceOldFile($word,$replace,$oldfile);
    $file = "main.cpp";
    open (INPUT,"$file") || die "kann Datei nicht öffnen: $!";
    open (OUTPUT,">>cpp");
    while ( defined ($line = <INPUT> )) {
      print OUTPUT $line;
    }
    close (INPUT);
    close (OUTPUT);
    rename ("cpp" , $file);
    unlink ("cpp");  
  }
  
  #if .kdelnk-file was chosen in kAppWizard
  if ($processes{KDELNK} eq "yes") {
    
    #copying, rename and replace in the kdelnk-file
    chdir ($kdedirectory);
    copy ("share/apps/kdevelop/templates/kdelnk_template", $underDirectory);
    chdir ($underDirectory);
    rename ("kdelnk_template", $nameLittle . ".kdelnk");
    $word = PROJECT_NAME;
    $oldfile = $nameLittle . ".kdelnk";
    $replace = $nameLittle;
    replaceOldFile($word,$replace,$oldfile);
  }

  #if no ProgIcon was chosen in kAppWizard
  if ($processes{PROGICON} eq "no") {} 
  
  #if the default ProgIcon was chosen in kAppWizard
  elsif ($processes{PROGICON} eq "(null)") {
    chdir ($kdedirectory);
    copy ("share/icons/edit.xpm", $underDirectory);
    chdir ($underDirectory);
    chmod (0666,"edit.xpm");
    rename ("edit.xpm", $nameLittle . ".xpm");
  }
  
  #if a new ProgIcon was chosen in kAppWizard
  else {
    $icon = $kdedirectory . "/share/icons/" . $processes{PROGICON};
    $copying = copy ($icon, $underDirectory);
    if ($copying)  {
      $icon = $homedirectory . "/.kde/share/icons/" . $processes{PROGICON};
      copy ($icon, $underDirectory);
    }
    chmod (0666,$processes{PROGICON});
    rename ($processes{PROGICON}, $nameLittle . ".xpm");
  }
  
  #if no MiniIcon was chosen in kAppWizard
  if ($processes{MINIICON} eq "no") {}
  
  #if the default MiniIcon was chosen in kAppWizard
  elsif ($processes{MINIICON} eq "(null)") {
    chdir ($kdedirectory);
    copy ("share/icons/mini/application_settings.xpm", $underDirectory);
    chdir ($underDirectory);
    chmod (0666,"application_settings.xpm");
    rename ("application_settings.xpm","mini-" . $nameLittle . ".xpm");
  }
  
  #if a new MiniIcon was chosen in kAppWizard
  else {
    $icon = $kdedirectory . "/share/icons/mini/" . $processes{MINIICON};
    $copying = copy ($icon, $underDirectory);
    if ($copying)  {
      $icon = $homedirectory . "/.kde/share/icons/mini/" . $processes{MINIICON};
      copy ($icon, $underDirectory);
    }
    chmod (0666,$processes{MINIICON});
    rename ($processes{MINIICON}, "mini-" . $nameLittle . ".xpm");
  }
  
  #if API-Files was chosen in kAppWizard
  if ($processes{API} eq "yes") {
    
    #create the API-documentation
    chdir ($underDirectory);
    mkdir ("api",0777);
    system ("kdoc -d ./api/ $name *.h");
  }
}

sub changeQtApp() {
  #copy the file in the projectdirectory and unpacked it
  chdir ($overDirectory);
  printflush (STDOUT,"unzip file...\n");
  system ("gunzip qt.tar.gz");
  printflush (STDOUT,"untar file...\n");
  system ("tar -xf qt.tar");
  unlink "qt.tar";

  #renamed the directory
  printflush (STDOUT,"change files...\n");
  chdir ($overDirectory);
  rename ("skel", $nameLittle);

  chdir ($underDirectory);
  rename ("bank.cpp","skel.cpp");
  rename ("bank.h","skel.h");
  rename ("bankdoc.cpp",$nameLittle . "doc.cpp");
  rename ("bankdoc.h",$nameLittle . "doc.h");
  rename ("bankview.cpp",$nameLittle . "view.cpp");
  rename ("bankview.h",$nameLittle . "view.h");

  #create the templatedirectory
  chdir ($underDirectory);
  mkdir ("templates",0777);

  #copying the templates in the templatedirectoy
  if ($processes{CPP} eq "no" and $processes{HEADER} eq "no") {}
  else {
    if ($processes{HEADER} eq "yes") {
      $directory = $homedirectory . "/.kde/share/apps/kdevelop/header";
      $targetdirectory = $underDirectory . "/templates";
      copy ($directory, $targetdirectory);
      chdir ($targetdirectory);
      rename ("header","header_template");
    }
    
    if ($processes{CPP} eq "yes") {
      $directory = $homedirectory . "/.kde/share/apps/kdevelop/cpp";
      $targetdirectory = $underDirectory . "/templates";
      copy ($directory, $targetdirectory);
      chdir ($targetdirectory);
      rename ("cpp","cpp_template");
    }
  }

  #replaced skel with the projectname in different files
  chdir ($overDirectory);
  $word = "skel";
  $oldfile = "Makefile.am";
  $replace = $nameLittle;
  replaceOldFile($word,$replace,$oldfile);
  $oldfile = "configure.in";
  replaceOldFile($word,$replace,$oldfile);
  $word = "VERSION";
  $replace = $processes{VERSION};
  replaceOldFile($word,$replace,$oldfile);
  
  chdir ($underDirectory);
  $word = "skel";
  $oldfile = "Makefile.am";
  $replace = $nameLittle;
  replaceOldFile($word,$replace,$oldfile);

  $word = "bank";
  $oldfile = "main.cpp";
  replaceOldFile($word,$replace,$oldfile);

  $oldfile = "skel.cpp";
  replaceOldFile($word,$replace,$oldfile);

  $oldfile = "skel.h";
  replaceOldFile($word,$replace,$oldfile);

  #replaced Bank with the projectname in different files
  $word = "Bank";
  $oldfile = "main.cpp";
  $replace = $name;
  replaceOldFile($word,$replace,$oldfile);
  $oldfile = "skel.cpp";
  replaceOldFile($word,$replace,$oldfile);
  $oldfile = "skel.h";
  replaceOldFile($word,$replace,$oldfile);
  
  #replaced AUTHOR with the authorname in bank.cpp
  $word = "AUTHOR";
  $oldfile = "skel.cpp";
  $replace = $processes{AUTHOR};
  replaceOldFile($word,$replace,$oldfile);
  
  #replaced BANK with the projectname in bank.h
  $word = "BANK";
  $oldfile = "skel.h";
  $replace = $nameBig;
  replaceOldFile($word,$replace,$oldfile);
  
  #renamed bank with the projectname
  rename ("skel.cpp", $nameLittle . ".cpp");
  rename ("skel.h", $nameLittle . ".h");

  chdir ($underDirectory);
  $word = "Bank";
  $oldfile = $nameLittle . "doc.cpp";
  $replace = $name;
  replaceOldFile($word,$replace,$oldfile);
  $oldfile = $nameLittle . "doc.h";
  replaceOldFile($word,$replace,$oldfile);
  $oldfile = $nameLittle . "view.cpp";
  replaceOldFile($word,$replace,$oldfile);
  $oldfile = $nameLittle . "view.h";
  replaceOldFile($word,$replace,$oldfile);
  $oldfile = "resource.h";
  replaceOldFile($word,$replace,$oldfile);
  $word = "BANK";
  $replace = $nameBig;
  $oldfile = $nameLittle . "doc.h";
  replaceOldFile($word,$replace,$oldfile);
  $oldfile = $nameLittle . "view.h";
  replaceOldFile($word,$replace,$oldfile);
  $word = "bank";
  $oldfile = $nameLittle . "doc.cpp";
  $replace = $nameLittle;
  replaceOldFile($word,$replace,$oldfile);
  $oldfile = $nameLittle . "view.cpp";
  replaceOldFile($word,$replace,$oldfile);
  $oldfile = $nameLittle . "view.h";
  replaceOldFile($word,$replace,$oldfile);

  chdir ($underDirectory . "/docs/en");
  $word = "bank";
  $oldfile = "Makefile.am";
  $replace = $nameLittle;
  replaceOldFile($word,$replace,$oldfile);

  #if GNU-Files was chosen in kAppWizard
  if ($processes{GNU} eq "yes") {
  
    #copying the GNU-Files and renamed these
    chdir ($kdedirectory);
    copy ("share/apps/kdevelop/templates/AUTHORS_template", $overDirectory);
    copy ("share/apps/kdevelop/templates/COPYING_template", $overDirectory);
    copy ("share/apps/kdevelop/templates/ChangeLog_template", $overDirectory);
    copy ("share/apps/kdevelop/templates/INSTALL_template", $overDirectory);
    copy ("share/apps/kdevelop/templates/README_template", $overDirectory);
    copy ("share/apps/kdevelop/templates/TODO_template", $overDirectory);
    chdir ($overDirectory);
    rename ("AUTHORS_template", "AUTHORS");
    rename ("COPYING_template", "COPYING");
    rename ("ChangeLog_template", "ChangeLog");
    rename ("INSTALL_template", "INSTALL");
    rename ("README_template", "README");
    rename ("TODO_template", "TODO");
    
    # replaced AUTHOR and EMAIL
    $word = "AUTHOR";
    $replace = $processes{AUTHOR};
    $oldfile = "AUTHORS";
    replaceOldFile($word,$replace,$oldfile);
    $word = "EMAIL";
    $replace = $processes{EMAIL};
    $oldfile = "AUTHORS";
    replaceOldFile($word,$replace,$oldfile);
  }
  #if LSM-Files was chosen in kAppWizard
  if ($processes{LSM} eq "yes") {
    
    #copying, rename and replace in the lsm-template
    chdir ($kdedirectory);
    copy ("share/apps/kdevelop/templates/lsm_template", $overDirectory);
    chdir ($overDirectory);
    rename ("lsm_template", $nameLittle . ".lsm");
    $oldfile = $nameLittle . ".lsm";
    $word = "PROJECT_NAME";
    $replace = $name;
    replaceOldFile($word,$replace,$oldfile);
    $word = "AUTHOR";
    $replace = $processes{AUTHOR};
    replaceOldFile($word,$replace,$oldfile);
    $word = "EMAIL";
    $replace = $processes{EMAIL};
    replaceOldFile($word,$replace,$oldfile);
    $word = "VERSION";
    $replace = $processes{VERSION};
    replaceOldFile($word,$replace,$oldfile); 
  }

  #if USER-Docs was chosen in kAppWizard
  if ($processes{USER} eq "yes") {
    
    #copying, rename and replace in the handbook-en-template
    chdir ($kdedirectory);
    $targetdirectory = $underDirectory . "/docs/en";
    copy ("share/apps/kdevelop/templates/handbook_en_template", $targetdirectory);
    chdir ($targetdirectory);
    rename ("handbook_en_template", "index.sgml");
    $oldfile = "index.sgml";
    $word = "PROJECT_NAME";
    $replace = $name;
    replaceOldFile($word,$replace,$oldfile);
    $word = "AUTHOR";
    $replace = $processes{AUTHOR};
    replaceOldFile($word,$replace,$oldfile);
    $word = "EMAIL";
    $replace = $processes{EMAIL};
    replaceOldFile($word,$replace,$oldfile);
    $word = "VERSION";
    $replace = $processes{VERSION};
    replaceOldFile($word,$replace,$oldfile);
    $word = "DATE";
    $replace = $date;
    replaceOldFile($word,$replace,$oldfile);
    $word = "YEAR";
    $replace = $year;
    replaceOldFile($word,$replace,$oldfile);
  }

  #if USER-Docs was not chosen in kAppWizard
  if ($processes{USER} eq "no") {
    chdir ($underDirectory);
    $word = "SUBDIRS = docs";
    $replace = ""; 
    $oldfile = "Makefile.am";
    replaceOldFile($word,$replace,$oldfile);
    chdir ($overDirectory);
    $word = $nameLittle . "/docs/Makefile \\";
    $replace = ""; 
    $oldfile = "configure.in";
    replaceOldFile(  $word,$replace,$oldfile);
    $word = $nameLittle . "/docs/en/Makefile \\";
    replaceOldFile($word,$replace ,$oldfile);
  }
  
  #if cpp-template was chosen in kAppWizard
  if ($processes{CPP} eq "yes") {
    chdir ($homedirectory);
    copy (".kde/share/apps/kdevelop/cpp", $underDirectory);
    chdir ($underDirectory);
    $oldfile = "cpp";
    $word = "FILENAME";
    $replace = "main.cpp";
    replaceOldFile($word,$replace,$oldfile);
    $word = "AUTHOR";
    $replace = $processes{AUTHOR};
    replaceOldFile($word,$replace,$oldfile);
    $word = "EMAIL";
    $replace = $processes{EMAIL};
    replaceOldFile($word,$replace,$oldfile);
    $word = "DATE";
    $replace = $date;
    replaceOldFile($word,$replace,$oldfile);
    $word = "YEAR";
    $replace = $year;
    replaceOldFile($word,$replace,$oldfile);
    $file = "main.cpp";
    open (INPUT,"$file") || die "kann Datei nicht öffnen: $!";
    open (OUTPUT,">>cpp");
    while ( defined ($line = <INPUT> )) {
      print OUTPUT $line;
    }
    close (INPUT);
    close (OUTPUT);
    rename ("cpp" , $file);
    unlink ("cpp");
    #copying, rename and replace in the cpp-file
    chdir ($homedirectory);
    copy (".kde/share/apps/kdevelop/cpp", $underDirectory);
    chdir ($underDirectory);
    $oldfile = "cpp";
    $word = "FILENAME";
    $replace = $nameLittle . ".cpp";
    replaceOldFile($word,$replace,$oldfile);
    $word = "AUTHOR";
    $replace = $processes{AUTHOR};
    replaceOldFile($word,$replace,$oldfile);
    $word = "EMAIL";
    $replace = $processes{EMAIL};
    replaceOldFile($word,$replace,$oldfile);
    $word = "DATE";
    $replace = $date;
    replaceOldFile($word,$replace,$oldfile);
    $word = "YEAR";
    $replace = $year;
    replaceOldFile($word,$replace,$oldfile);
    # summarized the files cpp and project.cpp
    $file = $nameLittle . ".cpp";
    open (INPUT,"$file") || die "kann Datei nicht öffnen: $!";
    open (OUTPUT,">>cpp");
    while ( defined ($line = <INPUT> )) {
      print OUTPUT $line;
    }
    close (INPUT);
    close (OUTPUT);
    rename ("cpp" , $file);
    unlink ("cpp");
    
    chdir ($homedirectory);
    copy (".kde/share/apps/kdevelop/cpp", $underDirectory);
    chdir ($underDirectory);
    $oldfile = "cpp";
    $word = "FILENAME";
    $replace = $nameLittle . "view.cpp";
    replaceOldFile($word,$replace,$oldfile);
    $word = "AUTHOR";
    $replace = $processes{AUTHOR};
    replaceOldFile($word,$replace,$oldfile);
    $word = "EMAIL";
    $replace = $processes{EMAIL};
    replaceOldFile($word,$replace,$oldfile);
    $word = "DATE";
    $replace = $date;
    replaceOldFile($word,$replace,$oldfile);
    $word = "YEAR";
    $replace = $year;
    replaceOldFile($word,$replace,$oldfile);
    
    # summarized the files cpp and projectview.cpp
    $file = $nameLittle . "view.cpp";
    open (INPUT,"$file") || die "kann Datei nicht öffnen: $!";
    open (OUTPUT,">>cpp");
    while ( defined ($line = <INPUT> )) {
      print OUTPUT $line;
    }
    close (INPUT);
    close (OUTPUT);
    rename ("cpp" , $file);
    unlink ("cpp");
    
    chdir ($homedirectory);
    copy (".kde/share/apps/kdevelop/cpp", $underDirectory);
    chdir ($underDirectory);
    $oldfile = "cpp";
    $word = "FILENAME";
    $replace = $nameLittle . "doc.cpp";
    replaceOldFile($word,$replace,$oldfile);
    $word = "AUTHOR";
    $replace = $processes{AUTHOR};
    replaceOldFile($word,$replace,$oldfile);
    $word = "EMAIL";
    $replace = $processes{EMAIL};
    replaceOldFile($word,$replace,$oldfile);
    $word = "DATE";
    $replace = $date;
    replaceOldFile($word,$replace,$oldfile);
    $word = "YEAR";
    $replace = $year;
    replaceOldFile($word,$replace,$oldfile);
    
    # summarized the files cpp and projectdoc.cpp
    $file = $nameLittle . "doc.cpp";
    open (INPUT,"$file") || die "kann Datei nicht öffnen: $!";
    open (OUTPUT,">>cpp");
    while ( defined ($line = <INPUT> )) {
      print OUTPUT $line;
    }
    close (INPUT);
    close (OUTPUT);
    rename ("cpp" , $file);
    unlink ("cpp");
}

  #if header-template was chosen in kAppWizard
  if ($processes{HEADER} eq "yes") {
    #copying, rename and replace in the header-file
    chdir ($homedirectory);
    copy (".kde/share/apps/kdevelop/header", $underDirectory);
    chdir ($underDirectory);
    $oldfile = "header";
    $word = "FILENAME";
    $replace = $nameLittle . ".h";
    replaceOldFile($word,$replace,$oldfile);
    $word = "AUTHOR";
    $replace = $processes{AUTHOR};
    replaceOldFile($word,$replace,$oldfile);
    $word = "EMAIL";
    $replace = $processes{EMAIL};
    replaceOldFile($word,$replace,$oldfile);
    $word = "DATE";
    $replace = $date;
    replaceOldFile($word,$replace,$oldfile);
    $word = "YEAR";
    $replace = $year;
    replaceOldFile($word,$replace,$oldfile);
    # summarized the files header and project.h
    $file = $nameLittle . ".h";
    open (INPUT,"$file") || die "kann Datei nicht öffnen: $!";
    open (OUTPUT,">>header");
    while ( defined ($line = <INPUT> )) {
      print OUTPUT $line;
    }
    close (INPUT);
    close (OUTPUT);
    rename ("header" , $file);
    unlink ("header");
  
    chdir ($homedirectory);
    copy (".kde/share/apps/kdevelop/header",$underDirectory);
    chdir ($underDirectory);
    $oldfile = "header";
    $word = "FILENAME";
    $replace = $nameLittle . "view.h";
    replaceOldFile($word,$replace,$oldfile);
    $word = "AUTHOR";
    $replace = $processes{AUTHOR};
    replaceOldFile($word,$replace,$oldfile);
    $word = "EMAIL";
    $replace = $processes{EMAIL};
    replaceOldFile($word,$replace,$oldfile);
    $word = "DATE";
    $replace = $date;
    replaceOldFile($word,$replace,$oldfile);
    $word = "YEAR";
    $replace = $year;
    replaceOldFile($word,$replace,$oldfile);
    
    # summarized the files header and project.h
    $file = $nameLittle . "view.h";
    open (INPUT,"$file") || die "kann Datei nicht öffnen: $!";
    open (OUTPUT,">>header");
    while ( defined ($line = <INPUT> )) {
      print OUTPUT $line;
    }
    close (INPUT);
    close (OUTPUT);
    rename ("header" , $file);
    unlink ("header");
    chdir ($homedirectory);
    copy (".kde/share/apps/kdevelop/header", $underDirectory);
    chdir ($underDirectory);
    $oldfile = "header";
    $word = "FILENAME";
    $replace = $nameLittle . "doc.h";
    replaceOldFile($word,$replace,$oldfile);
    $word = "AUTHOR";
    $replace = $processes{AUTHOR};
    replaceOldFile($word,$replace,$oldfile);
    $word = "EMAIL";
    $replace = $processes{EMAIL};
    replaceOldFile($word,$replace,$oldfile);
    $word = "DATE";
    $replace = $date;
    replaceOldFile($word,$replace,$oldfile);
    $word = "YEAR";
    $replace = $year;
    replaceOldFile($word,$replace,$oldfile);
    
    # summarized the files header and project.h
    $file = $nameLittle . "doc.h";
    open (INPUT,"$file") || die "kann Datei nicht öffnen: $!";
    open (OUTPUT,">>header");
    while ( defined ($line = <INPUT> )) {
      print OUTPUT $line;
    }
    close (INPUT);
    close (OUTPUT);
    rename ("header" , $file);
    unlink ("header");
    chdir ($homedirectory);
    copy (".kde/share/apps/kdevelop/header", $underDirectory);
    chdir ($underDirectory);
    $oldfile = "header";
    $word = "FILENAME";
    $replace = "resource.h";
    replaceOldFile($word,$replace,$oldfile);
    $word = "AUTHOR";
    $replace = $processes{AUTHOR};
    replaceOldFile($word,$replace,$oldfile);
    $word = "EMAIL";
    $replace = $processes{EMAIL};
    replaceOldFile($word,$replace,$oldfile);
    $word = "DATE";
    $replace = $date;
    replaceOldFile($word,$replace,$oldfile);
    $word = "YEAR";
    $replace = $year;
    replaceOldFile($word,$replace,$oldfile);
    
    # summarized the files header and project.h
    $file = "resource.h";
    open (INPUT,"$file") || die "kann Datei nicht öffnen: $!";
    open (OUTPUT,">>header");
    while ( defined ($line = <INPUT> )) {
      print OUTPUT $line;
    }
    close (INPUT);
    close (OUTPUT);
    rename ("header" , $file);
    unlink ("header");
  }

  #if .kdelnk-file was chosen in kAppWizard
  if ($processes{KDELNK} eq "yes") {
  
    #copying, rename and replace in the kdelnk-file
    chdir ($kdedirectory);
    copy ("share/apps/kdevelop/templates/kdelnk_template", $underDirectory);
    chdir ($underDirectory);
    rename ("kdelnk_template", $nameLittle . ".kdelnk");
    $word = PROJECT_NAME;
    $oldfile = $nameLittle . ".kdelnk";
    $replace = $nameLittle;
    replaceOldFile($word,$replace,$oldfile);
  }

  #if no ProgIcon was chosen in kAppWizard
  if ($processes{PROGICON} eq "no") {} 
  
  #if the default ProgIcon was chosen in kAppWizard
  elsif ($processes{PROGICON} eq "(null)") {
    chdir ($kdedirectory);
    copy ("share/icons/edit.xpm", $underDirectory);
    chdir ($underDirectory);
    chmod (0666,"edit.xpm");
    rename ("edit.xpm", $nameLittle . ".xpm");
  }

  #if a new ProgIcon was chosen in kAppWizard
  else {
    $icon = $kdedirectory . "/share/icons/" . $processes{PROGICON};
    $copying = copy ($icon, $underDirectory);
    if ($copying)  {
      $icon = $homedirectory . "/.kde/share/icons/" . $processes{PROGICON};
      copy ($icon, $underDirectory);
    }
    chmod (0666,$processes{PROGICON});
    rename ($processes{PROGICON}, $nameLittle . ".xpm");
  }

  #if no MiniIcon was chosen in kAppWizard
  if ($processes{MINIICON} eq "no") {}
  
  #if the default MiniIcon was chosen in kAppWizard
  elsif ($processes{MINIICON} eq "(null)") {
    chdir ($kdedirectory);
    copy ("share/icons/mini/application_settings.xpm", $underDirectory);
    chdir ($underDirectory);
    chmod (0666,"application_settings.xpm");
    rename ("application_settings.xpm","mini-" . $nameLittle . ".xpm");
  }

  #if a new MiniIcon was chosen in kAppWizard
  else {
    $icon = $kdedirectory . "/share/icons/mini/" . $processes{MINIICON};
    $copying = copy ($icon, $underDirectory);
    if ($copying)  {
      $icon = $homedirectory . "/.kde/share/icons/mini/" . $processes{MINIICON};
      copy ($icon, $underDirectory);
    }
    chmod (0666,$processes{MINIICON});
    rename ($processes{MINIICON}, "mini-" . $nameLittle . ".xpm");
  }
  
  #if API-Files was chosen in kAppWizard
  if ($processes{API} eq "yes") {
    
    #create the API-documentation
    chdir ($underDirectory);
    mkdir ("api",0777);
    system ("kdoc -d ./api/ $name *.h");
  }
}
