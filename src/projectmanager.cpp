#include <qfile.h>
#include <qfileinfo.h>
#include <qdom.h>
#include <qstringlist.h>
#include <qptrlist.h>

class QDomDocument;


#include <kmessagebox.h>
#include <kdebug.h>
#include <klocale.h>
#include <kservice.h>
#include <ktrader.h>
#include <kfiledialog.h>
#include <kmainwindow.h>
#include <kparts/componentfactory.h>

#include "kdevproject.h"
#include "kdevlanguagesupport.h"
#include "kdevplugin.h"


#include "toplevel.h"
#include "core.h"
#include "api.h"
#include "plugincontroller.h"
#include "partcontroller.h"
#include "classstore.h"


#include "projectmanager.h"


class ProjectInfo
{
public:

  ~ProjectInfo();

  QString      m_fileName;
  QDomDocument m_document;
  QString      m_projectPlugin, m_language;
  QStringList  m_ignoreParts, m_loadParts, m_keywords;
  QPtrList<KXMLGUIClient> m_localParts;

};


ProjectInfo::~ProjectInfo()
{
}


ProjectManager *ProjectManager::s_instance = 0;


ProjectManager::ProjectManager()
{
  m_info = 0;
}


ProjectManager::~ProjectManager()
{
  delete m_info;
}


ProjectManager *ProjectManager::getInstance()
{
  if (!s_instance)
    s_instance = new ProjectManager;
  return s_instance;
}


void ProjectManager::loadProject(const QString &fileName)
{
  closeProject();

  m_info = new ProjectInfo;

  if (!loadProjectFile(fileName))
  {
    delete m_info;
    m_info = 0;
    return;
  }

  m_info->m_fileName = fileName;

  getGeneralInfo();

  loadProjectPart();
  loadLanguageSupport();
  loadLocalParts();

  initializeProjectSupport();
}


bool ProjectManager::loadProjectFile(const QString &fileName)
{
  QFile fin(fileName);
  if (!fin.open(IO_ReadOnly))
  {
    KMessageBox::sorry(TopLevel::getInstance()->main(), i18n("Could not read project file: %1").arg(fileName));
    return false;
  }

  int errorLine, errorCol;
  QString errorMsg;
  if (!m_info->m_document.setContent(&fin, &errorMsg, &errorLine, &errorCol))
  {
    KMessageBox::sorry(TopLevel::getInstance()->main(), i18n("This is not a valid project file.\n"
                                                             "XML error in line %1, column %2:\n%3")
                       .arg(errorLine).arg(errorCol).arg(errorMsg));
    fin.close();
    return false;
  }
  if (m_info->m_document.doctype().name() != "kdevelop")
  {
    KMessageBox::sorry(TopLevel::getInstance()->main(), i18n("This is not a valid project file."));
    fin.close();
    return false;
  }

  API::getInstance()->setProjectDom(&m_info->m_document);

  fin.close();

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


void ProjectManager::loadProjectPart()
{
  KService::Ptr projectService = KService::serviceByName(m_info->m_projectPlugin);
  if (projectService)
  {
    KDevProject *projectPart = KParts::ComponentFactory
      ::createInstanceFromService< KDevProject >( projectService, API::getInstance(), 0,
                                                  PluginController::argumentsFromService( projectService ) );
    if ( !projectPart )
      return;

    API::getInstance()->setProject( projectPart );
    integratePart( projectPart );
  }
  else
    KMessageBox::sorry(TopLevel::getInstance()->main(), i18n("No project management plugin %1 found.").arg(m_info->m_projectPlugin));
}


void ProjectManager::loadLanguageSupport()
{
  KTrader::OfferList languageSupportOffers =
    KTrader::self()->query(QString::fromLatin1("KDevelop/LanguageSupport"),
                           QString::fromLatin1("[X-KDevelop-Language] == '%1'").arg(m_info->m_language));
  if (languageSupportOffers.isEmpty())
    KMessageBox::sorry(TopLevel::getInstance()->main(), i18n("No language plugin for %1 found.").arg(m_info->m_language));
  
  KService::Ptr languageSupportService = *languageSupportOffers.begin();
  KDevLanguageSupport *langSupport = KParts::ComponentFactory
      ::createInstanceFromService<KDevLanguageSupport>( languageSupportService,
                                                        API::getInstance(),
                                                        0,
                                                        PluginController::argumentsFromService(  languageSupportService ) );

    if ( !langSupport )
      return;

    API::getInstance()->setLanguageSupport( langSupport );
    integratePart( langSupport );
}


void ProjectManager::loadLocalParts()
{
  KTrader::OfferList localOffers = PluginController::pluginServices( "Project" );
  for (KTrader::OfferList::ConstIterator it = localOffers.begin(); it != localOffers.end(); ++it)
  {
    if (m_info->m_ignoreParts.contains((*it)->name()))
      continue;

    if (m_info->m_loadParts.contains((*it)->name()))
      loadService( *it );
    else
      checkNewService(*it);
  }
}


void ProjectManager::checkNewService(const KService::Ptr &service)
{
  QVariant var = service->property("X-KDevelop-ProgrammingLanguages");
  QStringList langlist = var.asStringList();
  if (langlist.contains(m_info->m_language) || langlist.isEmpty()) // empty means it support all languages
  {
    // the language is ok, now check if the keywords match
    bool keywordsMatch = true;
    QStringList serviceKeywords = service->keywords();
    QStringList::Iterator is = serviceKeywords.begin();
    while (is != serviceKeywords.end())
    {
      if (m_info->m_keywords.contains(*is) == 0 && keywordsMatch)
      {
        // no match
        keywordsMatch = false;
        kdDebug(9000) << "ignoreParts because Keyword doesn't match: " << service->name() << endl;
        m_info->m_ignoreParts << service->name();
      }
      is++;
    }

    // the language and all keywords match or no keywords available
    if(keywordsMatch)
    {
      if ( loadService( service ) )
        m_info->m_loadParts << service->name();
    }
  }
  else
  {
    // the language doesn't match
    m_info->m_ignoreParts << service->name();
  }
}


void ProjectManager::initializeProjectSupport()
{
  if (!API::getInstance()->project())
    return;

  QFileInfo fi(m_info->m_fileName);
  QString projectDir = fi.dirPath();
  QString projectName = fi.baseName();
  kdDebug(9000) << "projectDir: " << projectDir << "  projectName: " << projectName << endl;

  API::getInstance()->project()->openProject(projectDir, projectName);

  Core::getInstance()->doEmitProjectOpened();
}

bool ProjectManager::loadService( const KService::Ptr &service ) 
{
  KXMLGUIClient *part = PluginController::loadPlugin( service ); 
  if ( !part ) return false;

  integratePart( part );
  m_info->m_localParts.append( part );

  return true;
}

void ProjectManager::integratePart(KXMLGUIClient *part)
{
  TopLevel::getInstance()->main()->guiFactory()->addClient(part);
}


void ProjectManager::removePart(KXMLGUIClient *part)
{
  TopLevel::getInstance()->main()->guiFactory()->removeClient(part);
   
  delete part;
}


void ProjectManager::closeProject()
{
  if (!m_info)
    return;

  if (API::getInstance()->project())
  {
    if (!closeProjectSources())
      return;
      
    Core::getInstance()->doEmitProjectClosed();

    API::getInstance()->project()->closeProject();

    unloadLocalParts();
    unloadLanguageSupport();
    saveProjectFile();

    removePart(API::getInstance()->project());
    API::getInstance()->setProject(0);
  }

  API::getInstance()->classStore()->wipeout();
  API::getInstance()->ccClassStore()->wipeout();

  delete m_info;
  m_info = 0;
}


bool ProjectManager::closeProjectSources()
{
  QStringList sources = API::getInstance()->project()->allSourceFiles();
  return PartController::getInstance()->closeDocuments(sources);
}


void ProjectManager::unloadLocalParts()
{
  while (!m_info->m_localParts.isEmpty())
  {
    removePart(m_info->m_localParts.first());
    m_info->m_localParts.removeFirst();
  }
}


void ProjectManager::unloadLanguageSupport()
{
  if (API::getInstance()->languageSupport())
  {
    removePart(API::getInstance()->languageSupport());
    API::getInstance()->setLanguageSupport(0);
  }
}


void ProjectManager::saveProjectFile()
{
  if (API::getInstance()->projectDom())
  {
    QFile fout(m_info->m_fileName);
    if (fout.open(IO_WriteOnly))
    {
      QTextStream stream(&fout);
      API::getInstance()->projectDom()->save(stream, 2);
    }
    else
      KMessageBox::sorry(TopLevel::getInstance()->main(), i18n("Could not write the project file."));

    fout.close();

    API::getInstance()->setProjectDom(0);
  }
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


#include "projectmanager.moc"
