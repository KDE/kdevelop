/***************************************************************************
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
 *   Copyright 2009 Hugo Parente Lima <hugo.pl@gmail.com>                  *
 *   Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "gitplugincheckinrepositoryjob.h"

#include <KDebug>
#include <KTextEditor/Document>
#include <qtextcodec.h>
#include <QDir>

GitPluginCheckInRepositoryJob::GitPluginCheckInRepositoryJob(KTextEditor::Document* document,
                                                             const QString& rootDirectory)
    : CheckInRepositoryJob(document)
    , m_hashjob(0)
    , m_findjob(0)
    , m_rootDirectory(rootDirectory)
{}

void GitPluginCheckInRepositoryJob::start()
{
    const QTextCodec* codec = QTextCodec::codecForName(document()->encoding().toAscii());

    const QDir workingDirectory(m_rootDirectory);
    if ( !workingDirectory.exists() ) {
        emit finished(false);
        return;
    }

    m_findjob = new QProcess(this);
    m_findjob->setWorkingDirectory(m_rootDirectory);

    m_hashjob = new QProcess(this);
    m_hashjob->setWorkingDirectory(m_rootDirectory);
    m_hashjob->setStandardOutputProcess(m_findjob);

    connect(m_findjob, SIGNAL(finished(int)), SLOT(repositoryQueryFinished(int)));
    connect(m_hashjob, SIGNAL(error(QProcess::ProcessError)), SLOT(processFailed(QProcess::ProcessError)));
    connect(m_findjob, SIGNAL(error(QProcess::ProcessError)), SLOT(processFailed(QProcess::ProcessError)));

    m_hashjob->start("git", QStringList() << "hash-object" << "--stdin");
    m_findjob->start("git", QStringList() << "cat-file" << "--batch-check");

    for ( int i = 0; i < document()->lines(); i++ ) {
        m_hashjob->write(codec->fromUnicode(document()->line(i)));
        if ( i != document()->lines() - 1 ) {
            m_hashjob->write("\n");
        }
    }
    m_hashjob->closeWriteChannel();

}

GitPluginCheckInRepositoryJob::~GitPluginCheckInRepositoryJob()
{
    if ( m_findjob && m_findjob->state() == QProcess::Running ) {
        m_findjob->kill();
    }
    if ( m_hashjob && m_hashjob->state() == QProcess::Running ) {
        m_hashjob->kill();
    }
}

void GitPluginCheckInRepositoryJob::processFailed(QProcess::ProcessError err)
{
    kDebug() << "calling git failed with error:" << err;
    emit finished(false);
}

void GitPluginCheckInRepositoryJob::repositoryQueryFinished(int)
{
    const QByteArray output = m_findjob->readAllStandardOutput();
    bool requestSucceeded = output.contains(" blob ");
    emit finished(requestSucceeded);
}

