/*
    SPDX-FileCopyrightText: 2014 Sven Brauch <svenbrauch@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

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
    if (!codec) {
        codec = QTextCodec::codecForLocale();
    }

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

#include "moc_gitplugincheckinrepositoryjob.cpp"
