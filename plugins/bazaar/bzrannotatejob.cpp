/***************************************************************************
 *   Copyright 2013-2014 Maciej Poleski                                    *
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

#include "bzrannotatejob.h"

#include <functional>

#include <QtCore/QTimer>
#include <QtCore/QDateTime>
#include <QtCore/QDir>

#include <vcs/dvcs/dvcsjob.h>
#include <vcs/vcsannotation.h>
#include <vcs/vcsrevision.h>
#include <interfaces/iplugin.h>

BzrAnnotateJob::BzrAnnotateJob(const QDir& workingDir, const QString& revisionSpec, const QUrl& localLocation, KDevelop::IPlugin* parent, KDevelop::OutputJob::OutputJobVerbosity verbosity)
    : VcsJob(parent, verbosity), m_workingDir(workingDir), m_revisionSpec(revisionSpec), m_localLocation(localLocation), m_vcsPlugin(parent), m_status(KDevelop::VcsJob::JobNotStarted)
{
    setType(JobType::Annotate);
    setCapabilities(Killable);
}

bool BzrAnnotateJob::doKill()
{
    m_status = KDevelop::VcsJob::JobCanceled;
    if (m_job)
        return m_job->kill(KJob::Quietly);
    else
        return true;
}

void BzrAnnotateJob::start()
{
    if (m_status != KDevelop::VcsJob::JobNotStarted)
        return;
    KDevelop::DVcsJob* job = new KDevelop::DVcsJob(m_workingDir, m_vcsPlugin, KDevelop::OutputJob::Silent);
    *job << "bzr" << "annotate" << "--all" << m_revisionSpec << m_localLocation;
    connect(job, &KDevelop::DVcsJob::readyForParsing, this, &BzrAnnotateJob::parseBzrAnnotateOutput);
    m_status = KDevelop::VcsJob::JobRunning;
    m_job = job;
    job->start();
}

void BzrAnnotateJob::parseBzrAnnotateOutput(KDevelop::DVcsJob* job)
{
    m_outputLines = job->output().split('\n');
    m_currentLine = 0;
    if (m_status == KDevelop::VcsJob::JobRunning)
        QTimer::singleShot(0, this, SLOT(parseNextLine()));
}

void BzrAnnotateJob::parseNextLine()
{
    for(;;)
    {
        Q_ASSERT(m_currentLine<=m_outputLines.size());
        if (m_currentLine == m_outputLines.size()) {
            m_status = KDevelop::VcsJob::JobSucceeded;
            emitResult();
            emit resultsReady(this);
            break;
        }
        QString currentLine = m_outputLines[m_currentLine];
        if (currentLine.isEmpty()) {
            ++m_currentLine;
            continue;
        }
        bool revOk;
        auto revision = currentLine.left(currentLine.indexOf(' ')).toULong(&revOk);
        if (!revOk) {
            // Future compatibility - not a revision yet
            ++m_currentLine;
            continue;
        }
        auto i = m_commits.find(revision);
        if (i != m_commits.end()) {
            KDevelop::VcsAnnotationLine line;
            line.setAuthor(i.value().author());
            line.setCommitMessage(i.value().message());
            line.setDate(i.value().date());
            line.setLineNumber(m_currentLine);
            line.setRevision(i.value().revision());
            m_results.append(QVariant::fromValue(line));
            ++m_currentLine;
            continue;
        } else {
            prepareCommitInfo(revision);
            break;  //Will reenter this function when commit info will be ready
        }
    }
}

void BzrAnnotateJob::prepareCommitInfo(std::size_t revision)
{
    if (m_status != KDevelop::VcsJob::JobRunning)
        return;
    KDevelop::DVcsJob* job = new KDevelop::DVcsJob(m_workingDir, m_vcsPlugin, KDevelop::OutputJob::Silent);
    job->setType(KDevelop::VcsJob::Log);
    *job << "bzr" << "log" << "--long" << "-r" << QString::number(revision);
    connect(job, &KDevelop::DVcsJob::readyForParsing, this, &BzrAnnotateJob::parseBzrLog);
    m_job = job;
    job->start();
}

/*
 * This is slightly different from BazaarUtils::parseBzrLogPart(...).
 * This function parses only commit general info. It does not parse signle
 * actions. In fact output parsed by this function is slightly different
 * from output parsed by BazaarUtils. As a result parsing this output using
 * BazaarUtils would yield different results.
 * NOTE: This is all about parsing 'message'.
 */
void BzrAnnotateJob::parseBzrLog(KDevelop::DVcsJob* job)
{
    QStringList outputLines = job->output().split('\n');
    KDevelop::VcsEvent commitInfo;
    int revision=-1;
    bool atMessage = false;
    QString message;
    for (const QString &line : outputLines) {
        if (!atMessage) {
            if (line.startsWith("revno")) {
                QString revno = line.mid(QString("revno: ").length());
                // In future there is possibility that "revno: " will change to
                // "revno??". If that's all, then we recover matching only
                // "revno" prefix and assuming placeholder of length 2 (": " or
                // "??").
                // The same below with exception of "commiter" which possibly
                // can have also some suffix which changes meaning like
                // "commiter-some_property: "...
                revno = revno.left(revno.indexOf(' '));
                revision = revno.toInt();
                KDevelop::VcsRevision revision;
                revision.setRevisionValue(revno.toLongLong(), KDevelop::VcsRevision::GlobalNumber);
                commitInfo.setRevision(revision);
            } else if (line.startsWith("committer: ")) {
                QString commiter = line.mid(QString("committer: ").length());
                commitInfo.setAuthor(commiter);     // Author goes after commiter, but only if is different
            } else if (line.startsWith("author")) {
                QString author = line.mid(QString("author: ").length());
                commitInfo.setAuthor(author);       // It may override commiter (In fact commiter is not supported by VcsEvent)
            } else if (line.startsWith("timestamp")) {
                const QString formatString = "yyyy-MM-dd hh:mm:ss";
                QString timestamp = line.mid(QString("timestamp: ddd ").length(), formatString.length());
                commitInfo.setDate(QDateTime::fromString(timestamp, formatString));
            } else if (line.startsWith("message")) {
                atMessage = true;
            }
        } else {
            message += line.trimmed() + "\n";
        }
    }
    if (atMessage)
        commitInfo.setMessage(message.trimmed());
    Q_ASSERT(revision!=-1);
    m_commits[revision] = commitInfo;
    // Invoke from event loop to protect against stack overflow (it could happen
    // on very big files with very big history of changes if tail-recursion
    // optimization had failed here).
    QTimer::singleShot(0, this, SLOT(parseNextLine()));
}

QVariant BzrAnnotateJob::fetchResults()
{
    return m_results;
}

KDevelop::VcsJob::JobStatus BzrAnnotateJob::status() const
{
    return m_status;
}

KDevelop::IPlugin* BzrAnnotateJob::vcsPlugin() const
{
    return m_vcsPlugin;
}

