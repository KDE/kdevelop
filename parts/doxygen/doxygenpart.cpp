/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "doxygenpart.h"

#include <qvbox.h>
#include <qfile.h>
#include <qtextstream.h>

#include <kdebug.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <kaction.h>
#include <kmessagebox.h>
#include <kprocess.h>

#include "kdevproject.h"
#include "kdevmakefrontend.h"
#include "kdevcore.h"
#include "doxygenconfigwidget.h"
#include "domutil.h"


#include "config.h"

/**
\class DoxygenPart
Integrates Doxygen - http://www.doxygen.org into KDevelop.
It allows you to call the Doxygen executable from a KDevelop menu.
It integrates a GUI into the "KDevelop Project Configuration dialog" to
configure Doxygen (it reads and writes Doxygen Conpatible configuration files).

To update to a newer doxygen version:

1. Merge over the config.h and config.cpp files from the addon/doxywizard/
   directory in the doxygen sources.
   You'll need:
   - GNU bison v1.34 or better
   - GNU flex v2.5.4 or better
   - Kompare from KDEsdk

   You'll have to issue:<br>
       <code>$ ./configure --with-doxywizard && make</code><br>
   to get the config.h and config.cpp files on this directory.
   You should also copy config.l if you'd like to do some debugging.

   To do the merge on config.cpp you should use:<br>
       <code>$ kompare doxygen-location/addon/doxywizard/config.cpp  kdevelop-location/parts/doxygen/config.cpp</code><br>
   - Be careful because doxygen uses the deprecated QList class while KDevelop
   uses the new QPtrList so you should not merge certain lines of code!!!
   - Be careful because doxygen uses the deprecated QRegExp::match() method while
   KDevelop uses the new QRegExp::search() and QRegExp::matchedLength() mehtods
   instead so you should not merge certain lines of code!!!
   - Be careful because KDevlop uses the QFile::encodeName() function and the
   original doxygen code doesn't. So do NOT delete the QFile::encodeName() calls.

   To do the merge on config.h you should use:
          <code>$ kompare doxygen-location/addon/doxywizard/config.h    kdevelop-location/parts/doxygen/config.h</code>
   - Again, be careful because doxygen uses the deprecated QList class while KDevelop
   uses the new QPtrList so you should not merge certain lines of code!!!


2. Copy over the file lang_cfg.h from the src/ directory in the doxygen sources.

3. Add the new i18n strings to messages.cpp.
   To do so, run KDevelop and go to Project-> Project Options -> Doxygen.
   Take a look at KDevelop' console output, it will warn you about the missing
   messages. Add them to messages.cpp file.

4. Update version.cpp

5. Add anything else you needed to do, to get it to compile, to the doxygenpart.cpp file.


Everything else we can customize for our look & feel.


\maintainer <a href="mailto:a.lucas at tu-bs dot de">Amilcar Lucas</a>


\feature Call Doxygen executable from a KDevelop menu.
\feature Configure Doxygen inside KDevelop.
\feature Create a configuration file with correct ProjectName, Author, Version and
    location of source files whenever a new project KDevelop is created.
\feature Automaticaly generate a Doxygen search database whenever you "Run Doxygen"
  and the "search engine" option in Doxygen configuration is activated.


\requirement You need to install the Doxygen binaries in your machine. This part supports
all versions up to the version stated on the version.cpp file. As usual it is
recomended to use the latest version.
\requirement It is recomended that you also install dot (tool to generate graphical
dependencies).


\bug doesn't resize the configuration GUI properly.

*/

typedef KGenericFactory<DoxygenPart> DoxygenFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevdoxygen, DoxygenFactory( "kdevdoxygen" ) );

DoxygenPart::DoxygenPart(QObject *parent, const char *name, const QStringList &)
    : KDevPlugin("Doxgen", "doxygen", parent, name ? name : "DoxygenPart")
{
    setInstance(DoxygenFactory::instance());
    setXMLFile("kdevdoxygen.rc");

    KAction *action;

    action = new KAction( i18n("Build API Documentation"), 0,
                          this, SLOT(slotDoxygen()),
                          actionCollection(), "build_doxygen" );

    action = new KAction( i18n("Clean API Documentation"), 0,
                          this, SLOT(slotDoxClean()),
                          actionCollection(), "clean_doxygen" );

    connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
             this, SLOT(projectConfigWidget(KDialogBase*)) );
}


DoxygenPart::~DoxygenPart()
{
}


void DoxygenPart::projectConfigWidget(KDialogBase *dlg)
{
    adjustDoxyfile();

    QVBox *vbox;
    vbox = dlg->addVBoxPage(i18n("Doxygen"));
    DoxygenConfigWidget *w = new DoxygenConfigWidget(project()->projectDirectory() + "/Doxyfile", vbox);
    connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
}


/** If a Doxygen configuration file doesn't exist, create one.
  * And copy some of the project settings to it.
  */
void DoxygenPart::adjustDoxyfile()
{
  QString fileName = project()->projectDirectory() + "/Doxyfile";
  if (QFile::exists(fileName))
    return;

  // Initialize configuration
  Config::instance()->init();

  // Do some checks and improve the configuration a bit
  Config::instance()->check();

  QFile f(fileName);
  if (f.open(IO_ReadOnly))
  {
    QTextStream is(&f);

    Config::instance()->parse(QFile::encodeName(fileName));
    Config::instance()->convertStrToVal();

    f.close();
  }

  // set "General/PROJECT_NAME"
  ConfigString *name = dynamic_cast<ConfigString*>(Config::instance()->get("PROJECT_NAME"));
  if (name)
  {
    name->setDefaultValue(project()->projectName().latin1());
    name->init();
  }

  // set "General/PROJECT_NUMBER"
  ConfigString *version = dynamic_cast<ConfigString*>(Config::instance()->get("PROJECT_NUMBER"));
  if (version)
  {
    version->setDefaultValue(DomUtil::readEntry(*projectDom(), "/general/version").latin1());
    version->init();
  }

  // insert input files into "Input/INPUT"
  ConfigList *input_files = dynamic_cast<ConfigList*>(Config::instance()->get("INPUT"));
  if (input_files)
  {
    input_files->init();
    input_files->addValue(QFile::encodeName(project()->projectDirectory()));
  }

  // insert file patterns into "Input/FILE_PATTERNS"
  ConfigList *patterns = dynamic_cast<ConfigList*>(Config::instance()->get("FILE_PATTERNS"));
  if (patterns)
  {
    // Remove Doxygen's default patterns
//    patterns->init();

    // Add this ones:
    patterns->addValue("*.C");
    patterns->addValue("*.H");
    patterns->addValue("*.tlh");
    patterns->addValue("*.diff");
    patterns->addValue("*.patch");
    patterns->addValue("*.moc");
    patterns->addValue("*.xpm");
    patterns->addValue("*.dox");
  }

  // set "Input/RECURSIVE" to recurse into subdirectories
  ConfigBool *recursive = dynamic_cast<ConfigBool*>(Config::instance()->get("RECURSIVE"));
  if (recursive)
  {
    recursive->setValueString("yes");
  }

  // set "XML/GENERATE_XML" to generate XML information to be used with code hinting
  ConfigBool *gen_xml = dynamic_cast<ConfigBool*>(Config::instance()->get("GENERATE_XML"));
  if (gen_xml)
  {
    gen_xml->setValueString("yes");
  }

  // write doxy file
  QFile f2(fileName);
  if (!f2.open(IO_WriteOnly))
    KMessageBox::information(0, i18n("Cannot write Doxyfile."));
  else
  {
    Config::instance()->writeTemplate(&f2, true, true);

    f2.close();
  }
}


void DoxygenPart::slotDoxygen()
{
    bool searchDatabase = false;
    QString outputDirectory;
    QString htmlDirectory;

    adjustDoxyfile();

    QString fileName = project()->projectDirectory() + "/Doxyfile";

    Config::instance()->init();

    QFile f(fileName);
    if (f.open(IO_ReadOnly))
    {
      QTextStream is(&f);

      Config::instance()->parse(QFile::encodeName(fileName));
      Config::instance()->convertStrToVal();

      f.close();
    }

    // search engine
    ConfigBool *search = dynamic_cast<ConfigBool*>(Config::instance()->get("SEARCHENGINE"));
    if (search)
    {
      searchDatabase = Config_getBool("SEARCHENGINE");

      if (searchDatabase)
      {
        // get input files
        outputDirectory = Config_getString("OUTPUT_DIRECTORY");
        if ( outputDirectory.isEmpty() == false )
          outputDirectory += "/";
        htmlDirectory   = Config_getString("HTML_OUTPUT");
        if ( htmlDirectory.isEmpty() == true )
          htmlDirectory = "html";
        htmlDirectory.prepend(outputDirectory);
      }
    }

    QString dir = project()->projectDirectory();
    QString cmdline = "cd ";
    cmdline += KShellProcess::quote( dir );
    cmdline += " && doxygen Doxyfile";
    if (searchDatabase)
    {
      // create search database in the same directory where the html docs are
      if ( htmlDirectory.length() > 0 )
        cmdline += " && cd " + KShellProcess::quote( htmlDirectory );
      cmdline += " && doxytag -s search.idx ";
    }

    kdDebug(9026) << "Doxygen command line: " << cmdline << endl;

    makeFrontend()->queueCommand(dir, cmdline);
}


void DoxygenPart::slotDoxClean()
{

    QString outputDirectory = Config_getString("OUTPUT_DIRECTORY");
    if ( outputDirectory.isEmpty() )
        outputDirectory = project()->projectDirectory();
    if ( outputDirectory.right(1) != "/" )
        outputDirectory += "/";

    QString htmlDirectory   = Config_getString("HTML_OUTPUT");
    if ( htmlDirectory.isEmpty() )
        htmlDirectory = "html";
    if ( htmlDirectory.right(1) != "/" )
        htmlDirectory += "/";

    QString cmdline = "rm -f " + KShellProcess::quote( outputDirectory + htmlDirectory + "*");
    
    kdDebug(9026) << "Cleaning Doxygen html generated API documentation using: " << cmdline << endl;

    makeFrontend()->queueCommand(KShellProcess::quote(project()->projectDirectory()), cmdline);
}


#include "doxygenpart.moc"
