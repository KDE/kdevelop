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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _PROCESSLINEMAKER_H_
#define _PROCESSLINEMAKER_H_

#include <QObject>
#include "kdevexport.h"

/**
@file processlinemaker.h
Utility objects for process output views.
*/

class QProcess;

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
    ProcessLineMaker( QProcess* );
    void clearBuffers();

public slots:
    void slotReceivedStdout( const QString& );
    void slotReceivedStderr( const QString& );
    void slotReceivedStdout( const char* );
    void slotReceivedStderr( const char* );

signals:
    void receivedStdoutLines( const QStringList& );
    void receivedStderrLines( const QStringList& );

private:
    Q_PRIVATE_SLOT(d, void slotReadyReadStdout( ) )
    Q_PRIVATE_SLOT(d, void slotReadyReadStderr( ) )
    class ProcessLineMakerPrivate* const d;
    friend class ProcessLineMakerPrivate;
};

#endif
// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
