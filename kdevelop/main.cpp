/***************************************************************************
                     main.cpp - the main-function
                             -------------------

    begin                : 20 Jul 1998
    copyright            : (C) 1998 by Sandy Meier
    email                : smeier@rz.uni-potsdam.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ckdevelop.h"
#include "ckdevinstall.h"
#include "kstartuplogo.h"

#include <dcopclient.h>
#include <kaboutdata.h>
#include <kapp.h>
#include <kconfig.h>
#include <kcmdlineargs.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

static const char *description =
        I18N_NOOP("The KDevelop C/C++ Integrated Development Environment");

static KCmdLineOptions options[] =
{
   { "setup", I18N_NOOP("Setup KDevelop"), 0 },
   { "+[file]", I18N_NOOP("The file to open"), 0 },
   { 0, 0, 0 }
};

int main(int argc, char* argv[])
{
  KConfig* config;
  KStartupLogo* start_logo;

  KAboutData aboutData( "kdevelop",
                        I18N_NOOP("KDevelop"),
                        VERSION,
                        description,
                        KAboutData::License_GPL,
                        "(c) 1998-2001, The KDevelop Team",
                        I18N_NOOP("The KDevelop C/C++ IDE - a success story "
                                  "in the world of free software since 1998.\n\n"
                                  "Think about joining our project to contribute "
                                  "and enhance the product you are using for your\n"
                                  "everyday work - or think about writing a KDE "
                                  "application that will contribute to the KDE project.\n\n"
                                  "For questions about this product, its usage and license issues,\n"
                                  "please contact the KDE project (http://www.kde.org) or any \n"
                                  "of the team officials in the Authors section."),
                                  "http://www.kdevelop.org"  );

  aboutData.addAuthor("Sandy Meier",I18N_NOOP("Maintainer"), "smeier@kdevelop.org");
  aboutData.addAuthor("Ralf Nolden",I18N_NOOP("Configuration Functionality,"
                          "Online Help, Popups, port of KDevelop 1.3 to KDE 2.1"),"nolden@kde.org");
  aboutData.addAuthor("Walter Tasin",I18N_NOOP("Many, Many Bugfixes, General Enhancements"), "tasin@kdevelop.org");
  aboutData.addAuthor("Bernd Gehrmann",I18N_NOOP("Documentation Tree"), "bernd@physik.hu-berlin.de");
  aboutData.addAuthor("Jonas Nordin",I18N_NOOP("Classviewer and Classparser"), "jonas.nordin@syncom.se");
  aboutData.addAuthor("Jörgen Olsson",I18N_NOOP("Graphical Classviewer"),"jorgen@trej.net");
  aboutData.addAuthor("Stefan Heidrich",I18N_NOOP("KAppWizard, Printing"), "sheidric@rz.uni-potsdam.de");
  aboutData.addAuthor("Stefan Bartel",I18N_NOOP("Real-File-Viewer, Project Options"), "bartel@rz.uni-potsdam.de");
  aboutData.addAuthor("John Birch",I18N_NOOP("Internal Debugger, port of KDevelop 1.3 to KDE 2.1"), "jbb@kdevelop.org");
	aboutData.addAuthor("Ian Reinhart Geiser",I18N_NOOP("Kicker templates and the RPM build facility"), "geiseri@linuxppc.com");
	aboutData.addAuthor("F@lk Brettschneider",I18N_NOOP("MDI"), "falk.brettschneider@gmx.de");

	KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options );
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  KApplication a;
  a.dcopClient()->attach();
  a.dcopClient()->registerAs("kdevelop");

  config = KGlobal::config();
  config->setGroup("General Options");
  bool bInstall=config->readBoolEntry("Install",true);

  if (args->isSet("setup"))
    bInstall = true; // start the setupwizard

//  if (kapp->isRestored())
//  {
//    RESTORE(CKDevelop);
//  }
//  else
  {
    if (bInstall)
    {
      CKDevInstall* install=new CKDevInstall(0,"install");
      install->show();
      delete install;
    }

  config->setGroup("General Options");
  start_logo=NULL;
  if (config->readBoolEntry("Logo",true) && (!kapp->isRestored() ) )
  {
    start_logo= new KStartupLogo();
    start_logo->show();
    start_logo->raise();
    QApplication::flushX();
  }

    CKDevelop* kdevelop = new CKDevelop();
    /* rokrau: i believe this has to be set by hand           *
     * and i hope this doesnt screw things up badly,           *
     * the guys on #kde wouldn't give me an answer, no really */
    a.setMainWidget(kdevelop);

   if (start_logo)
	delete start_logo;

    kdevelop->completeStartup(args->count() == 0);

    if (bInstall)
      kdevelop->refreshTrees();  // this is because of the new documentation

    config->setGroup("General Options");
    kdevelop->slotTCurrentTab(config->readNumEntry("LastActiveTree",DOC));
    
    if (args->count())
      kdevelop->slotProjectOpenCmdl(args->arg(0));

    kdevelop->bStartupIsPending = false;  // see queryClose()
  }
  
  args->clear();

  int rc = a.exec();
  return rc;
}
