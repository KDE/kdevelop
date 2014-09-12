/***************************************************************************
 *   This file was partly taken from cervisia's cvsservice                 *
 *   Copyright 2002-2003 Christian Loose <christian.loose@hamburg.de>      *
 *                                                                         *
 *   Adapted for KDevelop                                                  *
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cvsjob.h"

#include <QStringList>
#include <KDebug>
#include <KLocalizedString>

#include <interfaces/iplugin.h>
#include <QDir>

class CvsJobPrivate
{
public:
    QString     server;
    QString     rsh;
};


CvsJob::CvsJob(const QDir& workingDir, KDevelop::IPlugin* parent, KDevelop::OutputJob::OutputJobVerbosity verbosity)
    : DVcsJob(workingDir, parent, verbosity), d(new CvsJobPrivate)
{
}

CvsJob::CvsJob(KDevelop::IPlugin* parent, KDevelop::OutputJob::OutputJobVerbosity verbosity)
    : DVcsJob(QDir::home(), parent, verbosity), d(new CvsJobPrivate)
{
}

QString CvsJob::cvsCommand()
{
    return dvcsCommand().join(" ");
}

void CvsJob::clear()
{
    process()->clearEnvironment();
}

void CvsJob::setDirectory(const QString& directory)
{
    process()->setWorkingDirectory(directory);
}

QString CvsJob::getDirectory()
{
    return directory().absolutePath();
}

void CvsJob::setRSH(const QString& rsh)
{
    d->rsh = rsh;
}

void CvsJob::setServer(const QString& server)
{
    d->server = server;
}

void CvsJob::start()
{
    if( !d->rsh.isEmpty() ) {
        process()->setEnv("CVS_RSH", d->rsh);
    }

    if( !d->server.isEmpty() ) {
        process()->setEnv("CVS_SERVER", d->server);
    }

    DVcsJob::start();
}

