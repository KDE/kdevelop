/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef GREPVIEWPROCESS_H
#define GREPVIEWPROCESS_H

#include <kprocess.h>

/**
 * Simple KProcess which deletes itself when process is terminated.
 */
class GrepviewProcess : public KProcess
{
    Q_OBJECT

public:
    GrepviewProcess( QObject *parent );
    ~GrepviewProcess();

private Q_SLOTS:
    void slotFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void slotError( QProcess::ProcessError );

};

#endif
