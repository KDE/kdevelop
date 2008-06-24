/*
* This file is part of KDevelop
*
* Copyright 2006 Adam Treat <treat@kde.org>
* Copyright 2006-2008 Hamish Rodda <rodda@kde.org>
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
#include "hashedstring.h"

#include "backgroundparser.h"
#include "parserdependencypolicy.h"

using namespace KTextEditor;

namespace KDevelop
{

struct ParseJobPrivate
{
    ParseJobPrivate(const KUrl& url)
        : duContext(0)
        , document( IndexedString(url.pathOrUrl()) )
        , backgroundParser( 0 )
        , abortMutex(new QMutex)
        , revisionToken(-1)
        , abortRequested( false )
        , aborted( false )
    {
    }

    ~ParseJobPrivate()
    {
        delete abortMutex;
    }

    TopDUContext* duContext;

    KDevelop::IndexedString document;
    QString errorMessage;
    BackgroundParser* backgroundParser;

    QMutex* abortMutex;
    int revisionToken;

    volatile bool abortRequested : 1;
    bool aborted : 1;
};

ParseJob::ParseJob( const KUrl &url,
                            QObject *parent )
        : ThreadWeaver::JobSequence( parent ),
        d(new ParseJobPrivate(url))
{}

ParseJob::~ParseJob()
{
    delete d;
}

IndexedString ParseJob::document() const
{
    return d->document;
}

bool ParseJob::success() const
{
    return !d->aborted;
}

void ParseJob::setDuChain(KDevelop::TopDUContext* duChain)
{
    d->duContext = duChain;
}

TopDUContext* ParseJob::duChain() const
{
    return d->duContext;
}

bool ParseJob::contentsAvailableFromEditor()
{
    KTextEditor::Document* doc = EditorIntegrator::documentForUrl(HashedString(d->document.str()));
    if (!doc)
        return false;

    finaliseChangedRanges();

    if (d->revisionToken == -1) {
        SmartInterface* iface = qobject_cast<SmartInterface*>(doc);
        if (iface) {
            QMutexLocker smartLock(iface->smartMutex());
            d->revisionToken = EditorIntegrator::saveCurrentRevision(doc);
}
    }

    return true;
}

int ParseJob::revisionToken() const
{
    return d->revisionToken;
}

QString ParseJob::contentsFromEditor()
{
    KTextEditor::Document* doc = EditorIntegrator::documentForUrl(HashedString(d->document.str()));
    if( !doc )
        return QString();

    // You must have called contentsAvailableFromEditor, it sets state
    QMutexLocker l(changeMutex());
    rangeChangesFinalised();

    return doc->text();
}

int ParseJob::priority() const
{
    ///@todo adymo: reenable after documentcontroller is ported
    return 0;
/*    if (d->openDocument)
        if (d->openDocument->isActive())
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
    return d->backgroundParser;
}

void ParseJob::setBackgroundParser(BackgroundParser* parser)
{
    if (parser) {
        assignQueuePolicy(parser->dependencyPolicy());

        for (int i = 0; i < jobListLength(); ++i)
            jobAt(i)->assignQueuePolicy(parser->dependencyPolicy());

    } else if (d->backgroundParser) {

        removeQueuePolicy(d->backgroundParser->dependencyPolicy());

        for (int i = 0; i < jobListLength(); ++i)
            jobAt(i)->removeQueuePolicy(d->backgroundParser->dependencyPolicy());
    }

    d->backgroundParser = parser;
}

bool ParseJob::addDependency(ParseJob* dependency, ThreadWeaver::Job* actualDependee)
{
    if (!backgroundParser())
        return false;

    return backgroundParser()->dependencyPolicy()->addDependency(dependency, this, actualDependee);
}

bool ParseJob::abortRequested() const
{
    QMutexLocker lock(d->abortMutex);

    return d->abortRequested;
}

void ParseJob::requestAbort()
{
    QMutexLocker lock(d->abortMutex);

    d->abortRequested = true;
}

void ParseJob::abortJob()
{
    d->aborted = true;
    setFinished(true);
}

}

#include "parsejob.moc"

