#include <kdebug.h>
#include <ktrader.h>
#include <klibloader.h>
#include "ClassParser.h"
#include "kdevelop.h"
#include "kdevelopcore.h"


KDevelopCore::KDevelopCore(KDevelop *gui)
    : QObject(gui, "kdevelop core")
{
    loadComponents(gui);
    initComponents(gui);

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
}


KDevelopCore::~KDevelopCore()
{}


void KDevelopCore::loadComponents(KDevelop *gui)
{
    KTrader::OfferList offers = KTrader::self()->query("KDevelop/Component");
    if (offers.isEmpty())
        kdDebug(9000) << "No KDevelop components" << endl;

    KTrader::OfferList::ConstIterator it;
    for (it = offers.begin(); it != offers.end(); ++it) {
        
        kdDebug(9000) << "Found Component " << (*it)->name().latin1() << endl;
        KLibFactory *factory = KLibLoader::self()->factory((*it)->library());

        QStringList args;
        QVariant prop = (*it)->property("X-KDevelop-Args");
        if (prop.isValid())
            args = QStringList::split(" ", prop.toString());
        
        QObject *obj = factory->create(gui, (*it)->name().latin1(),
                                       "KDevComponent", args);
        
        if (!obj->inherits("KDevComponent")) {
            kdDebug(9000) << "Component does not inherit KDevComponent" << endl;
            return;
        }
        KDevComponent *comp = (KDevComponent*) obj;
        gui->guiFactory()->addClient(comp);
        m_components.append(comp);
    }
}


void KDevelopCore::initComponents(KDevelop *gui)
{
    // Connect all signals
    QListIterator<KDevComponent> it(m_components);
    for (; it.current(); ++it) {
        connect( *it, SIGNAL(gotoSourceFile(const QString&, int)),
                 this, SLOT(gotoSourceFile(const QString&, int)) );
        connect( *it, SIGNAL(gotoDocumentationFile(const QString&)),
                 this, SLOT(gotoDocumentationFile(const QString&)) );
        connect( *it, SIGNAL(gotoProjectApiDoc()),
                 this, SLOT(gotoProjectApiDoc()) );
        connect( *it, SIGNAL(gotoProjectManual()),
                 this, SLOT(gotoProjectManual()) );
        connect( *it, SIGNAL(embedWidget(QWidget*, KDevComponent::Role, const QString&, const QString&)),
                 gui, SLOT(embedWidget(QWidget *, KDevComponent::Role, const QString&, const QString&)) );
    }

    // Call the setup routines
    QListIterator<KDevComponent> it2(m_components);
    for (; it2.current(); ++it2) {
        (*it2)->setupGUI();
    }
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
