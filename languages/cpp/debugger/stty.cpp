/***************************************************************************
    begin                : Mon Sep 13 1999
    copyright            : (C) 1999 by John Birch
    email                : jbb@kdevelop.org

  This code was originally written by Judin Maxim, from the
	KDEStudio project.

  It was then updated with later code from konsole (KDE).

	It has also been enhanced with an idea from the code in kdbg
	written by Johannes Sixt<Johannes.Sixt@telecom.at>

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef __osf__
#define _XOPEN_SOURCE_EXTENDED
#define O_NDELAY O_NONBLOCK
#endif

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

#ifdef HAVE_SYS_STROPTS_H
#include <sys/stropts.h>
#define _NEW_TTY_CTRL
#endif

#include <assert.h>
#include <fcntl.h>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#if defined (_HPUX_SOURCE)
#define _TERMIOS_INCLUDED
#include <bsdtty.h>
#endif

#include <qintdict.h>
#include <qsocketnotifier.h>
#include <qstring.h>
#include <qfile.h>

#include <klocale.h>
#include <kstandarddirs.h>
#include <kapplication.h>

#include "stty.h"

#define PTY_FILENO 3
#define BASE_CHOWN "konsole_grantpty"

namespace GDBDebugger
{

static int chownpty(int fd, int grant)
// param fd: the fd of a master pty.
// param grant: 1 to grant, 0 to revoke
// returns 1 on success 0 on fail
{
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

        QString path = locate("exe", BASE_CHOWN);
        execle(QFile::encodeName(path), BASE_CHOWN, grant?"--grant":"--revoke", (void *)0, NULL);
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
        //      Shell* sh = shells.find(rc);
        //      if (sh) { shells.remove(rc); sh->doneShell(w); }
        //      goto retry;
        //    }
        signal(SIGCHLD,tmp);
        return (rc != -1 && WIFEXITED(w) && WEXITSTATUS(w) == 0);
    }
    signal(SIGCHLD,tmp);
    return 0; //dummy.
}

// **************************************************************************

STTY::STTY(bool ext, const QString &termAppName)
    : QObject(),
      out(0),
      ttySlave(""),
      pid_(0),
      external_(ext)
{
    if (ext) {
        findExternalTTY(termAppName);
    } else {
        fout = findTTY();
        if (fout >= 0) {
            ttySlave = QString(tty_slave);
            out = new QSocketNotifier(fout, QSocketNotifier::Read, this);
            connect( out, SIGNAL(activated(int)), this, SLOT(OutReceived(int)) );
        }
    }
}

// **************************************************************************

STTY::~STTY()
{
    if (pid_)
        ::kill(pid_, SIGTERM);

    if (out) {
        ::close(fout);
        delete out;
    }
}

// **************************************************************************

int STTY::findTTY()
{
    int ptyfd = -1;
    bool needGrantPty = TRUE;

    // Find a master pty that we can open ////////////////////////////////

#ifdef __sgi__
    ptyfd = open("/dev/ptmx",O_RDWR);
    if (ptyfd < 0) {
        perror("Can't open a pseudo teletype");
        return(-1);
    }
    strncpy(tty_slave, ptsname(ptyfd), 50);
    grantpt(ptyfd);
    unlockpt(ptyfd);
    needGrantPty = FALSE;
#endif

    // first we try UNIX PTY's
#ifdef TIOCGPTN
    strcpy(pty_master,"/dev/ptmx");
    strcpy(tty_slave,"/dev/pts/");
    ptyfd = open(pty_master,O_RDWR);
    if (ptyfd >= 0) { // got the master pty
        int ptyno;
        if (ioctl(ptyfd, TIOCGPTN, &ptyno) == 0) {
            struct stat sbuf;
            sprintf(tty_slave,"/dev/pts/%d",ptyno);
            if (stat(tty_slave,&sbuf) == 0 && S_ISCHR(sbuf.st_mode))
                needGrantPty = FALSE;
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
        if (needGrantPty && !chownpty(ptyfd, TRUE)) {
            fprintf(stderr,"kdevelop: chownpty failed for device %s::%s.\n",pty_master,tty_slave);
            fprintf(stderr,"        : This means the session can be eavesdroped.\n");
            fprintf(stderr,"        : Make sure konsole_grantpty is installed and setuid root.\n");
        }

        ::fcntl(ptyfd, F_SETFL, O_NDELAY);
#ifdef TIOCSPTLCK
        int flag = 0;
        ioctl(ptyfd, TIOCSPTLCK, &flag); // unlock pty
#endif
    }

    return ptyfd;
}

// **************************************************************************

void STTY::OutReceived(int f)
{
    char buf[1024];
    int n;

    // read until socket is empty. We shouldn't be receiving a continuous
    // stream of data, so the loop is unlikely to cause problems.
    while ((n = ::read(f, buf, sizeof(buf)-1)) > 0) {
        *(buf+n) = 0;         // a standard string
        emit OutOutput(buf);
    }
    if (n == 0 || n == -1)
    {
        // Found eof or error. Disable socket notifier, otherwise Qt
        // will repeatedly call this method, eating CPU
        // cycles.
        out->setEnabled(false);
    }
    
}

void STTY::readRemaining()
{
    if (!external_)
        OutReceived(fout);
}

// **************************************************************************

#define FIFO_FILE "/tmp/debug_tty.XXXXXX"

bool STTY::findExternalTTY(const QString &termApp)
{
    QString appName(termApp.isEmpty() ? QString("xterm") : termApp);

    char fifo[] = FIFO_FILE;
    int fifo_fd;
    if ((fifo_fd = mkstemp(fifo)) == -1)
        return false;

    ::close(fifo_fd);
    ::unlink(fifo);

    // create a fifo that will pass in the tty name
#ifdef HAVE_MKFIFO
    if (::mkfifo(fifo, S_IRUSR|S_IWUSR) < 0)
#else
        if (::mknod(fifo, S_IFIFO | S_IRUSR|S_IWUSR, 0) < 0)
#endif
            return false;

    int pid = ::fork();
    if (pid < 0) {             // No process
        ::unlink(fifo);
        return false;
    }

    if (pid == 0) {            // child process
        /*
         * Spawn a console that in turn runs a shell script that passes us
         * back the terminal name and then only sits and waits.
         */

        const char* prog      = appName.latin1();
        QString script = QString("tty>") + QString(fifo) +
            QString(";"                  // fifo name
                    "trap \"\" INT QUIT TSTP;"	  // ignore various signals
                    "exec<&-;exec>&-;"		        // close stdin and stdout
                    "while :;do sleep 3600;done");
        const char* scriptStr = script.latin1();
        const char* end       = 0;

        if ( termApp == "konsole" )
        {
            ::execlp( prog,       prog,
                  "-caption", i18n("kdevelop: Debug application console").local8Bit().data(),
                  "-e",       "sh",
                  "-c",       scriptStr,
                  end);
        }
        else
        {        
            ::execlp( prog,       prog,
                  "-e",       "sh",
                  "-c",       scriptStr,
                  end);
        }

        // Should not get here, as above should always work
        ::exit(1);
    }

    // parent process
    if (pid <= 0)
        ::exit(1);

    // Open the communication between us (the parent) and the
    // child (the process running on a tty console)
    fifo_fd = ::open(fifo, O_RDONLY);
    if (fifo_fd < 0)
        return false;

    // Get the ttyname from the fifo buffer that the child process
    // has sent.
    char ttyname[50];
    int n = ::read(fifo_fd, ttyname, sizeof(ttyname)-sizeof(char));

    ::close(fifo_fd);
    ::unlink(fifo);

    // No name??
    if (n <= 0)
        return false;

    // remove whitespace
    ttyname[n] = 0;
    if (char* newline = strchr(ttyname, '\n'))
        *newline = 0;      // clobber the new line

    ttySlave = ttyname;
    pid_ = pid;

    return true;
}

}

// **************************************************************************
#include "stty.moc"
