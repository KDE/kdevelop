#include <qfile.h>
#include <qfileinfo.h>
#include <qdom.h>
#include <qstringlist.h>
#include <qptrlist.h>
#include <qvbox.h>
#include <qsize.h>

class QDomDocument;

#include <kmessagebox.h>
#include <kdebug.h>
#include <klocale.h>
#include <kservice.h>
#include <ktrader.h>
#include <kfiledialog.h>
#include <kmainwindow.h>
#include <kparts/componentfactory.h>
#include <kaction.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kprocess.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kio/netaccess.h>
#include <ktempfile.h>

#include "kdevproject.h"
#include "kdevlanguagesupport.h"
#include "kdevplugin.h"
#include "kdevcreatefile.h"


#include "toplevel.h"
#include "core.h"
#include "api.h"
#include "plugincontroller.h"
#include "partcontroller.h"
#include "codemodel.h"
#include "partselectwidget.h"
#include "languageselectwidget.h"
#include "generalinfowidget.h"
#include "projectsession.h"
#include "domutil.h"

#include "projectmanager.h"


class ProjectInfo
{
public:
  KURL         m_projectURL;
  QDomDocument m_document;
  QString      m_projectPlugin, m_language, m_activeLanguage;
  QStringList  m_ignoreParts, m_loadParts, m_keywords, m_secondaryLanguages;
  QDict<KDevPlugin> m_localParts;

  QString sessionFile() const;
};

QString ProjectInfo::sessionFile() const
{
    QString sf = m_projectURL.path(-1);
    sf.truncate(sf.length() - 8); // without ".kdevelop"
    sf += "kdevses"; // suffix for a KDevelop session file
    return sf;
}

QString ProjectManager::projectDirectory( const QString& path, bool absolute ) {
    if(absolute)
        return path;
    KURL url(ProjectManager::getInstance()->projectFile(), path);
    url.cleanPath();
    return url.path(-1);
}

ProjectManager *ProjectManager::s_instance = 0;

ProjectManager::ProjectManager()
: m_info(0L)
 ,m_pProjectSession(new ProjectSession)
{
}

ProjectManager::~ProjectManager()
{
  delete m_pProjectSession;
  delete m_info;
}

ProjectManager *ProjectManager::getInstance()
{
  if (!s_instance)
    s_instance = new ProjectManager;
  return s_instance;
}

void ProjectManager::createActions( KActionCollection* ac )
{
  KAction *action;

  action = new KAction(i18n("&Open Project..."), "project_open", 0,
                       this, SLOT(slotOpenProject()),
                       ac, "project_open");
  action->setToolTip( i18n("Open project"));
  action->setWhatsThis(i18n("<b>Open project</b><p>Opens a KDevelop3 or KDevelop2 project."));

  m_openRecentProjectAction =
    new KRecentFilesAction(i18n("Open &Recent Project..."), 0,
                          this, SLOT(loadProject(const KURL &)),
                          ac, "project_open_recent");
  m_openRecentProjectAction->setToolTip(i18n("Open recent project"));
  m_openRecentProjectAction->setWhatsThis(i18n("<b>Open recent project</b><p>Opens recently opened project."));
  m_openRecentProjectAction->loadEntries(kapp->config(), "RecentProjects");

  m_closeProjectAction =
    new KAction(i18n("C&lose Project"), "fileclose",0,
                this, SLOT(closeProject()),
                ac, "project_close");
  m_closeProjectAction->setEnabled(false);
  m_closeProjectAction->setToolTip(i18n("Close project"));
  m_closeProjectAction->setWhatsThis(i18n("<b>Close project</b><p>Closes the current project."));

  m_projectOptionsAction = new KAction(i18n("Project &Options..."), "configure", 0,
                this, SLOT(slotProjectOptions()),
                ac, "project_options" );
  m_projectOptionsAction->setToolTip(i18n("Project options"));
  m_projectOptionsAction->setWhatsThis(i18n("<b>Project options</b><p>Lets you customize project options."));
  m_projectOptionsAction->setEnabled(false);

  m_activeLanguage = new KSelectAction(i18n("&Active Language"), 0, ac, "project_active_language");
  m_activeLanguage->setWhatsThis(i18n("<b>Active language</b><p>Sets the active programming language."));
  m_activeLanguage->setEnabled(false);
  connect(m_activeLanguage, SIGNAL(activated(const QString&)),
          this, SLOT(switchLanguage(const QString&)));
}

void ProjectManager::slotOpenProject()
{
  QString defaultProjectsDir = kapp->config()->readPathEntry("DefaultProjectsDir", QDir::homeDirPath()+"/");

  KURL url = KFileDialog::getOpenURL(defaultProjectsDir,
		i18n("*.kdevelop|KDevelop 3 Project Files\n"
		     "*.kdevprj|KDevelop 2 Project Files"),
		TopLevel::getInstance()->main(), i18n("Open Project") );

  if( url.isEmpty() )
      return;

  if (url.path().endsWith("kdevprj"))
      loadKDevelop2Project( url );
  else
      loadProject( url );
}

void ProjectManager::slotProjectOptions()
{
  KDialogBase dlg(KDialogBase::TreeList, i18n("Project Options"),
                  KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, TopLevel::getInstance()->main(),
                  "project options dialog");

    QVBox *box = dlg.addVBoxPage(i18n("General"));
    GeneralInfoWidget *g = new GeneralInfoWidget(*API::getInstance()->projectDom(), box, "general informations widget");
    connect (&dlg, SIGNAL(okClicked()), g, SLOT(accept()));

  QVBox *vbox = dlg.addVBoxPage(i18n("Plugins"));
  PartSelectWidget *w = new PartSelectWidget(*API::getInstance()->projectDom(), vbox, "part selection widget");
  vbox = dlg.addVBoxPage(i18n("Languages"));
  LanguageSelectWidget *lw = new LanguageSelectWidget(*API::getInstance()->projectDom(), vbox, "language selection widget");
  connect( &dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
  connect( &dlg, SIGNAL(okClicked()), lw, SLOT(accept()) );
  connect( w, SIGNAL(accepted()), this, SLOT(loadLocalParts()) );
  connect( lw, SIGNAL(accepted()), this, SLOT(updateActiveLangMenu()) );

  KConfig *config = kapp->config();
  config->setGroup("Project Settings Dialog");
  int height = config->readNumEntry( "Height", 600 );
  int width = config->readNumEntry( "Width", 800 );
  
  dlg.resize( width, height );
  
  Core::getInstance()->doEmitProjectConfigWidget(&dlg);
  dlg.exec();

  saveProjectFile();
  
  config->setGroup("Project Settings Dialog");
  config->writeEntry( "Height", dlg.size().height() );
  config->writeEntry( "Width", dlg.size().width() );
}

void ProjectManager::updateActiveLangMenu()
{
  getGeneralInfo();
  QStringList list( m_info->m_secondaryLanguages );
  list.prepend( m_info->m_language ); //make sure primary lang comes first
  m_activeLanguage->setItems( list );
  m_activeLanguage->setEnabled( m_info->m_secondaryLanguages.count() > 0 );
  m_activeLanguage->setCurrentItem(m_activeLanguage->items().findIndex(m_info->m_activeLanguage));
}

void ProjectManager::switchLanguage(const QString& lang)
{
  // make sure there is a project loaded
  if ( !m_info ) return;

  unloadLocalParts();
  unloadLanguageSupport();
  m_info->m_loadParts.clear();
  loadLanguageSupport(lang);
  loadLocalParts();
  Core::getInstance()->doEmitLanguageChanged();
}

void ProjectManager::loadSettings()
{
}

void ProjectManager::saveSettings()
{
  KConfig *config = kapp->config();

  if (projectLoaded())
  {
    config->setGroup("General Options");
#if defined(KDE_IS_VERSION)
# if KDE_IS_VERSION(3,1,3)
#  ifndef _KDE_3_1_3_
#   define _KDE_3_1_3_
#  endif
# endif
#endif
#if defined(_KDE_3_1_3_)
    config->writePathEntry("Last Project", ProjectManager::getInstance()->projectFile().url());
#else
    config->writeEntry("Last Project", ProjectManager::getInstance()->projectFile().url());
#endif
  }

  m_openRecentProjectAction->saveEntries(config, "RecentProjects");
}

void ProjectManager::loadDefaultProject()
{
  KConfig *config = kapp->config();
  config->setGroup("General Options");
  QString project = config->readPathEntry("Last Project");
  bool readProject = config->readBoolEntry("Read Last Project On Startup", true);
  if (!project.isEmpty() && readProject)
  {
      loadProject(KURL(project));
  }
  kapp->processEvents();
}

bool ProjectManager::loadProject(const KURL &url)
{
  if (url.isMalformed())
    return false;

  // reopen the already opened project?
  if( url.path() == projectFile().path() )
  {
    if (KMessageBox::questionYesNo(TopLevel::getInstance()->main(),
        i18n("Are you sure you want to reload the current project?")) == KMessageBox::No)
      return false;
  }

  if( projectLoaded() && !closeProject() )
  {
    m_openRecentProjectAction->setCurrentItem( -1 );
    return false;
  }

  m_info = new ProjectInfo;
  m_info->m_projectURL = url;

  if( !loadProjectFile() )
  {
    delete m_info; m_info = 0;
    m_openRecentProjectAction->removeURL(url);
    saveSettings();
    return false;
  }

  getGeneralInfo();

  updateActiveLangMenu();
  loadCreateFileSupport();

  if( !loadLanguageSupport(m_info->m_language) ) {
    unloadCreateFileSupport();
    delete m_info; m_info = 0;
    return false;
  }

  if( !loadProjectPart() ) {
    unloadCreateFileSupport();
    unloadLanguageSupport();
    delete m_info; m_info = 0;
    return false;
}

  loadLocalParts();

//  Core::getInstance()->doEmitProjectOpened();

  // shall we try to load a session file from network?? Probably not.
  if (m_info->m_projectURL.isLocalFile()) {
    const QDict<KDevPlugin>& globalParts = PluginController::getInstance()->globalParts();
    QDict<KDevPlugin> allParts = m_info->m_localParts;
    QDictIterator<KDevPlugin> it(globalParts);
    for (; it.current(); ++it) {
      allParts.insert(it.currentKey(), it.current());
    }
    // first restore the project session stored in a .kdevses file
    if (!m_pProjectSession->restoreFromFile(m_info->sessionFile(), allParts)) {
      kdWarning() << i18n("error during restoring of the KDevelop session !") << endl;
    }
  }

  m_openRecentProjectAction->addURL(projectFile());

  m_closeProjectAction->setEnabled(true);
  m_projectOptionsAction->setEnabled(true);

  Core::getInstance()->doEmitProjectOpened();

  return true;
}

bool ProjectManager::closeProject()
{
  if( !projectLoaded() )
    return false;

  Q_ASSERT( API::getInstance()->project() );

  // save the session if it is a local file
  if (m_info->m_projectURL.isLocalFile()) {
    const QDict<KDevPlugin>& globalParts = PluginController::getInstance()->globalParts();
    QDict<KDevPlugin> allParts = m_info->m_localParts;
    QDictIterator<KDevPlugin> it(globalParts);
    for (; it.current(); ++it) {
      allParts.insert(it.currentKey(), it.current());
    }
    m_pProjectSession->saveToFile(m_info->sessionFile(), allParts);
  }
  if ( !PartController::getInstance()->closeAllWindows() )
    return false;
  
  Core::getInstance()->doEmitProjectClosed();

  TopLevel::getInstance()->prepareToCloseViews();

  unloadLocalParts();
  unloadCreateFileSupport();
  unloadLanguageSupport();
  unloadProjectPart();

  /// @todo if this fails, user is screwed
  saveProjectFile();

  API::getInstance()->setProjectDom(0);
  API::getInstance()->codeModel()->wipeout();

  delete m_info;
  m_info = 0;

  m_closeProjectAction->setEnabled(false);
  m_projectOptionsAction->setEnabled(false);
  m_activeLanguage->setEnabled(false);
  
  PartController::getInstance()->slotCloseAllWindows();

  return true;
}

bool ProjectManager::loadProjectFile()
{
  QString path;
  if (!KIO::NetAccess::download(m_info->m_projectURL, path)) {
    KMessageBox::sorry(TopLevel::getInstance()->main(),
        i18n("Could not read project file: %1").arg(m_info->m_projectURL.prettyURL()));
    return false;
  }

  QFile fin(path);
  if (!fin.open(IO_ReadOnly))
  {
    KMessageBox::sorry(TopLevel::getInstance()->main(),
        i18n("Could not read project file: %1").arg(m_info->m_projectURL.prettyURL()));
    return false;
  }

  int errorLine, errorCol;
  QString errorMsg;
  if (!m_info->m_document.setContent(&fin, &errorMsg, &errorLine, &errorCol))
  {
    KMessageBox::sorry(TopLevel::getInstance()->main(),
        i18n("This is not a valid project file.\n"
             "XML error in line %1, column %2:\n%3")
             .arg(errorLine).arg(errorCol).arg(errorMsg));
    fin.close();
    KIO::NetAccess::removeTempFile(path);
    return false;
  }
  if (m_info->m_document.documentElement().nodeName() != "kdevelop")
  {
    KMessageBox::sorry(TopLevel::getInstance()->main(),
        i18n("This is not a valid project file."));
    fin.close();
    KIO::NetAccess::removeTempFile(path);
    return false;
  }

  fin.close();
  KIO::NetAccess::removeTempFile(path);

  API::getInstance()->setProjectDom(&m_info->m_document);

  return true;
}

bool ProjectManager::saveProjectFile()
{
  Q_ASSERT( API::getInstance()->projectDom() );

  if (m_info->m_projectURL.isLocalFile()) {
    QFile fout(m_info->m_projectURL.path());
    if( !fout.open(IO_WriteOnly) ) {
      KMessageBox::sorry(TopLevel::getInstance()->main(), i18n("Could not write the project file."));
      return false;
    }

    QTextStream stream(&fout);
    API::getInstance()->projectDom()->save(stream, 2);
    fout.close();
  } else {
    KTempFile fout(QString::fromLatin1("kdevelop3"));
    fout.setAutoDelete(true);
    if (fout.status() != 0) {
      KMessageBox::sorry(TopLevel::getInstance()->main(), i18n("Could not write the project file."));
      return false;
    }
    API::getInstance()->projectDom()->save(*(fout.textStream()), 2);
    fout.close();
    KIO::NetAccess::upload(fout.name(), m_info->m_projectURL);
  }

  return true;
}

static QString getAttribute(QDomElement elem, QString attr)
{
  QDomElement el = elem.namedItem(attr).toElement();
  return el.firstChild().toText().data();
}

static void getAttributeList(QDomElement elem, QString attr, QString tag, QStringList &list)
{
  list.clear();

  QDomElement el = elem.namedItem(attr).toElement();
  QDomElement item = el.firstChild().toElement();
  while (!item.isNull())
  {
    if (item.tagName() == tag)
      list << item.firstChild().toText().data();
    item = item.nextSibling().toElement();
  }
}

void ProjectManager::getGeneralInfo()
{
  QDomElement docEl = m_info->m_document.documentElement();
  QDomElement generalEl = docEl.namedItem("general").toElement();

  m_info->m_projectPlugin = getAttribute(generalEl, "projectmanagement");
  m_info->m_language = getAttribute(generalEl, "primarylanguage");

  getAttributeList(generalEl, "ignoreparts", "part", m_info->m_ignoreParts);
  getAttributeList(generalEl, "keywords", "keyword", m_info->m_keywords);
  getAttributeList(generalEl, "secondaryLanguages", "language", m_info->m_secondaryLanguages);
}

bool ProjectManager::loadProjectPart()
{
  KService::Ptr projectService = KService::serviceByDesktopName(m_info->m_projectPlugin);
  if (!projectService) {
    // this is for backwards compatibility with pre-alpha6 projects
    projectService = KService::serviceByDesktopName(m_info->m_projectPlugin.lower());
  }
  if (!projectService) {
    KMessageBox::sorry(TopLevel::getInstance()->main(),
        i18n("No project management plugin %1 found.")
            .arg(m_info->m_projectPlugin));
    return false;
  }

  KDevProject *projectPart = KParts::ComponentFactory
    ::createInstanceFromService< KDevProject >( projectService, API::getInstance(), 0,
                                                  PluginController::argumentsFromService( projectService ) );
  if ( !projectPart ) {
    KMessageBox::sorry(TopLevel::getInstance()->main(),
        i18n("Could not create project management plugin %1.")
            .arg(m_info->m_projectPlugin));
    return false;
  }

  API::getInstance()->setProject( projectPart );

  QDomDocument& dom = *API::getInstance()->projectDom();
  QString path = DomUtil::readEntry(dom,"/general/projectdirectory", ".");
  bool absolute = DomUtil::readBoolEntry(dom,"/general/absoluteprojectpath",false);
  QString projectDir = projectDirectory( path, absolute );
  kdDebug(9000) << "projectDir: " << projectDir << "  projectName: " << m_info->m_projectURL.fileName() << endl;

  projectPart->openProject(projectDir, m_info->m_projectURL.fileName());

  PluginController::getInstance()->integratePart( projectPart );

  return true;
}

void ProjectManager::unloadProjectPart()
{
  KDevProject *projectPart = API::getInstance()->project();
  if( !projectPart ) return;
  PluginController::getInstance()->removePart( projectPart );
  projectPart->closeProject();
  delete projectPart;
  API::getInstance()->setProject(0);
}

bool ProjectManager::loadLanguageSupport(const QString& lang)
{
  kdDebug(9000) << "Looking for language support for " << lang << endl;

  if (lang == m_info->m_activeLanguage)
    // language already loaded...
    return true;

  KTrader::OfferList languageSupportOffers =
    KTrader::self()->query(QString::fromLatin1("KDevelop/LanguageSupport"),
                           QString::fromLatin1("[X-KDevelop-Language] == '%1' and [X-KDevelop-Version] == %2").arg(m_info->m_language).arg(KDEVELOP_PLUGIN_VERSION));

  if (languageSupportOffers.isEmpty()) {
    KMessageBox::sorry(TopLevel::getInstance()->main(),
        i18n("No language plugin for %1 found.")
            .arg(lang));
    return false;
  }

  KService::Ptr languageSupportService = *languageSupportOffers.begin();
  KDevLanguageSupport *langSupport = KParts::ComponentFactory
      ::createInstanceFromService<KDevLanguageSupport>( languageSupportService,
                                                        API::getInstance(),
                                                        0,
                                                        PluginController::argumentsFromService(  languageSupportService ) );

  if ( !langSupport ) {
    KMessageBox::sorry(TopLevel::getInstance()->main(),
        i18n("Could not create language plugin for %1.")
            .arg(m_info->m_language));
    return false;
  }

  API::getInstance()->setLanguageSupport( langSupport );
  PluginController::getInstance()->integratePart( langSupport );
  m_info->m_activeLanguage = lang;
  m_activeLanguage->setCurrentItem(m_activeLanguage->items().findIndex(lang));

  return true;
}

void ProjectManager::unloadLanguageSupport()
{
  KDevLanguageSupport *langSupport = API::getInstance()->languageSupport();
  if( !langSupport ) return;
  PluginController::getInstance()->removePart( langSupport );
  delete langSupport;
  API::getInstance()->setLanguageSupport(0);
}

bool ProjectManager::loadCreateFileSupport() {
  kdDebug(9000) << "Looing for CreateFile support" << endl;
  KTrader::OfferList createFileOffers =
    KTrader::self()->query(QString::fromLatin1("KDevelop/CreateFile"), QString::fromLatin1("[X-KDevelop-Version] == %1").arg(KDEVELOP_PLUGIN_VERSION));

  if (createFileOffers.isEmpty()) {
    kdDebug(9000) << "No offers found" << endl;
    API::getInstance()->setCreateFile(0);
    return false;
  }

  KService::Ptr createFileService = *createFileOffers.begin();
  KDevCreateFile *crfileSupport =
    KParts::ComponentFactory::createInstanceFromService<KDevCreateFile>(createFileService,
                                                                        API::getInstance(),
                                                                        0,
                                                                        PluginController::argumentsFromService(createFileService) );
  API::getInstance()->setCreateFile(crfileSupport);
  if (!crfileSupport) {
    kdDebug(9000) << "Could not load CreateFile plugin" << endl;
    return false;
  }
  PluginController::getInstance()->integratePart( crfileSupport );
  kdDebug(9000) << "CreateFile support loaded OK" << endl;
  return true;


}

void ProjectManager::unloadCreateFileSupport() {
  KDevCreateFile *crfileSupport = API::getInstance()->createFile();
  if (!crfileSupport) return;
  PluginController::getInstance()->removePart(crfileSupport);
  delete crfileSupport;
  API::getInstance()->setCreateFile(0);
}

void ProjectManager::loadLocalParts()
{
  // Make sure to refresh load/ignore lists
  getGeneralInfo();

  KTrader::OfferList localOffers = PluginController::pluginServices( "Project" );
  for (KTrader::OfferList::ConstIterator it = localOffers.begin(); it != localOffers.end(); ++it)
  {
    QString name = (*it)->name();
    kdDebug(9000) << "-----------------------------> load part " << name << endl;

    // Unload it if it is marked as ignored and loaded
    if (m_info->m_ignoreParts.contains(name)) {
      KDevPlugin* part = m_info->m_localParts[name];
      if( part ) {
        PluginController::getInstance()->removePart( part );
        m_info->m_localParts.remove( name );
        part->deleteLater();
      }
      continue;
    }

    // Check if it is already loaded
    if( m_info->m_localParts[ name ] != 0 )
      continue;

    if( m_info->m_loadParts.contains( name ) ||
        checkNewService( *it ) )
    {
      KDevPlugin *part = PluginController::loadPlugin( *it );
      if ( !part ) continue;

      PluginController::getInstance()->integratePart( part );
      m_info->m_localParts.insert( name, part );
    }
// do NOT do processEvents() here, it will delete the unloading plugins before they're ready!
//    kapp->processEvents();
  }
}

void ProjectManager::unloadLocalParts()
{
  for( QDictIterator<KDevPlugin> it( m_info->m_localParts ); !it.isEmpty(); )
  {
    KDevPlugin* part = it.current();
    PluginController::getInstance()->removePart( part );
    m_info->m_localParts.remove( it.currentKey() );
    delete part;
  }
}

bool ProjectManager::checkNewService(const KService::Ptr &service)
{
  QVariant var = service->property("X-KDevelop-ProgrammingLanguages");
  QStringList langlist = var.asStringList();

  // empty means it supports all languages
  if( !langlist.isEmpty() && !langlist.contains(m_info->m_activeLanguage) ) {
    m_info->m_ignoreParts << service->name();
    return false;
  }

  // the language is ok, now check if the keywords match
  QStringList serviceKeywords = service->keywords();
  for ( QStringList::Iterator is = serviceKeywords.begin();
        is != serviceKeywords.end(); ++is )
  {
    if ( !m_info->m_keywords.contains(*is) ) {
      // no match
      kdDebug(9000) << "ignoreParts because Keyword doesn't match: " << service->name() << endl;
      m_info->m_ignoreParts << service->name();
      return false;
    }
  }

  m_info->m_loadParts << service->name();
  return true;
}

KURL ProjectManager::projectFile() const
{
  if (!m_info)
    return QString::null;
  return m_info->m_projectURL;
}

bool ProjectManager::projectLoaded() const
{
  return m_info != 0;
}

ProjectSession* ProjectManager::projectSession() const
{
  return m_pProjectSession;
}

bool ProjectManager::loadKDevelop2Project( const KURL & url )
{
    if( url.isMalformed() || !url.isLocalFile() ){
        KMessageBox::sorry(0, i18n("Invalid URL."));
        return false;
    }

    QString cmd = KGlobal::dirs()->findExe( "kdevprj2kdevelop" );
    if (cmd.isEmpty()) {
        KMessageBox::sorry(0, i18n("You don't have 'kdevprj2kdevelop' installed."));
        return false;
    }

    QFileInfo fileInfo( url.path() );

    KShellProcess proc( "/bin/sh" );
    proc.setWorkingDirectory( fileInfo.dirPath(true) );
    proc << "perl" << cmd << KShellProcess::quote( url.path() );
    proc.start( KProcess::Block );

    QString projectFile = fileInfo.dirPath( true ) + "/" + fileInfo.baseName() + ".kdevelop";
    return loadProject( KURL(projectFile) );
}

#include "projectmanager.moc"
