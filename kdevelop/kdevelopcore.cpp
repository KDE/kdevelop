/***************************************************************************
 *   Copyright (C) 2000 by The KDevelop Team                               *
 *   kdevelop-team@kdevelop.org                                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kdebug.h>
#include <ktrader.h>
#include <klibloader.h>
#include <klocale.h>
#include <kstdaction.h>
#include <kdialogbase.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <qdom.h>


#include "classstore.h"
#include "cproject.h"
#include "projectoptionsdlg.h"
#include "kdevelop.h"
#include "kdevcomponent.h"
#include "kdevversioncontrol.h"
#include "kdevlanguagesupport.h"
#include "kdevelopcore.h"
#include "kdevviewhandler.h"
#include "projectspace.h"
#include "newprojectdlg.h"


KDevelopCore::KDevelopCore(KDevelop *pGUI)
    : QObject(pGUI, "kdevelop core")
   ,m_pKDevelopGUI(pGUI)
   ,m_pVersionControl(0L)
   ,m_pLanguageSupport(0L)
   ,m_pMakeFrontend(0L)
   ,m_pAppFrontend(0L)
   ,m_pProject(0L)
   ,m_pViewHandler(0L)
{
    m_pClassStore = new ClassStore();
    m_pProjectSpace = 0L;
    initActions();
}


KDevelopCore::~KDevelopCore()
{}


void KDevelopCore::initComponent(KDevComponent *pComponent)
{
    connect( pComponent, SIGNAL(executeMakeCommand(const QString&)),
             this, SLOT(executeMakeCommand(const QString&)) );
    connect( pComponent, SIGNAL(executeAppCommand(const QString&)),
             this, SLOT(executeAppCommand(const QString&)) );
    connect( pComponent, SIGNAL(running(bool)),
             this, SLOT(running(bool)) );
    connect( pComponent, SIGNAL(gotoSourceFile(const QString&, int)),
             this, SLOT(gotoSourceFile(const QString&, int)) );
    connect( pComponent, SIGNAL(gotoDocumentationFile(const QString&)),
             this, SLOT(gotoDocumentationFile(const QString&)) );
    connect( pComponent, SIGNAL(gotoProjectApiDoc()),
             this, SLOT(gotoProjectApiDoc()) );
    connect( pComponent, SIGNAL(gotoProjectManual()),
             this, SLOT(gotoProjectManual()) );
    connect( pComponent, SIGNAL(embedWidget(QWidget*, KDevComponent::Role, const QString&, const QString&)),
             m_pKDevelopGUI, SLOT(embedWidget(QWidget *, KDevComponent::Role, const QString&, const QString&)) );

    pComponent->setupGUI();
    m_components.append(pComponent);
}


void KDevelopCore::initActions()
{
    KAction *pAction;
    
   pAction = KStdAction::print( this, SLOT( slotFilePrint() ),
                               m_pKDevelopGUI->actionCollection(), "file_print_advanced");
   pAction->setStatusText( i18n("Prints the current document") );
   pAction->setWhatsThis( i18n("Print\n\n"
                              "Opens the printing dialog. There, you can "
                              "configure which printing program you wish "
                              "to use, and print your project files.") );

    
    pAction = new KAction( i18n("&New..."),0, this, SLOT( slotProjectNew() ),
                          m_pKDevelopGUI->actionCollection(), "project_new");
    pAction->setStatusText( i18n("Creates a new Projectspace/Project") );
    pAction->setWhatsThis(  i18n("new project...") );

    pAction = new KAction( i18n("&Open..."), "openprj", 0, this, SLOT( slotProjectOpen() ),
                          m_pKDevelopGUI->actionCollection(), "project_open");
    pAction->setStatusText( i18n("Opens an existing project") );

    pAction = new KRecentFilesAction( i18n("Open &recent project..."), 0, this, SLOT( slotProjectOpenRecent(const KURL&) ),
                                     m_pKDevelopGUI->actionCollection(), "project_open_recent");

    pAction = new KAction( i18n("C&lose"), 0, this, SLOT( slotProjectClose() ),
                          m_pKDevelopGUI->actionCollection(), "project_close");
    pAction->setEnabled(false);
    pAction->setStatusText( i18n("Closes the current project") );

    

    pAction = new KAction( i18n("&Add existing File(s)..."), 0, this, SLOT( slotProjectAddExistingFiles() ),
                          m_pKDevelopGUI->actionCollection(), "project_add_existing_files");
    pAction->setEnabled(false);
    pAction->setStatusText( i18n("Adds existing file(s) to the project") );
    
    pAction = new KAction( i18n("&Options..."), 0, this, SLOT( slotProjectOptions() ),
                          m_pKDevelopGUI->actionCollection(), "project_options");
    pAction->setStatusText( i18n("Sets project and compiler options") );


    pAction = new KAction( i18n("&KDevelop Setup..."), 0, this, SLOT( slotOptionsKDevelopSetup() ),
                          m_pKDevelopGUI->actionCollection(), "options_kdevelop_setup");
    pAction->setStatusText( i18n("Configures KDevelop") );

    pAction = new KAction( i18n("&Stop"), "stop_proc", 0, this, SLOT( slotStop() ),
                          m_pKDevelopGUI->actionCollection(), "stop_everything");
    pAction->setEnabled(false);
}


void KDevelopCore::loadInitialComponents()
{
  KTrader::OfferList::ConstIterator it;
  KTrader::OfferList viewHandlerOffers;
  bool bUseViewHandler = true;  // TODO: make this configurable!

  //
  // first, load a view handling component (MDI main frame or KDockWidget-based view docking site or ...)
  // (the precondition for other components which want to embed views!)
  //
  if( bUseViewHandler)
    viewHandlerOffers = KTrader::self()->query("KDevelop/Component/ViewHandler");

  if (!viewHandlerOffers.isEmpty()) {
    // check the list of hits and take the right one (TODO: chosen in kdevelop options)
    bool bFound = false;
    for (it = viewHandlerOffers.begin(); (it != viewHandlerOffers.end()) || !bFound; ++it) {
      // at the moment it's a trivial rule: take the first one it finds
      bFound = true;
      break;
    }

    kdDebug(9000) << "Found view-handler component " << (*it)->name() << endl;
    KLibFactory *pFactory = KLibLoader::self()->factory((*it)->library());

    QStringList args;
    QVariant prop = (*it)->property("X-KDevelop-Args");
    if (prop.isValid())
      args = QStringList::split(" ", prop.toString());

    QObject *pObj = pFactory->create(m_pKDevelopGUI, (*it)->name().latin1(), "KDevComponent", args);
    if (!pObj->inherits("KDevComponent")) {
      kdDebug(9000) << "Component does not inherit KDevComponent" << endl;
      return;
    }
    m_pViewHandler = (KDevViewHandler*) pObj;
    QObject::connect( m_pKDevelopGUI, SIGNAL(addView(QWidget*)), m_pViewHandler, SLOT(addView(QWidget*)) );
    // initialize the view handler component and it's GUI (the mainframe widget)
    initComponent( m_pViewHandler);
    m_pKDevelopGUI->guiFactory()->addClient( m_pViewHandler);
  }

  if (viewHandlerOffers.isEmpty() || !bUseViewHandler) {
    kdDebug(9000) << "No KDevelop view-handler components. Setting to default (stacked KDockWidgets)..." << endl;
    // create a default dockwidget (required as main widget)
    QWidget* pW = new QWidget( m_pKDevelopGUI, "default");
    m_pKDevelopGUI->embedWidget( pW, KDevComponent::AreaOfDocumentViews, "default view", 0L );
    QObject::connect( m_pKDevelopGUI, SIGNAL(addView(QWidget*)), m_pKDevelopGUI, SLOT(stackView(QWidget*)) );
  }

  //
  // second, load all other components (and let them create their GUI)
  //
  KTrader::OfferList restOfOffers = KTrader::self()->query("KDevelop/Component");
  if (restOfOffers.isEmpty())
    kdDebug(9000) << "No other KDevelop components" << endl;

  for (it = restOfOffers.begin(); it != restOfOffers.end(); ++it) {
        
    kdDebug(9000) << "Found Component " << (*it)->name() << endl;
    KLibFactory *factory = KLibLoader::self()->factory((*it)->library());

    QStringList args;
    QVariant prop = (*it)->property("X-KDevelop-Args");
    if (prop.isValid())
      args = QStringList::split(" ", prop.toString());

    QObject *pObj = factory->create(m_pKDevelopGUI, (*it)->name().latin1(),
                                  "KDevComponent", args);

    if (!pObj->inherits("KDevComponent")) {
      kdDebug(9000) << "Component does not inherit KDevComponent" << endl;
      return;
    }
    KDevComponent *pComp = (KDevComponent*) pObj;

    if (!m_pMakeFrontend && (*it)->hasServiceType("KDevelop/MakeFrontend")) {
      m_pMakeFrontend = pComp;
      kdDebug(9000) << "is make frontend" << endl;
    }

    if (!m_pAppFrontend && (*it)->hasServiceType("KDevelop/AppFrontend")) {
      m_pAppFrontend = pComp;
      kdDebug(9000) << "is app frontend" << endl;
    }

    initComponent(pComp);
    m_pKDevelopGUI->guiFactory()->addClient(pComp);
  }
}


void KDevelopCore::loadVersionControl(const QString &name)
{
    KService::Ptr service = KService::serviceByName(name);
    if (!service) {
        KMessageBox::sorry(m_pKDevelopGUI,
                           i18n("No version control component %1 found").arg(name));
        return;
    }

    kdDebug(9000) << "Loading VersionControl Component " << service->name() << endl;

    KLibFactory *pFactory = KLibLoader::self()->factory(service->library());

    QStringList args;
    QVariant prop = service->property("X-KDevelop-Args");
    if (prop.isValid())
        args = QStringList::split(" ", prop.toString());

    QObject *pObj = pFactory->create(m_pKDevelopGUI, service->name().latin1(),
                                    "KDevVersionControl", args);

    if (!pObj->inherits("KDevVersionControl")) {
        kdDebug(9000) << "Component does not inherit KDevVersionControl" << endl;
        return;
    }
    KDevVersionControl *pComp = (KDevVersionControl*) pObj;
    m_pVersionControl = pComp;
    initComponent(pComp);
}


void KDevelopCore::unloadVersionControl()
{
    m_components.remove(m_pVersionControl);
    delete m_pVersionControl;
    m_pVersionControl = 0;
}


void KDevelopCore::loadLanguageSupport(const QString &lang)
{
    QString constraint = QString("[X-KDevelop-Language] == '%1'").arg(lang);
    KTrader::OfferList offers = KTrader::self()->query("KDevelop/LanguageSupport", constraint);
    if (offers.isEmpty()) {
        KMessageBox::sorry(m_pKDevelopGUI,
                           i18n("No language support component for %1 found").arg(lang));
        return;
    }

    KService *pService = *offers.begin();
    kdDebug(9000) << "Found LanguageSupport Component " << pService->name() << endl;

    KLibFactory *pFactory = KLibLoader::self()->factory(pService->library());

    QStringList args;
    QVariant prop = pService->property("X-KDevelop-Args");
    if (prop.isValid())
        args = QStringList::split(" ", prop.toString());

    QObject *pObj = pFactory->create(m_pKDevelopGUI, pService->name().latin1(),
                                     "KDevLanguageSupport", args);

    if (!pObj->inherits("KDevLanguageSupport")) {
        kdDebug(9000) << "Component does not inherit KDevLanguageSupport" << endl;
        return;
    }
    KDevLanguageSupport *pComp = (KDevLanguageSupport*) pObj;
    m_pLanguageSupport = pComp;
    initComponent(pComp);
}


void KDevelopCore::unloadLanguageSupport()
{
  m_components.remove(m_pLanguageSupport);
  delete m_pLanguageSupport;
  m_pLanguageSupport = 0;
}

bool KDevelopCore::loadProjectSpace(const QString &name){
  QString constraint = QString("[Name] == '%1'").arg(name);
  KTrader::OfferList offers = KTrader::self()->query("KDevelop/ProjectSpace", constraint);
  if (offers.isEmpty()) {
    KMessageBox::sorry(m_pKDevelopGUI,
		       i18n("No ProjectSpace component for %1 found").arg(name));
    return false;
  }

  KService *pService = *offers.begin();
  kdDebug(9000) << "Found ProjectSpace Component " << pService->name() << endl;

  KLibFactory *pFactory = KLibLoader::self()->factory(pService->library());

  QStringList args;
  QVariant prop = pService->property("X-KDevelop-Args");
  if (prop.isValid())
    args = QStringList::split(" ", prop.toString());

  QObject *pObj = pFactory->create(m_pKDevelopGUI, pService->name().latin1(),
				                       "ProjectSpace", args);

  if (!pObj->inherits("ProjectSpace")) {
    kdDebug(9000) << "Component does not inherit ProjectSpace" << endl;
    return false;
  }
  ProjectSpace *pComp = (ProjectSpace*) pObj;
  m_pProjectSpace = pComp;
  initComponent(pComp);
  m_pKDevelopGUI->guiFactory()->addClient( m_pProjectSpace);

  return true;
}


void KDevelopCore::newFile()
{
  //just a test!
  QWidget* pEV = new QWidget(0L);
  m_pKDevelopGUI->embedWidget( pEV, KDevComponent::DocumentView, "Document", 0L);
  pEV->show();
}

void KDevelopCore::unloadProjectSpace(){
  QDomDocument* pDoc=0;
  pDoc = m_pProjectSpace->writeGlobalDocument();
  QListIterator<KDevComponent> it1(m_components);
  for (; it1.current(); ++it1){
    (*it1)->writeProjectSpaceGlobalConfig(*pDoc);
  }
  
  pDoc = m_pProjectSpace->writeUserDocument();
  QListIterator<KDevComponent> it2(m_components);
  for (; it2.current(); ++it2){
    (*it2)->writeProjectSpaceUserConfig(*pDoc);
  }
  m_pProjectSpace->saveConfig();
  m_components.remove(m_pProjectSpace);
  delete m_pProjectSpace;
  m_pProjectSpace = 0L;
}

void KDevelopCore::loadProject(const QString &fileName)
{

  m_pProject = new CProject("../kdevelop.kdevprj"); // will be removed soon
  // project must define a version control system
  // hack until implemented
  QString vcservice = QString::fromLatin1("CVSInterface");

    //ok, a little bit bootstrapping
  QString projectSpace = ProjectSpace::projectSpacePluginName(fileName);

  if(loadProjectSpace(projectSpace)){
    m_pProjectSpace->readConfig(fileName);
    m_pProjectSpace->dump();
    loadLanguageSupport(m_pProjectSpace->programmingLanguage());
    loadVersionControl(vcservice);
    
    // read the config for all components (maybe also other plugins?)
    QDomDocument* pDoc = m_pProjectSpace->readGlobalDocument();
    QListIterator<KDevComponent> it1(m_components);
    for (; it1.current(); ++it1){
      (*it1)->readProjectSpaceGlobalConfig(*pDoc);
    }

    pDoc = m_pProjectSpace->readUserDocument();
    QListIterator<KDevComponent> it2(m_components);
    for (; it2.current(); ++it2){
      (*it2)->readProjectSpaceUserConfig(*pDoc);
    }
   
  }
  
  QListIterator<KDevComponent> it1(m_components);
  for (; it1.current(); ++it1)
    (*it1)->projectOpened(m_pProject);
  
  QListIterator<KDevComponent> it2(m_components);
  for (; it2.current(); ++it2)
        (*it2)->classStoreOpened(m_pClassStore);
  
  
  if (m_pVersionControl) {
    QListIterator<KDevComponent> it3(m_components);
       for (; it3.current(); ++it3)
        (*it3)->versionControlOpened(m_pVersionControl);
  }

  
  if (m_pLanguageSupport) {
    QListIterator<KDevComponent> it4(m_components);
        for (; it4.current(); ++it4)
            (*it4)->languageSupportOpened(m_pLanguageSupport);
  }
  
  // some actions
  KActionCollection *pAC = m_pKDevelopGUI->actionCollection();
  pAC->action("project_close")->setEnabled(true);
  /*
    pAC->action("project_add_existing_files")->setEnabled(true);
    pAC->action("project_add_translation")->setEnabled(true);
    pAC->action("project_file_properties")->setEnabled(true);
    pAC->action("project_options")->setEnabled(true);
    
    ((KRecentFilesAction*)pAC->action("project_open_recent"))->addURL(KURL(fileName));
  */
  
#if 1
  // Hack to test the class viewer
  QListIterator<KDevComponent> it5(m_components);
  for (; it5.current(); ++it5)
    (*it5)->savedFile("parts/classview/test.cpp");
#endif
}


void KDevelopCore::unloadProject()
{
  unloadProjectSpace(); // temp
    if (m_pLanguageSupport) {
        QListIterator<KDevComponent> it1(m_components);
        for (; it1.current(); ++it1)
            (*it1)->languageSupportClosed();
        unloadLanguageSupport();
    }

    if (m_pVersionControl) {
        QListIterator<KDevComponent> it2(m_components);
        for (; it2.current(); ++it2)
            (*it2)->versionControlClosed();
        unloadVersionControl();
    }

    QListIterator<KDevComponent> it3(m_components);
    for (; it3.current(); ++it3)
        (*it3)->classStoreClosed();
    m_pClassStore->wipeout();

    QListIterator<KDevComponent> it4(m_components);
    for (; it4.current(); ++it4)
        (*it4)->projectClosed();

    delete m_pProject;
    m_pProject = 0;

    KActionCollection *pAC = m_pKDevelopGUI->actionCollection();
    pAC->action("project_close")->setEnabled(false);
    /*pAC->action("project_add_existing_files")->setEnabled(false);
    pAC->action("project_add_translation")->setEnabled(false);
    pAC->action("project_file_properties")->setEnabled(false);
    pAC->action("project_options")->setEnabled(false);
    */
}


void KDevelopCore::slotFilePrint()
{
    // this hardcoded file name is hack ...
    // will be replaced by a trader-based solution later
    KLibFactory *pFactory = KLibLoader::self()->factory("libkdevprintplugin");
    if (!pFactory)
        return;

    QStringList args;
    args << "/vmlinuz"; // temporary ;-)
    QObject *pObj = pFactory->create(m_pKDevelopGUI, "print dialog", "KDevPrintDialog", args);
    if (!pObj->inherits("QDialog")) {
        kdDebug(9000) << "Print plugin doesn't provide a dialog" << endl;
        return;
    }

    QDialog *pDlg = (QDialog *)pObj;
    pDlg->exec();
    delete pDlg;
}

void KDevelopCore::slotFileNew()
{
  kdDebug("running KDevelopCore::slotFileNew...");
  newFile();
}

void KDevelopCore::slotProjectNew(){
  // if m_pProjectSpace == 0, create a new one
  NewProjectDlg* dlg = new NewProjectDlg(m_pProjectSpace);
  dlg->show();
  delete dlg;
}

void KDevelopCore::slotProjectOpen()
{
    QString fileName = KFileDialog::getOpenFileName(QString::null, "*.kdevpsp",
                                                    m_pKDevelopGUI, i18n("Open project"));
    if (fileName ==""){
      return; // cancel

    }

    if (m_pProject)
        unloadProject();

    loadProject(fileName);
}


void KDevelopCore::slotProjectOpenRecent(const KURL &url)
{
    QString fileName = url.fileName();

    if (m_pProject)
        unloadProject();

    loadProject(fileName);
}


void KDevelopCore::slotProjectClose()
{
    // Ask for confirmation?
    unloadProject();
}


void KDevelopCore::slotProjectAddExistingFiles()
{
}


void KDevelopCore::slotProjectAddNewTranslationFile()
{
}


void KDevelopCore::slotProjectOptions()
{
    ProjectOptionsDialog *pDlg = new ProjectOptionsDialog(m_pKDevelopGUI, "project options dialog");
    pDlg->exec();
    delete pDlg;
}


void KDevelopCore::slotStop()
{
    QListIterator<KDevComponent> it(m_components);
    for (; it.current(); ++it)
        (*it)->stopButtonClicked();
}




void KDevelopCore::slotOptionsKDevelopSetup()
{
    KDialogBase *pDlg = new KDialogBase(KDialogBase::TreeList, i18n("Customize KDevelop"),
                                        KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, m_pKDevelopGUI,
                                        "customize dialog");

    QListIterator<KDevComponent> it(m_components);
    for (; it.current(); ++it) {
        (*it)->configWidgetRequested(pDlg);
    }

    pDlg->exec();
    delete pDlg;
}



void KDevelopCore::executeMakeCommand(const QString &command)
{
    if (!m_pMakeFrontend) {
        kdDebug(9000) << "No make frontend!" << command << endl;
        return;
    }

    m_pMakeFrontend->commandRequested(command);
}


void KDevelopCore::executeAppCommand(const QString &command)
{
    if (!m_pAppFrontend) {
        kdDebug(9000) << "No app frontend!" << command << endl;
        return;
    }

    m_pAppFrontend->commandRequested(command);
}


void KDevelopCore::running(bool runs)
{
    const KDevComponent *comp = static_cast<const KDevComponent*>(sender());

    if (runs)
        m_runningComponents.append(comp);
    else
        m_runningComponents.remove(comp);

    kdDebug(9000) << "Running components" << endl;
    QListIterator<KDevComponent> it(m_runningComponents);
    for (; it.current(); ++it)
        kdDebug(9000) << comp->name() << endl;

    KActionCollection *pAC = m_pKDevelopGUI->actionCollection();
    pAC->action("stop_everything")->setEnabled(!m_runningComponents.isEmpty());
}


void KDevelopCore::gotoSourceFile(const QString &fileName, int lineNo)
{
    kdDebug(9000) << "KDevelopCore::gotoSourceFile" << endl;
}


void KDevelopCore::gotoDocumentationFile(const QString &fileName)
{
    kdDebug(9000) << "KDevelopCore::gotoDocumentationFile" << endl;
}


void KDevelopCore::gotoProjectApiDoc()
{
    kdDebug(9000) << "KDevelopCore::gotoProjectApiDoc" << endl;
}


void KDevelopCore::gotoProjectManual()
{
    kdDebug(9000) << "KDevelopCore::gotoProjectManual" << endl;
}

KDevViewHandler* KDevelopCore::viewHandler()
{
   return m_pViewHandler;
}
void KDevelopCore::writeProjectSpaceGlobalConfig(QDomDocument& doc){
  QListIterator<KDevComponent> it(m_components);
  for (; it.current(); ++it){
    (*it)->writeProjectSpaceGlobalConfig(doc);
  }
}
void KDevelopCore::writeProjectSpaceUserConfig(QDomDocument& doc){
  QListIterator<KDevComponent> it(m_components);
  for (; it.current(); ++it){
    (*it)->writeProjectSpaceUserConfig(doc);
  }
}
void KDevelopCore::readProjectSpaceGlobalConfig(QDomDocument& doc){
  QListIterator<KDevComponent> it(m_components);
  for (; it.current(); ++it){
    (*it)->readProjectSpaceGlobalConfig(doc);
  }
}
void KDevelopCore::readProjectSpaceUserConfig(QDomDocument& doc){
  QListIterator<KDevComponent> it(m_components);
  for (; it.current(); ++it){
    (*it)->readProjectSpaceGlobalConfig(doc);
  }
}

#include "kdevelopcore.moc"
