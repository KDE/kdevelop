#include <config.h>


#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kmainwindow.h>
#include <dcopclient.h>


#include "toplevel.h"
#include "plugincontroller.h"
#include "partcontroller.h"
#include "core.h"
#include "splashscreen.h"
#include "projectmanager.h"

static KCmdLineOptions options[] =
{
    { "+file",          I18N_NOOP("Project to open"), 0 },
    { 0,0,0 }
};

int main(int argc, char *argv[])
{
  static const char *description = I18N_NOOP("Gideon");
  KAboutData aboutData("gideon", I18N_NOOP("Gideon"),
                       VERSION, description, KAboutData::License_GPL,
                       "(c) 1999-2003 Various Authors");
  aboutData.addAuthor("Caleb Tennis", "Current Maintainer and release coordinator, KTabBar, bugfixes", "caleb@aei-tech.com");
  aboutData.addAuthor("Bernd Gehrmann", "Initial idea, basic architecture, much initial source code", "bernd@kdevelop.org");
  aboutData.addAuthor("Richard Dale", "Java & Objective C support", "Richard_Dale@tipitina.demon.co.uk");
  aboutData.addAuthor("John Birch", "Debugger frontend", "jbb@kdevelop.org");
  aboutData.addAuthor("Sandy Meier", "PHP support, context menu stuff", "smeier@kdevelop.org");
  aboutData.addAuthor("Kurt Granroth", "KDE application templates", "kurth@granroth.org");
  aboutData.addAuthor("Ian Reinhart Geiser", "Application templates", "geiseri@yahoo.com");
  aboutData.addAuthor("Matthias Hoelzer-Kluepfel", "Several components, htdig indexing", "hoelzer@kde.org");
  aboutData.addAuthor("Victor Roeder", "Help with Automake manager and persistant class store", "victor_roeder@gmx.de");
  aboutData.addAuthor("Daniel Engelschalt", "C++ code completion, persistant class store", "daniel.engelschalt@gmx.net");
  aboutData.addAuthor("Harald Fernengel", "Ported to Qt 3, patches, valgrind, diff and perforce support", "harry@kdevelop.org");
  aboutData.addAuthor("Roberto Raggi", "QEditor component, code completion, Abbrev component, C++ support, Java support", "roberto@kdevelop.org");
  aboutData.addAuthor("Simon Hausmann", "Help with KParts infrastructure", "hausmann@kde.org");
  aboutData.addAuthor("Jakob Simon-Gaarde", "QMake projectmanager", "jsgaarde@tdcspace.dk");
  aboutData.addAuthor("Yann Hodique", "Patches", "Yann.Hodique@lifl.fr");
  aboutData.addAuthor("Alexander Dymo", "Help with C++ support, Filecreate and Doctreeview configuration, minor fixes", "cloudtemple@mksat.net");
  aboutData.addAuthor("F@lk Brettschneider", "MDI modes, QEditor, bugfixes", "falkbr@kdevelop.org");
  
  aboutData.addCredit("The KWrite authors", "Kate editor component", "kwrite-devel@kde.org");
  aboutData.addCredit("The KHTML authors", "HTML documentation component", "kwrite-devel@kde.org");
  aboutData.addCredit("Robert Moniot", "Fortran documentation", "moniot@fordham.edu");
  aboutData.addCredit("Ka-Ping Yee", "Python documentation utility", "ping@lfw.org");
  aboutData.addCredit("Dimitri van Heesch", "Doxygen wizard", "dimitri@stack.nl");
  aboutData.addCredit("Hugo Varotto", "Fileselector component", "hugo@varotto-usa.com");
  aboutData.addCredit("Matt Newell", "Fileselector component", "newellm@proaxis.com");
  aboutData.addCredit("Jonas Nordin", "C++ source parser", "jonas.nordin@syncom.se");
  aboutData.addCredit("Trolltech AS", "Designer code", "info@trolltech.com");
  aboutData.addCredit("Stephane ANCELOT", "Patches", "sancelot@free.fr");
  
  KCmdLineArgs::init(argc, argv, &aboutData);
  KCmdLineArgs::addCmdLineOptions( options );
  
  KApplication app;

  SplashScreen *splash = new SplashScreen;
 
  app.processEvents();

  QObject::connect(PluginController::getInstance(), SIGNAL(loadingPlugin(const QString &)),
		   splash, SLOT(showMessage(const QString &)));

  PluginController::getInstance()->loadInitialPlugins();

  TopLevel::getInstance()->loadSettings();
  ProjectManager::getInstance()->loadDefaultProject();
  TopLevel::getInstance()->main()->show();
  Core::getInstance()->doEmitCoreInitialized();
  
  delete splash;

  kapp->dcopClient()->registerAs("gideon");

  return app.exec();
}
