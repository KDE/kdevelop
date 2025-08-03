/*
    SPDX-FileCopyrightText: 1999-2001 John Birch <jbb@kdevelop.org>
    SPDX-FileCopyrightText: 2001 Bernd Gehrmann <bernd@kdevelop.org>
    SPDX-FileCopyrightText: 2006 Vladimir Prus <ghost@cs.msu.su>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "debugsession.h"

#include "gdb.h"
#include "gdbbreakpointcontroller.h"
#include "gdbframestackmodel.h"
#include "variablecontroller.h"

#include <debuglog.h>

#include "mi/micommand.h"
#include "stty.h"

#include <debugger/breakpoint/breakpoint.h>
#include <debugger/breakpoint/breakpointmodel.h>
#include <execute/iexecuteplugin.h>
#include <interfaces/icore.h>
#include <interfaces/idebugcontroller.h>
#include <interfaces/ilaunchconfiguration.h>
#include <util/environmentprofilelist.h>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KShell>

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QGuiApplication>
#include <QRegularExpression>
#include <QVersionNumber>

using namespace KDevMI::GDB;
using namespace KDevMI::MI;
using namespace KDevelop;

DebugSession::DebugSession()
{
    m_breakpointController = new BreakpointController(this);
    m_variableController = new VariableController(this);
    m_frameStackModel = new GdbFrameStackModel(this);
}

DebugSession::~DebugSession() = default;

void DebugSession::setAutoDisableASLR(bool enable)
{
    m_autoDisableASLR = enable;
}

BreakpointController *DebugSession::breakpointController() const
{
    return m_breakpointController;
}

VariableController *DebugSession::variableController() const
{
    return m_variableController;
}

GdbFrameStackModel *DebugSession::frameStackModel() const
{
    return m_frameStackModel;
}

GdbDebugger *DebugSession::createDebugger() const
{
    return new GdbDebugger;
}

void DebugSession::initializeDebugger()
{
    //addCommand(new GDBCommand(GDBMI::EnableTimings, "yes"));

    addCommand(
        std::make_unique<CliCommand>(MI::GdbShow, QStringLiteral("version"), this, &DebugSession::handleVersion));

    // This makes gdb pump a variable out on one line.
    addCommand(MI::GdbSet, QStringLiteral("width 0"));
    addCommand(MI::GdbSet, QStringLiteral("height 0"));

    addCommand(MI::SignalHandle, QStringLiteral("SIG32 pass nostop noprint"));
    addCommand(MI::SignalHandle, QStringLiteral("SIG41 pass nostop noprint"));
    addCommand(MI::SignalHandle, QStringLiteral("SIG42 pass nostop noprint"));
    addCommand(MI::SignalHandle, QStringLiteral("SIG43 pass nostop noprint"));

    addCommand(MI::EnablePrettyPrinting);

    addCommand(MI::GdbSet, QStringLiteral("charset UTF-8"));
    addCommand(MI::GdbSet, QStringLiteral("print sevenbit-strings off"));

    QString fileName = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                              QStringLiteral("kdevgdb/printers/gdbinit"));
    if (!fileName.isEmpty()) {
        QFileInfo fileInfo(fileName);
        QString quotedPrintersPath = fileInfo.dir().path()
                                             .replace(QLatin1Char('\\'), QLatin1String("\\\\"))
                                             .replace(QLatin1Char('"'), QLatin1String("\\\""));
        addCommand(MI::NonMI,
                   QStringLiteral("python sys.path.insert(0, \"%0\")").arg(quotedPrintersPath));
        addCommand(MI::NonMI, QLatin1String("source ") + fileName);
    }

    // GDB can't disable ASLR on CI server.
    addCommand(MI::GdbSet,
               QStringLiteral("disable-randomization %1").arg(m_autoDisableASLR ? QLatin1String("on") : QLatin1String("off")));

    qCDebug(DEBUGGERGDB) << "Initialized GDB";
}

void DebugSession::configInferior(ILaunchConfiguration *cfg, IExecutePlugin *iexec, const QString &)
{
    // Read Configuration values
    KConfigGroup grp = cfg->config();
    bool breakOnStart = grp.readEntry(KDevMI::Config::BreakOnStartEntry, false);
    bool displayStaticMembers = grp.readEntry(Config::StaticMembersEntry, false);
    bool asmDemangle = grp.readEntry(Config::DemangleNamesEntry, true);

    if (breakOnStart) {
        BreakpointModel* m = ICore::self()->debugController()->breakpointModel();
        bool found = false;
        const auto breakpoints = m->breakpoints();
        for (Breakpoint* b : breakpoints) {
            if (b->location() == QLatin1String("main")) {
                found = true;
                break;
            }
        }
        if (!found) {
            m->addCodeBreakpoint(QStringLiteral("main"));
        }
    }
    // Needed so that breakpoint widget has a chance to insert breakpoints.
    // FIXME: a bit hacky, as we're really not ready for new commands.
    setDebuggerStateOn(s_dbgBusy);
    raiseEvent(debugger_ready);

    if (displayStaticMembers) {
        addCommand(MI::GdbSet, QStringLiteral("print static-members on"));
    } else {
        addCommand(MI::GdbSet, QStringLiteral("print static-members off"));
    }

    if (asmDemangle) {
        addCommand(MI::GdbSet, QStringLiteral("print asm-demangle on"));
    } else {
        addCommand(MI::GdbSet, QStringLiteral("print asm-demangle off"));
    }

    // Set the environment variables
    const EnvironmentProfileList environmentProfiles(KSharedConfig::openConfig());
    QString envProfileName = iexec->environmentProfileName(cfg);
    if (envProfileName.isEmpty()) {
        qCWarning(DEBUGGERGDB) << i18n("No environment profile specified, looks like a broken "
                                       "configuration, please check run configuration '%1'. "
                                       "Using default environment profile.", cfg->name());
        envProfileName = environmentProfiles.defaultProfileName();
    }
    const auto& envvars = environmentProfiles.createEnvironment(envProfileName, {});
    for (const auto& envvar : envvars) {
        addCommand(GdbSet, QLatin1String("environment ") + envvar);
    }

    qCDebug(DEBUGGERGDB) << "Per inferior configuration done";
}

bool DebugSession::execInferior(KDevelop::ILaunchConfiguration *cfg, IExecutePlugin *, const QString &executable)
{
    qCDebug(DEBUGGERGDB) << "Executing inferior";

    KConfigGroup grp = cfg->config();
    QUrl configGdbScript = grp.readEntry(Config::RemoteGdbConfigEntry, QUrl());
    QUrl runShellScript = grp.readEntry(Config::RemoteGdbShellEntry, QUrl());
    QUrl runGdbScript = grp.readEntry(Config::RemoteGdbRunEntry, QUrl());

    // handle remote debug
    if (configGdbScript.isValid()) {
        addCommand(MI::NonMI, QLatin1String("source ") + configGdbScript.toLocalFile());
    }

    // FIXME: have a check box option that controls remote debugging
    if (runShellScript.isValid()) {
        // Special for remote debug, the remote inferior is started by this shell script
        const auto tty = m_tty->getSlave();
        const auto options = QString(QLatin1String(">") + tty + QLatin1String("  2>&1 <") + tty);

        const QStringList arguments {
            QStringLiteral("-c"),
            KShell::quoteArg(runShellScript.toLocalFile()) + QLatin1Char(' ') + KShell::quoteArg(executable) + options,
        };

        qCDebug(DEBUGGERGDB) << "starting sh" << arguments;
        QProcess::startDetached(QStringLiteral("sh"), arguments);
    }

    if (runGdbScript.isValid()) {
        // Special for remote debug, gdb script at run is requested, to connect to remote inferior

        // Race notice: wait for the remote gdbserver/executable
        // - but that might be an issue for this script to handle...

        // Note: script could contain "run" or "continue"

        // Future: the shell script should be able to pass info (like pid)
        // to the gdb script...

        addCommand(std::make_unique<SentinelCommand>(
            [this, runGdbScript]() {
                breakpointController()->initSendBreakpoints();

                breakpointController()->setDeleteDuplicateBreakpoints(true);
                qCDebug(DEBUGGERGDB) << "Running gdb script " << KShell::quoteArg(runGdbScript.toLocalFile());

                addCommand(
                    MI::NonMI, QLatin1String("source ") + runGdbScript.toLocalFile(),
                    [this](const MI::ResultRecord&) { breakpointController()->setDeleteDuplicateBreakpoints(false); },
                    CmdMaybeStartsRunning);
                raiseEvent(connected_to_program);
            },
            CmdMaybeStartsRunning));
    } else {
        // normal local debugging
        addCommand(MI::FileExecAndSymbols, KShell::quoteArg(executable),
                   this, &DebugSession::handleFileExecAndSymbols,
                   CmdHandlesError);
        raiseEvent(connected_to_program);

        addCommand(std::make_unique<SentinelCommand>(
            [this]() {
                breakpointController()->initSendBreakpoints();
                addCommand(MI::ExecRun, QString(), CmdMaybeStartsRunning);
            },
            CmdMaybeStartsRunning));
    }
    return true;
}

bool DebugSession::loadCoreFile(KDevelop::ILaunchConfiguration*,
                                const QString& debugee, const QString& corefile)
{
    addCommand(MI::FileExecAndSymbols, debugee,
               this, &DebugSession::handleFileExecAndSymbols,
               CmdHandlesError);
    raiseEvent(connected_to_program);

    addCommand(NonMI, QLatin1String("core ") + corefile,
               this, &DebugSession::handleCoreFile,
               CmdHandlesError);
    return true;
}

void DebugSession::handleVersion(const QStringList& s)
{
    static const QVersionNumber minRequiredVersion(7, 0, 0);
    static const QRegularExpression versionRegExp(QStringLiteral("([0-9]+)\\.([0-9]+)(\\.([0-9]+))?"));
    QString detectedVersion = i18n("<unknown version>");

    for (const QString& response : s) {
        qCDebug(DEBUGGERGDB) << response;

        if (!response.contains(QLatin1String{"GNU gdb"})) {
            continue; // this line is not a version string, skip it
        }

        const auto match = versionRegExp.match(response);
        if (match.hasMatch() && QVersionNumber::fromString(match.capturedView()) >= minRequiredVersion) {
            return; // Early exit. Version check passed.
        }

        detectedVersion = response;
    }

    if (!qobject_cast<QGuiApplication*>(qApp)) {
        //for unittest
        qFatal("You need a graphical application.");
    }

    // TODO: reuse minRequiredVersion in the error message text when the minimum
    // required GDB version changes or the message is modified for some other reason.
    const QString messageText = i18n("<b>You need gdb 7.0.0 or higher.</b><br />"
        "You are using: %1",
        detectedVersion);
    stopDebuggerOnError(messageText);
}

void DebugSession::handleFileExecAndSymbols(const ResultRecord& r)
{
    if (r.isReasonError()) {
        const QString messageText =
            i18n("<b>Could not start debugger:</b><br />")+
            r.errorMessage();
        stopDebuggerOnError(messageText);
    }
}

void DebugSession::handleCoreFile(const ResultRecord& r)
{
    if (r.isReasonError()) {
        failedToLoadCoreFile(r.errorMessage());
        return;
    }
    coreFileLoaded();
}

#include "moc_debugsession.cpp"
