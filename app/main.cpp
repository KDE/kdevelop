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
 *   Copyright 2015 Kevin Funk <kfunk@kde.org>                             *
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

#include "urlinfo.h"

#include <KLocalizedString>
#include <Kdelibs4ConfigMigrator>
#include <kaboutdata.h>
#include <kmessagebox.h>
#include <ktexteditor/cursor.h>
#include <kcrash.h>

#include <QApplication>
#include <QElapsedTimer>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QFileInfo>
#include <QDir>
#include <QProcess>
#include <QSessionManager>
#include <QTextStream>
#include <QDBusInterface>
#include <QDBusReply>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(APP)
Q_LOGGING_CATEGORY(APP, "kdevelop.app")

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
#include <util/path.h>

#include "kdevideextension.h"
#if KDEVELOP_SINGLE_APP
#include "qtsingleapplication.h"
#endif

#include <iostream>

#ifdef Q_OS_MAC
#include <CoreFoundation/CoreFoundation.h>
#endif

using namespace KDevelop;

namespace {

#if KDEVELOP_SINGLE_APP
QString serializeOpenFilesMessage(const QVector<UrlInfo> &infos)
{
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream << QByteArrayLiteral("open");
    stream << infos;
    return QString::fromLatin1(message.toHex());
}
#endif

void openFiles(const QVector<UrlInfo>& infos)
{
    foreach (const UrlInfo& file, infos) {
        if (!ICore::self()->documentController()->openDocument(file.url, file.cursor)) {
            qWarning() << i18n("Could not open %1", file.url.toDisplayString(QUrl::PreferLocalFile));
        }
    }
}

}

class KDevelopApplication:
#if KDEVELOP_SINGLE_APP
    public SharedTools::QtSingleApplication
#else
    public QApplication
#endif
{
public:
    explicit KDevelopApplication(int &argc, char **argv, bool GUIenabled = true)
#if KDEVELOP_SINGLE_APP
        : SharedTools::QtSingleApplication(QStringLiteral("KDevelop"), argc, argv)
#else
        : QApplication(argc, argv, GUIenabled)
#endif
        {
#if KDEVELOP_SINGLE_APP
            Q_UNUSED(GUIenabled);
#endif

            connect(this, &QGuiApplication::saveStateRequest, this, &KDevelopApplication::saveState);
        }

#if KDEVELOP_SINGLE_APP
public Q_SLOTS:
    void remoteArguments(const QString &message, QObject *socket)
    {
        Q_UNUSED(socket);

        QByteArray ba = QByteArray::fromHex(message.toLatin1());
        QDataStream stream(ba);
        QByteArray command;
        stream >> command;

        qCDebug(APP) << "Received remote command: " << command;

        if (command == "open") {
            QVector<UrlInfo> infos;
            stream >> infos;
            openFiles(infos);
        } else {
            qCWarning(APP) << "Unknown remote command: " << command;
        }
    }

    void fileOpenRequested(const QString &file)
    {
        openFiles({UrlInfo(file)});
    }
#endif

private Q_SLOTS:
    void saveState( QSessionManager& sm ) {
        if (KDevelop::Core::self() && KDevelop::Core::self()->sessionController()) {
            QString x11SessionId = QString("%1_%2").arg(sm.sessionId()).arg(sm.sessionKey());
            QString kdevelopSessionId = KDevelop::Core::self()->sessionController()->activeSession()->id().toString();

            sm.setRestartCommand(QStringList() << QCoreApplication::applicationFilePath() << "-session" << x11SessionId << "-s" << kdevelopSessionId);
        }
    }
};

/// Tries to find a session identified by @p data in @p sessions.
/// The @p data may be either a session's name or a string-representation of its UUID.
/// @return pointer to the session or NULL if nothing appropriate has been found
static const KDevelop::SessionInfo* findSessionInList( const SessionInfos& sessions, const QString& data )
{
    // We won't search a session without input data, since that could lead to false-positives
    // with unnamed sessions
    if( data.isEmpty() )
        return 0;

    for( auto it = sessions.constBegin(); it != sessions.constEnd(); ++it ) {
        if ( ( it->name == data ) || ( it->uuid.toString() == data ) ) {
            const KDevelop::SessionInfo& sessionRef = *it;
            return &sessionRef;
        }
    }
    return 0;
}

/// Performs a DBus call to open the given @p files in the running kdev instance identified by @p pid
/// Returns the exit status
static int openFilesInRunningInstance(const QVector<UrlInfo>& files, qint64 pid)
{
    const QString service = QString("org.kdevelop.kdevelop-%1").arg(pid);
    QDBusInterface iface(service, "/org/kdevelop/DocumentController", "org.kdevelop.DocumentController");

    QStringList urls;
    bool errors_occured = false;
    foreach ( const UrlInfo& file, files ) {
        QDBusReply<bool> result = iface.call("openDocumentSimple", file.url.toString(), file.cursor.line(), file.cursor.column());
        if ( ! result.value() ) {
            QTextStream err(stderr);
            err << i18n("Could not open file %1.", file.url.toDisplayString(QUrl::PreferLocalFile)) << "\n";
            errors_occured = true;
        }
    }
    // make the window visible
    QDBusMessage makeVisible = QDBusMessage::createMethodCall( service, "/kdevelop/MainWindow", "org.kdevelop.MainWindow",
                                                               "ensureVisible" );
    QDBusConnection::sessionBus().asyncCall( makeVisible );
    return errors_occured ? 1 : 0;
}

/// Gets the PID of a running KDevelop instance, eventually asking the user if there is more than one.
/// Returns -1 in case there are no running sessions.
static qint64 getRunningSessionPid()
{
    SessionInfos candidates;
    foreach( const KDevelop::SessionInfo& si, KDevelop::SessionController::availableSessionInfos() ) {
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

static QString findSessionId(const SessionInfos& availableSessionInfos, const QString& session)
{
    //If there is a session and a project with the same name, always open the session
    //regardless of the order encountered
    QString projectAsSession;
    foreach(const KDevelop::SessionInfo& si, availableSessionInfos)
    {
        if ( session == si.name || session == si.uuid.toString() ) {
            return si.uuid.toString();
        } else if (projectAsSession.isEmpty()) {
            foreach(const QUrl& k, si.projects) {
                QString fn(k.fileName());
                fn = fn.left(fn.indexOf('.'));
                if ( session == fn ) {
                    projectAsSession = si.uuid.toString();
                }
            }
        }
    }

    if (projectAsSession.isEmpty())  {
        QTextStream qerr(stderr);
        qerr << endl << i18n("Cannot open unknown session %1. See `--list-sessions` switch for available sessions or use `-n` to create a new one.",
                             session) << endl;
    }
    return projectAsSession;
}

static qint64 findSessionPid(const QString &sessionId)
{
    KDevelop::SessionRunInfo sessionInfo = KDevelop::SessionController::sessionRunInfo( sessionId );
    return sessionInfo.holderPid;
}

int main( int argc, char *argv[] )
{
    QElapsedTimer timer;
    timer.start();

    // TODO: Maybe generalize, add KDEVELOP_STANDALONE build option
#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
    qputenv("KDE_FORK_SLAVES", "1"); // KIO slaves will be forked off instead of being started via DBus
#endif

    // Useful for valgrind runs, just `export KDEV_DISABLE_JIT=1`
    if (qEnvironmentVariableIsSet("KDEV_DISABLE_JIT")) {
        qputenv("KDEV_DISABLE_WELCOMEPAGE", "1");
        qputenv("QT_ENABLE_REGEXP_JIT", "0");
    }

    // Don't show any debug output by default.
    // If you need to enable additional logging for debugging use a rules file
    // as explained in the QLoggingCategory documentation:
    // http://qt-project.org/doc/qt-5/qloggingcategory.html#logging-rules
    QLoggingCategory::setFilterRules(QStringLiteral("*.debug=false\ndefault.debug=true\n"));
    KLocalizedString::setApplicationDomain("kdevelop");
    QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

#ifdef Q_OS_MAC
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    if (mainBundle) {
        // get the application's Info Dictionary. For app bundles this would live in the bundle's Info.plist,
        // for regular executables it is obtained in another way.
        CFMutableDictionaryRef infoDict = (CFMutableDictionaryRef) CFBundleGetInfoDictionary(mainBundle);
        if (infoDict) {
            // Try to prevent App Nap on OS X. This can be tricky in practice, at least in 10.9 .
            CFDictionarySetValue(infoDict, CFSTR("NSAppSleepDisabled"), kCFBooleanTrue);
            CFDictionarySetValue(infoDict, CFSTR("NSSupportsAutomaticTermination"), kCFBooleanFalse);
        }
    }
#endif

    static const char description[] = I18N_NOOP( "The KDevelop Integrated Development Environment" );
    KAboutData aboutData( "kdevelop", i18n( "KDevelop" ), QByteArray(VERSION), i18n(description), KAboutLicense::GPL,
                          i18n("Copyright 1999-2015, The KDevelop developers"), QString(), "http://www.kdevelop.org/");
    aboutData.setDesktopFileName(QStringLiteral("org.kde.kdevelop.desktop"));
    aboutData.addAuthor( i18n("Aleix Pol Gonzalez"), i18n( "Co-Maintainer, CMake Support, Run Support, Kross Support" ), "aleixpol@gmail.com" );
    aboutData.addAuthor( i18n("Milian Wolff"), i18n( "Co-Maintainer, C++/Clang, Generic manager, Webdevelopment Plugins, Snippets, Performance" ), "mail@milianw.de" );
    aboutData.addAuthor( i18n("Kevin Funk"), i18n( "C++/Clang, General Improvements, QA, Windows Support" ), "kfunk@kde.org" );
    aboutData.addAuthor( i18n("Olivier JG"), i18n( "C++/Clang, DUChain, Bug Fixes" ), "olivier.jg@gmail.com" );
    aboutData.addAuthor( i18n("Sven Brauch"), i18n( "Python Support, User Interface improvements" ), "svenbrauch@gmail.com" );
    aboutData.addAuthor( i18n("Andreas Pakulat"), i18n( "Architecture, VCS Support, Project Management Support, QMake Projectmanager" ), "apaku@gmx.de" );
    aboutData.addAuthor( i18n("Alexander Dymo"), i18n( "Architecture, Sublime UI, Ruby support" ), "adymo@kdevelop.org" );
    aboutData.addAuthor( i18n("David Nolden"), i18n( "Definition-Use Chain, C++ Support, Code Navigation, Code Completion, Coding Assistance, Refactoring" ), "david.nolden.kdevelop@art-master.de" );
    aboutData.addAuthor( i18n("Vladimir Prus"), i18n( "GDB integration" ), "ghost@cs.msu.su" );
    aboutData.addAuthor( i18n("Hamish Rodda"), i18n( "Text editor integration, definition-use chain" ), "rodda@kde.org" );
    aboutData.addAuthor( i18n("Amilcar do Carmo Lucas"), i18n( "Website admin, API documentation, Doxygen and autoproject patches" ), "amilcar@kdevelop.org" );
    aboutData.addAuthor( i18n("Niko Sams"), i18n( "GDB integration, Webdevelopment Plugins" ), "niko.sams@gmail.com" );

    aboutData.addCredit( i18n("Matt Rogers"), QString(), "mattr@kde.org");
    aboutData.addCredit( i18n("Cédric Pasteur"), i18n("astyle and indent support"), "cedric.pasteur@free.fr" );
    aboutData.addCredit( i18n("Evgeniy Ivanov"), i18n("Distributed VCS, Git, Mercurial"), "powerfox@kde.ru" );
    // QTest integration is separate in playground currently.
    //aboutData.addCredit( i18n("Manuel Breugelmanns"), i18n( "Veritas, QTest integration"), "mbr.nxi@gmail.com" );
    aboutData.addCredit( i18n("Robert Gruber") , i18n( "SnippetPart, debugger and usability patches" ), "rgruber@users.sourceforge.net" );
    aboutData.addCredit( i18n("Dukju Ahn"), i18n( "Subversion plugin, Custom Make Manager, Overall improvements" ), "dukjuahn@gmail.com" );
    aboutData.addCredit( i18n("Harald Fernengel"), i18n( "Ported to Qt 3, patches, valgrind, diff and perforce support" ), "harry@kdevelop.org" );
    aboutData.addCredit( i18n("Roberto Raggi"), i18n( "C++ parser" ), "roberto@kdevelop.org" );
    aboutData.addCredit( i18n("The KWrite authors"), i18n( "Kate editor component" ), "kwrite-devel@kde.org" );
    aboutData.addCredit( i18n("Nokia Corporation/Qt Software"), i18n( "Designer code" ), "qt-info@nokia.com" );

    aboutData.addCredit( i18n("Contributors to older versions:"), QString(), "" );
    aboutData.addCredit( i18n("Bernd Gehrmann"), i18n( "Initial idea, basic architecture, much initial source code" ), "bernd@kdevelop.org" );
    aboutData.addCredit( i18n("Caleb Tennis"), i18n( "KTabBar, bugfixes" ), "caleb@aei-tech.com" );
    aboutData.addCredit( i18n("Richard Dale"), i18n( "Java & Objective C support" ), "Richard_Dale@tipitina.demon.co.uk" );
    aboutData.addCredit( i18n("John Birch"), i18n( "Debugger frontend" ), "jbb@kdevelop.org" );
    aboutData.addCredit( i18n("Sandy Meier"), i18n( "PHP support, context menu stuff" ), "smeier@kdevelop.org" );
    aboutData.addCredit( i18n("Kurt Granroth"), i18n( "KDE application templates" ), "kurth@granroth.org" );
    aboutData.addCredit( i18n("Ian Reinhart Geiser"), i18n( "Dist part, bash support, application templates" ), "geiseri@yahoo.com" );
    aboutData.addCredit( i18n("Matthias Hoelzer-Kluepfel"), i18n( "Several components, htdig indexing" ), "hoelzer@kde.org" );
    aboutData.addCredit( i18n("Victor Roeder"), i18n( "Help with Automake manager and persistent class store" ), "victor_roeder@gmx.de" );
    aboutData.addCredit( i18n("Simon Hausmann"), i18n( "Help with KParts infrastructure" ), "hausmann@kde.org" );
    aboutData.addCredit( i18n("Oliver Kellogg"), i18n( "Ada support" ), "okellogg@users.sourceforge.net" );
    aboutData.addCredit( i18n("Jakob Simon-Gaarde"), i18n( "QMake projectmanager" ), "jsgaarde@tdcspace.dk" );
    aboutData.addCredit( i18n("Falk Brettschneider"), i18n( "MDI modes, QEditor, bugfixes" ), "falkbr@kdevelop.org" );
    aboutData.addCredit( i18n("Mario Scalas"), i18n( "PartExplorer, redesign of CvsPart, patches, bugs(fixes)" ), "mario.scalas@libero.it" );
    aboutData.addCredit( i18n("Jens Dagerbo"), i18n( "Replace, Bookmarks, FileList and CTags2 plugins. Overall improvements and patches" ), "jens.dagerbo@swipnet.se" );
    aboutData.addCredit( i18n("Julian Rockey"), i18n( "Filecreate part and other bits and patches" ), "linux@jrockey.com" );
    aboutData.addCredit( i18n("Ajay Guleria"), i18n( "ClearCase support" ), "ajay_guleria@yahoo.com" );
    aboutData.addCredit( i18n("Marek Janukowicz"), i18n( "Ruby support" ), "child@t17.ds.pwr.wroc.pl" );
    aboutData.addCredit( i18n("Robert Moniot"), i18n( "Fortran documentation" ), "moniot@fordham.edu" );
    aboutData.addCredit( i18n("Ka-Ping Yee"), i18n( "Python documentation utility" ), "ping@lfw.org" );
    aboutData.addCredit( i18n("Dimitri van Heesch"), i18n( "Doxygen wizard" ), "dimitri@stack.nl" );
    aboutData.addCredit( i18n("Hugo Varotto"), i18n( "Fileselector component" ), "hugo@varotto-usa.com" );
    aboutData.addCredit( i18n("Matt Newell"), i18n( "Fileselector component" ), "newellm@proaxis.com" );
    aboutData.addCredit( i18n("Daniel Engelschalt"), i18n( "C++ code completion, persistent class store" ), "daniel.engelschalt@gmx.net" );
    aboutData.addCredit( i18n("Stephane Ancelot"), i18n( "Patches" ), "sancelot@free.fr" );
    aboutData.addCredit( i18n("Jens Zurheide"), i18n( "Patches" ), "jens.zurheide@gmx.de" );
    aboutData.addCredit( i18n("Luc Willems"), i18n( "Help with Perl support" ), "Willems.luc@pandora.be" );
    aboutData.addCredit( i18n("Marcel Turino"), i18n( "Documentation index view" ), "M.Turino@gmx.de" );
    aboutData.addCredit( i18n("Yann Hodique"), i18n( "Patches" ), "Yann.Hodique@lifl.fr" );
    aboutData.addCredit( i18n("Tobias Gl\303\244\303\237er") , i18n( "Documentation Finder,  qmake projectmanager patches, usability improvements, bugfixes ... " ), "tobi.web@gmx.de" );
    aboutData.addCredit( i18n("Andreas Koepfle") , i18n( "QMake project manager patches" ), "koepfle@ti.uni-mannheim.de" );
    aboutData.addCredit( i18n("Sascha Cunz") , i18n( "Cleanup and bugfixes for qEditor, AutoMake and much other stuff" ), "mail@sacu.de" );
    aboutData.addCredit( i18n("Zoran Karavla"), i18n( "Artwork for the ruby language" ), "webmaster@the-error.net", "http://the-error.net" );


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

    KDevelopApplication app(argc, argv);

    KCrash::initialize();

    Kdelibs4ConfigMigrator migrator(QStringLiteral("kdevelop"));
    migrator.setConfigFiles({QStringLiteral("kdeveloprc")});
    migrator.setUiFiles({QStringLiteral("kdevelopui.rc")});
    migrator.migrate();

    // High DPI support
    app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);

    qCDebug(APP) << "Startup";

    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);

    parser.addOption(QCommandLineOption{QStringList{"n", "new-session"}, i18n("Open KDevelop with a new session using the given name."), "name"});
    parser.addOption(QCommandLineOption{QStringList{"s", "open-session"}, i18n("Open KDevelop with the given session.\n"
                     "You can pass either hash or the name of the session." ), "session"});
    parser.addOption(QCommandLineOption{QStringList{"rm", "remove-session"}, i18n("Delete the given session.\n"
                     "You can pass either hash or the name of the session." ), "session"});
    parser.addOption(QCommandLineOption{QStringList{"ps", "pick-session"}, i18n("Shows all available sessions and lets you select one to open.")});
    parser.addOption(QCommandLineOption{QStringList{"pss", "pick-session-shell"}, i18n("List all available sessions on shell and lets you select one to open.")});
    parser.addOption(QCommandLineOption{QStringList{"l", "list-sessions"}, i18n("List available sessions and quit.")});
    parser.addOption(QCommandLineOption{QStringList{"p", "project"}, i18n("Open KDevelop and load the given project."), "project"});
    parser.addOption(QCommandLineOption{QStringList{"d", "debug"},
                     i18n("Start debugging an application in KDevelop with the given debugger.\n"
                     "The binary that should be debugged must follow - including arguments.\n"
                     "Example: kdevelop --debug gdb myapp --foo bar"), "debugger"});

    // this is used by the 'kdevelop!' script to retrieve the pid of a KDEVELOP
    // instance. When this is called, then we should just print the PID on the
    // standard-output. If a session is specified through open-session, then
    // we should return the PID of that session. Otherwise, if only a single
    // session is running, then we should just return the PID of that session.
    // Otherwise, we should print a command-line session-chooser dialog ("--pss"),
    // which only shows the running sessions, and the user can pick one.
    parser.addOption(QCommandLineOption{QStringList{"pid"}});

    parser.addPositionalArgument("files", i18n( "Files to load" ), "[FILE...]");

    // The session-controller needs to arguments to eventually pass them to newly opened sessions
    KDevelop::SessionController::setArguments(argc, argv);

    parser.process(app);
    aboutData.processCommandLine(&parser);

    if(parser.isSet("list-sessions"))
    {
        QTextStream qout(stdout);
        qout << endl << ki18n("Available sessions (use '-s HASH' or '-s NAME' to open a specific one):").toString() << endl << endl;
        qout << QString("%1").arg(ki18n("Hash").toString(), -38) << '\t' << ki18n("Name: Opened Projects").toString() << endl;
        foreach(const KDevelop::SessionInfo& si, KDevelop::SessionController::availableSessionInfos())
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
    QVector<UrlInfo> initialFiles;
    foreach (const QString &file, parser.positionalArguments()) {
        initialFiles.append(UrlInfo(file));
    }

    const auto availableSessionInfos = KDevelop::SessionController::availableSessionInfos();

    if (!initialFiles.isEmpty() && !parser.isSet("new-session")) {
#if KDEVELOP_SINGLE_APP
        if (app.isRunning()) {
            bool success = app.sendMessage(serializeOpenFilesMessage(initialFiles));
            if (success) {
                return 0;
            }
        }
#else
        qint64 pid = -1;
        if (parser.isSet("open-session")) {
            const QString session = findSessionId(availableSessionInfos, parser.value("open-session"));
            if (session.isEmpty()) {
                return 1;
            } else if (KDevelop::SessionController::isSessionRunning(session)) {
                pid = findSessionPid(session);
            }
        } else {
            pid = getRunningSessionPid();
        }
        if ( pid > 0 ) {
            return openFilesInRunningInstance(initialFiles, pid);
        }
        // else there are no running sessions, and the generated list of files will be opened below.
#endif
    }

    // if empty, restart kdevelop with last active session, see SessionController::defaultSessionId
    QString session;

    uint nRunningSessions = 0;
    foreach(const KDevelop::SessionInfo& si, availableSessionInfos)
        if(KDevelop::SessionController::isSessionRunning(si.uuid.toString()))
            ++nRunningSessions;

    // also show the picker dialog when a pid shall be retrieved and multiple
    // sessions are running.
    if(parser.isSet("pss") || (parser.isSet("pid") && !parser.isSet("open-session") && !parser.isSet("ps") && nRunningSessions > 1))
    {
        QTextStream qerr(stderr);
        SessionInfos candidates;
        foreach(const KDevelop::SessionInfo& si, availableSessionInfos)
            if( (!si.name.isEmpty() || !si.projects.isEmpty() || parser.isSet("pid")) &&
                (!parser.isSet("pid") || KDevelop::SessionController::isSessionRunning(si.uuid.toString())))
                candidates << si;

        if(candidates.size() == 0)
        {
            qerr << "no session available" << endl;
            return 1;
        }

        if(candidates.size() == 1 && parser.isSet("pid"))
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

    if(parser.isSet("ps"))
    {
        bool onlyRunning = parser.isSet("pid");
        session = KDevelop::SessionController::showSessionChooserDialog(i18n("Select the session you would like to use"), onlyRunning);
        if(session.isEmpty())
            return 1;
    }

    if ( parser.isSet("debug") ) {
        if ( debugArgs.isEmpty() ) {
            QTextStream qerr(stderr);
            qerr << endl << i18nc("@info:shell", "Specify the binary you want to debug.") << endl;
            return 1;
        }
        debugeeName = i18n("Debug %1", QUrl( debugArgs.first() ).fileName());
        session = debugeeName;
    } else if ( parser.isSet("new-session") )
    {
        session = parser.value("new-session");
        foreach(const KDevelop::SessionInfo& si, availableSessionInfos)
        {
            if ( session == si.name ) {
                QTextStream qerr(stderr);
                qerr << endl << i18n("A session with the name %1 exists already. Use the -s switch to open it.", session) << endl;
                return 1;
            }
        }
        // session doesn't exist, we can create it
    } else if ( parser.isSet("open-session") ) {
        session = findSessionId(availableSessionInfos, parser.value("open-session"));
        if (session.isEmpty()) {
            return 1;
        }
    } else if ( parser.isSet("remove-session") )
    {
        session = parser.value("remove-session");
        auto si = findSessionInList(availableSessionInfos, session);
        if (!si) {
            QTextStream qerr(stderr);
            qerr << endl << i18n("No session with the name %1 exists.", session) << endl;
            return 1;
        }

        auto sessionLock = KDevelop::SessionController::tryLockSession(si->uuid.toString());
        if (!sessionLock.lock) {
            QTextStream qerr(stderr);
            qerr << endl << i18n("Could not lock session %1 for deletion.", session) << endl;
            return 1;
        }
        KDevelop::SessionController::deleteSessionFromDisk(sessionLock.lock);
        QTextStream qout(stdout);
        qout << endl << i18n("Session with name %1 was successfully removed.", session) << endl;
        return 0;
    }

    if(parser.isSet("pid")) {
        if (session.isEmpty())
        {   // just pick the first running session
            foreach(const KDevelop::SessionInfo& si, availableSessionInfos)
                if(KDevelop::SessionController::isSessionRunning(si.uuid.toString()))
                    session = si.uuid.toString();
        }
        const KDevelop::SessionInfo* sessionData = findSessionInList(availableSessionInfos, session);

        if( !sessionData ) {
            qCritical() << "session not given or does not exist";
            return 5;
        }

        const auto pid = findSessionPid(sessionData->uuid.toString());
        if (pid > 0) {
            // Print the PID and we're ready
            std::cout << pid << std::endl;
            return 0;
        } else {
            qCritical() << sessionData->uuid.toString() << sessionData->name << "is not running";
            return 5;
        }
    }

    KDevIDEExtension::init();

    if(!Core::initialize(nullptr, Core::Default, session))
        return 5;

    // register a DBUS service for this process, so that we can open files in it from other invocations
    QDBusConnection::sessionBus().registerService(QString("org.kdevelop.kdevelop-%1").arg(app.applicationPid()));

//     TODO: port to kf5
//     KGlobal::locale()->insertCatalog( Core::self()->componentData().catalogName() );
    Core* core = Core::self();
    if (!QProcessEnvironment::systemEnvironment().contains("KDEV_DISABLE_WELCOMEPAGE")) {
        core->pluginController()->loadPlugin("KDevWelcomePage");
    }

    QStringList projectNames = parser.values("project");
    if(!projectNames.isEmpty())
    {
        foreach(const QString& p, projectNames)
        {
            QFileInfo info( p );
            if( info.suffix() == "kdev4" ) {
                // make sure the project is not already opened by the session controller
                bool shouldOpen = true;
                Path path(info.absoluteFilePath());
                foreach(KDevelop::IProject* p, core->projectController()->projects()) {
                    if (p->projectFile() == path) {
                        shouldOpen = false;
                        break;
                    }
                }
                if (shouldOpen) {
                    core->projectController()->openProject( path.toUrl() );
                }
            }
        }
    }

    if ( parser.isSet("debug") ) {
        Q_ASSERT( !debugeeName.isEmpty() );
        QString launchName = debugeeName;

        KDevelop::LaunchConfiguration* launch = 0;
        qCDebug(APP) << launchName;
        foreach (KDevelop::LaunchConfiguration *l, core->runControllerInternal()->launchConfigurationsInternal()) {
            qCDebug(APP) << l->name();
            if (l->name() == launchName) {
                launch = l;
            }
        }

        KDevelop::LaunchConfigurationType *type = 0;
        foreach (KDevelop::LaunchConfigurationType *t, core->runController()->launchConfigurationTypes()) {
            qCDebug(APP) << t->id();
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
        if (launch && launch->launcherForMode("debug") != parser.value("debug")) launch = 0;
        if (!launch) {
            qCDebug(APP) << launchName << "not found, creating a new one";
            QPair<QString,QString> launcher;
            launcher.first = "debug";
            foreach (KDevelop::ILauncher *l, type->launchers()) {
                if (l->id() == parser.value("debug")) {
                    if (l->supportedModes().contains("debug")) {
                        launcher.second = l->id();
                    }
                }
            }
            if (launcher.second.isEmpty()) {
                QTextStream qerr(stderr);
                qerr << endl << i18n("Cannot find launcher %1", parser.value("debug")) << endl;
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
        openFiles(initialFiles);
    }

#if KDEVELOP_SINGLE_APP
    // Set up remote arguments.
    QObject::connect(&app, &SharedTools::QtSingleApplication::messageReceived,
                     &app, &KDevelopApplication::remoteArguments);

    QObject::connect(&app, &SharedTools::QtSingleApplication::fileOpenRequest,
                     &app, &KDevelopApplication::fileOpenRequested);
#endif


#ifdef WITH_WELCOMEPAGE
    // make it possible to disable the welcome page, useful for valgrind runs e.g.
    if (!qEnvironmentVariableIsSet("KDEV_DISABLE_WELCOMEPAGE")) {
        trySetupWelcomePageView();
    }
#endif

    qCDebug(APP) << "Done startup" << "- took:" << timer.elapsed() << "ms";
    timer.invalidate();

    return app.exec();
}
