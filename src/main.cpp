#include <config.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kmainwindow.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <dcopclient.h>

#include <qfileinfo.h>

#include "toplevel.h"
#include "plugincontroller.h"
#include "partcontroller.h"
#include "core.h"
#include "splashscreen.h"
#include "projectmanager.h"

static KCmdLineOptions options[] =
{
    { "profile <profile>",	I18N_NOOP("Profile to load"), 0 },
    { "+file(s)",		I18N_NOOP("Files to load"), 0 },
    { 0,0,0 }
};

int main(int argc, char *argv[])
{
  static const char description[] = I18N_NOOP("KDevelop");
  KAboutData aboutData("kdevelop", I18N_NOOP("KDevelop"),
                       VERSION, description, KAboutData::License_GPL,
                       I18N_NOOP("(c) 1999,2000,2001,2002,2003 Various Authors"));
  aboutData.addAuthor("Caleb Tennis", I18N_NOOP("Current Maintainer and release coordinator, KTabBar, bugfixes"), "caleb@aei-tech.com");
  aboutData.addAuthor("Bernd Gehrmann", I18N_NOOP("Initial idea, basic architecture, much initial source code"), "bernd@kdevelop.org");
  aboutData.addAuthor("Richard Dale", I18N_NOOP("Java & Objective C support"), "Richard_Dale@tipitina.demon.co.uk");
  aboutData.addAuthor("John Birch", I18N_NOOP("Debugger frontend"), "jbb@kdevelop.org");
  aboutData.addAuthor("Sandy Meier", I18N_NOOP("PHP support, context menu stuff"), "smeier@kdevelop.org");
  aboutData.addAuthor("Kurt Granroth", I18N_NOOP("KDE application templates"), "kurth@granroth.org");
  aboutData.addAuthor("Ian Reinhart Geiser", I18N_NOOP("Application templates"), "geiseri@yahoo.com");
  aboutData.addAuthor("Matthias Hoelzer-Kluepfel", I18N_NOOP("Several components, htdig indexing"), "hoelzer@kde.org");
  aboutData.addAuthor("Victor Roeder", I18N_NOOP("Help with Automake manager and persistant class store"), "victor_roeder@gmx.de");
  aboutData.addAuthor("Daniel Engelschalt", I18N_NOOP("C++ code completion, persistant class store"), "daniel.engelschalt@gmx.net");
  aboutData.addAuthor("Harald Fernengel", I18N_NOOP("Ported to Qt 3, patches, valgrind, diff and perforce support"), "harry@kdevelop.org");
  aboutData.addAuthor("Roberto Raggi", I18N_NOOP("QEditor component, code completion, Abbrev component, C++ support, Java support"), "roberto@kdevelop.org");
  aboutData.addAuthor("Simon Hausmann", I18N_NOOP("Help with KParts infrastructure"), "hausmann@kde.org");
  aboutData.addAuthor("Jakob Simon-Gaarde", I18N_NOOP("QMake projectmanager"), "jsgaarde@tdcspace.dk");
  aboutData.addAuthor("Yann Hodique", I18N_NOOP("Patches"), "Yann.Hodique@lifl.fr");
  aboutData.addAuthor("F@lk Brettschneider", I18N_NOOP("MDI modes, QEditor, bugfixes"), "falkbr@kdevelop.org");
  aboutData.addAuthor("Alexander Dymo", I18N_NOOP("Pascal support, help with C++ support, Filecreate and Doctreeview"), "cloudtemple@mksat.net");
  aboutData.addAuthor("Mario Scalas", I18N_NOOP("PartExplorer, redesign of CvsPart, patches, bugs(fixes)"), "mario.scalas@libero.it");

  aboutData.addCredit("The KWrite authors", I18N_NOOP("Kate editor component"), "kwrite-devel@kde.org");
  aboutData.addCredit("The KHTML authors", I18N_NOOP("HTML documentation component"), "kwrite-devel@kde.org");
  aboutData.addCredit("Robert Moniot", I18N_NOOP("Fortran documentation"), "moniot@fordham.edu");
  aboutData.addCredit("Ka-Ping Yee", I18N_NOOP("Python documentation utility"), "ping@lfw.org");
  aboutData.addCredit("Dimitri van Heesch", I18N_NOOP("Doxygen wizard"), "dimitri@stack.nl");
  aboutData.addCredit("Hugo Varotto", I18N_NOOP("Fileselector component"), "hugo@varotto-usa.com");
  aboutData.addCredit("Matt Newell", I18N_NOOP("Fileselector component"), "newellm@proaxis.com");
  aboutData.addCredit("Jonas Nordin", I18N_NOOP("C++ source parser"), "jonas.nordin@syncom.se");
  aboutData.addCredit("Trolltech AS", I18N_NOOP("Designer code"), "info@trolltech.com");
  aboutData.addCredit("Stephane ANCELOT", I18N_NOOP("Patches"), "sancelot@free.fr");
  aboutData.addCredit("Jens Zurheide", I18N_NOOP("Patches"), "jens.zurheide@gmx.de");
  aboutData.addCredit("Julian Rockey", I18N_NOOP("Filecreate part and other bits and patches"), "linux@jrockey.com");
  aboutData.addCredit("Amilcar do Carmo Lucas", I18N_NOOP("Doxygen and autoproject patches"), "amilcar@ida.ing.tu-bs.de");
  aboutData.addCredit("Ajay Guleria", I18N_NOOP("ClearCase support"), "ajay_guleria@yahoo.com");
  aboutData.addCredit("Marek Janukowicz", I18N_NOOP("Ruby support"), "child@t17.ds.pwr.wroc.pl");
  aboutData.addCredit("Jens Dagerbo", I18N_NOOP("Replace part, patches"), "jens.dagerbo@swipnet.se");
  aboutData.addCredit("Luc Willems", I18N_NOOP("Help with Perl support"), "Willems.luc@pandora.be");
  aboutData.addCredit("Marcel Turino", I18N_NOOP("Documentation index view"), "M.Turino@gmx.de");
  aboutData.addCredit("Oliver Kellogg", I18N_NOOP("Ada support"), "Oliver.Kellogg@t-online.de");
  aboutData.addCredit( "Tobias Gl\303\244\303\237er" , I18N_NOOP("QMake projectmanager patches"), "tobi.web@gmx.de");

  KCmdLineArgs::init(argc, argv, &aboutData);
  KCmdLineArgs::addCmdLineOptions( options );
  KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

  KApplication app;

  SplashScreen *splash = new SplashScreen;

  app.processEvents();

  QObject::connect(PluginController::getInstance(), SIGNAL(loadingPlugin(const QString &)),
		   splash, SLOT(showMessage(const QString &)));

  splash->showMessage( i18n( "Loading Settings" ) );
  TopLevel::getInstance()->loadSettings();

  splash->showMessage( i18n( "Starting GUI" ) );
  TopLevel::getInstance()->main()->show();

  PluginController::getInstance()->loadInitialPlugins();

  Core::getInstance()->doEmitCoreInitialized();

  splash->showMessage( i18n( "Loading Project" ) );

  delete splash;

  for( int i=0; i<args->count(); ++i ){
      kdDebug(9000) << "------> arg " << args->arg(i) << endl;
  }

  bool openProject = false;
  if( args->count() == 0 ){
      ProjectManager::getInstance()->loadDefaultProject();
      openProject = true;
  } else if( args->count() > 0 ){
      KURL url = args->url( 0 );
      QString ext = QFileInfo( url.fileName() ).extension();
      if( ext == "kdevelop" ){
	  ProjectManager::getInstance()->loadProject( url );
	  openProject = true;
      }
  }

  if( !openProject ){
      for( int a=0; a<args->count(); ++a ){
	  PartController::getInstance()->editDocument( KURL(args->url(a)) );
      }
  }

  kapp->dcopClient()->registerAs("kdevelop");

  return app.exec();
}
