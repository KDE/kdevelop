/* This file is part of the KDE project
   Copyright 2002 John Firebaugh <jfirebaugh@kde.org>
   Copyright 2007 Andreas Pakulat <apaku@gmx.de>
   Copyright 2007 Oswald Buddenhagen <ossi@kde.org>

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

#include <QtCore/QObject>
#include "utilexport.h"

/**
@file processlinemaker.h
Utility objects for process output views.
*/

class QProcess;

class QStringList;

/**
Convenience class to catch output of QProcess.
*/

namespace KDevelop
{

class KDEVPLATFORMUTIL_EXPORT ProcessLineMaker : public QObject
{
    Q_OBJECT

public:
    ProcessLineMaker( QObject* parent = 0 );
    ProcessLineMaker( QProcess* process, QObject* parent = 0 );
    
    ~ProcessLineMaker();

    /**
     * clears out the internal buffers, this drops any data without
     * emitting the related signal
     */
    void discardBuffers();
    /**
     * Flush the data from the buffers and then clear them. 
     * This should be called once when the process has 
     * exited to make sure all data that was received from the
     * process is properly converted and emitted.
     *
     * Note: Connecting this class to the process finished signal
     * is not going to work, as the user of this class will do
     * that itself too and possibly delete the process, making 
     * it impossible to fetch the last output.
     */
    void flushBuffers();

public Q_SLOTS:
    /**
     * This should be used (instead of hand-crafted code) when
     * you need to do custom things with the process output
     * before feeding it to the linemaker and have it convert
     * it to QString lines.
     * @param buffer the output from the process
     */
    void slotReceivedStdout( const QByteArray& buffer );

    /**
     * This should be used (instead of hand-crafted code) when
     * you need to do custom things with the process error output
     * before feeding it to the linemaker and have it convert
     * it to QString lines.
     * @param buffer the output from the process
     */
    void slotReceivedStderr( const QByteArray& buffer );

Q_SIGNALS:
    /**
     * Emitted whenever the process prints something
     * to its standard output. The output is converted
     * to a QString using fromLocal8Bit() and will
     * be split on '\n'.
     * @param lines the lines that the process printed
     */
    void receivedStdoutLines( const QStringList& lines );

    /**
     * Emitted whenever the process prints something
     * to its error output. The output is converted
     * to a QString using fromLocal8Bit() and will
     * be split on '\n'.
     * @param lines the lines that the process printed
     */
    void receivedStderrLines( const QStringList& lines );

private:
    Q_PRIVATE_SLOT(d, void slotReadyReadStdout( ) )
    Q_PRIVATE_SLOT(d, void slotReadyReadStderr( ) )
    Q_PRIVATE_SLOT(d, void slotTimeoutStdout( ) )
    Q_PRIVATE_SLOT(d, void slotTimeoutStderr( ) )
    class ProcessLineMakerPrivate* const d;
    friend class ProcessLineMakerPrivate;
};

}

#endif
