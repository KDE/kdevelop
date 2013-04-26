/*
 * This file is part of KDevelop
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef KDEVPLATFORM_KDEVSIGNALSPY_H
#define KDEVPLATFORM_KDEVSIGNALSPY_H

#include <QtCore/QObject>
#include "kdevplatformtestsexport.h"

class QEventLoop;
class QTimer;

namespace KDevelop
{

/*! A signal spy which exits the event loop when the signal is called,
 *  and remembers that the signal was emitted.
 *  adapted version of kdelibs/kdecore/utils/qtest_kde.cpp */
class KDEVPLATFORMTESTS_EXPORT KDevSignalSpy : public QObject
{
Q_OBJECT
public:
    /*! Constructor. @p obj the object that is expected to emit @p signal. */
    KDevSignalSpy(QObject *obj, const char *signal,
                  Qt::ConnectionType ct = Qt::AutoConnection );

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
