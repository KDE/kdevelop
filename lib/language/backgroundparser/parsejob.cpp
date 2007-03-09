/*
* This file is part of KDevelop
*
* Copyright (c) 2006 Adam Treat <treat@kde.org>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU Library General Public License as
* published by the Free Software Foundation; either version 2 of the
* License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public
* License along with this program; if not, write to the
* Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "parsejob.h"

#include <cassert>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <QFile>
#include <QByteArray>
#include <QMutex>
#include <QMutexLocker>

#include <kdebug.h>
#include <klocale.h>

#include <ktexteditor/document.h>
#include <ktexteditor/smartinterface.h>

#include "editorintegrator.h"

#include "backgroundparser.h"
#include "parserdependencypolicy.h"

using namespace KTextEditor;

namespace KDevelop
{

ParseJob::ParseJob( const KUrl &url,
                            QObject *parent )
        : ThreadWeaver::JobSequence( parent ),
        m_document( url ),
        m_backgroundParser( 0 ),
        m_abortMutex(new QMutex),
        m_abortRequested( false ),
        m_aborted( false ),
        m_revisionToken(-1)
{}

ParseJob::~ParseJob()
{
    ///@todo adymo: reenable after documentcontroller is ported
/*    if (m_revisionToken != -1) {
        Q_ASSERT(m_openDocument);
        SmartInterface* smart = dynamic_cast<SmartInterface*>(m_openDocument->textDocument());
        Q_ASSERT(smart);
        smart->releaseRevision(m_revisionToken);
    }*/
}

const KUrl& ParseJob::document() const
{
    return m_document;
}

bool ParseJob::success() const
{
    return !m_aborted;
}

const QString & ParseJob::errorMessage() const
{
    return m_errorMessage;
}

// TopDUContext* ParseJob::duChain() const
// {
//     // No definition-use chain available by default
//     return 0L;
// }

bool ParseJob::contentsAvailableFromEditor() const
{
    ///@todo adymo: reenable after documentcontroller is ported
    return false;
//     return m_openDocument && m_openDocument->textDocument() &&
//         EditorIntegrator::documentLoaded(m_openDocument->textDocument());
}

int ParseJob::revisionToken() const
{
    return m_revisionToken;
}

QString ParseJob::contentsFromEditor(bool saveRevisionToken)
{
    ///@todo adymo: reenable after documentcontroller is ported
    return "";
/*    SmartInterface* smart = dynamic_cast<SmartInterface*>(m_openDocument->textDocument());

    QMutexLocker lock(smart ? smart->smartMutex() : 0);

    if (smart && saveRevisionToken) {
        m_revisionToken = smart->currentRevision();
    }

    return m_openDocument->textDocument()->text();*/
}

void ParseJob::setErrorMessage(const QString& message)
{
    m_errorMessage = message;
}

int ParseJob::priority() const
{
    ///@todo adymo: reenable after documentcontroller is ported
    return 0;
/*    if (m_openDocument)
        if (m_openDocument->isActive())
            return 2;
        else
            return 1;
    else
        return 0;*/
}

void ParseJob::addJob(Job* job)
{
    if (backgroundParser())
        job->assignQueuePolicy(backgroundParser()->dependencyPolicy());

    JobSequence::addJob(job);
}

BackgroundParser* ParseJob::backgroundParser() const
{
    return m_backgroundParser;
}

void ParseJob::setBackgroundParser(BackgroundParser* parser)
{
    if (parser) {
        assignQueuePolicy(parser->dependencyPolicy());

        for (int i = 0; i < jobListLength(); ++i)
            jobAt(i)->assignQueuePolicy(parser->dependencyPolicy());

    } else if (m_backgroundParser) {

        removeQueuePolicy(m_backgroundParser->dependencyPolicy());

        for (int i = 0; i < jobListLength(); ++i)
            jobAt(i)->removeQueuePolicy(m_backgroundParser->dependencyPolicy());
    }

    m_backgroundParser = parser;
}

bool ParseJob::addDependency(ParseJob* dependency, ThreadWeaver::Job* actualDependee)
{
    if (!backgroundParser())
        return false;

    return backgroundParser()->dependencyPolicy()->addDependency(dependency, this, actualDependee);
}

bool ParseJob::abortRequested() const
{
    QMutexLocker lock(m_abortMutex);

    return m_abortRequested;
}

void ParseJob::requestAbort()
{
    QMutexLocker lock(m_abortMutex);

    m_abortRequested = true;
}

void ParseJob::abortJob()
{
    m_aborted = true;
    setFinished(true);
}

}
#include "parsejob.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
