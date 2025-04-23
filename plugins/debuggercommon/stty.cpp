/*
    SPDX-FileCopyrightText: 1999 John Birch <jbb@kdevelop.org>

    This code was copied originally from the KDEStudio project:
    SPDX-FileCopyrightText: Judin Maxim

    It was then updated with later code from konsole (KDE).

    It has also been enhanced with an idea from the code in kdbg:
    SPDX-FileCopyrightText: Johannes Sixt<Johannes.Sixt@telecom.at>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef __osf__
#define _XOPEN_SOURCE_EXTENDED
#endif

#include <sys/types.h>
#ifndef _MSC_VER
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#endif
#include <sys/stat.h>

#ifdef HAVE_SYS_STROPTS_H
#include <sys/stropts.h>
#define _NEW_TTY_CTRL
#endif

#include <cassert>
#include <fcntl.h>
#ifndef _MSC_VER
#include <grp.h>
#include <termios.h>
#include <unistd.h>
#endif
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#if defined (_HPUX_SOURCE)
#define _TERMIOS_INCLUDED
#include <bsdtty.h>
#endif

#include <interfaces/icore.h>

#include <QSocketNotifier>
#include <QString>
#include <QFile>
#include <QPointer>
#include <QProcess>
#include <QTemporaryFile>

#include <KLocalizedString>
#include <KShell>

#include <QCoreApplication>
#include <QStandardPaths>

#include "stty.h"
#include "debuglog.h"

#include <memory>

#define PTY_FILENO 3
#define BASE_CHOWN "konsole_grantpty"

using namespace KDevMI;

namespace {

struct LaterDeleter
{
    void operator()(QObject* object) const
    {
        object->deleteLater();
    }
};

static int chownpty(int fd, int grant)
// param fd: the fd of a master pty.
// param grant: 1 to grant, 0 to revoke
// returns 1 on success 0 on fail
{
#ifndef Q_OS_WIN
    void(*tmp)(int) = signal(SIGCHLD,SIG_DFL);
    pid_t pid = fork();
    if (pid < 0) {
        signal(SIGCHLD,tmp);
        return 0;
    }
    if (pid == 0) {
        /* We pass the master pseudo terminal as file descriptor PTY_FILENO. */
        if (fd != PTY_FILENO && dup2(fd, PTY_FILENO) < 0)
            ::exit(1);

        QString path = QStandardPaths::findExecutable(QStringLiteral(BASE_CHOWN));
        const QByteArray encodedPath = QFile::encodeName(path);
        execle(encodedPath.constData(), BASE_CHOWN, grant?"--grant":"--revoke", (void *)nullptr, NULL);
        ::exit(1); // should not be reached
    }
    if (pid > 0) {
        int w;
        //  retry:
        int rc = waitpid (pid, &w, 0);
        if (rc != pid)
            ::exit(1);

        //    { // signal from other child, behave like catchChild.
        //      // guess this gives quite some control chaos...
        //      Shell* sh = shells.indexOf(rc);
        //      if (sh) { shells.remove(rc); sh->doneShell(w); }
        //      goto retry;
        //    }
        signal(SIGCHLD,tmp);
        return (rc != -1 && WIFEXITED(w) && WEXITSTATUS(w) == 0);
    }
    signal(SIGCHLD,tmp);
#endif
    return 0; //dummy.
}

} // unnamed namespace

// **************************************************************************

STTY::STTY(const QStringList& externalTerminal)
    : external_{!externalTerminal.empty()}
{
    if (external_) {
        findExternalTTY(externalTerminal);
        return;
    }

    fout = findTTY();
    if (fout >= 0) {
        ttySlave = QString::fromLatin1(tty_slave);
        out = new QSocketNotifier(fout, QSocketNotifier::Read, this);
#ifndef Q_OS_WIN
        // This connection does not compile on Windows and produces the following error since Qt 6:
        // error C2338: static_assert failed: 'Signal and slot arguments are not compatible (narrowing)'
        // That's because the first argument of QSocketNotifier::activated() is QSocketDescriptor,
        // which is implicitly convertible to int on non-Windows platforms, to Qt::HANDLE and
        // qintptr on Windows; and STTY::OutReceived() takes an int argument.
        // The entire definition of OutReceived() is disabled on Windows,
        // so just disable the connection on this platform too.
        connect(out, &QSocketNotifier::activated, this, &STTY::OutReceived);
#endif
    }
}

// **************************************************************************

STTY::~STTY()
{
#ifndef Q_OS_WIN
    if (out) {
        ::close(fout);
        delete out;
    }
#endif
}

// **************************************************************************

int STTY::findTTY()
{
    int ptyfd = -1;
    bool needGrantPty = true;
#ifndef Q_OS_WIN
    // Find a master pty that we can open ////////////////////////////////

#ifdef __sgi__
    ptyfd = open("/dev/ptmx",O_RDWR);
#elif defined(Q_OS_MAC) || defined(Q_OS_FREEBSD)
    ptyfd = posix_openpt(O_RDWR);
#endif
#if defined(__sgi__) || defined(Q_OS_MAC) || defined(Q_OS_FREEBSD)
    if (ptyfd == -1) {
        perror("Can't open a pseudo teletype");
        return(-1);
    } else if (ptyfd >= 0) {
        strncpy(tty_slave, ptsname(ptyfd), 50);
        grantpt(ptyfd);
        unlockpt(ptyfd);
        needGrantPty = false;
    }
#endif

    // first we try UNIX PTY's
#if defined(TIOCGPTN) && !defined(Q_OS_FREEBSD)
    strcpy(pty_master,"/dev/ptmx");
    strcpy(tty_slave,"/dev/pts/");
    ptyfd = open(pty_master,O_RDWR);
    if (ptyfd >= 0) { // got the master pty
        int ptyno;
        if (ioctl(ptyfd, TIOCGPTN, &ptyno) == 0) {
            struct stat sbuf;
            sprintf(tty_slave,"/dev/pts/%d",ptyno);
            if (stat(tty_slave,&sbuf) == 0 && S_ISCHR(sbuf.st_mode))
                needGrantPty = false;
            else {
                close(ptyfd);
                ptyfd = -1;
            }
        } else {
            close(ptyfd);
            ptyfd = -1;
        }
    }
#endif

#if defined(_SCO_DS) || defined(__USLC__) /* SCO OSr5 and UnixWare */
    if (ptyfd < 0) {
        for (int idx = 0; idx < 256; idx++)
            { sprintf(pty_master, "/dev/ptyp%d", idx);
            sprintf(tty_slave, "/dev/ttyp%d", idx);
            if (access(tty_slave, F_OK) < 0) { idx = 256; break; }
            if ((ptyfd = open (pty_master, O_RDWR)) >= 0)
                { if (access (tty_slave, R_OK|W_OK) == 0) break;
                close(ptyfd); ptyfd = -1;
                }
            }
    }
#endif
    if (ptyfd < 0) { /// \FIXME Linux, Trouble on other systems?
        for (const char* s3 = "pqrstuvwxyzabcde"; *s3 != 0; s3++) {
            for (const char* s4 = "0123456789abcdef"; *s4 != 0; s4++) {
                sprintf(pty_master,"/dev/pty%c%c",*s3,*s4);
                sprintf(tty_slave,"/dev/tty%c%c",*s3,*s4);
                if ((ptyfd = open(pty_master, O_RDWR)) >= 0) {
                    if (geteuid() == 0 || access(tty_slave, R_OK|W_OK) == 0)
                        break;

                    close(ptyfd);
                    ptyfd = -1;
                }
            }

            if (ptyfd >= 0)
                break;
        }
    }

    if (ptyfd >= 0) {
        if (needGrantPty && !chownpty(ptyfd, true)) {
            fprintf(stderr,"kdevelop: chownpty failed for device %s::%s.\n",pty_master,tty_slave);
            fprintf(stderr,"        : This means the session can be eavesdroped.\n");
            fprintf(stderr,"        : Make sure konsole_grantpty is installed and setuid root.\n");
        }

        ::fcntl(ptyfd, F_SETFL, O_NONBLOCK);
#ifdef TIOCSPTLCK
        int flag = 0;
        ioctl(ptyfd, TIOCSPTLCK, &flag); // unlock pty
#endif
    }
    if (ptyfd==-1) {
        m_lastError = i18n("Cannot use the tty* or pty* devices.\n"
                                    "Check the settings on /dev/tty* and /dev/pty*\n"
                                    "As root you may need to \"chmod ug+rw\" tty* and pty* devices "
                                    "and/or add the user to the tty group using "
                                    "\"usermod -aG tty username\".");
    }
#endif
    return ptyfd;
}

// **************************************************************************

void STTY::OutReceived(int f)
{
#ifndef Q_OS_WIN
    char buf[1024];
    int n;

    // read until socket is empty. We shouldn't be receiving a continuous
    // stream of data, so the loop is unlikely to cause problems.
    while ((n = ::read(f, buf, sizeof(buf)-1)) > 0) {
        *(buf+n) = 0;         // a standard string
        QByteArray ba(buf);
        emit OutOutput(ba);
    }
    // Note: for some reason, n can be 0 here.
    // I can understand that non-blocking read returns 0,
    // but I don't understand how OutReceived can be even
    // called when there's no input.
    if (n == 0 /* eof */
        || (n == -1 && errno != EAGAIN))
    {
        // Found eof or error. Disable socket notifier, otherwise Qt
        // will repeatedly call this method, eating CPU
        // cycles.
        out->setEnabled(false);
    }
#endif
}

void STTY::readRemaining()
{
    if (!external_)
        OutReceived(fout);
}

void STTY::findExternalTTY(QStringList terminalCommandLine)
{
#ifndef Q_OS_WIN
    Q_ASSERT(!terminalCommandLine.empty());
    const auto appName = terminalCommandLine.takeFirst();

    if (QStandardPaths::findExecutable(appName).isEmpty()) {
        m_lastError = i18n("%1 is incorrect terminal name", appName);
        return;
    }

    QTemporaryFile file;
    if (!file.open()) {
        m_lastError = i18n("Can't create a temporary file");
        return;
    }

    std::unique_ptr<QProcess, LaterDeleter> externalTerminal(new QProcess);

    terminalCommandLine << QStringLiteral("sh") << QStringLiteral("-c")
                        << QLatin1String("tty>") + file.fileName()
            + QLatin1String(";exec<&-;exec>&-;while :;do sleep 3600;done");

    externalTerminal->start(appName, terminalCommandLine);

    if (!externalTerminal->waitForStarted(500)) {
        m_lastError = QLatin1String("Can't run terminal: ") + appName;
        externalTerminal->terminate();
        return;
    }

    for (int i = 0; i < 800; i++) {
        if (!file.bytesAvailable()) {
            if (externalTerminal->state() == QProcess::NotRunning && externalTerminal->exitCode()) {
                break;
            }
            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
            usleep(8000);
        } else {
            qCDebug(DEBUGGERCOMMON) << "Received terminal output(tty)";
            break;
        }
    }

    usleep(1000);
    ttySlave = QString::fromUtf8(file.readAll().trimmed());

    file.close();

    if (ttySlave.isEmpty()) {
        m_lastError = i18n(
            "Can't receive %1 tty/pty. Check that %1 is actually a terminal and that it accepts these arguments: %2",
            appName, KShell::joinArgs(terminalCommandLine));
        return;
    }

    if (externalTerminal->state() == QProcess::NotRunning) {
        // This branch may be taken in case of inadequate external terminal command configuration.
        // For example, it is sometimes (though not always) taken in case of a disadvantageous modification
        // of a standard external terminal command, such as `xfce4-terminal --hold -x` or `gnome-terminal --`.
        // Do not warn about this, because such a behavior is not necessarily problematic for every use case.
        qCDebug(DEBUGGERCOMMON) << "the external terminal process finished unexpectedly early";
        // Let the destruction of externalTerminal invoke deleteLater() on the no longer needed QProcess object.
        return;
    }

    // The external terminal process is running properly. Destroy the QProcess object only when its process exits.
    auto* const terminalProcess = externalTerminal.release();
    connect(terminalProcess, &QProcess::finished, terminalProcess, &QObject::deleteLater);

    // Set the Core as the parent of the QProcess object in order to destroy it on KDevelop exit and prevent a leak.
    auto* const core = KDevelop::ICore::self();
    terminalProcess->setParent(core);
    // Terminate the external terminal process when KDevelop exit is imminent. Usually the process terminates
    // gracefully in time, though occasionally ~QProcess() prints a warning and kills it when the Core is destroyed.
    connect(core, &KDevelop::ICore::aboutToShutdown, terminalProcess, [sttyGuard = QPointer{this}, terminalProcess] {
        // Terminate the external terminal process only after its STTY creator is
        // destroyed, and consequently the associated debug session no longer uses it.
        if (const auto* const stty = sttyGuard.get()) {
            connect(stty, &QObject::destroyed, terminalProcess, &QProcess::terminate);
        } else {
            terminalProcess->terminate();
        }
    });
#endif
}
// **************************************************************************

#include "moc_stty.cpp"
