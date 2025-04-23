/*
    SPDX-FileCopyrightText: 1999 John Birch <jbb@kdevelop.org>

    This code was copied originally from the KDEStudio project:
    SPDX-FileCopyrightText: Judin Maxim

    It was then updated with later code from konsole (KDE).

    It has also been enhanced with an idea from the code in kdbg:
    SPDX-FileCopyrightText: Johannes Sixt<Johannes.Sixt@telecom.at>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _STTY_H_
#define _STTY_H_

class QSocketNotifier;

#include <QObject>
#include <QStringList>

namespace KDevMI {

class STTY : public QObject
{
    Q_OBJECT

public:
    /**
     * Create an STTY.
     *
     * @param externalTerminal a list that consists of an external terminal command and its arguments,
     *                         to which a program to execute and the program's arguments are to be appended;
     *                         if empty, @c pty and @c tty devices are managed internally, without an external terminal
     */
    explicit STTY(const QStringList& externalTerminal);
    ~STTY() override;

    ///Call it if getSlave returns an empty string.
    QString lastError(){return m_lastError;}
    QString getSlave()    { return ttySlave; };
    void readRemaining();

private Q_SLOTS:
    void OutReceived(int);

Q_SIGNALS:
    void OutOutput(const QByteArray&);

private:
    int findTTY();
    /**
     * Run @c tty in the specified external terminal.
     *
     * @param terminalCommandLine a nonempty list that consists of an external terminal command and its arguments,
     *                            to which a program to execute and the program's arguments are to be appended
     */
    void findExternalTTY(QStringList terminalCommandLine);

private:
    int fout;
    QSocketNotifier *out = nullptr;
    QString ttySlave;
    QString m_lastError;
    bool external_;

    char pty_master[50];  // "/dev/ptyxx" | "/dev/ptmx"
    char tty_slave[50];   // "/dev/ttyxx" | "/dev/pts/########..."
};

} // end of namespace KDevMI

#endif
