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
class QProcess;

#include <QObject>
#include <QString>
#include <QScopedPointer>

namespace KDevMI {

class STTY : public QObject
{
    Q_OBJECT

public:
    explicit STTY(bool ext=false, const QString &termAppName=QString());
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
    bool findExternalTTY(const QString &termApp);

private:
    int fout;
    QSocketNotifier *out = nullptr;
    QString ttySlave;
    QString m_lastError;
    QScopedPointer<QProcess> m_externalTerminal;
    bool external_;

    char pty_master[50];  // "/dev/ptyxx" | "/dev/ptmx"
    char tty_slave[50];   // "/dev/ttyxx" | "/dev/pts/########..."
};

} // end of namespace KDevMI

#endif
