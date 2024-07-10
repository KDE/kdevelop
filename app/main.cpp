/*
    SPDX-FileCopyrightText: 2003-2009 Alexander Dymo <adymo@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Ralf Habacker <Ralf.Habacker@freenet.de>
    SPDX-FileCopyrightText: 2006-2007 Matt Rogers <mattr@kde.org>
    SPDX-FileCopyrightText: 2006-2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2005-2007 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2003-2007 Jens Dagerbo <jens.dagerbo@swipnet.se>
    SPDX-FileCopyrightText: 2001-2002 Bernd Gehrmann <bernd@mail.berlios.de>
    SPDX-FileCopyrightText: 2001-2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2003 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2010 Niko Sams <niko.sams@gmail.com>
    SPDX-FileCopyrightText: 2015 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "config-kdevelop.h"
#include "kdevelop_version.h"

#include "urlinfo.h"

#include <KLocalizedString>
#include <KAboutData>
#include <KCrash>

#include <QApplication>
#include <QElapsedTimer>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QFileInfo>
#include <QProcessEnvironment>
#include <QSessionManager>
#include <QTextStream>
#include <QDBusInterface>
#include <QDBusReply>

#include <QQuickWindow>

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
#include <debug.h>

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
    for (const UrlInfo& info : infos) {
        if (!ICore::self()->documentController()->openDocument(info.url, info.cursor)) {
            qCWarning(APP) << i18n("Could not open %1", info.url.toDisplayString(QUrl::PreferLocalFile));
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
    Q_OBJECT
public:
    explicit KDevelopApplication(int &argc, char **argv, bool GUIenabled = true)
#if KDEVELOP_SINGLE_APP
        : SharedTools::QtSingleApplication(QStringLiteral("KDevelop"), argc, argv)
#else
        : QApplication(argc, argv, GUIenabled)
#endif
        {
            Q_UNUSED(GUIenabled);
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

            QVector<UrlInfo> files, directories;
            for (const auto& info : infos)
                if (info.isDirectory())
                    directories << info;
                else
                    files << info;

            openFiles(files);
            for(const auto &urlinfo : directories)
                ICore::self()->projectController()->openProjectForUrl(urlinfo.url);
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
            const auto activeSession = KDevelop::Core::self()->sessionController()->activeSession();
            if (!activeSession) {
                qCWarning(APP) << "No active session, can't save state";
                return;
            }

            const QString x11SessionId = sm.sessionId() + QLatin1Char('_') + sm.sessionKey();
            QString kdevelopSessionId = activeSession->id().toString();
            sm.setRestartCommand({
                QCoreApplication::applicationFilePath(),
                QStringLiteral("-session"),
                x11SessionId,
                QStringLiteral("-s"),
                kdevelopSessionId
            });
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
        return nullptr;

    for( auto it = sessions.constBegin(); it != sessions.constEnd(); ++it ) {
        if ( ( it->name == data ) || ( it->uuid.toString() == data ) ) {
            const KDevelop::SessionInfo& sessionRef = *it;
            return &sessionRef;
        }
    }
    return nullptr;
}

/// Tries to find sessions containing project @p projectUrl in @p sessions.
static const KDevelop::SessionInfos findSessionsWithProject(const SessionInfos& sessions, const QUrl& projectUrl)
{
    if (!projectUrl.isValid())
        return {};

    KDevelop::SessionInfos infos;
    for (auto& session : sessions) {
        if (session.projects.contains(projectUrl)) {
            infos << session;
        }
    }
    return infos;
}

/// Performs a DBus call to open the given @p files in the running kdev instance identified by @p pid
/// Returns the exit status
static int openFilesInRunningInstance(const QVector<UrlInfo>& files, qint64 pid)
{
    const QString service = QStringLiteral("org.kdevelop.kdevelop-%1").arg(pid);
    QDBusInterface iface(service, QStringLiteral("/org/kdevelop/DocumentController"), QStringLiteral("org.kdevelop.DocumentController"));

    QStringList urls;
    bool errors_occurred = false;
    for (const UrlInfo& file : files) {
        QDBusReply<bool> result = iface.call(QStringLiteral("openDocumentSimple"), file.url.toString(), file.cursor.line(), file.cursor.column());
        if ( ! result.value() ) {
            QTextStream err(stderr);
            err << i18n("Could not open file '%1'.", file.url.toDisplayString(QUrl::PreferLocalFile)) << "\n";
            errors_occurred = true;
        }
    }
    // make the window visible
    QDBusMessage makeVisible = QDBusMessage::createMethodCall( service, QStringLiteral("/kdevelop/MainWindow"), QStringLiteral("org.kdevelop.MainWindow"),
                                                               QStringLiteral("ensureVisible") );
    QDBusConnection::sessionBus().asyncCall( makeVisible );
    return errors_occurred;
}

/// Performs a DBus call to open the given @p files in the running kdev instance identified by @p pid
/// Returns the exit status
static int openProjectInRunningInstance(const QVector<UrlInfo>& paths, qint64 pid)
{
    const QString service = QStringLiteral("org.kdevelop.kdevelop-%1").arg(pid);
    QDBusInterface iface(service, QStringLiteral("/org/kdevelop/ProjectController"), QStringLiteral("org.kdevelop.ProjectController"));
    int errors = 0;

    for (const UrlInfo& path : paths) {
        QDBusReply<void> result = iface.call(QStringLiteral("openProjectForUrl"), path.url.toString());
        if ( !result.isValid() ) {
            QTextStream err(stderr);
            err << i18n("Could not open project '%1': %2", path.url.toDisplayString(QUrl::PreferLocalFile), result.error().message()) << "\n";
            ++errors;
        }
    }
    // make the window visible
    QDBusMessage makeVisible = QDBusMessage::createMethodCall( service, QStringLiteral("/kdevelop/MainWindow"), QStringLiteral("org.kdevelop.MainWindow"),
                                                               QStringLiteral("ensureVisible") );
    QDBusConnection::sessionBus().asyncCall( makeVisible );
    return errors;
}

/// Gets the PID of a running KDevelop instance, eventually asking the user if there is more than one.
/// Returns -1 in case there are no running sessions.
static qint64 getRunningSessionPid()
{
    SessionInfos candidates;
    const auto availableSessionInfos = KDevelop::SessionController::availableSessionInfos();
    for (const KDevelop::SessionInfo& si : availableSessionInfos) {
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
    for (const KDevelop::SessionInfo& si : availableSessionInfos) {
        if ( session == si.name || session == si.uuid.toString() ) {
            return si.uuid.toString();
        } else if (projectAsSession.isEmpty()) {
            for (const QUrl& k : si.projects) {
                QString fn(k.fileName());
                fn = fn.left(fn.indexOf(QLatin1Char('.')));
                if ( session == fn ) {
                    projectAsSession = si.uuid.toString();
                }
            }
        }
    }

    if (projectAsSession.isEmpty())  {
        QTextStream qerr(stderr);
        qerr << QLatin1Char('\n') << i18n("Cannot open unknown session %1. See `--list-sessions` switch for available sessions or use `-n` to create a new one.",
                             session) << QLatin1Char('\n');
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

    // If possible, use the Software backend for QQuickWidget (currently used in the
    // welcome page plugin). This means we don't need OpenGL at all, avoiding issues
    // like https://bugs.kde.org/show_bug.cgi?id=386527.
    QQuickWindow::setSceneGraphBackend(QSGRendererInterface::Software);

    // Useful for valgrind runs, just `export KDEV_DISABLE_JIT=1`
    if (qEnvironmentVariableIsSet("KDEV_DISABLE_JIT")) {
        qputenv("QT_ENABLE_REGEXP_JIT", "0");
    }

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
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
                debugArgs << QString::fromUtf8(argv[i]);
            } else if ((qstrcmp(argv[i], "--debug") == 0) || (qstrcmp(argv[i], "-d") == 0)) {
                if (argc > (i + 1)) {
                    i++;
                }
                argc = i + 1;
                debugFound = true;
            } else if (QByteArray(argv[i]).startsWith("--debug=")) {
                argc = i + 1;
                debugFound = true;
            }
        }
    }

    KDevelopApplication app(argc, argv);
    // Prevent SIGPIPE, then "ICE default IO error handler doing an exit(), pid = <PID>, errno = 32"
    // crash when the first event loop starts at least 60 seconds after KDevelop launch. This can
    // happen during a Debug Launch of KDevelop from KDevelop, especially if a breakpoint is hit
    // before any event loop is entered.
    QCoreApplication::processEvents();

    KLocalizedString::setApplicationDomain("kdevelop");

    KAboutData aboutData(QStringLiteral("kdevelop"), i18n("KDevelop"),
                         QStringLiteral(KDEVELOP_VERSION_STRING " (" RELEASE_SERVICE_VERSION_STRING ")"),
                         i18n("The KDevelop Integrated Development Environment"), KAboutLicense::GPL,
                         i18n("Copyright 1999-%1, The KDevelop developers", QStringLiteral("2024")), QString(),
                         QStringLiteral("https://www.kdevelop.org/"));
    aboutData.setDesktopFileName(QStringLiteral("org.kde.kdevelop"));
    aboutData.addAuthor( i18n("Kevin Funk"), i18n( "Co-maintainer, C++/Clang, QA, Windows Support, Performance, Website" ), QStringLiteral("kfunk@kde.org") );
    aboutData.addAuthor( i18n("Sven Brauch"), i18n( "Co-maintainer, AppImage, Python Support, User Interface improvements" ), QStringLiteral("svenbrauch@gmail.com") );
    aboutData.addAuthor( i18n("Aleix Pol Gonzalez"), i18n( "CMake Support, Run Support, Kross Support" ), QStringLiteral("aleixpol@kde.org") );
    aboutData.addAuthor( i18n("Milian Wolff"), i18n( "C++/Clang, Generic manager, Webdevelopment Plugins, Snippets, Performance" ), QStringLiteral("mail@milianw.de") );
    aboutData.addAuthor( i18n("Olivier JG"), i18n( "C++/Clang, DUChain, Bug Fixes" ), QStringLiteral("olivier.jg@gmail.com") );
    aboutData.addAuthor( i18n("Andreas Pakulat"), i18n( "Architecture, VCS Support, Project Management Support, QMake Projectmanager" ), QStringLiteral("apaku@gmx.de") );
    aboutData.addAuthor( i18n("Alexander Dymo"), i18n( "Architecture, Sublime UI, Ruby support" ), QStringLiteral("adymo@kdevelop.org") );
    aboutData.addAuthor( i18n("David Nolden"), i18n( "Definition-Use Chain, C++ Support, Code Navigation, Code Completion, Coding Assistance, Refactoring" ), QStringLiteral("david.nolden.kdevelop@art-master.de") );
    aboutData.addAuthor( i18n("Vladimir Prus"), i18n( "GDB integration" ), QStringLiteral("ghost@cs.msu.su") );
    aboutData.addAuthor( i18n("Hamish Rodda"), i18n( "Text editor integration, definition-use chain" ), QStringLiteral("rodda@kde.org") );
    aboutData.addAuthor( i18n("Amilcar do Carmo Lucas"), i18n( "Website admin, API documentation, Doxygen and autoproject patches" ), QStringLiteral("amilcar@kdevelop.org") );
    aboutData.addAuthor( i18n("Niko Sams"), i18n( "GDB integration, Webdevelopment Plugins" ), QStringLiteral("niko.sams@gmail.com") );
    aboutData.addAuthor( i18n("Friedrich W. H. Kossebau"), QString(), QStringLiteral("kossebau@kde.org") );

    aboutData.addCredit( i18n("Matt Rogers"), QString(), QStringLiteral("mattr@kde.org"));
    aboutData.addCredit( i18n("Cédric Pasteur"), i18n("astyle and indent support"), QStringLiteral("cedric.pasteur@free.fr") );
    aboutData.addCredit( i18n("Evgeniy Ivanov"), i18n("Distributed VCS, Git, Mercurial"), QStringLiteral("powerfox@kde.ru") );
    // QTest integration is separate in playground currently.
    //aboutData.addCredit( i18n("Manuel Breugelmanns"), i18n( "Veritas, QTest integration"), "mbr.nxi@gmail.com" );
    aboutData.addCredit( i18n("Robert Gruber") , i18n( "SnippetPart, debugger and usability patches" ), QStringLiteral("rgruber@users.sourceforge.net") );
    aboutData.addCredit( i18n("Dukju Ahn"), i18n( "Subversion plugin, Custom Make Manager, Overall improvements" ), QStringLiteral("dukjuahn@gmail.com") );
    aboutData.addCredit( i18n("Harald Fernengel"), i18n( "Ported to Qt 3, patches, valgrind, diff and perforce support" ), QStringLiteral("harry@kdevelop.org") );
    aboutData.addCredit( i18n("Roberto Raggi"), i18n( "C++ parser" ), QStringLiteral("roberto@kdevelop.org") );
    aboutData.addCredit( i18n("The KWrite authors"), i18n( "Kate editor component" ), QStringLiteral("kwrite-devel@kde.org") );
    aboutData.addCredit( i18n("Nokia Corporation/Qt Software"), i18n( "Designer code" ), QStringLiteral("qt-info@nokia.com") );

    aboutData.addCredit( i18n("Contributors to older versions:"), QString(), QString() );
    aboutData.addCredit( i18n("Bernd Gehrmann"), i18n( "Initial idea, basic architecture, much initial source code" ), QStringLiteral("bernd@kdevelop.org") );
    aboutData.addCredit( i18n("Caleb Tennis"), i18n( "KTabBar, bugfixes" ), QStringLiteral("caleb@aei-tech.com") );
    aboutData.addCredit( i18n("Richard Dale"), i18n( "Java & Objective C support" ), QStringLiteral("Richard_Dale@tipitina.demon.co.uk") );
    aboutData.addCredit( i18n("John Birch"), i18n( "Debugger frontend" ), QStringLiteral("jbb@kdevelop.org") );
    aboutData.addCredit( i18n("Sandy Meier"), i18n( "PHP support, context menu stuff" ), QStringLiteral("smeier@kdevelop.org") );
    aboutData.addCredit( i18n("Kurt Granroth"), i18n( "KDE application templates" ), QStringLiteral("kurth@granroth.org") );
    aboutData.addCredit( i18n("Ian Reinhart Geiser"), i18n( "Dist part, bash support, application templates" ), QStringLiteral("geiseri@yahoo.com") );
    aboutData.addCredit( i18n("Matthias Hoelzer-Kluepfel"), i18n( "Several components, htdig indexing" ), QStringLiteral("hoelzer@kde.org") );
    aboutData.addCredit( i18n("Victor Roeder"), i18n( "Help with Automake manager and persistent class store" ), QStringLiteral("victor_roeder@gmx.de") );
    aboutData.addCredit( i18n("Simon Hausmann"), i18n( "Help with KParts infrastructure" ), QStringLiteral("hausmann@kde.org") );
    aboutData.addCredit( i18n("Oliver Kellogg"), i18n( "Ada support" ), QStringLiteral("okellogg@users.sourceforge.net") );
    aboutData.addCredit( i18n("Jakob Simon-Gaarde"), i18n( "QMake projectmanager" ), QStringLiteral("jsgaarde@tdcspace.dk") );
    aboutData.addCredit( i18n("Falk Brettschneider"), i18n( "MDI modes, QEditor, bugfixes" ), QStringLiteral("falkbr@kdevelop.org") );
    aboutData.addCredit( i18n("Mario Scalas"), i18n( "PartExplorer, redesign of CvsPart, patches, bugs(fixes)" ), QStringLiteral("mario.scalas@libero.it") );
    aboutData.addCredit( i18n("Jens Dagerbo"), i18n( "Replace, Bookmarks, FileList and CTags2 plugins. Overall improvements and patches" ), QStringLiteral("jens.dagerbo@swipnet.se") );
    aboutData.addCredit( i18n("Julian Rockey"), i18n( "Filecreate part and other bits and patches" ), QStringLiteral("linux@jrockey.com") );
    aboutData.addCredit( i18n("Ajay Guleria"), i18n( "ClearCase support" ), QStringLiteral("ajay_guleria@yahoo.com") );
    aboutData.addCredit( i18n("Marek Janukowicz"), i18n( "Ruby support" ), QStringLiteral("child@t17.ds.pwr.wroc.pl") );
    aboutData.addCredit( i18n("Robert Moniot"), i18n( "Fortran documentation" ), QStringLiteral("moniot@fordham.edu") );
    aboutData.addCredit( i18n("Ka-Ping Yee"), i18n( "Python documentation utility" ), QStringLiteral("ping@lfw.org") );
    aboutData.addCredit( i18n("Dimitri van Heesch"), i18n( "Doxygen wizard" ), QStringLiteral("dimitri@stack.nl") );
    aboutData.addCredit( i18n("Hugo Varotto"), i18n( "Fileselector component" ), QStringLiteral("hugo@varotto-usa.com") );
    aboutData.addCredit( i18n("Matt Newell"), i18n( "Fileselector component" ), QStringLiteral("newellm@proaxis.com") );
    aboutData.addCredit( i18n("Daniel Engelschalt"), i18n( "C++ code completion, persistent class store" ), QStringLiteral("daniel.engelschalt@gmx.net") );
    aboutData.addCredit( i18n("Stephane Ancelot"), i18n( "Patches" ), QStringLiteral("sancelot@free.fr") );
    aboutData.addCredit( i18n("Jens Zurheide"), i18n( "Patches" ), QStringLiteral("jens.zurheide@gmx.de") );
    aboutData.addCredit( i18n("Luc Willems"), i18n( "Help with Perl support" ), QStringLiteral("Willems.luc@pandora.be") );
    aboutData.addCredit( i18n("Marcel Turino"), i18n( "Documentation index view" ), QStringLiteral("M.Turino@gmx.de") );
    aboutData.addCredit( i18n("Yann Hodique"), i18n( "Patches" ), QStringLiteral("Yann.Hodique@lifl.fr") );
    aboutData.addCredit( i18n("Tobias Gl\303\244\303\237er") , i18n( "Documentation Finder,  qmake projectmanager patches, usability improvements, bugfixes ... " ), QStringLiteral("tobi.web@gmx.de") );
    aboutData.addCredit( i18n("Andreas Koepfle") , i18n( "QMake project manager patches" ), QStringLiteral("koepfle@ti.uni-mannheim.de") );
    aboutData.addCredit( i18n("Sascha Cunz") , i18n( "Cleanup and bugfixes for qEditor, AutoMake and much other stuff" ), QStringLiteral("mail@sacu.de") );
    aboutData.addCredit( i18n("Zoran Karavla"), i18n( "Artwork for the ruby language" ), QStringLiteral("webmaster@the-error.net"), QStringLiteral("http://the-error.net") );

    KAboutData::setApplicationData(aboutData);
    // set icon for shells which do not use desktop file metadata
    // but without setting replacing an existing icon with an empty one!
    QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("kdevelop"), QApplication::windowIcon()));

    KCrash::initialize();


    // High DPI support
    app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);

    qCDebug(APP) << "Startup";

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);

    parser.addOption(QCommandLineOption{QStringList{QStringLiteral("n"), QStringLiteral("new-session")},
                     i18n("Open KDevelop with a new session using the given name."),
                     QStringLiteral("name")});
    parser.addOption(QCommandLineOption{QStringList{QStringLiteral("s"), QStringLiteral("open-session")},
                     i18n("Open KDevelop with the given session.\n"
                          "You can pass either hash or the name of the session."),
                     QStringLiteral("session")});
    parser.addOption(QCommandLineOption{QStringList{QStringLiteral("rm"), QStringLiteral("remove-session")},
                     i18n("Delete the given session.\n"
                          "You can pass either hash or the name of the session." ),
                     QStringLiteral("session")});
    parser.addOption(QCommandLineOption{QStringList{QStringLiteral("ps"), QStringLiteral("pick-session")},
                     i18n("Shows all available sessions and lets you select one to open.")});
    parser.addOption(QCommandLineOption{QStringList{QStringLiteral("pss"), QStringLiteral("pick-session-shell")},
                     i18n("List all available sessions on shell and lets you select one to open.")});
    parser.addOption(QCommandLineOption{QStringList{QStringLiteral("l"), QStringLiteral("list-sessions")},
                     i18n("List available sessions and quit.")});
    parser.addOption(QCommandLineOption{QStringList{QStringLiteral("f"), QStringLiteral("fetch")},
                     i18n("Open KDevelop and fetch the project from the given <repo url>."),
                     QStringLiteral("repo url")});
    parser.addOption(QCommandLineOption{QStringList{QStringLiteral("p"), QStringLiteral("project")},
                     i18n("Open KDevelop and load the given project. <project> can be either a .kdev4 file or a directory path."),
                     QStringLiteral("project")});
    parser.addOption(QCommandLineOption{QStringList{QStringLiteral("d"), QStringLiteral("debug")},
                     i18n("Start debugging an application in KDevelop with the given debugger.\n"
                     "The executable that should be debugged must follow - including arguments.\n"
                     "Example: kdevelop --debug gdb myapp --foo bar"), QStringLiteral("debugger")});

    // this is used by the 'kdevelop!' script to retrieve the pid of a KDEVELOP
    // instance. When this is called, then we should just print the PID on the
    // standard-output. If a session is specified through open-session, then
    // we should return the PID of that session. Otherwise, if only a single
    // session is running, then we should just return the PID of that session.
    // Otherwise, we should print a command-line session-chooser dialog ("--pss"),
    // which only shows the running sessions, and the user can pick one.
    parser.addOption(QCommandLineOption{QStringList{QStringLiteral("pid")}});

    parser.addPositionalArgument(QStringLiteral("files"),
                     i18n( "Files to load, or directories to load as projects" ), QStringLiteral("[FILE[:line[:column]] | DIRECTORY]..."));

    // The session-controller needs to arguments to eventually pass them to newly opened sessions
    KDevelop::SessionController::setArguments(argc, argv);

    parser.process(app);
    aboutData.processCommandLine(&parser);

    if(parser.isSet(QStringLiteral("list-sessions")))
    {
        QTextStream qout(stdout);
        qout << QLatin1Char('\n') << ki18n("Available sessions (use '-s HASH' or '-s NAME' to open a specific one):").toString() << QLatin1String("\n\n");
        qout << QStringLiteral("%1").arg(ki18n("Hash").toString(), -38) << '\t' << ki18n("Name: Opened Projects").toString() << QLatin1Char('\n');
        const auto availableSessionInfos = KDevelop::SessionController::availableSessionInfos();
        for (const KDevelop::SessionInfo& si : availableSessionInfos) {
            if ( si.name.isEmpty() && si.projects.isEmpty() ) {
                continue;
            }
            qout << si.uuid.toString() << '\t' << si.description;

            if(KDevelop::SessionController::isSessionRunning(si.uuid.toString()))
                qout << "     " << i18n("[running]");

            qout << QLatin1Char('\n');
        }
        return 0;
    }

    // Handle extra arguments, which stand for files to open
    QVector<UrlInfo> initialFiles;
    QVector<UrlInfo> initialDirectories;
    const auto files = parser.positionalArguments();
    for (const QString& file : files) {
        const UrlInfo info(file);
        if (info.isDirectory()) {
            initialDirectories.append(info);
        } else {
            initialFiles.append(info);
        }
    }

    const auto availableSessionInfos = KDevelop::SessionController::availableSessionInfos();

    if ((!initialFiles.isEmpty() || !initialDirectories.isEmpty()) && !parser.isSet(QStringLiteral("new-session"))) {
#if KDEVELOP_SINGLE_APP
        if (app.isRunning()) {
            bool success = app.sendMessage(serializeOpenFilesMessage(initialFiles << initialDirectories));
            if (success) {
                return 0;
            }
        }
#else
        qint64 pid = -1;
        if (parser.isSet(QStringLiteral("open-session"))) {
            const QString session = findSessionId(availableSessionInfos, parser.value(QStringLiteral("open-session")));
            if (session.isEmpty()) {
                return 1;
            } else if (KDevelop::SessionController::isSessionRunning(session)) {
                pid = findSessionPid(session);
            }
        } else {
            pid = getRunningSessionPid();
        }

        if ( pid > 0 ) {
            return openFilesInRunningInstance(initialFiles, pid) + openProjectInRunningInstance(initialDirectories, pid);
        }
        // else there are no running sessions, and the generated list of files will be opened below.
#endif
    }

    // if empty, restart kdevelop with last active session, see SessionController::defaultSessionId
    QString session;

    uint nRunningSessions = 0;
    for (const KDevelop::SessionInfo& si : availableSessionInfos) {
        if(KDevelop::SessionController::isSessionRunning(si.uuid.toString()))
            ++nRunningSessions;
    }

    // also show the picker dialog when a pid shall be retrieved and multiple
    // sessions are running.
    if(parser.isSet(QStringLiteral("pss")) || (parser.isSet(QStringLiteral("pid")) && !parser.isSet(QStringLiteral("open-session")) && !parser.isSet(QStringLiteral("ps")) && nRunningSessions > 1))
    {
        SessionInfos candidates;
        for (const KDevelop::SessionInfo& si : availableSessionInfos) {
            if( (!si.name.isEmpty() || !si.projects.isEmpty() || parser.isSet(QStringLiteral("pid"))) &&
                (!parser.isSet(QStringLiteral("pid")) || KDevelop::SessionController::isSessionRunning(si.uuid.toString())))
                candidates << si;
        }

        if(candidates.size() == 0)
        {
            QTextStream qerr(stderr);
            qerr << "no session available" << QLatin1Char('\n');
            return 1;
        }

        if(candidates.size() == 1 && parser.isSet(QStringLiteral("pid")))
        {
            session = candidates.constFirst().uuid.toString();
        }else{
            QTextStream qout(stdout);
            for(int i = 0; i < candidates.size(); ++i)
                qout << "[" << i << "]: " << candidates.at(i).description << QLatin1Char('\n');
            qout.flush();

            int chosen;
            std::cin >> chosen;
            if(std::cin.good() && (chosen >= 0 && chosen < candidates.size()))
            {
                session = candidates.at(chosen).uuid.toString();
            }else{
                QTextStream qerr(stderr);
                qerr << "invalid selection" << QLatin1Char('\n');
                return 1;
            }
        }
    }

    if(parser.isSet(QStringLiteral("ps")))
    {
        bool onlyRunning = parser.isSet(QStringLiteral("pid"));
        session = KDevelop::SessionController::showSessionChooserDialog(i18n("Select the session you would like to use"), onlyRunning);
        if(session.isEmpty())
            return 1;
    }

    if ( parser.isSet(QStringLiteral("debug")) ) {
        if ( debugArgs.isEmpty() ) {
            QTextStream qerr(stderr);
            qerr << QLatin1Char('\n') << i18nc("@info:shell", "Specify the executable you want to debug.") << QLatin1Char('\n');
            return 1;
        }

        QFileInfo executableFileInfo(debugArgs.first());
        if (!executableFileInfo.exists()) {
            executableFileInfo = QFileInfo(QStandardPaths::findExecutable(debugArgs.first()));
            if (!executableFileInfo.exists()) {
                QTextStream qerr(stderr);
                qerr << QLatin1Char('\n') << i18nc("@info:shell", "Specified executable does not exist.") << QLatin1Char('\n');
                return 1;
            }
        }

        debugArgs.first() = executableFileInfo.absoluteFilePath();
        debugeeName = i18n("Debug %1", executableFileInfo.fileName());
        session = debugeeName;
    } else if ( parser.isSet(QStringLiteral("new-session")) )
    {
        session = parser.value(QStringLiteral("new-session"));
        for (const KDevelop::SessionInfo& si : availableSessionInfos) {
            if ( session == si.name ) {
                QTextStream qerr(stderr);
                qerr << QLatin1Char('\n') << i18n("A session with the name %1 exists already. Use the -s switch to open it.", session) << QLatin1Char('\n');
                return 1;
            }
        }
        // session doesn't exist, we can create it
    } else if ( parser.isSet(QStringLiteral("open-session")) ) {
        session = findSessionId(availableSessionInfos, parser.value(QStringLiteral("open-session")));
        if (session.isEmpty()) {
            return 1;
        }
    } else if ( parser.isSet(QStringLiteral("remove-session")) )
    {
        session = parser.value(QStringLiteral("remove-session"));
        auto si = findSessionInList(availableSessionInfos, session);
        if (!si) {
            QTextStream qerr(stderr);
            qerr << QLatin1Char('\n') << i18n("No session with the name %1 exists.", session) << QLatin1Char('\n');
            return 1;
        }

        auto sessionLock = KDevelop::SessionController::tryLockSession(si->uuid.toString());
        if (!sessionLock.lock) {
            QTextStream qerr(stderr);
            qerr << QLatin1Char('\n') << i18n("Could not lock session %1 for deletion.", session) << QLatin1Char('\n');
            return 1;
        }
        KDevelop::SessionController::deleteSessionFromDisk(sessionLock.lock);
        QTextStream qout(stdout);
        qout << QLatin1Char('\n') << i18n("Session with name %1 was successfully removed.", session) << QLatin1Char('\n');
        return 0;
    }

    if(parser.isSet(QStringLiteral("pid"))) {
        if (session.isEmpty())
        {   // just pick the first running session
            for (const KDevelop::SessionInfo& si : availableSessionInfos) {
                if(KDevelop::SessionController::isSessionRunning(si.uuid.toString()))
                    session = si.uuid.toString();
            }
        }
        const KDevelop::SessionInfo* sessionData = findSessionInList(availableSessionInfos, session);

        if( !sessionData ) {
            qCCritical(APP) << "session not given or does not exist";
            return 5;
        }

        const auto pid = findSessionPid(sessionData->uuid.toString());
        if (pid > 0) {
            // Print the PID and we're ready
            std::cout << pid << std::endl;
            return 0;
        } else {
            qCCritical(APP) << sessionData->uuid.toString() << sessionData->name << "is not running";
            return 5;
        }
    }

    if (parser.isSet(QStringLiteral("project"))) {
        const auto project = parser.value(QStringLiteral("project"));
        QFileInfo info(project);
        QUrl projectUrl;
        if (info.suffix() == QLatin1String("kdev4")) {
            projectUrl = QUrl::fromLocalFile(info.absoluteFilePath());
        } else if (info.isDir()) {
            QDir dir(info.absoluteFilePath());
            const auto potentialProjectFiles = dir.entryList({QStringLiteral("*.kdev4")}, QDir::Files, QDir::Name);
            qCDebug(APP) << "Found these potential project files:" << potentialProjectFiles;
            if (!potentialProjectFiles.isEmpty()) {
                projectUrl = QUrl::fromLocalFile(dir.absoluteFilePath(potentialProjectFiles.value(0)));
            }
        } else {
            QTextStream qerr(stderr);
            qerr << "Invalid project: " << project << " - should be either a path to a .kdev4 file or a directory containing a .kdev4 file";
            return 1;
        }

        qCDebug(APP) << "Attempting to find a suitable session for project" << projectUrl;
        const auto sessionInfos = findSessionsWithProject(availableSessionInfos, projectUrl);
        qCDebug(APP) << "Found matching sessions:" << sessionInfos.size();
        if (!sessionInfos.isEmpty()) {
            // TODO: If there's more than one match: Allow the user to select which session to open?
            qCDebug(APP) << "Attempting to open session:" << sessionInfos.at(0).name;
            session = sessionInfos.at(0).uuid.toString();
        }
    }

    KDevIDEExtension::init();

    qCDebug(APP) << "Attempting to initialize session:" << session;
    if(!Core::initialize(Core::Default, session))
        return 5;

    // register a DBUS service for this process, so that we can open files in it from other invocations
    QDBusConnection::sessionBus().registerService(QStringLiteral("org.kdevelop.kdevelop-%1").arg(app.applicationPid()));

    Core* core = Core::self();

    const auto fetchUrlStrings = parser.values(QStringLiteral("fetch"));
    for (const auto& fetchUrlString : fetchUrlStrings) {
        core->projectControllerInternal()->fetchProjectFromUrl(QUrl::fromUserInput(fetchUrlString));
    }

    const QString debugStr = QStringLiteral("debug");
    if ( parser.isSet(debugStr) ) {
        Q_ASSERT( !debugeeName.isEmpty() );
        QString launchName = debugeeName;

        KDevelop::LaunchConfiguration* launch = nullptr;
        qCDebug(APP) << launchName;
        const auto launchconfigurations = core->runControllerInternal()->launchConfigurationsInternal();
        for (KDevelop::LaunchConfiguration* l : launchconfigurations) {
            qCDebug(APP) << l->name();
            if (l->name() == launchName) {
                launch = l;
            }
        }

        KDevelop::LaunchConfigurationType *type = nullptr;
        const auto launchConfigurationTypes = core->runController()->launchConfigurationTypes();
        for (KDevelop::LaunchConfigurationType* t : launchConfigurationTypes) {
            qCDebug(APP) << t->id();
            if (t->id() == QLatin1String("Native Application")) {
                type = t;
                break;
            }
        }
        if (!type) {
            QTextStream qerr(stderr);
            qerr << QLatin1Char('\n') << i18n("Cannot find native launch configuration type") << QLatin1Char('\n');
            return 1;
        }

        if (launch && launch->type()->id() != QLatin1String("Native Application")) launch = nullptr;
        if (launch && launch->launcherForMode(debugStr) != parser.value(debugStr)) launch = nullptr;
        if (!launch) {
            qCDebug(APP) << launchName << "not found, creating a new one";
            QPair<QString,QString> launcher;
            launcher.first = debugStr;
            const auto typeLaunchers = type->launchers();
            for (KDevelop::ILauncher* l : typeLaunchers) {
                if (l->id() == parser.value(debugStr)) {
                    if (l->supportedModes().contains(debugStr)) {
                        launcher.second = l->id();
                    }
                }
            }
            if (launcher.second.isEmpty()) {
                QTextStream qerr(stderr);
                qerr << QLatin1Char('\n') << i18n("Cannot find launcher %1", parser.value(debugStr)) << QLatin1Char('\n');
                return 1;
            }
            KDevelop::ILaunchConfiguration* ilaunch = core->runController()->createLaunchConfiguration(type, launcher, nullptr, launchName);
            launch = static_cast<KDevelop::LaunchConfiguration*>(ilaunch);
        }

        type->configureLaunchFromCmdLineArguments(launch->config(), debugArgs);
        launch->config().writeEntry("Break on Start", true);
        core->runControllerInternal()->setDefaultLaunch(launch);

        core->runControllerInternal()->execute(debugStr, launch);
    } else {
        openFiles(initialFiles);

        for(const auto& urlinfo: qAsConst(initialDirectories))
            core->projectController()->openProjectForUrl(urlinfo.url);
    }

#if KDEVELOP_SINGLE_APP
    // Set up remote arguments.
    QObject::connect(&app, &SharedTools::QtSingleApplication::messageReceived,
                     &app, &KDevelopApplication::remoteArguments);

    QObject::connect(&app, &SharedTools::QtSingleApplication::fileOpenRequest,
                     &app, &KDevelopApplication::fileOpenRequested);
#endif


    qCDebug(APP) << "Done startup" << "- took:" << timer.elapsed() << "ms";
    timer.invalidate();

    return app.exec();
}

#include "main.moc"
