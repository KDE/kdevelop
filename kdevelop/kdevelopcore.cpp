#include <kdebug.h>
#include <ktrader.h>
#include <klibloader.h>
#include <klocale.h>
#include <kstdaction.h>
#include <kdialogbase.h>
#include <kmessagebox.h>

#include "ClassStore.h"
#include "kdevelop.h"
#include "kdevcomponent.h"
#include "kdevversioncontrol.h"
#include "kdevlanguagesupport.h"
#include "kdevelopcore.h"


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

#if 0
    // Hack to test the class viewer
    CClassParser *classparser = new CClassParser;
    kdDebug(9000) << "Parsing kdevelop.cpp" << endl;
    classparser->parse("kdevelop.cpp");
    kdDebug(9000) << "Parsing doctreewidget.cpp" << endl;
    classparser->parse("parts/doctreeview/doctreewidget.cpp");

    QListIterator<KDevComponent> it(m_components);
    for (; it.current(); ++it) {
        (*it)->classStoreOpened(&classparser->store);
    }
#endif
}


KDevelopCore::~KDevelopCore()
{}


void KDevelopCore::initComponent(KDevComponent *component)
{
    connect( component, SIGNAL(executeMakeCommand(const QString&)),
             this, SLOT(executeMakeCommand(const QString&)) );
    connect( component, SIGNAL(executeAppCommand(const QString&)),
             this, SLOT(executeAppCommand(const QString&)) );
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
    
    action = new KAction( i18n("&KDevelop Setup..."), 0, this, SLOT( slotOptionsKDevelopSetup() ),
                          m_kdevelopgui->actionCollection(), "options_kdevelop_setup");
    action->setStatusText( i18n("Configures KDevelop") );
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

        m_kdevelopgui->guiFactory()->addClient(comp);
        initComponent(comp);
    }
}


void KDevelopCore::loadVersionControl(const QString &vcsystem)
{
    QString constraint = "[X-KDevelop-VersionControlSystem] == " + vcsystem;
    KTrader::OfferList offers = KTrader::self()->query("KDevelop/VersionControl", constraint);
    if (offers.isEmpty()) {
        KMessageBox::sorry(m_kdevelopgui,
                           i18n("No version control component for %1 found").arg(vcsystem));
        return;
    }

    KService *service = *offers.begin();
    kdDebug(9000) << "Found VersionControl Component " << service->name() << endl;

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
    QString constraint = "[X-KDevelop-Language] == " + lang;
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


void KDevelopCore::loadProject()
{
    // project must define a version control system
    // hack until implemented
    QString vcsystem = QString::fromLatin1("CVS");
    QString lang = QString::fromLatin1("C++");
    
    QListIterator<KDevComponent> it1(m_components);
    for (; it1.current(); ++it1)
        (*it1)->projectOpened(m_project);

    QListIterator<KDevComponent> it2(m_components);
    for (; it2.current(); ++it2)
        (*it2)->classStoreOpened(m_classstore);

    loadVersionControl(vcsystem);
    if (m_versioncontrol) {
        QListIterator<KDevComponent> it3(m_components);
        for (; it3.current(); ++it3)
            (*it3)->versionControlOpened(m_versioncontrol);
    }

    loadLanguageSupport(lang);
    if (m_languagesupport) {
        QListIterator<KDevComponent> it4(m_components);
        for (; it4.current(); ++it4)
            (*it4)->languageSupportOpened(m_languagesupport);
    }
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
