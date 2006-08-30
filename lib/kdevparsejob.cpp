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

#include "kdevparsejob.h"

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

#include "editor/kdeveditorintegrator.h"
#include <kdevdocumentcontroller.h>

#include "kdevbackgroundparser.h"
#include "kdevparserdependencypolicy.h"

KDevParseJob::KDevParseJob( const KUrl &url,
                            QObject *parent )
        : ThreadWeaver::JobSequence( parent ),
        m_document( url ),
        m_openDocument( 0 ),
        m_backgroundParser( 0 ),
        m_abortMutex(new QMutex),
        m_abortRequested( false ),
        m_aborted( false )
{}

KDevParseJob::KDevParseJob( KDevDocument *document,
                            QObject *parent )
        : ThreadWeaver::JobSequence( parent ),
        m_document( document->url() ),
        m_openDocument( document ),
        m_backgroundParser( 0 ),
        m_abortMutex(new QMutex),
        m_abortRequested( false ),
        m_aborted( false )
{}

KDevParseJob::~KDevParseJob()
{}

const KUrl& KDevParseJob::document() const
{
    return m_document;
}

KDevDocument* KDevParseJob::openDocument() const
{
    return m_openDocument;
}

bool KDevParseJob::success() const
{
    return !m_aborted;
}

const QString & KDevParseJob::errorMessage() const
{
    return m_errorMessage;
}

TopDUContext* KDevParseJob::duChain() const
{
    // No definition-use chain available by default
    return 0L;
}

bool KDevParseJob::contentsAvailableFromEditor() const
{
    return m_openDocument && m_openDocument->textDocument() && KDevEditorIntegrator::documentLoaded(m_openDocument->textDocument());
}

QString KDevParseJob::contentsFromEditor() const
{
    return m_openDocument->textDocument()->text();
}

void KDevParseJob::setErrorMessage(const QString& message)
{
    m_errorMessage = message;
}

int KDevParseJob::priority() const
{
    if (m_openDocument)
        if (m_openDocument->isActive())
            return 2;
        else
            return 1;
    else
        return 0;
}

void KDevParseJob::addJob(Job* job)
{
    if (backgroundParser())
        job->assignQueuePolicy(backgroundParser()->dependencyPolicy());

    JobSequence::addJob(job);
}

KDevBackgroundParser* KDevParseJob::backgroundParser() const
{
    return m_backgroundParser;
}

void KDevParseJob::setBackgroundParser(KDevBackgroundParser* parser)
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

bool KDevParseJob::addDependency(KDevParseJob* dependency, ThreadWeaver::Job* actualDependee)
{
    if (!backgroundParser())
        return false;

    return backgroundParser()->dependencyPolicy()->addDependency(dependency, this, actualDependee);
}

bool KDevParseJob::abortRequested() const
{
    QMutexLocker lock(m_abortMutex);

    return m_abortRequested;
}

void KDevParseJob::requestAbort()
{
    QMutexLocker lock(m_abortMutex);

    m_abortRequested = true;
}

void KDevParseJob::abortJob()
{
    m_aborted = true;
    setFinished(true);
}

#include "kdevparsejob.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
