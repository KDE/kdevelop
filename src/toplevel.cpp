#include <qlayout.h>
#include <qstatusbar.h>
#include <qmultilineedit.h>
#include <qvbox.h>
#include <qcheckbox.h>


#include <kapplication.h>
#include <kstdaction.h>
#include <kdebug.h>
#include <kaction.h>
#include <klocale.h>
#include <kconfig.h>
#include <kfiledialog.h>


#include "widgets/ktabzoomwidget.h"


#include "projectmanager.h"
#include "partcontroller.h"
#include "partselectwidget.h"
#include "api.h"
#include "core.h"
#include "settingswidget.h"


#include "toplevel.h"


TopLevel *TopLevel::s_instance = 0;


TopLevel::TopLevel(QWidget *parent, const char *name)
  : KParts::MainWindow(parent, name)
{
  s_instance = this;

  setXMLFile("gideonui.rc");

  createStatusBar();
  createFramework();
  createActions();

  createGUI(0);
}


TopLevel::~TopLevel()
{
}


TopLevel *TopLevel::getInstance()
{
  if (!s_instance)
    s_instance = new TopLevel(0, "toplevel");
  return s_instance;
}


void TopLevel::createStatusBar()
{
  (void) new QStatusBar(this);
}


void TopLevel::createFramework()
{
  m_leftBar = new KTabZoomWidget(this, KTabZoomPosition::Left);
  setCentralWidget(m_leftBar);

  m_bottomBar = new KTabZoomWidget(m_leftBar, KTabZoomPosition::Bottom);
  m_leftBar->addContent(m_bottomBar);

  PartController::createInstance(m_bottomBar, this);

  m_bottomBar->addContent(PartController::getInstance());
}


void TopLevel::createActions()
{
  KStdAction::quit(this, SLOT(slotQuit()), actionCollection());

  KAction *action;

  action = new KAction(i18n("&Open project..."), "project_open", 0,
                       this, SLOT(slotOpenProject()),
                       actionCollection(), "project_open");
  action->setStatusText( i18n("Opens a project"));

  m_openRecentProjectAction =
    new KRecentFilesAction(i18n("Open &recent project..."), 0,
                          this, SLOT(slotOpenRecentProject(const KURL &)),
                          actionCollection(), "project_open_recent");
  m_openRecentProjectAction->setStatusText(i18n("Opens a recent project"));

  m_closeProjectAction =
    new KAction(i18n("C&lose project"), "fileclose",0,
                this, SLOT(slotCloseProject()),
                actionCollection(), "project_close");
  m_closeProjectAction->setEnabled(false);
  m_closeProjectAction->setStatusText(i18n("Closes the current project"));

  m_projectOptionsAction = new KAction(i18n("Project &Options..."), 0,
                this, SLOT(slotProjectOptions()),
                actionCollection(), "project_options" );
  m_projectOptionsAction->setEnabled(false);
  
  action = KStdAction::preferences(this, SLOT(slotSettings()),
                actionCollection(), "settings_configure" );
  action->setStatusText( i18n("Lets you customize KDevelop") );
}



void TopLevel::slotQuit()
{
  saveSettings();

  kapp->quit();
}


void TopLevel::embedSelectView(QWidget *view, const QString &name)
{
  m_leftBar->addTab(view, name);
}


void TopLevel::embedOutputView(QWidget *view, const QString &name)
{
  m_bottomBar->addTab(view, name);
}


void TopLevel::removeView(QWidget *view)
{
}


void TopLevel::raiseView(QWidget *view)
{
  m_leftBar->raiseWidget(view);
  m_bottomBar->raiseWidget(view);
}


void TopLevel::createGUI(KParts::Part *part)
{
  KParts::MainWindow::createGUI(part);
}


void TopLevel::loadSettings()
{
  KConfig *config = kapp->config();
  config->setGroup("General Options");
  QString project = config->readEntry("Last Project", "");
  bool readProject = config->readBoolEntry("Read Last Project On Startup", true);
  if (!project.isEmpty() && readProject)
    ProjectManager::getInstance()->loadProject(project);

  m_openRecentProjectAction->loadEntries(config, "RecentProjects");
}


void TopLevel::saveSettings()
{
  KConfig *config = kapp->config();

  if (ProjectManager::getInstance()->projectLoaded())
  {
    config->setGroup("General Options");
    config->writeEntry("Last Project", ProjectManager::getInstance()->projectFile());
  }

  m_openRecentProjectAction->saveEntries(config, "RecentProjects");
}


void TopLevel::slotOpenProject()
{
  QString fileName = KFileDialog::getOpenFileName(QString::null, "*.kdevelop", TopLevel::getInstance(), i18n("Open project"));
  if (fileName.isNull())
    return;

  ProjectManager::getInstance()->loadProject(fileName);

  QString pf = ProjectManager::getInstance()->projectFile();
  if (ProjectManager::getInstance()->projectLoaded())
  {
    m_openRecentProjectAction->addURL(KURL(pf));
    m_closeProjectAction->setEnabled(true);
    m_projectOptionsAction->setEnabled(true);
  }
}


void TopLevel::slotOpenRecentProject(const KURL &url)
{
  ProjectManager::getInstance()->loadProject(url.path());

  QString pf = ProjectManager::getInstance()->projectFile();
  if (ProjectManager::getInstance()->projectLoaded())
  {
    m_openRecentProjectAction->addURL(KURL(pf));
    m_closeProjectAction->setEnabled(true);
    m_projectOptionsAction->setEnabled(true);
  }

}


void TopLevel::slotCloseProject()
{
  ProjectManager::getInstance()->closeProject();
  m_closeProjectAction->setEnabled(ProjectManager::getInstance()->projectLoaded());
  m_projectOptionsAction->setEnabled(ProjectManager::getInstance()->projectLoaded());
}


void TopLevel::slotProjectOptions()
{
  KDialogBase dlg(KDialogBase::TreeList, i18n("Project Options"),
                  KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, this,
                  "project options dialog");

  QVBox *vbox = dlg.addVBoxPage(i18n("Plugins"));
  PartSelectWidget *w = new PartSelectWidget(*API::getInstance()->projectDom(), vbox, "part selection widget");
  connect(&dlg, SIGNAL(okClicked()), w, SLOT(accept()) );

  Core::getInstance()->doEmitProjectConfigWidget(&dlg);
  dlg.exec();
}


void TopLevel::slotSettings()
{
  KDialogBase dlg(KDialogBase::TreeList, i18n("Customize KDevelop"),
                  KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, this,
                  "customization dialog");

  QVBox *vbox = dlg.addVBoxPage(i18n("General"));
  SettingsWidget *gsw = new SettingsWidget(vbox, "general settings widget");
  
  KConfig* config = kapp->config();
  config->setGroup("General Options");
  gsw->lastProjectCheckbox->setChecked(config->readBoolEntry("Read Last Project On Startup",true));


  vbox = dlg.addVBoxPage(i18n("Plugins"));
  PartSelectWidget *w = new PartSelectWidget(vbox, "part selection widget");
  connect( &dlg, SIGNAL(okClicked()), w, SLOT(accept()) );

  Core::getInstance()->doEmitConfigWidget(&dlg);
  dlg.exec();

  config->setGroup("General Options");
  config->writeEntry("Read Last Project On Startup",gsw->lastProjectCheckbox->isChecked());
}



#include "toplevel.moc"
