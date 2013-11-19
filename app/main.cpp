/***************************************************************************
 *   Copyright 2003-2009 Alexander Dymo <adymo@kdevelop.org>               *
 *   Copyright 2007 Ralf Habacker  <Ralf.Habacker@freenet.de>              *
 *   Copyright 2006-2007 Matt Rogers  <mattr@kde.org>                      *
 *   Copyright 2006-2007 Hamish Rodda <rodda@kde.org>                      *
 *   Copyright 2005-2007 Adam Treat <treat@kde.org>                        *
 *   Copyright 2003-2007 Jens Dagerbo <jens.dagerbo@swipnet.se>            *
 *   Copyright 2001-2002 Bernd Gehrmann <bernd@mail.berlios.de>            *
 *   Copyright 2001-2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>       *
 *   Copyright 2003 Roberto Raggi <roberto@kdevelop.org>                   *
 *   Copyright 2010 Niko Sams <niko.sams@gmail.com>                        *
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

#include <config.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <ksplashscreen.h>
#include <kmessagebox.h>
#include <ktexteditor/cursor.h>

#include <QFileInfo>
#include <QDir>
#include <QProcess>
#include <QSessionManager>
#include <QTextStream>
#include <QDBusInterface>
#include <QDBusReply>

#include <shell/core.h>
#include <shell/mainwindow.h>
#include <shell/projectcontroller.h>
#include <shell/documentcontroller.h>
#include <shell/plugincontroller.h>
#include <shell/sessioncontroller.h>
#include <shell/runcontroller.h>
#include <shell/launchconfiguration.h>
#include <shell/session.h>
#include <interfaces/ilauncher.h>
#include <interfaces/iproject.h>
#include <interfaces/launchconfigurationtype.h>

#include "kdevideextension.h"

#include <iostream>

#include "welcomepage/welcomepageview.h"
#include "splash.h"

using KDevelop::Core;

// Represents a file to be opened, consisting of its URL and the linenumber to jump to
typedef QPair<QString, int> File;

class KDevelopApplication: public KApplication {
public:
    explicit KDevelopApplication(bool GUIenabled = true): KApplication(GUIenabled) {}

    void saveState( QSessionManager& sm ) {
        if (KDevelop::Core::self() && KDevelop::Core::self()->sessionController()) {
            QString x11SessionId = QString("%1_%2").arg(sm.sessionId()).arg(sm.sessionKey());
            QString kdevelopSessionId = KDevelop::Core::self()->sessionController()->activeSession()->id().toString();

            sm.setRestartCommand(QStringList() << QCoreApplication::applicationFilePath() << "-session" << x11SessionId << "-s" << kdevelopSessionId);
        }

        KApplication::saveState(sm);
    }
};

/// Tries to find a session identified by @p data in @p sessions.
/// The @p data may be either a session's name or a string-representation of its UUID.
/// @return pointer to the session or NULL if nothing appropriate has been found
static const KDevelop::SessionInfo* findSessionInList( QList<KDevelop::SessionInfo>& sessions, const QString& data )
{
    // We won't search a session without input data, since that could lead to false-positives
    // with unnamed sessions
    if( data.isEmpty() )
        return 0;

    for( QList<KDevelop::SessionInfo>::const_iterator it = sessions.constBegin(); it != sessions.constEnd(); ++it ) {
        if ( ( it->name == data ) || ( it->uuid.toString() == data ) ) {
            const KDevelop::SessionInfo& sessionRef = *it;
            return &sessionRef;
        }
    }
    return 0;
}

/// Parses a filename given as an argument by determining its line number and full path
static File parseFilename(QString argument)
{
    if ( KUrl::isRelativeUrl(argument) && ! argument.startsWith('/') ) {
        argument = QDir::currentPath() + "/" + argument;
    }
    //Allow opening specific lines in documents, like mydoc.cpp:10
    int lineNumberOffset = argument.lastIndexOf(':');
    int line = -1;
    if( lineNumberOffset != -1 )
    {
        bool isInt;
        int lineNr = argument.mid(lineNumberOffset+1).toInt(&isInt);
        if (isInt)
        {
            argument = argument.left(lineNumberOffset);
            line = lineNr;
        }
    }
    return File(argument, line);
}

/// Performs a DBus call to open the given @p files in the running kdev instance identified by @p pid
/// Returns the exit status
static int openFilesInRunningInstance(const QVector<File>& files, int pid)
{
    const QString service = QString("org.kdevelop.kdevelop-%1").arg(pid);
    QDBusInterface iface(service, "/org/kdevelop/DocumentController", "org.kdevelop.DocumentController");

    QStringList urls;
    foreach ( const File& file, files ) {
        urls << file.first;
    }
    QDBusReply<bool> result = iface.call("openDocumentsSimple", QVariant(urls));
    if ( ! result.value() ) {
        KMessageBox::sorry(0, i18n("Some of the requested files could not be opened."));
        return 1;
    }
    // make the window visible
    QDBusMessage makeVisible = QDBusMessage::createMethodCall( service, "/kdevelop/MainWindow", "org.kdevelop.MainWindow",
                                                           "ensureVisible" );
    QDBusConnection::sessionBus().asyncCall( makeVisible );
    return 0;
}

/// Gets the PID of a running KDevelop instance, eventually asking the user if there is more than one.
/// Returns -1 in case there are no running sessions.
static int getRunningSessionPid()
{
    QList<KDevelop::SessionInfo> candidates;
    foreach( const KDevelop::SessionInfo& si, KDevelop::SessionController::availableSessionInfo() ) {
        if( KDevelop::SessionController::isSessionRunning(si.uuid.toString()) ) {
            candidates << si;
        }
    }
    if ( candidates.isEmpty() ) {
        return -1;
    }

    QString sessionUuid;
    if ( candidates.size() == 1 ) {
        sessionUuid = candidates.first().uuid.toString();
    }
    else {
        const QString title = i18n("Select the session to open the document in");
        sessionUuid = KDevelop::SessionController::showSessionChooserDialog(title, true);
    }
    return KDevelop::SessionController::sessionRunInfo(sessionUuid).holderPid;
}

int main( int argc, char *argv[] )
{
    static const char description[] = I18N_NOOP( "The KDevelop Integrated Development Environment" );
    KAboutData aboutData( "kdevelop", 0, ki18n( "KDevelop" ), QByteArray(VERSION), ki18n(description), KAboutData::License_GPL,
                          ki18n( "Copyright 1999-2013, The KDevelop developers" ), KLocalizedString(), "http://www.kdevelop.org/" );
    aboutData.addAuthor( ki18n("Andreas Pakulat"), ki18n( "Architecture, VCS Support, Project Management Support, QMake Projectmanager" ), "apaku@gmx.de" );
    aboutData.addAuthor( ki18n("Alexander Dymo"), ki18n( "Architecture, Sublime UI, Ruby support" ), "adymo@kdevelop.org" );
    aboutData.addAuthor( ki18n("David Nolden"), ki18n( "Definition-Use Chain, C++ Support, Code Navigation, Code Completion, Coding Assistance, Refactoring" ), "david.nolden.kdevelop@art-master.de" );
    aboutData.addAuthor( ki18n("Aleix Pol Gonzalez"), ki18n( "Co-Maintainer, CMake Support, Run Support, Kross Support" ), "aleixpol@gmail.com" );
    aboutData.addAuthor( ki18n("Vladimir Prus"), ki18n( "GDB integration" ), "ghost@cs.msu.su" );
    aboutData.addAuthor( ki18n("Hamish Rodda"), ki18n( "Text editor integration, definition-use chain" ), "rodda@kde.org" );
    aboutData.addAuthor( ki18n("Amilcar do Carmo Lucas"), ki18n( "Website admin, API documentation, Doxygen and autoproject patches" ), "amilcar@kdevelop.org" );
    aboutData.addAuthor( ki18n("Niko Sams"), ki18n( "GDB integration, Webdevelopment Plugins" ), "niko.sams@gmail.com" );
    aboutData.addAuthor( ki18n("Milian Wolff"), ki18n( "Co-Maintainer, Generic manager, Webdevelopment Plugins, Snippets, Performance" ), "mail@milianw.de" );
    aboutData.addAuthor( ki18n("Sven Brauch"), ki18n( "Python Support, User Interface improvements" ), "svenbrauch@gmail.com" );

    aboutData.addCredit( ki18n("Matt Rogers"), KLocalizedString(), "mattr@kde.org");
    aboutData.addCredit( ki18n("Cédric Pasteur"), ki18n("astyle and indent support"), "cedric.pasteur@free.fr" );
    aboutData.addCredit( ki18n("Evgeniy Ivanov"), ki18n("Distributed VCS, Git, Mercurial"), "powerfox@kde.ru" );
    // QTest integration is separate in playground currently.
    //aboutData.addCredit( ki18n("Manuel Breugelmanns"), ki18n( "Veritas, QTest integration"), "mbr.nxi@gmail.com" );
    aboutData.addCredit( ki18n("Robert Gruber") , ki18n( "SnippetPart, debugger and usability patches" ), "rgruber@users.sourceforge.net" );
    aboutData.addCredit( ki18n("Dukju Ahn"), ki18n( "Subversion plugin, Custom Make Manager, Overall improvements" ), "dukjuahn@gmail.com" );
    aboutData.addCredit( ki18n("Harald Fernengel"), ki18n( "Ported to Qt 3, patches, valgrind, diff and perforce support" ), "harry@kdevelop.org" );
    aboutData.addCredit( ki18n("Roberto Raggi"), ki18n( "C++ parser" ), "roberto@kdevelop.org" );
    aboutData.addCredit( ki18n("The KWrite authors"), ki18n( "Kate editor component" ), "kwrite-devel@kde.org" );
    aboutData.addCredit( ki18n("Nokia Corporation/Qt Software"), ki18n( "Designer code" ), "qt-info@nokia.com" );

    aboutData.addCredit( ki18n("Contributors to older versions:"), KLocalizedString(), "" );
    aboutData.addCredit( ki18n("The KHTML authors"), ki18n( "HTML documentation component" ), "kfm-devel@kde.org" );
    aboutData.addCredit( ki18n("Bernd Gehrmann"), ki18n( "Initial idea, basic architecture, much initial source code" ), "bernd@kdevelop.org" );
    aboutData.addCredit( ki18n("Caleb Tennis"), ki18n( "KTabBar, bugfixes" ), "caleb@aei-tech.com" );
    aboutData.addCredit( ki18n("Richard Dale"), ki18n( "Java & Objective C support" ), "Richard_Dale@tipitina.demon.co.uk" );
    aboutData.addCredit( ki18n("John Birch"), ki18n( "Debugger frontend" ), "jbb@kdevelop.org" );
    aboutData.addCredit( ki18n("Sandy Meier"), ki18n( "PHP support, context menu stuff" ), "smeier@kdevelop.org" );
    aboutData.addCredit( ki18n("Kurt Granroth"), ki18n( "KDE application templates" ), "kurth@granroth.org" );
    aboutData.addCredit( ki18n("Ian Reinhart Geiser"), ki18n( "Dist part, bash support, application templates" ), "geiseri@yahoo.com" );
    aboutData.addCredit( ki18n("Matthias Hoelzer-Kluepfel"), ki18n( "Several components, htdig indexing" ), "hoelzer@kde.org" );
    aboutData.addCredit( ki18n("Victor Roeder"), ki18n( "Help with Automake manager and persistent class store" ), "victor_roeder@gmx.de" );
    aboutData.addCredit( ki18n("Simon Hausmann"), ki18n( "Help with KParts infrastructure" ), "hausmann@kde.org" );
    aboutData.addCredit( ki18n("Oliver Kellogg"), ki18n( "Ada support" ), "okellogg@users.sourceforge.net" );
    aboutData.addCredit( ki18n("Jakob Simon-Gaarde"), ki18n( "QMake projectmanager" ), "jsgaarde@tdcspace.dk" );
    aboutData.addCredit( ki18n("Falk Brettschneider"), ki18n( "MDI modes, QEditor, bugfixes" ), "falkbr@kdevelop.org" );
    aboutData.addCredit( ki18n("Mario Scalas"), ki18n( "PartExplorer, redesign of CvsPart, patches, bugs(fixes)" ), "mario.scalas@libero.it" );
    aboutData.addCredit( ki18n("Jens Dagerbo"), ki18n( "Replace, Bookmarks, FileList and CTags2 plugins. Overall improvements and patches" ), "jens.dagerbo@swipnet.se" );
    aboutData.addCredit( ki18n("Julian Rockey"), ki18n( "Filecreate part and other bits and patches" ), "linux@jrockey.com" );
    aboutData.addCredit( ki18n("Ajay Guleria"), ki18n( "ClearCase support" ), "ajay_guleria@yahoo.com" );
    aboutData.addCredit( ki18n("Marek Janukowicz"), ki18n( "Ruby support" ), "child@t17.ds.pwr.wroc.pl" );
    aboutData.addCredit( ki18n("Robert Moniot"), ki18n( "Fortran documentation" ), "moniot@fordham.edu" );
    aboutData.addCredit( ki18n("Ka-Ping Yee"), ki18n( "Python documentation utility" ), "ping@lfw.org" );
    aboutData.addCredit( ki18n("Dimitri van Heesch"), ki18n( "Doxygen wizard" ), "dimitri@stack.nl" );
    aboutData.addCredit( ki18n("Hugo Varotto"), ki18n( "Fileselector component" ), "hugo@varotto-usa.com" );
    aboutData.addCredit( ki18n("Matt Newell"), ki18n( "Fileselector component" ), "newellm@proaxis.com" );
    aboutData.addCredit( ki18n("Daniel Engelschalt"), ki18n( "C++ code completion, persistent class store" ), "daniel.engelschalt@gmx.net" );
    aboutData.addCredit( ki18n("Stephane Ancelot"), ki18n( "Patches" ), "sancelot@free.fr" );
    aboutData.addCredit( ki18n("Jens Zurheide"), ki18n( "Patches" ), "jens.zurheide@gmx.de" );
    aboutData.addCredit( ki18n("Luc Willems"), ki18n( "Help with Perl support" ), "Willems.luc@pandora.be" );
    aboutData.addCredit( ki18n("Marcel Turino"), ki18n( "Documentation index view" ), "M.Turino@gmx.de" );
    aboutData.addCredit( ki18n("Yann Hodique"), ki18n( "Patches" ), "Yann.Hodique@lifl.fr" );
    aboutData.addCredit( ki18n("Tobias Gl\303\244\303\237er") , ki18n( "Documentation Finder,  qmake projectmanager patches, usability improvements, bugfixes ... " ), "tobi.web@gmx.de" );
    aboutData.addCredit( ki18n("Andreas Koepfle") , ki18n( "QMake project manager patches" ), "koepfle@ti.uni-mannheim.de" );
    aboutData.addCredit( ki18n("Sascha Cunz") , ki18n( "Cleanup and bugfixes for qEditor, AutoMake and much other stuff" ), "mail@sacu.de" );
    aboutData.addCredit( ki18n("Zoran Karavla"), ki18n( "Artwork for the ruby language" ), "webmaster@the-error.net", "http://the-error.net" );

    //we can't use KCmdLineArgs as it doesn't allow arguments for the debugee
    //so lookup the --debug switch and eat everything behind by decrementing argc
    //debugArgs is filled with args after --debug <debuger>
    QStringList debugArgs;
    QString debugeeName;
    {
        bool debugFound = false;
        int c = argc;
        for (int i=0; i < c; ++i) {
            if (debugFound) {
                debugArgs << argv[i];
            } else if (qstrcmp(argv[i], "--debug") == 0 || qstrcmp(argv[i], "-d") == 0) {
                if (argc <= i+1) {
                    argc = i + 1;
                } else {
                    i++;
                    argc = i + 1;
                }
                debugFound = true;
            } else if (QString(argv[i]).startsWith("--debug=")) {
                argc = i + 1;
                debugFound = true;
            }
        }
    }

    KCmdLineArgs::init( argc, argv, &aboutData );
    KCmdLineOptions options;
    options.add("n")
           .add("new-session <name>", ki18n("Open KDevelop with a new session using the given name."));
    options.add("s")
           .add("open-session <session>", ki18n("Open KDevelop with the given session.\n"
                                           "You can pass either hash or the name of the session." ));
    options.add("ps").add("pick-session", ki18n("Shows all available sessions and lets you select one to open." ));
    options.add("pss").add("pick-session-shell", ki18n("List all available sessions on shell and lets you select one to open." ));
    options.add("l")
           .add("list-sessions", ki18n( "List available sessions and quit." ));
    options.add("p")
           .add("project <project>", ki18n( "Open KDevelop and load the given project." ));
    options.add("d")
           .add("debug <debugger>",
                ki18n( "Start debugging an application in KDevelop with the given debugger.\n"
                       "The binary that should be debugged must follow - including arguments.\n"
                       "Example: kdevelop --debug gdb myapp --foo bar"));

    options.add("pid");

    options.add("+files", ki18n( "Files to load" ));

    options.add(":", ki18n("Deprecated options:"));
    options.add("sessions", ki18n( "Same as -l / --list-sessions" ));
    options.add("cs <name>", ki18n( "Same as -n / --new-session" ));

    KCmdLineArgs::addCmdLineOptions( options );
    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
    KDevelopApplication app;

    // The session-controller needs to arguments to eventually pass them to newly opened sessions
    KDevelop::SessionController::setArguments(argc, argv);

    if(args->isSet("sessions") || args->isSet("list-sessions"))
    {
        QTextStream qout(stdout);
        qout << endl << ki18n("Available sessions (use '-s HASH' or '-s NAME' to open a specific one):").toString() << endl << endl;
        qout << QString("%1").arg(ki18n("Hash").toString(), -38) << '\t' << ki18n("Name: Opened Projects").toString() << endl;
        foreach(const KDevelop::SessionInfo& si, KDevelop::SessionController::availableSessionInfo())
        {
            if ( si.name.isEmpty() && si.projects.isEmpty() ) {
                continue;
            }
            qout << si.uuid.toString() << '\t' << si.description;

            if(KDevelop::SessionController::isSessionRunning(si.uuid.toString()))
                qout << "     " << i18n("[running]");

            qout << endl;
        }
        return 0;
    }

    // Handle extra arguments, which stand for files to open
    QVector<File> initialFiles;
    for ( int i = 0; i < args->count(); i++ ) {
        initialFiles.append(parseFilename(args->arg(i)));
    }
    if ( ! initialFiles.isEmpty() && ! args->isSet("open-session") && ! args->isSet("new-session") )
    {
        int pid = getRunningSessionPid();
        if ( pid > 0 ) {
            return openFilesInRunningInstance(initialFiles, pid);
        }
        // else there are no running sessions, and the generated list of files will be opened below.
    }

    // if empty, restart kdevelop with last active session, see SessionController::defaultSessionId
    QString session;

    if(args->isSet("pss"))
    {
        QTextStream qerr(stderr);
        QList<KDevelop::SessionInfo> candidates;
        foreach(const KDevelop::SessionInfo& si, KDevelop::SessionController::availableSessionInfo())
            if( (!si.name.isEmpty() || !si.projects.isEmpty() || args->isSet("pid")) &&
                (!args->isSet("pid") || KDevelop::SessionController::isSessionRunning(si.uuid.toString())))
                candidates << si;
        
        if(candidates.size() == 0)
        {
            qerr << "no session available" << endl;
            return 1;
        }
        
        if(candidates.size() == 1 && args->isSet("pid"))
        {
            session = candidates[0].uuid.toString();
        }else{
            for(int i = 0; i < candidates.size(); ++i)
                qerr << "[" << i << "]: " << candidates[i].description << endl;
            
            int chosen;
            std::cin >> chosen;
            if(std::cin.good() && (chosen >= 0 && chosen < candidates.size()))
            {
                session = candidates[chosen].uuid.toString();
            }else{
                qerr << "invalid selection" << endl;
                return 1;
            }
        }
    }
    
    if(args->isSet("ps"))
    {
        bool onlyRunning = args->isSet("pid");
        session = KDevelop::SessionController::showSessionChooserDialog(i18n("Select the session you would like to use"), onlyRunning);
        if(session.isEmpty())
            return 1;
    }

    if ( args->isSet("debug") ) {
        if ( debugArgs.isEmpty() ) {
            QTextStream qerr(stderr);
            qerr << endl << i18nc("@info:shell", "Specify the binary you want to debug.") << endl;
            return 1;
        }
        debugeeName = i18n("Debug %1", KUrl( debugArgs.first() ).fileName());
        session = debugeeName;
    } else if ( args->isSet("cs") || args->isSet("new-session") )
    {
        session = args->isSet("cs") ? args->getOption("cs") : args->getOption("new-session");
        foreach(const KDevelop::SessionInfo& si, KDevelop::SessionController::availableSessionInfo())
        {
            if ( session == si.name ) {
                QTextStream qerr(stderr);
                qerr << endl << i18n("A session with the name %1 exists already. Use the -s switch to open it.", session) << endl;
                return 1;
            }
        }
        // session doesn't exist, we can create it
    } else if ( args->isSet("open-session") ) {
        session = args->getOption("open-session");
        //If there is a session and a project with the same name, always open the session
        //regardless of the order encountered
        QString projectAsSession;
        bool found = false;
        foreach(const KDevelop::SessionInfo& si, KDevelop::SessionController::availableSessionInfo())
        {
            if ( session == si.name || session == si.uuid.toString() ) {
                found = true;
                break;
            }
            else if (projectAsSession.isEmpty()) {
                foreach(const KUrl& k, si.projects)
                {
                    QString fn(k.fileName());
                    fn = fn.left(fn.indexOf('.'));
                    if ( session == fn )
                        projectAsSession = si.uuid;
                }
            }
        }
        if ( !found ) {
            if ( projectAsSession.isEmpty() ) {
                QTextStream qerr(stderr);
                qerr << endl << i18n("Cannot open unknown session %1. See --sessions switch for available sessions or use -cs to create a new one.", session) << endl;
                return 1;
            }
            else
                session = projectAsSession;
        }
    }

    QList<KDevelop::SessionInfo> sessions = KDevelop::SessionController::availableSessionInfo();
    const KDevelop::SessionInfo* sessionData = findSessionInList( sessions, session );

    if(args->isSet("pid")) {
        if( !sessionData ) {
            kError() << "session not given or does not exist";
            return 5;
        }

        KDevelop::SessionRunInfo sessionInfo = KDevelop::SessionController::sessionRunInfo( sessionData->uuid.toString() );
        if (!sessionInfo.isRunning) {
            kError() << session << sessionData->name << "is not running";
            return 5;
        } else {
            // Print the PID and we're ready
            std::cout << sessionInfo.holderPid << std::endl;
            return 0;
        }
    }

    KDevIDEExtension::init();

    if ( !QProcessEnvironment::systemEnvironment().contains("KDEV_DISABLE_SPLASH") ) {
        KDevSplashScreen* splash = new KDevSplashScreen;
        splash->show();
        splash->repaint();
        app.processEvents();
    }

    if(!Core::initialize(splash, Core::Default, session))
        return 5;

    KGlobal::locale()->insertCatalog( Core::self()->componentData().catalogName() );
    Core* core = Core::self();

    QStringList projectNames = args->getOptionList("project");
    if(!projectNames.isEmpty())
    {
        foreach(const QString& p, projectNames)
        {
            QFileInfo info( p );
            if( info.suffix() == "kdev4" ) {
                // make sure the project is not already opened by the session controller
                bool shouldOpen = true;
                KUrl url(info.absoluteFilePath());
                foreach(KDevelop::IProject* p, core->projectController()->projects()) {
                    if (p->projectFileUrl() == url) {
                        shouldOpen = false;
                        break;
                    }
                }
                if (shouldOpen) {
                    core->projectController()->openProject( url );
                }
            }
        }
    }

    if ( args->isSet("debug") ) {
        Q_ASSERT( !debugeeName.isEmpty() );
        QString launchName = debugeeName;

        KDevelop::LaunchConfiguration* launch = 0;
        kDebug() << launchName;
        foreach (KDevelop::LaunchConfiguration *l, core->runControllerInternal()->launchConfigurationsInternal()) {
            kDebug() << l->name();
            if (l->name() == launchName) {
                launch = l;
            }
        }

        KDevelop::LaunchConfigurationType *type = 0;
        foreach (KDevelop::LaunchConfigurationType *t, core->runController()->launchConfigurationTypes()) {
            kDebug() << t->id();
            if (t->id() == "Native Application") {
                type = t;
                break;
            }
        }
        if (!type) {
            QTextStream qerr(stderr);
            qerr << endl << i18n("Cannot find native launch configuration type") << endl;
            return 1;
        }

        if (launch && launch->type()->id() != "Native Application") launch = 0;
        if (launch && launch->launcherForMode("debug") != args->getOption("debug")) launch = 0;
        if (!launch) {
            kDebug() << launchName << "not found, creating a new one";
            QPair<QString,QString> launcher;
            launcher.first = "debug";
            foreach (KDevelop::ILauncher *l, type->launchers()) {
                if (l->id() == args->getOption("debug")) {
                    if (l->supportedModes().contains("debug")) {
                        launcher.second = l->id();
                    }
                }
            }
            if (launcher.second.isEmpty()) {
                QTextStream qerr(stderr);
                qerr << endl << i18n("Cannot find launcher %1", args->getOption("debug")) << endl;
                return 1;
            }
            KDevelop::ILaunchConfiguration* ilaunch = core->runController()->createLaunchConfiguration(type, launcher, 0, launchName);
            launch = dynamic_cast<KDevelop::LaunchConfiguration*>(ilaunch);
        }

        type->configureLaunchFromCmdLineArguments(launch->config(), debugArgs);
        launch->config().writeEntry("Break on Start", true);
        core->runControllerInternal()->setDefaultLaunch(launch);

        core->runControllerInternal()->execute("debug", launch);
    } else {
        foreach ( const File& file, initialFiles ) {
            if(!core->documentController()->openDocument(file.first, KTextEditor::Cursor(file.second, 0))) {
                kWarning() << i18n("Could not open %1", file.first);
            }
        }
    }

#ifdef WITH_WELCOMEPAGE
    // make it possible to disable the welcome page, useful for valgrind runs e.g.
    if (!QProcessEnvironment::systemEnvironment().contains("KDEV_DISABLE_WELCOMEPAGE")) {
        trySetupWelcomePageView();
    }
#endif
    return app.exec();
}
