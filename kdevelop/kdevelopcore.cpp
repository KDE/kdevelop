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
#include <klocale.h>
#include <kstdaction.h>
#include <kdialogbase.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <qdom.h>
#include <qobjectlist.h>

#include "classstore.h"
#include "projectoptionsdlg.h"
#include "kdevelop.h"
#include "kdevcomponent.h"
#include "kdevversioncontrol.h"
#include "kdevlanguagesupport.h"
#include "kdevmakefrontend.h"
#include "kdevappfrontend.h"
#include "kdeveditormanager.h"
#include "kdevelopcore.h"
#include "kdevviewhandler.h"
#include "projectspace.h"
#include "newprojectdlg.h"
#include "partloader.h"


KDevelopCore::KDevelopCore(KDevelop *pGUI)
    : QObject(pGUI, "kdevelop core")
   ,m_pKDevelopGUI(pGUI)
   ,m_pVersionControl(0L)
   ,m_pLanguageSupport(0L)
   ,m_pEditorManager(0)
   ,m_pMakeFrontend(0L)
   ,m_pAppFrontend(0L)
   ,m_pProjectSpace(0L)
   ,m_pViewHandler(0L)
{
    m_pClassStore = new ClassStore();
    initActions();
}


KDevelopCore::~KDevelopCore()
{}


void KDevelopCore::initComponent(KDevComponent *pComponent)
{
    connect( pComponent, SIGNAL(running(bool)),
             this, SLOT(running(bool)) );
    connect( pComponent,SIGNAL(needKDevNodeActions(KDevNode*,QList<KAction>*)),
	     this,SLOT(needKDevNodeActions(KDevNode*,QList<KAction>*)));
    connect( pComponent, SIGNAL(embedWidget(QWidget*, KDevComponent::Role, const QString&, const QString&)),
             m_pKDevelopGUI, SLOT(embedWidget(QWidget *, KDevComponent::Role, const QString&, const QString&)) );

    pComponent->setupInternal(m_pProjectSpace, m_pClassStore,
                              m_pVersionControl, m_pLanguageSupport,
                              m_pEditorManager, m_pMakeFrontend,
                              m_pAppFrontend);
    pComponent->setupGUI();
    m_components.append(pComponent);
    m_pKDevelopGUI->guiFactory()->addClient(pComponent);
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

    pAction = new KRecentFilesAction( i18n("Open &recent project..."), 0, 
				      this, SLOT( slotProjectOpenRecent(const KURL&) ),
				      m_pKDevelopGUI->actionCollection(), "project_open_recent");

    pAction = new KAction( i18n("C&lose"), 0, this, SLOT( slotProjectClose() ),
                          m_pKDevelopGUI->actionCollection(), "project_close");
    pAction->setEnabled(false);
    pAction->setStatusText( i18n("Closes the current project") );

    
    
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

void KDevelopCore::saveProperties(KConfig* pConfig){
    KActionCollection *pAC = m_pKDevelopGUI->actionCollection();
    ((KRecentFilesAction*)pAC->action("project_open_recent"))->saveEntries(pConfig);
}

void KDevelopCore::readProperties(KConfig* pConfig){
    KActionCollection *pAC = m_pKDevelopGUI->actionCollection();
    ((KRecentFilesAction*)pAC->action("project_open_recent"))->loadEntries(pConfig);
}


void KDevelopCore::loadInitialComponents()
{
    // All generic components
    QObjectList list = PartLoader::loadAllByQuery(m_pKDevelopGUI,
                                                  "KDevelop/Component",
                                                  QString::null,
                                                  "KDevComponent");
    QObjectListIt it(list);
    for (; it.current(); ++it) 
        initComponent(static_cast<KDevComponent*>(it.current()));
}


bool KDevelopCore::openProjectSpace(const QString &fileName)
{
    // Project space component
    QString psService = ProjectSpace::projectSpacePluginName(fileName);
    QObject *psObj = PartLoader::loadByName(m_pKDevelopGUI, psService, "ProjectSpace");
    if (!psObj) {
        KMessageBox::sorry(m_pKDevelopGUI,
                           i18n("No valid project space component %1 found").arg(psService));
        return false;
    }
    m_pProjectSpace = static_cast<ProjectSpace*>(psObj);
    initComponent(m_pProjectSpace);
    m_pProjectSpace->readConfig(fileName);
    m_pProjectSpace->dump();

    // Language support component
    QString lang = m_pProjectSpace->programmingLanguage();
    QObject *lsObj = PartLoader::loadByQuery(m_pKDevelopGUI,
                                             QString::fromLatin1("KDevelop/LanguageSupport"),
                                             QString::fromLatin1("[X-KDevelop-Language] == '%1'").arg(lang),
                                             "KDevLanguageSupport");
    if (lsObj) {
        m_pLanguageSupport = static_cast<KDevLanguageSupport*>(lsObj);
        initComponent(m_pLanguageSupport);
    } else {
        KMessageBox::sorry(m_pKDevelopGUI,
                           i18n("No language support component for %1 found").arg(lang));
    }

    // Version control component (name should be retrieved from project space)
    QString vcService = QString::null;
    if (!vcService.isNull()) {
        QObject *vcObj = PartLoader::loadByName(m_pKDevelopGUI, vcService, "KDevVersionControl");
        if (vcObj) {
            m_pVersionControl = static_cast<KDevVersionControl*>(vcObj);
            initComponent(m_pVersionControl);
        } else {
            KMessageBox::sorry(m_pKDevelopGUI,
                               i18n("No valid version control component %1 found").arg(vcService));
        }
    }

    // Editor manager component
    QObject *emObj = PartLoader::loadByQuery(m_pKDevelopGUI,
                                             QString::fromLatin1("KDevelop/EditorManager"),
                                             QString::null,
                                             "KDevEditorManager");
    if (emObj) {
        m_pEditorManager = static_cast<KDevEditorManager*>(emObj);
        initComponent(m_pEditorManager);
    } else {
        KMessageBox::sorry(m_pKDevelopGUI,
                           i18n("No valid editor manager component found"));
    }

    // Make frontend component
    QObject *mfObj = PartLoader::loadByQuery(m_pKDevelopGUI,
                                             QString::fromLatin1("KDevelop/MakeFrontend"),
                                             QString::null,
                                             "KDevMakeFrontend");
    if (mfObj) {
        m_pMakeFrontend = static_cast<KDevMakeFrontend*>(mfObj);
        initComponent(m_pMakeFrontend);
    } else {
        KMessageBox::sorry(m_pKDevelopGUI,
                           i18n("No valid make frontend component found"));
    }

    // Application frontend component
    QObject *afObj = PartLoader::loadByQuery(m_pKDevelopGUI,
                                             QString::fromLatin1("KDevelop/AppFrontend"),
                                             QString::null,
                                             "KDevAppFrontend");
    if (afObj) {
        m_pAppFrontend = static_cast<KDevAppFrontend*>(afObj);
        initComponent(m_pMakeFrontend);
    } else {
        KMessageBox::sorry(m_pKDevelopGUI,
                           i18n("No valid application frontend component found"));
    }

    // Temporary hack
    QListIterator<KDevComponent> it0(m_components);
    for (; it0.current(); ++it0)
       (*it0)->setupInternal(m_pProjectSpace, m_pClassStore,
                             m_pVersionControl, m_pLanguageSupport,
                             m_pEditorManager, m_pMakeFrontend,
                             m_pAppFrontend);
    // Notifications
    QListIterator<KDevComponent> it1(m_components);
    for (; it1.current(); ++it1)
        (*it1)->projectSpaceOpened();
    
    if (m_pVersionControl) {
        QListIterator<KDevComponent> it2(m_components);
        for (; it2.current(); ++it2)
            (*it2)->versionControlOpened();
    }
    
    if (m_pLanguageSupport) {
        QListIterator<KDevComponent> it3(m_components);
        for (; it3.current(); ++it3)
            (*it3)->languageSupportOpened();
    }

    if (m_pEditorManager) {
        QListIterator<KDevComponent> it4(m_components);
        for (; it4.current(); ++it4)
            (*it4)->editorManagerOpened();
    }

    // Restore window layout
    QDomElement docel = m_pProjectSpace->readUserDocument()->documentElement();
    QDomElement layoutel = docel.namedItem("Layout").toElement();
    if (!layoutel.isNull())
        m_pKDevelopGUI->readDockConfig(layoutel);

    // some actions
    KActionCollection *pAC = m_pKDevelopGUI->actionCollection();
    pAC->action("project_close")->setEnabled(true);
    KURL url;
    url.setPath(fileName);
    ((KRecentFilesAction*)pAC->action("project_open_recent"))->addURL(url);

    return true;
}


void KDevelopCore::closeProjectSpace()
{
    kdDebug(9000) << "KDevelopCore::closeProjectSpace" << endl;
    if (!m_pProjectSpace)
        return;
    
    // Save window layout
    QDomElement docel = m_pProjectSpace->readUserDocument()->documentElement();
    QDomElement layoutel = docel.namedItem("Layout").toElement();
    if (layoutel.isNull()) {
        layoutel = docel.ownerDocument().createElement("Layout");
        docel.appendChild(layoutel);
    }
    m_pKDevelopGUI->writeDockConfig(layoutel);

    // Editor manager component
    if (m_pEditorManager) {
        QListIterator<KDevComponent> it1(m_components);
        for (; it1.current(); ++it1)
            (*it1)->editorManagerOpened();
        m_components.remove(m_pEditorManager);
        delete m_pEditorManager;
        m_pEditorManager = 0;
    }

    // Language support component
    if (m_pLanguageSupport) {
        QListIterator<KDevComponent> it2(m_components);
        for (; it2.current(); ++it2)
            (*it2)->languageSupportOpened();
        m_components.remove(m_pLanguageSupport);
        delete m_pLanguageSupport;
        m_pLanguageSupport = 0;
    }

    // Version control component
    if (m_pVersionControl) {
        QListIterator<KDevComponent> it3(m_components);
        for (; it3.current(); ++it3)
            (*it3)->versionControlOpened();
        m_components.remove(m_pVersionControl);
        delete m_pVersionControl;
        m_pVersionControl = 0;
    }

    m_pClassStore->wipeout();

    // Project space component
    QListIterator<KDevComponent> it4(m_components);
    for (; it4.current(); ++it4)
        (*it4)->projectSpaceOpened();
    
    m_pProjectSpace->saveConfig();
    m_components.remove(m_pProjectSpace);
    delete m_pProjectSpace;
    m_pProjectSpace = 0L;
    
    KActionCollection *pAC = m_pKDevelopGUI->actionCollection();
    pAC->action("project_close")->setEnabled(false);
}


void KDevelopCore::newFile()
{
  //just a test!
  QWidget* pEV = new QWidget(0L);
  m_pKDevelopGUI->embedWidget( pEV, KDevComponent::DocumentView, "Document", 0L);
  pEV->show();
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


void KDevelopCore::slotProjectNew()
{
    // if m_pProjectSpace == 0, create a new one
    NewProjectDlg* pDlg = new NewProjectDlg(m_pProjectSpace);
    if (pDlg->exec()) {
        if (pDlg->newProjectSpaceCreated()) {
            if (m_pProjectSpace)
                closeProjectSpace();
            QString file = pDlg->projectSpaceFile();
            kdDebug(9000) << "FILE" << file << endl;
            openProjectSpace(file);
        }
        else { // new project
            kdDebug(9000) << "KDevelopCore::slotProjectNew: add new project" << endl;
        }
    }
    delete pDlg;
}


void KDevelopCore::slotProjectOpen()
{
    QString fileName = KFileDialog::getOpenFileName(QString::null, "*.kdevpsp",
                                                    m_pKDevelopGUI, i18n("Open project"));
    if (fileName.isNull())
      return;

    closeProjectSpace();
    openProjectSpace(fileName);
}


void KDevelopCore::slotProjectOpenRecent(const KURL &url)
{
    QString fileName = url.path(0);

    closeProjectSpace();
    openProjectSpace(fileName);
}


void KDevelopCore::slotProjectClose()
{
    // Ask for confirmation?
    closeProjectSpace();
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


KDevViewHandler* KDevelopCore::viewHandler()
{
   return m_pViewHandler;
}


void KDevelopCore::needKDevNodeActions(KDevNode* pNode, QList<KAction> *pList)
{
    pList->clear();
        
    QListIterator<KDevComponent> it(m_components);
    for (; it.current(); ++it){ // ask every component
        QList<KAction> pSingleList = (*it)->kdevNodeActions(pNode);
        QListIterator<KAction> it(pSingleList);
        for (; it.current(); ++it)
            pList->append(it.current());
    }
}


#include "kdevelopcore.moc"
