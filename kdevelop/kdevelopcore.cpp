#include <kdebug.h>
#include <ktrader.h>
#include <klibloader.h>
#include <klocale.h>
#include <kstdaction.h>
#include <kdialogbase.h>
#include <kmessagebox.h>

#include "ClassParser.h"
#include "kdevelop.h"
#include "kdevcomponent.h"
#include "kdevversioncontrol.h"
#include "kdevelopcore.h"


KDevelopCore::KDevelopCore(KDevelop *gui)
    : QObject(gui, "kdevelop core")
{
    m_kdevelopgui = gui;
    m_versioncontrol = 0;
    m_project = 0;

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
    connect( component, SIGNAL(addFileToRepository(const QString&)),
             this, SLOT(addFileToRepository(const QString&)) );
    connect( component, SIGNAL(removeFileFromRepository(const QString&)),
             this, SLOT(removeFileFromRepository(const QString&)) );
    connect( component, SIGNAL(commitFileToRepository(const QString&)),
             this, SLOT(commitFileToRepository(const QString&)) );
    connect( component, SIGNAL(updateFileFromRepository(const QString&)),
             this, SLOT(updateFileFromRepository(const QString&)) );
    connect( component, SIGNAL(executeCommand(const QString&)),
             this, SLOT(executeCommand(const QString&)) );
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
    action->setShortText( i18n("Prints the current document") );
    action->setWhatsThis( i18n("Print\n\n"
                               "Opens the printing dialog. There, you can "
                               "configure which printing program you wish "
                               "to use, and print your project files.") );
    
    action = new KAction( i18n("&KDevelop Setup..."), 0, this, SLOT( slotOptionsKDevelopSetup() ),
                          m_kdevelopgui->actionCollection(), "options_kdevelop_setup");
    action->setShortText( i18n("Configures KDevelop") );
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
        m_kdevelopgui->guiFactory()->addClient(comp);
        initComponent(comp);
    }
}


void KDevelopCore::loadVersionControl(const QString &system)
{
    QString constraint = "[X-KDevelop-VersionControlSystem] == " + system;
    KTrader::OfferList offers = KTrader::self()->query("KDevelop/VersionControl", constraint);
    if (offers.isEmpty()) {
        KMessageBox::sorry(m_kdevelopgui,
                           i18n("No version control component for %1 found").arg(system));
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
    comp->projectOpened(m_project);
}


void KDevelopCore::unloadVersionControl()
{
    m_components.remove(m_versioncontrol);
    delete m_versioncontrol;
    m_versioncontrol = 0;
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


void KDevelopCore::addFileToRepository(const QString &filename)
{
}


void KDevelopCore::removeFileFromRepository(const QString &filename)
{
}


void KDevelopCore::commitFileToRepository(const QString &filename)
{
}


void KDevelopCore::updateFileFromRepository(const QString &filename)
{
}


void KDevelopCore::executeCommand(const QString &command)
{
    kdDebug(9000) << "KDevelopCore::executeCommand " << command << endl;
}


void KDevelopCore::gotoSourceFile(const QString &filename, int lineno)
{
    kdDebug(9000) << "KDevelopCore::gotoSourceFile" << endl;
}


void KDevelopCore::gotoDocumentationFile(const QString &filename)
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
