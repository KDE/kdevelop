/*
    SPDX-FileCopyrightText: 2008 Manuel Breugelmans <mbr.nxi@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_KDEVSIGNALSPY_H
#define KDEVPLATFORM_KDEVSIGNALSPY_H

#include <QObject>
#include "testsexport.h"

class QEventLoop;
class QTimer;

namespace KDevelop {
/*! A signal spy which exits the event loop when the signal is called,
 *  and remembers that the signal was emitted.
 *  adapted version of kdelibs/kdecore/utils/qtest_kde.cpp */
class KDEVPLATFORMTESTS_EXPORT KDevSignalSpy
    : public QObject
{
    Q_OBJECT

public:
    /*! Constructor. @p obj the object that is expected to emit @p signal. */
    KDevSignalSpy(QObject* obj, const char* signal,
                  Qt::ConnectionType ct = Qt::AutoConnection);

    /*! Blocks until either the expected signal has been emitted or
     *  @p timeout milliseconds have passed. */
    bool wait(int timeout);

private Q_SLOTS:
    void signalEmitted();

private:
    QObject* m_obj;
    bool m_emitted;
    QEventLoop* m_loop;
    QTimer* m_timer;
};
} // KDevelop

#endif
