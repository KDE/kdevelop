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

#include "../editor/editorintegrator.h"
#include "../editor/hashedstring.h"

#include "backgroundparser.h"
#include "parserdependencypolicy.h"
#include "duchain/topducontext.h"

#include "duchain/duchainlock.h"
#include "duchain/duchain.h"
#include "duchain/parsingenvironment.h"


using namespace KTextEditor;

Q_DECLARE_METATYPE(KDevelop::IndexedString)
Q_DECLARE_METATYPE(KDevelop::IndexedTopDUContext)
Q_DECLARE_METATYPE(KDevelop::ReferencedTopDUContext)

static QMutex minimumFeaturesMutex;
static QHash<KDevelop::IndexedString, QList<KDevelop::TopDUContext::Features> > staticMinimumFeatures;

namespace KDevelop
{

class ParseJobPrivate
{
public:

    ParseJobPrivate(const KUrl& url) :
          document( IndexedString(url.pathOrUrl()) )
        , backgroundParser( 0 )
        , abortMutex(new QMutex)
        , revisionToken(-1)
        , abortRequested( false )
        , aborted( false )
        , features( TopDUContext::VisibleDeclarationsAndContexts )
    {
    }

    ~ParseJobPrivate()
    {
        delete abortMutex;
    }

    ReferencedTopDUContext duContext;

    KDevelop::IndexedString document;
    QString errorMessage;
    BackgroundParser* backgroundParser;

    QMutex* abortMutex;
    int revisionToken;

    volatile bool abortRequested : 1;
    bool aborted : 1;
    TopDUContext::Features features;
    QString contentsFromEditor;
    QList<QPointer<QObject> > notify;
};

ParseJob::ParseJob( const KUrl &url )
        : ThreadWeaver::JobSequence(),
        d(new ParseJobPrivate(url))
{}

ParseJob::~ParseJob()
{
    if ( d->revisionToken != -1 ) {
        kWarning() << "You must call cleanupSmartRevision() when your run() method has finished!";
        cleanupSmartRevision();
    }

    typedef QPointer<QObject> QObjectPointer;
    foreach(const QObjectPointer &p, d->notify)
        if(p)
            QMetaObject::invokeMethod(p, "updateReady", Qt::QueuedConnection, Q_ARG(KDevelop::IndexedString, d->document), Q_ARG(KDevelop::ReferencedTopDUContext, d->duContext));

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

void ParseJob::setMinimumFeatures(TopDUContext::Features features)
{
    d->features = features;
}

bool ParseJob::hasStaticMinimumFeatures()
{
    QMutexLocker lock(&minimumFeaturesMutex);
    return ::staticMinimumFeatures.size();
}

TopDUContext::Features ParseJob::staticMinimumFeatures(IndexedString url)
{
    QMutexLocker lock(&minimumFeaturesMutex);
    TopDUContext::Features features = (TopDUContext::Features)0;
    
    if(::staticMinimumFeatures.contains(url))
        foreach(const TopDUContext::Features &f, ::staticMinimumFeatures[url])
            features = (TopDUContext::Features)(features | f);
    
    return features;
}

TopDUContext::Features ParseJob::minimumFeatures() const
{
    
    return (TopDUContext::Features)(d->features | staticMinimumFeatures(d->document));
}

void ParseJob::setDuChain(ReferencedTopDUContext duChain)
{
    d->duContext = duChain;
}

ReferencedTopDUContext ParseJob::duChain() const
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
            //Here we save the revision
            d->revisionToken = iface->currentRevision();
            iface->useRevision(d->revisionToken);

            // You must have called contentsAvailableFromEditor, it sets state

            d->contentsFromEditor = doc->text();
        }
    }

    return true;
}

void ParseJob::cleanupSmartRevision()
{
    if ( d->revisionToken != -1 ) {
        //Here we release the revision
        EditorIntegrator editor;
        editor.setCurrentUrl(d->document);

        if(KDevelop::LockedSmartInterface smart = editor.smart()) {
            smart->releaseRevision(d->revisionToken);
            smart->clearRevision();
            d->revisionToken = -1;
            ///@todo We need to know here what the currently used revision is, and assert that it still is being used
        }
    }
}

int ParseJob::revisionToken() const
{
    return d->revisionToken;
}

QString ParseJob::contentsFromEditor()
{
    return d->contentsFromEditor;
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
    cleanupSmartRevision();
}

void ParseJob::setNotifyWhenReady(QList<QPointer<QObject> > notify) {
    d->notify = notify;
}

void ParseJob::setStaticMinimumFeatures(IndexedString url, TopDUContext::Features features) {
    QMutexLocker lock(&minimumFeaturesMutex);
    ::staticMinimumFeatures[url].append(features);
}

void ParseJob::unsetStaticMinimumFeatures(IndexedString url, TopDUContext::Features features) {
    QMutexLocker lock(&minimumFeaturesMutex);
    ::staticMinimumFeatures[url].removeOne(features);
    if(::staticMinimumFeatures[url].isEmpty())
      ::staticMinimumFeatures.remove(url);
}

}

#include "parsejob.moc"

