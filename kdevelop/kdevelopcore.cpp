/***************************************************************************
 *   Copyright (C) 2000 by The KDevelop Team                               *
 *   kdevelop-team@fara.cs.uni-potsdam.de                                  *
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

#include "ClassStore.h"
#include "cproject.h"
#include "projectoptionsdlg.h"
#include "kdevelop.h"
#include "kdevcomponent.h"
#include "kdevversioncontrol.h"
#include "kdevlanguagesupport.h"
#include "kdevelopcore.h"
#include "projectspace.h"


KDevelopCore::KDevelopCore(KDevelop *gui)
    : QObject(gui, "kdevelop core")
{
    m_kdevelopgui = gui;
    m_versioncontrol = 0;
    m_languagesupport = 0;
    m_makefrontend = 0;
    m_appfrontend = 0;
    m_project = 0;
    m_classstore = new CClassStore();

    initActions();
}


KDevelopCore::~KDevelopCore()
{}


void KDevelopCore::initComponent(KDevComponent *component)
{
    connect( component, SIGNAL(executeMakeCommand(const QString&)),
             this, SLOT(executeMakeCommand(const QString&)) );
    connect( component, SIGNAL(executeAppCommand(const QString&)),
             this, SLOT(executeAppCommand(const QString&)) );
    connect( component, SIGNAL(running(bool)),
             this, SLOT(running(bool)) );
    connect( component, SIGNAL(gotoSourceFile(const QString&, int)),
             this, SLOT(gotoSourceFile(const QString&, int)) );
    connect( component, SIGNAL(gotoDocumentationFile(const QString&)),
             this, SLOT(gotoDocumentationFile(const QString&)) );
    connect( component, SIGNAL(gotoProjectApiDoc()),
             this, SLOT(gotoProjectApiDoc()) );
    connect( component, SIGNAL(gotoProjectManual()),
             this, SLOT(gotoProjectManual()) );
    connect( component, SIGNAL(embedWidget(QWidget*, KDevComponent::Role, const QString&, const QString&)),
             m_kdevelopgui, SLOT(embedWidget(QWidget *, KDevComponent::Role, const QString&, const QString&)) );

    component->setupGUI();
    m_components.append(component);
}


void KDevelopCore::initActions()
{
    KAction *action;
    
    action = KStdAction::print( this, SLOT( slotFilePrint() ),
                                m_kdevelopgui->actionCollection(), "file_print");
    action->setStatusText( i18n("Prints the current document") );
    action->setWhatsThis( i18n("Print\n\n"
                               "Opens the printing dialog. There, you can "
                               "configure which printing program you wish "
                               "to use, and print your project files.") );
    
    action = new KAction( i18n("&Open..."), "openprj", 0, this, SLOT( slotProjectOpen() ),
                          m_kdevelopgui->actionCollection(), "project_open");
    action->setStatusText( i18n("Opens an existing project") );
    action->setWhatsThis(  i18n("Open project\n\n"
                                "Shows the open project dialog "
                                "to select a project to be opened") );

    action = new KRecentFilesAction( i18n("Open &recent project..."), 0, this, SLOT( slotProjectOpenRecent(const KURL&) ),
                                     m_kdevelopgui->actionCollection(), "project_open_recent");

    action = new KAction( i18n("C&lose"), 0, this, SLOT( slotProjectClose() ),
                          m_kdevelopgui->actionCollection(), "project_close");
    action->setEnabled(false);
    action->setStatusText( i18n("Closes the current project") );
    
    action = new KAction( i18n("&Add existing File(s)..."), 0, this, SLOT( slotProjectAddExistingFiles() ),
                          m_kdevelopgui->actionCollection(), "project_add_existing_files");
    action->setEnabled(false);
    action->setStatusText( i18n("Adds existing file(s) to the project") );
    
    action = new KAction( i18n("Add new &Translation File..."), "locale", 0, this, SLOT( slotProjectAddNewTranslationFile() ),
                          m_kdevelopgui->actionCollection(), "project_add_translation");
    action->setEnabled(false);
    action->setStatusText( i18n("Adds a new language for internationalization to the project") );

    action = new KAction( i18n("&Options..."), 0, this, SLOT( slotProjectOptions() ),
                          m_kdevelopgui->actionCollection(), "project_options");
    action->setStatusText( i18n("Sets project and compiler options") );

    action = new KAction( i18n("&KDevelop Setup..."), 0, this, SLOT( slotOptionsKDevelopSetup() ),
                          m_kdevelopgui->actionCollection(), "options_kdevelop_setup");
    action->setStatusText( i18n("Configures KDevelop") );

    action = new KAction( i18n("&Stop"), "stop_proc", 0, this, SLOT( slotStop() ),
                          m_kdevelopgui->actionCollection(), "stop_everything");
    action->setEnabled(false);
}


void KDevelopCore::loadInitialComponents()
{
    KTrader::OfferList offers = KTrader::self()->query("KDevelop/Component");
    if (offers.isEmpty())
        kdDebug(9000) << "No KDevelop components" << endl;

    KTrader::OfferList::ConstIterator it;
    for (it = offers.begin(); it != offers.end(); ++it) {
        
        kdDebug(9000) << "Found Component " << (*it)->name() << endl;
        KLibFactory *factory = KLibLoader::self()->factory((*it)->library());

        QStringList args;
        QVariant prop = (*it)->property("X-KDevelop-Args");
        if (prop.isValid())
            args = QStringList::split(" ", prop.toString());
        
        QObject *obj = factory->create(m_kdevelopgui, (*it)->name().latin1(),
                                       "KDevComponent", args);
        
        if (!obj->inherits("KDevComponent")) {
            kdDebug(9000) << "Component does not inherit KDevComponent" << endl;
            return;
        }
        KDevComponent *comp = (KDevComponent*) obj;

        if (!m_makefrontend && (*it)->hasServiceType("KDevelop/MakeFrontend")) {
            m_makefrontend = comp;
            kdDebug(9000) << "is make frontend" << endl;
        }
            
        if (!m_appfrontend && (*it)->hasServiceType("KDevelop/AppFrontend")) {
            m_appfrontend = comp;
            kdDebug(9000) << "is app frontend" << endl;
        }

        initComponent(comp);
        m_kdevelopgui->guiFactory()->addClient(comp);
    }
}


void KDevelopCore::loadVersionControl(const QString &name)
{
    KService::Ptr service = KService::serviceByName(name);
    if (!service) {
        KMessageBox::sorry(m_kdevelopgui,
                           i18n("No version control component %1 found").arg(name));
        return;
    }

    kdDebug(9000) << "Loading VersionControl Component " << service->name() << endl;

    KLibFactory *factory = KLibLoader::self()->factory(service->library());

    QStringList args;
    QVariant prop = service->property("X-KDevelop-Args");
    if (prop.isValid())
        args = QStringList::split(" ", prop.toString());
    
    QObject *obj = factory->create(m_kdevelopgui, service->name().latin1(),
                                   "KDevVersionControl", args);
        
    if (!obj->inherits("KDevVersionControl")) {
        kdDebug(9000) << "Component does not inherit KDevVersionControl" << endl;
        return;
    }
    KDevVersionControl *comp = (KDevVersionControl*) obj;
    m_versioncontrol = comp;
    initComponent(comp);
}


void KDevelopCore::unloadVersionControl()
{
    m_components.remove(m_versioncontrol);
    delete m_versioncontrol;
    m_versioncontrol = 0;
}


void KDevelopCore::loadLanguageSupport(const QString &lang)
{
    QString constraint = QString("[X-KDevelop-Language] == '%1'").arg(lang);
    KTrader::OfferList offers = KTrader::self()->query("KDevelop/LanguageSupport", constraint);
    if (offers.isEmpty()) {
        KMessageBox::sorry(m_kdevelopgui,
                           i18n("No language support component for %1 found").arg(lang));
        return;
    }

    KService *service = *offers.begin();
    kdDebug(9000) << "Found LanguageSupport Component " << service->name() << endl;

    KLibFactory *factory = KLibLoader::self()->factory(service->library());

    QStringList args;
    QVariant prop = service->property("X-KDevelop-Args");
    if (prop.isValid())
        args = QStringList::split(" ", prop.toString());
    
    QObject *obj = factory->create(m_kdevelopgui, service->name().latin1(),
                                   "KDevLanguageSupport", args);
        
    if (!obj->inherits("KDevLanguageSupport")) {
        kdDebug(9000) << "Component does not inherit KDevLanguageSupport" << endl;
        return;
    }
    KDevLanguageSupport *comp = (KDevLanguageSupport*) obj;
    m_languagesupport = comp;
    initComponent(comp);
}


void KDevelopCore::unloadLanguageSupport()
{
  m_components.remove(m_languagesupport);
  delete m_languagesupport;
  m_languagesupport = 0;
}

void KDevelopCore::loadProjectSpace(const QString &name){
  QString constraint = QString("[X-KDevelop-ProjectSpace] == '%1'").arg(name);
  KTrader::OfferList offers = KTrader::self()->query("KDevelop/ProjectSpace", constraint);
  if (offers.isEmpty()) {
    KMessageBox::sorry(m_kdevelopgui,
		       i18n("No ProjectSpace component for %1 found").arg(name));
    return;
  }
  
  KService *service = *offers.begin();
  kdDebug(9000) << "Found ProjectSpace Component " << service->name() << endl;
  
  KLibFactory *factory = KLibLoader::self()->factory(service->library());

  QStringList args;
  QVariant prop = service->property("X-KDevelop-Args");
  if (prop.isValid())
    args = QStringList::split(" ", prop.toString());
    
  QObject *obj = factory->create(m_kdevelopgui, service->name().latin1(),
				 "ProjectSpace", args);
        
  if (!obj->inherits("ProjectSpace")) {
    kdDebug(9000) << "Component does not inherit ProjectSpace" << endl;
    return;
  }
  ProjectSpace *comp = (ProjectSpace*) obj;
  m_projectspace = comp;
  initComponent(comp);
}

void KDevelopCore::unloadProjectSpace(){
  m_components.remove(m_projectspace);
  delete m_projectspace;
  m_projectspace = 0;
}

void KDevelopCore::loadProject(const QString &fileName)
{
    m_project = new CProject("../kdevelop.kdevprj");
    // project must define a version control system
    // hack until implemented
    QString vcservice = QString::fromLatin1("CVSInterface");
    QString lang = QString::fromLatin1("C++");
    // name will be stored in the projectspace file
    QString projectspace = QString::fromLatin1("KDE");
    
    loadProjectSpace(projectspace);
    loadVersionControl(vcservice);
    loadLanguageSupport(lang);

    QListIterator<KDevComponent> it1(m_components);
    for (; it1.current(); ++it1)
        (*it1)->projectOpened(m_project);

    QListIterator<KDevComponent> it2(m_components);
    for (; it2.current(); ++it2)
        (*it2)->classStoreOpened(m_classstore);

    if (m_versioncontrol) {
        QListIterator<KDevComponent> it3(m_components);
        for (; it3.current(); ++it3)
            (*it3)->versionControlOpened(m_versioncontrol);
    }

    if (m_languagesupport) {
        QListIterator<KDevComponent> it4(m_components);
        for (; it4.current(); ++it4)
            (*it4)->languageSupportOpened(m_languagesupport);
    }

    KActionCollection *ac = m_kdevelopgui->actionCollection();
    ac->action("project_close")->setEnabled(true);
    ac->action("project_add_existing_files")->setEnabled(true);
    ac->action("project_add_translation")->setEnabled(true);
    ac->action("project_file_properties")->setEnabled(true);
    ac->action("project_options")->setEnabled(true);

    ((KRecentFilesAction*)ac->action("project_open_recent"))->addURL(KURL(fileName));
    
#if 1
    // Hack to test the class viewer
    QListIterator<KDevComponent> it5(m_components);
    for (; it5.current(); ++it5)
        (*it5)->savedFile("parts/doctreeview/doctreewidget.h");
#endif
}


void KDevelopCore::unloadProject()
{
    if (m_languagesupport) {
        QListIterator<KDevComponent> it1(m_components);
        for (; it1.current(); ++it1)
            (*it1)->languageSupportClosed();
        unloadLanguageSupport();
    }

    if (m_versioncontrol) {
        QListIterator<KDevComponent> it2(m_components);
        for (; it2.current(); ++it2)
            (*it2)->versionControlClosed();
        unloadVersionControl();
    }

    QListIterator<KDevComponent> it3(m_components);
    for (; it3.current(); ++it3)
        (*it3)->classStoreClosed();
    m_classstore->wipeout();

    QListIterator<KDevComponent> it4(m_components);
    for (; it4.current(); ++it4)
        (*it4)->projectClosed();

    delete m_project;
    m_project = 0;
    
    KActionCollection *ac = m_kdevelopgui->actionCollection();
    ac->action("project_close")->setEnabled(false);
    ac->action("project_add_existing_files")->setEnabled(false);
    ac->action("project_add_translation")->setEnabled(false);
    ac->action("project_file_properties")->setEnabled(false);
    ac->action("project_options")->setEnabled(false);
}


void KDevelopCore::slotFilePrint()
{
    // this hardcoded file name is hack ...
    // will be replaced by a trader-based solution later
    KLibFactory *factory = KLibLoader::self()->factory("libkdevprintplugin");
    if (!factory)
        return;

    QStringList args;
    args << "/vmlinuz"; // temporary ;-)
    QObject *obj = factory->create(m_kdevelopgui, "print dialog", "KDevPrintDialog", args);
    if (!obj->inherits("QDialog")) {
        kdDebug(9000) << "Print plugin doesn't provide a dialog" << endl;
        return;
    }

    QDialog *dlg = (QDialog *)obj;
    dlg->exec();
    delete dlg;
}


void KDevelopCore::slotProjectOpen()
{
    QString fileName = KFileDialog::getOpenFileName(QString::null, "*.kdevprj",
                                                    m_kdevelopgui, i18n("Open project"));

    if (m_project)
        unloadProject();

    loadProject(fileName);
}


void KDevelopCore::slotProjectOpenRecent(const KURL &url)
{
    QString fileName = url.fileName();

    if (m_project)
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
    ProjectOptionsDialog *dlg = new ProjectOptionsDialog(m_kdevelopgui, "project options dialog");
    dlg->exec();
    delete dlg;
}


void KDevelopCore::slotStop()
{
    QListIterator<KDevComponent> it(m_components);
    for (; it.current(); ++it)
        (*it)->stopButtonClicked();
}


void KDevelopCore::slotOptionsKDevelopSetup()
{
    KDialogBase *dlg = new KDialogBase(KDialogBase::TreeList, i18n("Customize KDevelop"),
                                       KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, m_kdevelopgui,
                                       "customize dialog");
    
    QListIterator<KDevComponent> it(m_components);
    for (; it.current(); ++it) {
        (*it)->configWidgetRequested(dlg);
    }

    dlg->exec();
    delete dlg;
}


void KDevelopCore::executeMakeCommand(const QString &command)
{
    if (!m_makefrontend) {
        kdDebug(9000) << "No make frontend!" << command << endl;
        return;
    }

    m_makefrontend->commandRequested(command);
}


void KDevelopCore::executeAppCommand(const QString &command)
{
    if (!m_appfrontend) {
        kdDebug(9000) << "No app frontend!" << command << endl;
        return;
    }

    m_appfrontend->commandRequested(command);
}


void KDevelopCore::running(bool runs)
{
    const KDevComponent *comp = static_cast<const KDevComponent*>(sender());

    if (runs)
        m_runningcomponents.append(comp);
    else
        m_runningcomponents.remove(comp);

    kdDebug(9000) << "Running components" << endl;
    QListIterator<KDevComponent> it(m_runningcomponents);
    for (; it.current(); ++it)
        kdDebug(9000) << comp->name() << endl;

    KActionCollection *ac = m_kdevelopgui->actionCollection();
    ac->action("stop_everything")->setEnabled(!m_runningcomponents.isEmpty());
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


#include "kdevelopcore.moc"
