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

#include "ckdevelop.h"
#include "ckdevinstall.h"

#include <dcopclient.h>
#include <kaboutdata.h>
#include <kapp.h>
#include <kcmdlineargs.h>
#include <kglobal.h>
#include <klocale.h>

#include <kdebug.h>


static KCmdLineOptions options[] =
{
   { "setup", I18N_NOOP("Setup KDevelop"), 0 },
   { "+[file]", I18N_NOOP("The file to open"), 0 },
   { 0, 0, 0 }
};

static KAboutData* aboutData;

int main(int argc, char* argv[])
{
  aboutData = new KAboutData( "kdevelop",
                              I18N_NOOP("KDevelop"),
                              "1.4",
                              i18n("kdevelop: a C++ IDE"),
                              KAboutData::License_GPL,
                              "(c) 1998-2000, The KDevelop Team",
                              i18n("Port of kdevelop1.3 to KDE2.1 - some minor enhancements and bug fixes included"),
                              "http://www.kdevelop.org"  );

  aboutData->addAuthor("Sandy Meier",I18N_NOOP("Maintainer"), "smeier@rz.uni-potsdam.de");
  aboutData->addAuthor("Stefan Heidrich",I18N_NOOP("KAppWizard, Printing"), "sheidric@rz.uni-potsdam.de");
  aboutData->addAuthor("Ralf Nolden",I18N_NOOP("KDevelop<->Dialogeditor Interface, Configuration Functionality, Online Help"), "");
  aboutData->addAuthor("Jonas Nordin",I18N_NOOP("Classviewer and Classparser"), "jonas.nordin@syncom.se");
  aboutData->addAuthor("Jörgen Olsson",I18N_NOOP("Graphical Classviewer"), "jorgen@trej.net");
  aboutData->addAuthor("Stefan Bartel",I18N_NOOP("Real-File-Viewer, Project Options"), "bartel@rz.uni-potsdam.de");
  aboutData->addAuthor("Bernd Gehrmann",I18N_NOOP("Documentation Tree"), "bernd@physik.hu-berlin.de");
  aboutData->addAuthor("Walter Tasin",I18N_NOOP("Many, Many Bugfixes, General Enhancements"), "tasin@e-technik.fh-muenchen.de");
  aboutData->addAuthor("John Birch",I18N_NOOP("Internal Debugger"), "jbb@kdevelop.org");

  KCmdLineArgs::init( argc, argv, aboutData );
  KCmdLineArgs::addCmdLineOptions( options );
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  KApplication a;   // (argc, argv, "kdevelop");

  a.dcopClient()->attach();
  a.dcopClient()->registerAs("kdevelop");

  KGlobal::config()->setGroup("General Options");
  bool bInstall=KGlobal::config()->readBoolEntry("Install",true);

  if (args->isSet("setup"))
    bInstall = true; // start the setupwizard

  if (kapp->isRestored())
  {
    RESTORE(CKDevelop);
  }
  else
  {
    if (bInstall)
    {
      CKDevInstall* install=new CKDevInstall(0,"install");
      install->show();
      delete install;
    }

    CKDevelop* kdevelop = new CKDevelop();

    // We don't want this happening - It shouldn't - but (indirectly) KHTMLPart connects :(
//    if (a.dcopClient() && a.dcopClient()->isAttached())
//      a.dcopClient()->suspend();

    kdevelop->completeStartup(args->count() == 0);

    if (bInstall)
      kdevelop->refreshTrees();  // this is because of the new documentation

    KGlobal::config()->setGroup("General Options");
    kdevelop->slotSCurrentTab(KGlobal::config()->readNumEntry("LastActiveTab",BROWSER));
    kdevelop->slotTCurrentTab(KGlobal::config()->readNumEntry("LastActiveTree",DOC));
    
    if (args->count())
      kdevelop->slotProjectOpenCmdl(args->arg(0));
  }
  
  args->clear();

  int rc = a.exec();
  return rc;
}
