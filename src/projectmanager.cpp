#include <qfile.h>
#include <qfileinfo.h>
#include <qdom.h>
#include <qstringlist.h>
#include <qptrlist.h>
#include <qvbox.h>

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

#include "kdevproject.h"
#include "kdevlanguagesupport.h"
#include "kdevplugin.h"
#include "kdevcreatefile.h"


#include "toplevel.h"
#include "core.h"
#include "api.h"
#include "plugincontroller.h"
#include "partcontroller.h"
#include "classstore.h"
#include "partselectwidget.h"
#include "generalinfowidget.h"
#include "projectsession.h"
#include "domutil.h"

#include "projectmanager.h"


class ProjectInfo
{
public:
  QString      m_fileName;
  QDomDocument m_document;
  QString      m_projectPlugin, m_language;
  QStringList  m_ignoreParts, m_loadParts, m_keywords;
  QDict<KDevPlugin> m_localParts;
};


QString ProjectManager::projectDirectory( const QString& path, bool absolute ) {
    if(absolute)
        return path;
    QFileInfo projectFile(ProjectManager::getInstance()->projectFile());
    KURL url;
    url.setPath(projectFile.dirPath());
    url.addPath(path);
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
  action->setStatusText( i18n("Opens a project"));

  m_openRecentProjectAction =
    new KRecentFilesAction(i18n("Open &Recent Project..."), 0,
                          this, SLOT(loadProject(const KURL &)),
                          ac, "project_open_recent");
  m_openRecentProjectAction->setStatusText(i18n("Opens a recent project"));

  m_closeProjectAction =
    new KAction(i18n("C&lose Project"), "fileclose",0,
                this, SLOT(closeProject()),
                ac, "project_close");
  m_closeProjectAction->setEnabled(false);
  m_closeProjectAction->setStatusText(i18n("Closes the current project"));

  m_projectOptionsAction = new KAction(i18n("Project &Options..."), "configure", 0,
                this, SLOT(slotProjectOptions()),
                ac, "project_options" );
  m_projectOptionsAction->setEnabled(false);
}

void ProjectManager::slotOpenProject()
{
  QString defaultProjectsDir = kapp->config()->readEntry("DefaultProjectsDir", QDir::homeDirPath()+"/");

  KURL url = KFileDialog::getOpenURL(defaultProjectsDir, "*.kdevelop", TopLevel::getInstance()->main(), i18n("Open Project"));
  loadProject(url);
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
  connect( &dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
  connect( w, SIGNAL(accepted()), this, SLOT(loadLocalParts()) );

  Core::getInstance()->doEmitProjectConfigWidget(&dlg);
  dlg.exec();

  saveProjectFile();
}

void ProjectManager::loadSettings()
{
  KConfig *config = kapp->config();
  m_openRecentProjectAction->loadEntries(config, "RecentProjects");
}

void ProjectManager::saveSettings()
{
  KConfig *config = kapp->config();

  if (projectLoaded())
  {
    config->setGroup("General Options");
    config->writeEntry("Last Project", ProjectManager::getInstance()->projectFile());
  }

  m_openRecentProjectAction->saveEntries(config, "RecentProjects");
}

void ProjectManager::loadDefaultProject()
{
  KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
  KConfig *config = kapp->config();
  config->setGroup("General Options");
  QString project = config->readEntry("Last Project", "");
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

  if( projectLoaded() && !closeProject() )
    return false;

  m_info = new ProjectInfo;
  m_info->m_fileName = url.path();

  if( !loadProjectFile() )
  {
    delete m_info; m_info = 0;
    m_openRecentProjectAction->removeURL(url);
    saveSettings();
    return false;
  }

  getGeneralInfo();

  loadCreateFileSupport();

  if( !loadLanguageSupport() ) {
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

  Core::getInstance()->doEmitProjectOpened();

  // first restore the project session stored in a .kdevses file
  QString projSessionFileName = m_info->m_fileName.left(m_info->m_fileName.length()-8); // without ".kdevelop"
  projSessionFileName += "kdevses"; // suffix for a KDevelop session file
  if (!m_pProjectSession->restoreFromFile(projSessionFileName, m_info->m_localParts)) {
    debug("error during restoring of the KDevelop session !\n");
  }

  m_openRecentProjectAction->addURL(KURL(projectFile()));
  m_closeProjectAction->setEnabled(true);
  m_projectOptionsAction->setEnabled(true);

  return true;
}

bool ProjectManager::closeProject()
{
  if( !projectLoaded() )
    return false;

  Q_ASSERT( API::getInstance()->project() );

  // save the session
  QString sessionFileName = m_info->m_fileName;
  sessionFileName = sessionFileName.left( sessionFileName.length() - 8); // without "kdevelop"
  sessionFileName += "kdevses";
  m_pProjectSession->saveToFile(sessionFileName, m_info->m_localParts);

  if( !closeProjectSources() )
    return false;

  Core::getInstance()->doEmitProjectClosed();

  TopLevel::getInstance()->prepareToCloseViews();

  unloadLocalParts();
  unloadCreateFileSupport();
  unloadLanguageSupport();
  unloadProjectPart();

  // TODO, if this fails, user is screwed
  saveProjectFile();

  API::getInstance()->setProjectDom(0);
  API::getInstance()->classStore()->wipeout();
  API::getInstance()->ccClassStore()->wipeout();

  delete m_info;
  m_info = 0;

  m_closeProjectAction->setEnabled(false);
  m_projectOptionsAction->setEnabled(false);

  return true;
}

bool ProjectManager::loadProjectFile()
{
  QFile fin(m_info->m_fileName);
  if (!fin.open(IO_ReadOnly))
  {
    KMessageBox::sorry(TopLevel::getInstance()->main(),
        i18n("Could not read project file: %1").arg(m_info->m_fileName));
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
    return false;
  }
  if (m_info->m_document.documentElement().nodeName() != "kdevelop")
  {
    KMessageBox::sorry(TopLevel::getInstance()->main(),
        i18n("This is not a valid project file."));
    fin.close();
    return false;
  }

  fin.close();

  API::getInstance()->setProjectDom(&m_info->m_document);

  return true;
}

bool ProjectManager::saveProjectFile()
{
  Q_ASSERT( API::getInstance()->projectDom() );

  QFile fout(m_info->m_fileName);
  if( !fout.open(IO_WriteOnly) ) {
    KMessageBox::sorry(TopLevel::getInstance()->main(), i18n("Could not write the project file."));
    return false;
  }

  QTextStream stream(&fout);
  API::getInstance()->projectDom()->save(stream, 2);
  fout.close();

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
}

bool ProjectManager::loadProjectPart()
{
  KService::Ptr projectService = KService::serviceByName(m_info->m_projectPlugin);
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

  QFileInfo fi(m_info->m_fileName);
  QDomDocument& dom = *API::getInstance()->projectDom();
  QString path = DomUtil::readEntry(dom,"/general/projectdirectory",".");
  bool absolute = DomUtil::readBoolEntry(dom,"/general/absoluteprojectpath",false);
  QString projectDir = projectDirectory( path, absolute );
  QString projectName = fi.baseName();
  kdDebug(9000) << "projectDir: " << projectDir << "  projectName: " << projectName << endl;

  projectPart->openProject(projectDir, projectName);

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

bool ProjectManager::loadLanguageSupport()
{

  kdDebug(9000) << "Looking for language support for " << m_info->m_language << endl;

  KTrader::OfferList languageSupportOffers =
    KTrader::self()->query(QString::fromLatin1("KDevelop/LanguageSupport"),
                           QString::fromLatin1("[X-KDevelop-Language] == '%1'").arg(m_info->m_language));
  if (languageSupportOffers.isEmpty()) {
    KMessageBox::sorry(TopLevel::getInstance()->main(),
        i18n("No language plugin for %1 found.")
            .arg(m_info->m_language));
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
    KTrader::self()->query(QString::fromLatin1("KDevelop/CreateFile"));

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
    kapp->processEvents();
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
  if( !langlist.isEmpty() && !langlist.contains(m_info->m_language) ) {
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

bool ProjectManager::closeProjectSources()
{
	QStringList sources = API::getInstance()->project()->allFiles();
	QStringList::iterator it;

	for ( it = sources.begin(); it != sources.end(); ++it )
	{
		( *it ).prepend ( "/" );
		( *it ).prepend ( API::getInstance()->project()->projectDirectory() );
	}

	return PartController::getInstance()->closeDocuments(sources);
}

QString ProjectManager::projectFile() const
{
  if (!m_info)
    return QString::null;
  return m_info->m_fileName;
}

bool ProjectManager::projectLoaded() const
{
  return m_info != 0;
}

ProjectSession* ProjectManager::projectSession() const
{
  return m_pProjectSession;
}

#include "projectmanager.moc"
