/***************************************************************************
 *   Copyright 2007 Alexander Dymo <adymo@kdevelop.org>             *
 *   Copyright 2007 Kris Wong <kris.p.wong@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#include "core.h"
#include "core_p.h"

#include <QApplication>
#include <QtCore/QTimer>

#include <kdebug.h>
#include <kglobal.h>
#include <KLocalizedString>
#include <KComponentData>
#include <ksplashscreen.h>

#include <language/duchain/duchain.h>

#include "mainwindow.h"
#include "sessioncontroller.h"
#include "uicontroller.h"
#include "plugincontroller.h"
#include "projectcontroller.h"
#include "partcontroller.h"
#include "languagecontroller.h"
#include "documentcontroller.h"
#include "runcontroller.h"
#include "session.h"
#include "documentationcontroller.h"
#include "sourceformattercontroller.h"
#include "progresswidget/progressmanager.h"
#include "selectioncontroller.h"
#include "debugcontroller.h"
#include "kdevplatformversion.h"
#include "workingsetcontroller.h"
#include "testcontroller.h"

#include <KMessageBox>

#include <KTextEditor/Document>
#include <ktexteditor/movinginterface.h>

#include <csignal>

volatile std::sig_atomic_t handlingSignal = 0;

void shutdownGracefully(int sig)
{
    if ( !handlingSignal ) {
        handlingSignal = 1;
        qDebug() << "signal " << sig << " received, shutting down gracefully";
        QCoreApplication* app = QCoreApplication::instance();
        if (QApplication* guiApp = qobject_cast<QApplication*>(app)) {
            guiApp->closeAllWindows();
        }
        app->quit();
        return;
    }

    // re-raise signal with default handler and trigger program termination
    std::signal(sig, SIG_DFL);
    std::raise(sig);
}

void installSignalHandler()
{
#ifdef SIGHUP
    std::signal(SIGHUP, shutdownGracefully);
#endif
#ifdef SIGINT
    std::signal(SIGINT, shutdownGracefully);
#endif
#ifdef SIGTERM
    std::signal(SIGTERM, shutdownGracefully);
#endif
}

namespace KDevelop {

Core *Core::m_self = 0;
KAboutData aboutData()
{
    KAboutData aboutData( "kdevplatform", 
                          i18n("KDevelop Platform"), KDEVPLATFORM_VERSION_STR,
                          i18n("Development Platform for IDE-like Applications"),
                          KAboutLicense::LGPL_V2, i18n( "Copyright 2004-2014, The KDevelop developers" ),
                          QString(), "http://www.kdevelop.org" );

    aboutData.addAuthor( i18n("Andreas Pakulat"), i18n( "Architecture, VCS Support, Project Management Support, QMake Projectmanager" ), "apaku@gmx.de" );
    aboutData.addAuthor( i18n("Alexander Dymo"), i18n( "Architecture, Sublime UI, Ruby support" ), "adymo@kdevelop.org" );
    aboutData.addAuthor( i18n("David Nolden"), i18n( "Definition-Use Chain, C++ Support" ), "david.nolden.kdevelop@art-master.de" );
    aboutData.addAuthor( i18n("Aleix Pol Gonzalez"), i18n( "Co-Maintainer, CMake Support, Run Support, Kross Support" ), "aleixpol@kde.org" );
    aboutData.addAuthor( i18n("Vladimir Prus"), i18n( "GDB integration" ), "ghost@cs.msu.su" );
    aboutData.addAuthor( i18n("Hamish Rodda"), i18n( "Text editor integration, definition-use chain" ), "rodda@kde.org" );
    
    aboutData.addCredit( i18n("Matt Rogers"), QString(), "mattr@kde.org");
    aboutData.addCredit( i18n("Cédric Pasteur"), i18n("astyle and indent support"), "cedric.pasteur@free.fr" );
    aboutData.addCredit( i18n("Evgeniy Ivanov"), i18n("Distributed VCS, Git, Mercurial"), "powerfox@kde.ru" );
    //Veritas is outside in playground currently.
    //aboutData.addCredit( i18n("Manuel Breugelmanns"), i18n( "Veritas, QTest integraton"), "mbr.nxi@gmail.com" );
    aboutData.addCredit( i18n("Robert Gruber") , i18n( "SnippetPart, debugger and usability patches" ), "rgruber@users.sourceforge.net" );
    aboutData.addCredit( i18n("Dukju Ahn"), i18n( "Subversion plugin, Custom Make Manager, Overall improvements" ), "dukjuahn@gmail.com" );
    aboutData.addAuthor( i18n("Niko Sams"), i18n( "GDB integration, Webdevelopment Plugins" ), "niko.sams@gmail.com" );
    aboutData.addAuthor( i18n("Milian Wolff"), i18n( "Co-Maintainer, Generic manager, Webdevelopment Plugins, Snippets, Performance" ), "mail@milianw.de" );

    return aboutData;
}

CorePrivate::CorePrivate(Core *core):
    m_componentData( aboutData().componentName().toLatin1() ), m_core(core), m_cleanedUp(false), m_shuttingDown(false)
{
}

bool CorePrivate::initialize(Core::Setup mode, QString session )
{
    m_mode=mode;
    emit m_core->startupProgress(0);
    if( !sessionController )
    {
        sessionController = new SessionController(m_core);
    }
    if( !workingSetController && !(mode & Core::NoUi) )
    {
        workingSetController = new WorkingSetController();
    }
    kDebug() << "Creating ui controller";
    if( !uiController )
    {
        uiController = new UiController(m_core);
    }
    emit m_core->startupProgress(10);
    kDebug() << "Creating plugin controller";

    if( !pluginController )
    {
        pluginController = new PluginController(m_core);
    }
    if( !partController && !(mode & Core::NoUi))
    {
        partController = new PartController(m_core, uiController.data()->defaultMainWindow());
    }
    emit m_core->startupProgress(20);

    if( !projectController )
    {
        projectController = new ProjectController(m_core);
    }

    if( !documentController )
    {
        documentController = new DocumentController(m_core);
    }

    if( !languageController )
    {
        // Must be initialized after documentController, because the background parser depends
        // on the document controller.
        languageController = new LanguageController(m_core);
    }
    emit m_core->startupProgress(25);

    if( !runController )
    {
        runController = new RunController(m_core);
    }

    if( !sourceFormatterController )
    {
        sourceFormatterController = new SourceFormatterController(m_core);
    }
    emit m_core->startupProgress(30);

    if ( !progressController) 
    {
        progressController = ProgressManager::instance();
    }

    if( !selectionController )
    {
        selectionController = new SelectionController(m_core);
    }
    emit m_core->startupProgress(35);

    if( !documentationController && !(mode & Core::NoUi) )
    {
        documentationController = new DocumentationController(m_core);
    }

    if( !debugController )
    {
        debugController = new DebugController(m_core);
    }
    emit m_core->startupProgress(40);

    if( !testController )
    {
        testController = new TestController(m_core);
    }
    emit m_core->startupProgress(47);

    kDebug() << "initializing ui controller";

    sessionController.data()->initialize( session );
    if( !sessionController.data()->activeSessionLock() ) {
        return false;
    }

    emit m_core->startupProgress(55);

    // TODO: Is this early enough, or should we put the loading of the session into
    // the controller construct
    DUChain::initialize();

    if(!(mode & Core::NoUi)) uiController.data()->initialize();
    languageController.data()->initialize();
    projectController.data()->initialize();
    documentController.data()->initialize();

    emit m_core->startupProgress(63);

    /* This is somewhat messy.  We want to load the areas before
        loading the plugins, so that when each plugin is loaded we
        know if an area wants some of the tool view from that plugin.
        OTOH, loading of areas creates documents, and some documents
        might require that a plugin is already loaded.
        Probably, the best approach would be to plugins to just add
        tool views to a list of available tool view, and then grab
        those tool views when loading an area.  */

    kDebug() << "loading session plugins";
    pluginController.data()->initialize();
    emit m_core->startupProgress(78);

    if(!(mode & Core::NoUi))
    {
        workingSetController.data()->initialize();
        /* Need to do this after everything else is loaded.  It's too
            hard to restore position of views, and toolbars, and whatever
            that are not created yet.  */
        uiController.data()->loadAllAreas(KSharedConfig::openConfig());
        uiController.data()->defaultMainWindow()->show();
    }
    emit m_core->startupProgress(90);
    runController.data()->initialize();
    sourceFormatterController.data()->initialize();
    selectionController.data()->initialize();
    if (documentationController) {
        documentationController.data()->initialize();
    }
    emit m_core->startupProgress(95);
    debugController.data()->initialize();
    testController.data()->initialize();

    installSignalHandler();

    if (partController) {
        // check features of kate and report to user if it does not fit
        KTextEditor::Document* doc = partController.data()->createTextPart();

        if ( !qobject_cast< KTextEditor::MovingInterface* >(doc) ) {
            KMessageBox::error(QApplication::activeWindow(),
                                i18n("The installed Kate version does not support the MovingInterface which is crucial for "
                                    "KDevelop starting from version 4.2.\n\n"
                                    "To use KDevelop with KDE SC prior to 4.6, where the SmartInterface is used instead "
                                    "of the MovingInterface, you need KDevelop 4.1 or lower."));
            delete doc;
            return false;
        }

        delete doc;
    }

    emit m_core->startupProgress(100);
    return true;
}
CorePrivate::~CorePrivate()
{
    delete selectionController.data();
    delete projectController.data();
    delete languageController.data();
    delete pluginController.data();
    delete uiController.data();
    delete partController.data();
    delete documentController.data();
    delete runController.data();
    delete sessionController.data();
    delete sourceFormatterController.data();
    delete documentationController.data();
    delete debugController.data();
    delete workingSetController.data();
    delete testController.data();

    selectionController.clear();
    projectController.clear();
    languageController.clear();
    pluginController.clear();
    uiController.clear();
    partController.clear();
    documentController.clear();
    runController.clear();
    sessionController.clear();
    sourceFormatterController.clear();
    documentationController.clear();
    debugController.clear();
    workingSetController.clear();
    testController.clear();
}

bool Core::initialize(KSplashScreen* splash, Setup mode, const QString& session )
{
    if (m_self)
        return true;

    m_self = new Core();
    if (splash) {
        connect(m_self, SIGNAL(startupProgress(int)), splash, SLOT(progress(int)));
    }
    bool ret = m_self->d->initialize(mode, session);
    if( splash ) {
        QTimer::singleShot( 200, splash, SLOT(deleteLater()) );
    }
    
    if(ret)
        emit m_self->initializationCompleted();
    
    return ret;
}

Core *KDevelop::Core::self()
{
    return m_self;
}

Core::Core(QObject *parent)
    : ICore(parent)
{
    d = new CorePrivate(this);

    connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), this, SLOT(shutdown()));
}

Core::Core(CorePrivate* dd, QObject* parent)
: ICore(parent), d(dd)
{
    connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), this, SLOT(shutdown()));
}

Core::~Core()
{
    kDebug() ;
    //Cleanup already called before mass destruction of GUI
    delete d;
    m_self = 0;
}

Core::Setup Core::setupFlags() const
{
    return d->m_mode;
}

void Core::shutdown()
{
    if (!d->m_shuttingDown) {
        cleanup();
        deleteLater();
    }
}

bool Core::shuttingDown() const
{
    return d->m_shuttingDown;
}

void Core::cleanup()
{
    d->m_shuttingDown = true;
    emit aboutToShutdown();

    if (!d->m_cleanedUp) {
        d->debugController.data()->cleanup();
        d->selectionController.data()->cleanup();
        // Save the layout of the ui here, so run it first
        d->uiController.data()->cleanup();

        if (d->workingSetController)
            d->workingSetController.data()->cleanup();

        /* Must be called before projectController.data()->cleanup(). */
        // Closes all documents (discards, as already saved if the user wished earlier)
        d->documentController.data()->cleanup();
        d->runController.data()->cleanup();

        d->projectController.data()->cleanup();
        d->sourceFormatterController.data()->cleanup();
        d->pluginController.data()->cleanup();
        d->sessionController.data()->cleanup();

        d->testController.data()->cleanup();

        //Disable the functionality of the language controller
        d->languageController.data()->cleanup();

        DUChain::self()->shutdown();
    }

    d->m_cleanedUp = true;
    emit shutdownCompleted();
}

KComponentData Core::componentData() const
{
    return d->m_componentData;
}

IUiController *Core::uiController()
{
    return d->uiController.data();
}

ISession* Core::activeSession()
{
    return sessionController()->activeSession();
}

ISessionLock::Ptr Core::activeSessionLock()
{
    return sessionController()->activeSessionLock();
}

SessionController *Core::sessionController()
{
    return d->sessionController.data();
}

UiController *Core::uiControllerInternal()
{
    return d->uiController.data();
}

IPluginController *Core::pluginController()
{
    return d->pluginController.data();
}

PluginController *Core::pluginControllerInternal()
{
    return d->pluginController.data();
}

IProjectController *Core::projectController()
{
    return d->projectController.data();
}

ProjectController *Core::projectControllerInternal()
{
    return d->projectController.data();
}

IPartController *Core::partController()
{
    return d->partController.data();
}

PartController *Core::partControllerInternal()
{
    return d->partController.data();
}

ILanguageController *Core::languageController()
{
    return d->languageController.data();
}

LanguageController *Core::languageControllerInternal()
{
    return d->languageController.data();
}

IDocumentController *Core::documentController()
{
    return d->documentController.data();
}

DocumentController *Core::documentControllerInternal()
{
    return d->documentController.data();
}

IRunController *Core::runController()
{
    return d->runController.data();
}

RunController *Core::runControllerInternal()
{
    return d->runController.data();
}

ISourceFormatterController* Core::sourceFormatterController()
{
    return d->sourceFormatterController.data();
}

SourceFormatterController* Core::sourceFormatterControllerInternal()
{
    return d->sourceFormatterController.data();
}


ProgressManager *Core::progressController()
{
    return d->progressController.data();
}

ISelectionController* Core::selectionController()
{
    return d->selectionController.data();
}

IDocumentationController* Core::documentationController()
{
    return d->documentationController.data();
}

DocumentationController* Core::documentationControllerInternal()
{
    return d->documentationController.data();
}

IDebugController* Core::debugController()
{
    return d->debugController.data();
}

DebugController* Core::debugControllerInternal()
{
    return d->debugController.data();
}

ITestController* Core::testController()
{
    return d->testController.data();
}

TestController* Core::testControllerInternal()
{
    return d->testController.data();
}

WorkingSetController* Core::workingSetControllerInternal()
{
    return d->workingSetController.data();
}

QString Core::version()
{
    return KDEVPLATFORM_VERSION_STR;
}

}

