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


#include "config.h"


typedef KGenericFactory<DoxygenPart> DoxygenFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevdoxygen, DoxygenFactory( "kdevdoxygen" ) );

DoxygenPart::DoxygenPart(QObject *parent, const char *name, const QStringList &)
    : KDevPlugin(parent, name ? name : "DoxygenPart")
{
    setInstance(DoxygenFactory::instance());
    setXMLFile("kdevdoxygen.rc");

    KAction *action;

    action = new KAction( i18n("Run Doxygen"), 0,
                          this, SLOT(slotDoxygen()),
                          actionCollection(), "build_doxygen" );

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


void DoxygenPart::adjustDoxyfile()
{
  QString fileName = project()->projectDirectory() + "/Doxyfile";
  if (QFile::exists(fileName))
    return;

  Config::instance()->init();
  Config::instance()->check();

  QFile f(fileName);
  if (f.open(IO_ReadOnly))
  {
    QTextStream is(&f);

    Config::instance()->parse(QFile::encodeName(fileName));
    Config::instance()->convertStrToVal();

    f.close();
  }

  // insert input files
  // set "General/PROJECT_NAME"
  ConfigString *name = dynamic_cast<ConfigString*>(Config::instance()->get("PROJECT_NAME"));
  if (name)
  {
    name->setDefaultValue(project()->projectName());
    name->init();
  }
  // set "General/PROJECT_NUMBER"
  ConfigString *version = dynamic_cast<ConfigString*>(Config::instance()->get("PROJECT_NUMBER"));
  if (version) 
  {
    version->setDefaultValue("Someone should replace this with a pointer to the version");
    version->init();
  }

  //insert input files into "Input/INPUT"  
  ConfigList *input_files = dynamic_cast<ConfigList*>(Config::instance()->get("INPUT"));
  if (input_files)
  {
    input_files->init();
    input_files->addValue(QFile::encodeName(project()->projectDirectory()));
  }

  // insert file patters into "Input/FILE_PATTERNS"
  ConfigList *patterns = dynamic_cast<ConfigList*>(Config::instance()->get("FILE_PATTERNS"));
  if (patterns)
  {
    //patterns->init();
    patterns->addValue("*.C");
    patterns->addValue("*.tlh");
    patterns->addValue("*.diff");
    patterns->addValue("*.patch");
    patterns->addValue("*.moc");
    patterns->addValue("*.xpm");
    patterns->addValue("*.hxx");
    patterns->addValue("*.h++");
    patterns->addValue("*.H");
  }

  // recurse
  // set "Input/RECURSIVE" to recurse into subdirectories
  ConfigBool *recursive = dynamic_cast<ConfigBool*>(Config::instance()->get("RECURSIVE"));
  if (recursive)
  {
    recursive->setValueString("yes");
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
	htmlDirectory = Config_getString("OUTPUT_DIRECTORY");
	if ( htmlDirectory.length() > 0 )
          htmlDirectory += "/";
        htmlDirectory += Config_getString("HTML_OUTPUT");
      }
    }

    QString dir = project()->projectDirectory();
    QString cmdline = "cd ";
    cmdline += KShellProcess::quote( dir );
    cmdline += " && doxygen Doxyfile";
    if (searchDatabase)
    {
      cmdline += " && doxytag -s search.idx ";
      cmdline += KShellProcess::quote( htmlDirectory );
    }

    kdDebug(9026) << "Doxygen command line: " << cmdline << endl;

    makeFrontend()->queueCommand(dir, cmdline);
}

#include "doxygenpart.moc"
