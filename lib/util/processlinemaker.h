/* This file is part of the KDE project
   Copyright (C) 2002 John Firebaugh <jfirebaugh@kde.org>
   Copyright (C) 2007 Andreas Pakulat <apaku@gmx.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef _PROCESSLINEMAKER_H_
#define _PROCESSLINEMAKER_H_

#include <QObject>
#include "kdevexport.h"

/**
@file processlinemaker.h
Utility objects for process output views.
*/

class K3Process;

class QString;
class QStringList;

/**
Convenience class to catch output of K3Process.
*/

class KDEVPLATFORMUTIL_EXPORT ProcessLineMaker : public QObject
{
    Q_OBJECT

public:
    ProcessLineMaker();
    ProcessLineMaker( const K3Process* );
    void clearBuffers();

signals:
    void receivedStdoutLines( const QStringList& );
    void receivedStderrLines( const QStringList& );

private:
    Q_PRIVATE_SLOT(d, void slotReceivedStdout( K3Process *, char *buffer, int buflen ) )
    Q_PRIVATE_SLOT(d, void slotReceivedStderr( K3Process *, char *buffer, int buflen ) )
    class ProcessLineMakerPrivate* const d;
    friend class ProcessLineMakerPrivate;
};

#endif
// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
