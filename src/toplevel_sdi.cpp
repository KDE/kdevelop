#include <qlayout.h>
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
#include <kstatusbar.h>


#include "widgets/ktabzoomwidget.h"


#include "projectmanager.h"
#include "partcontroller.h"
#include "partselectwidget.h"
#include "api.h"
#include "core.h"
#include "settingswidget.h"
#include "statusbar.h"


#include "toplevel_sdi.h"


TopLevelSDI::TopLevelSDI(QWidget *parent, const char *name)
  : KParts::MainWindow(parent, name), m_closing(false)
{
}


void TopLevelSDI::init()
{
  setXMLFile("gideonui.rc");

  createFramework();
  createActions();
  createStatusBar();

  createGUI(0);
}


TopLevelSDI::~TopLevelSDI()
{
}


bool TopLevelSDI::queryClose()
{
  if (m_closing)
    return true;

  emit wantsToQuit();
  return false;
}


void TopLevelSDI::realClose()
{
  saveSettings();
  
  m_closing = true;
  close();
}


KMainWindow *TopLevelSDI::main()
{
  return this;
}


void TopLevelSDI::createStatusBar()
{
  (void) new StatusBar(this);
}


void TopLevelSDI::createFramework()
{
  m_leftBar = new KTabZoomWidget(this, KTabZoomPosition::Left);
  setCentralWidget(m_leftBar);

  m_bottomBar = new KTabZoomWidget(m_leftBar, KTabZoomPosition::Bottom);
  m_leftBar->addContent(m_bottomBar);

  m_tabWidget = new QTabWidget(m_bottomBar);
  m_tabWidget->setMargin(2);

  PartController::createInstance(m_tabWidget);

  m_bottomBar->addContent(m_tabWidget);

  connect(m_tabWidget, SIGNAL(currentChanged(QWidget*)), 
	  PartController::getInstance(), SLOT(slotCurrentChanged(QWidget*)));

  connect(PartController::getInstance(), SIGNAL(activePartChanged(KParts::Part*)),
	  this, SLOT(createGUI(KParts::Part*)));
}


void TopLevelSDI::createActions()
{
  KStdAction::quit(this, SLOT(slotQuit()), actionCollection());

  KAction *action;

  action = new KAction(i18n("&Open Project..."), "project_open", 0,
                       this, SLOT(slotOpenProject()),
                       actionCollection(), "project_open");
  action->setStatusText( i18n("Opens a project"));

  m_openRecentProjectAction =
    new KRecentFilesAction(i18n("Open &Recent Project..."), 0,
                          this, SLOT(slotOpenRecentProject(const KURL &)),
                          actionCollection(), "project_open_recent");
  m_openRecentProjectAction->setStatusText(i18n("Opens a recent project"));

  m_closeProjectAction =
    new KAction(i18n("C&lose Project"), "fileclose",0,
                this, SLOT(slotCloseProject()),
                actionCollection(), "project_close");
  m_closeProjectAction->setEnabled(false);
  m_closeProjectAction->setStatusText(i18n("Closes the current project"));

  m_projectOptionsAction = new KAction(i18n("Project &Options..."), "configure", 0,
                this, SLOT(slotProjectOptions()),
                actionCollection(), "project_options" );
  m_projectOptionsAction->setEnabled(false);
  
  action = KStdAction::preferences(this, SLOT(slotSettings()),
                actionCollection(), "settings_configure" );
  action->setStatusText( i18n("Lets you customize KDevelop") );
}



void TopLevelSDI::slotQuit()
{
  (void) queryClose();
}


void TopLevelSDI::embedPartView(QWidget *view, const QString &name)
{
  m_tabWidget->addTab(view, name);
  m_tabWidget->showPage(view);
}


void TopLevelSDI::embedSelectView(QWidget *view, const QString &name)
{
  m_leftBar->addTab(view, name);
}


void TopLevelSDI::embedOutputView(QWidget *view, const QString &name)
{
  m_bottomBar->addTab(view, name);
}


void TopLevelSDI::removeView(QWidget *)
{
}


void TopLevelSDI::raiseView(QWidget *view)
{
  m_leftBar->raiseWidget(view);
  m_bottomBar->raiseWidget(view);
  m_tabWidget->showPage(view);
}


void TopLevelSDI::lowerView(QWidget *view)
{
  m_leftBar->lowerWidget(view);
  m_bottomBar->lowerWidget(view);
}


void TopLevelSDI::lowerAllViews()
{
  m_leftBar->lowerAllWidgets();
  m_bottomBar->lowerAllWidgets();
}


void TopLevelSDI::createGUI(KParts::Part *part)
{
  KParts::MainWindow::createGUI(part);
}


void TopLevelSDI::loadSettings()
{
  KConfig *config = kapp->config();
  config->setGroup("General Options");
  QString project = config->readEntry("Last Project", "");
  bool readProject = config->readBoolEntry("Read Last Project On Startup", true);
  if (!project.isEmpty() && readProject)
  {
    ProjectManager::getInstance()->loadProject(project);
    m_closeProjectAction->setEnabled(ProjectManager::getInstance()->projectLoaded());
    m_projectOptionsAction->setEnabled(ProjectManager::getInstance()->projectLoaded());
  }

  m_openRecentProjectAction->loadEntries(config, "RecentProjects");

  applyMainWindowSettings(config, "Mainwindow");
}


void TopLevelSDI::saveSettings()
{
  KConfig *config = kapp->config();

  if (ProjectManager::getInstance()->projectLoaded())
  {
    config->setGroup("General Options");
    config->writeEntry("Last Project", ProjectManager::getInstance()->projectFile());
  }

  m_openRecentProjectAction->saveEntries(config, "RecentProjects");

  saveMainWindowSettings(config, "Mainwindow");
}


void TopLevelSDI::slotOpenProject()
{
  QString fileName = KFileDialog::getOpenFileName(QString::null, "*.kdevelop", this, i18n("Open Project"));
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


void TopLevelSDI::slotOpenRecentProject(const KURL &url)
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


void TopLevelSDI::slotCloseProject()
{
  ProjectManager::getInstance()->closeProject();
  m_closeProjectAction->setEnabled(ProjectManager::getInstance()->projectLoaded());
  m_projectOptionsAction->setEnabled(ProjectManager::getInstance()->projectLoaded());
}


void TopLevelSDI::slotProjectOptions()
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


void TopLevelSDI::slotSettings()
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



#include "toplevel_sdi.moc"
