/***************************************************************************
 *   Copyright 2014 Sven Brauch <svenbrauch@gmail.com>                     *
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
#include "debug.h"

#include <KTextEditor/Document>

#include <QTextCodec>
#include <QDir>

GitPluginCheckInRepositoryJob::GitPluginCheckInRepositoryJob(KTextEditor::Document* document,
                                                             const QString& rootDirectory)
    : CheckInRepositoryJob(document)
    , m_hashjob(nullptr)
    , m_findjob(nullptr)
    , m_rootDirectory(rootDirectory)
{}

void GitPluginCheckInRepositoryJob::start()
{
    const QTextCodec* codec = QTextCodec::codecForName(document()->encoding().toLatin1());

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

    connect(m_findjob, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &GitPluginCheckInRepositoryJob::repositoryQueryFinished);
    connect(m_hashjob, &QProcess::errorOccurred, this, &GitPluginCheckInRepositoryJob::processFailed);
    connect(m_findjob, &QProcess::errorOccurred, this, &GitPluginCheckInRepositoryJob::processFailed);

    m_hashjob->start(QStringLiteral("git"), QStringList{QStringLiteral("hash-object"), QStringLiteral("--stdin")});
    m_findjob->start(QStringLiteral("git"), QStringList{QStringLiteral("cat-file"), QStringLiteral("--batch-check")});

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
    qCDebug(PLUGIN_GIT) << "calling git failed with error:" << err;
    emit finished(false);
}

void GitPluginCheckInRepositoryJob::repositoryQueryFinished(int)
{
    const QByteArray output = m_findjob->readAllStandardOutput();
    bool requestSucceeded = output.contains(" blob ");
    emit finished(requestSucceeded);
}

