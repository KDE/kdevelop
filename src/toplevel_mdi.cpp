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


#include "projectmanager.h"
#include "partcontroller.h"
#include "partselectwidget.h"
#include "api.h"
#include "core.h"
#include "settingswidget.h"
#include "statusbar.h"


#include "toplevel_mdi.h"


TopLevelMDI::TopLevelMDI(QWidget *parent, const char *name)
  : QextMdiMainFrm(parent, name), m_closing(false)
{
}


void TopLevelMDI::init()
{
  setXMLFile("gideonui.rc");

  createFramework();
  createActions();
  createStatusBar();

  createGUI(0);
}


TopLevelMDI::~TopLevelMDI()
{
}


bool TopLevelMDI::queryClose()
{
  if (m_closing)
    return true;

  emit wantsToQuit();
  return false;
}


void TopLevelMDI::realClose()
{
  saveSettings();

  m_closing = true;
  close();
}


KMainWindow *TopLevelMDI::main()
{
  return this;
}


void TopLevelMDI::createStatusBar()
{
  (void) new StatusBar(this);
}


void TopLevelMDI::createFramework()
{
  PartController::createInstance(this);

  connect(PartController::getInstance(), SIGNAL(activePartChanged(KParts::Part*)),
	  this, SLOT(createGUI(KParts::Part*)));

  setMenuForSDIModeSysButtons(menuBar());
}


void TopLevelMDI::createActions()
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

  m_projectOptionsAction = new KAction(i18n("Project &Options..."), 0,
                this, SLOT(slotProjectOptions()),
                actionCollection(), "project_options" );
  m_projectOptionsAction->setEnabled(false);

  action = KStdAction::preferences(this, SLOT(slotSettings()),
                actionCollection(), "settings_configure" );
  action->setStatusText(i18n("Lets you customize KDevelop") );
}


void TopLevelMDI::slotQuit()
{
  (void) queryClose();
}


QextMdiChildView *TopLevelMDI::wrapper(QWidget *view, const QString &name)
{
  Q_ASSERT( view ); // if this assert fails, then some part didn't return a widget. Fix the part ;)

  QextMdiChildView* pMDICover = new QextMdiChildView(name);
  QBoxLayout* pLayout = new QHBoxLayout( pMDICover, 0, -1, "layout");
  view->reparent(pMDICover, QPoint(0,0));
  pLayout->addWidget(view);
  pMDICover->setName(name);
  QString shortName = name;
  int length = shortName.length();
  shortName = shortName.right(length - (shortName.findRev('/') +1));
  pMDICover->setTabCaption(shortName);
  pMDICover->setCaption(name);

  m_widgetMap.insert(view, pMDICover);

  return pMDICover;
}


void TopLevelMDI::embedPartView(QWidget *view, const QString &name)
{
  QextMdiChildView *child = wrapper(view, name);

  unsigned int mdiFlags = QextMdi::StandardAdd | QextMdi::Maximize;
  
  addWindow(child, QPoint(0,0), mdiFlags);

  m_partViews.append(child);
}


void TopLevelMDI::embedSelectView(QWidget *view, const QString &name)
{
  QWidget *first = m_selectViews.first();

  QextMdiChildView *child = wrapper(view, name);

  if (!first)
  {
    if (mdiMode() == QextMdi::TabPageMode)
      first = m_partViews.first();

    if (!first)
      first = this;
    
    addToolWindow(child, KDockWidget::DockLeft, first, 25, name, name);
  }
  else
    addToolWindow(child, KDockWidget::DockCenter, first, 25, name, name);

  m_selectViews.append(child);
}


void TopLevelMDI::embedOutputView(QWidget *view, const QString &name)
{
  QWidget *first = m_outputViews.first();

  QextMdiChildView *child = wrapper(view, name);

  if (!first)
  {
    if (mdiMode() == QextMdi::TabPageMode)
      first = m_partViews.first();

    if (!first)
      first = this;

    addToolWindow(child, KDockWidget::DockBottom, first, 70, name, name);
  }
  else
    addToolWindow(child, KDockWidget::DockCenter, first, 70, name, name);

  m_outputViews.append(child);
}


void TopLevelMDI::removeView(QWidget *view)
{
  if (!view)
    return;

  QextMdiChildView *wrapper = m_widgetMap[view];

  if (wrapper)
  {
    removeWindowFromMdi(wrapper);

    m_selectViews.remove(wrapper);
    m_outputViews.remove(wrapper);
    m_partViews.remove(wrapper);

    m_widgetMap.remove(view);

    // Note: this reparenting is necessary. Otherwise, the view gets
    // deleted twice: once when the wrapper is deleted, and the second
    // time when the part is deleted.
    view->reparent(this, QPoint(0,0), false);

    closeWindow(wrapper);
  }
}


void TopLevelMDI::raiseView(QWidget *view)
{
  QextMdiChildView *wrapper = m_widgetMap[view];
  if (wrapper)
  {
    wrapper->activate();
    activateView(wrapper);
  }
}


void TopLevelMDI::lowerView(QWidget *)
{
  // ignored in MDI mode!
}


void TopLevelMDI::createGUI(KParts::Part *part)
{
  QextMdiMainFrm::createGUI(part);
}


void TopLevelMDI::loadSettings()
{
  loadMDISettings();

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


void TopLevelMDI::loadMDISettings()
{
  KConfig *config = kapp->config();
  config->setGroup("UI");

  int mdiMode = config->readNumEntry("MDI mode", QextMdi::ChildframeMode);
  switch (mdiMode) 
  {
  case QextMdi::ToplevelMode:
    {
      int childFrmModeHt = config->readNumEntry("Childframe mode height", kapp->desktop()->height() - 50);
      resize(width(), childFrmModeHt);
      switchToToplevelMode();
    }
    break;
  
  case QextMdi::ChildframeMode:
    break;
  
  case QextMdi::TabPageMode:
    {
      int childFrmModeHt = config->readNumEntry("Childframe mode height", kapp->desktop()->height() - 50);
      resize(width(), childFrmModeHt);
      switchToTabPageMode();
    }
    break;
  
  default:
    break;
  }
  
  // restore a possible maximized Childframe mode
  bool maxChildFrmMode = config->readBoolEntry("maximized childframes", true);
  setEnableMaximizedChildFrmMode(maxChildFrmMode);
}


void TopLevelMDI::saveSettings()
{
  KConfig *config = kapp->config();

  if (ProjectManager::getInstance()->projectLoaded())
  {
    config->setGroup("General Options");
    config->writeEntry("Last Project", ProjectManager::getInstance()->projectFile());
  }

  m_openRecentProjectAction->saveEntries(config, "RecentProjects");

  saveMDISettings();

  saveMainWindowSettings(config, "Mainwindow");
}


void TopLevelMDI::saveMDISettings()
{
  KConfig *config = kapp->config();
  config->setGroup("UI");

  config->writeEntry("MDI mode", mdiMode());
}


void TopLevelMDI::slotOpenProject()
{
  QString fileName = KFileDialog::getOpenFileName(QString::null, "*.kdevelop", this, i18n("Open project"));
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


void TopLevelMDI::slotOpenRecentProject(const KURL &url)
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


void TopLevelMDI::slotCloseProject()
{
  ProjectManager::getInstance()->closeProject();
  m_closeProjectAction->setEnabled(ProjectManager::getInstance()->projectLoaded());
  m_projectOptionsAction->setEnabled(ProjectManager::getInstance()->projectLoaded());
}


void TopLevelMDI::slotProjectOptions()
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


void TopLevelMDI::slotSettings()
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


void TopLevelMDI::resizeEvent(QResizeEvent *ev)
{
  QextMdiMainFrm::resizeEvent(ev);
  setSysButtonsAtMenuPosition();
}


#include "toplevel_mdi.moc"
