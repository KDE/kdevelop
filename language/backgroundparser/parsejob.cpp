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

#include <QFile>
#include <QByteArray>
#include <QMutex>
#include <QMutexLocker>
#include <threadweaver/qobjectdecorator.h>
#include <QApplication>

#include <kdebug.h>
#include <KLocalizedString>
#include <KFormat>
#include <ktexteditor/movinginterface.h>

#include "backgroundparser.h"
#include "duchain/topducontext.h"

#include "duchain/duchainlock.h"
#include "duchain/duchain.h"
#include "duchain/parsingenvironment.h"

#include <util/foregroundlock.h>
#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <codegen/coderepresentation.h>
#include <duchain/declaration.h>
#include <duchain/use.h>
#include <interfaces/icodehighlighting.h>
#include <duchain/problem.h>


using namespace KTextEditor;

static QMutex minimumFeaturesMutex;
static QHash<KDevelop::IndexedString, QList<KDevelop::TopDUContext::Features> > staticMinimumFeatures;

namespace KDevelop
{

class ParseJobPrivate
{
public:

    ParseJobPrivate(const IndexedString& url_, ILanguageSupport* languageSupport_) :
          url( url_ )
        , languageSupport( languageSupport_ )
        , abortRequested( 0 )
        , hasReadContents( false )
        , aborted( false )
        , features( TopDUContext::VisibleDeclarationsAndContexts )
        , parsePriority( 0 )
        , sequentialProcessingFlags( ParseJob::IgnoresSequentialProcessing )
    {
    }

    ~ParseJobPrivate()
    {
    }

    ReferencedTopDUContext duContext;

    IndexedString url;
    ILanguageSupport* languageSupport;

    ParseJob::Contents contents;

    QAtomicInt abortRequested;

    bool hasReadContents : 1;
    bool aborted : 1;
    TopDUContext::Features features;
    QList<QPointer<QObject> > notify;
    QPointer<DocumentChangeTracker> tracker;
    RevisionReference revision;
    RevisionReference previousRevision;

    int parsePriority;
    ParseJob::SequentialProcessingFlags sequentialProcessingFlags;
    ThreadWeaver::QObjectDecorator* decorator;
};

ParseJob::ParseJob( const IndexedString& url, KDevelop::ILanguageSupport* languageSupport )
        : ThreadWeaver::Sequence(),
        d(new ParseJobPrivate(url, languageSupport))
{
    d->decorator = new ThreadWeaver::QObjectDecorator(this);
}

ParseJob::~ParseJob()
{
    typedef QPointer<QObject> QObjectPointer;
    foreach(const QObjectPointer &p, d->notify) {
        if(p) {
            QMetaObject::invokeMethod(p.data(), "updateReady", Qt::QueuedConnection, Q_ARG(KDevelop::IndexedString, d->url), Q_ARG(KDevelop::ReferencedTopDUContext, d->duContext));
        }
    }

    delete d;
}

ILanguageSupport* ParseJob::languageSupport() const
{
    return d->languageSupport;
}

void ParseJob::setParsePriority(int priority)
{
    d->parsePriority = priority;
}

int ParseJob::parsePriority() const
{
    return d->parsePriority;
}

bool ParseJob::requiresSequentialProcessing() const
{
    return d->sequentialProcessingFlags & RequiresSequentialProcessing;
}

bool ParseJob::respectsSequentialProcessing() const
{
    return d->sequentialProcessingFlags & RespectsSequentialProcessing;
}

void ParseJob::setSequentialProcessingFlags(SequentialProcessingFlags flags)
{
    d->sequentialProcessingFlags = flags;
}

IndexedString ParseJob::document() const
{
    return d->url;
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

TopDUContext::Features ParseJob::staticMinimumFeatures(const IndexedString& url)
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
    return (TopDUContext::Features)(d->features | staticMinimumFeatures(d->url));
}

void ParseJob::setDuChain(ReferencedTopDUContext duChain)
{
    d->duContext = duChain;
}

ReferencedTopDUContext ParseJob::duChain() const
{
    return d->duContext;
}

bool ParseJob::abortRequested() const
{
    return d->abortRequested.load();
}

void ParseJob::requestAbort()
{
    d->abortRequested = 1;
}

void ParseJob::abortJob()
{
    d->aborted = true;
    setStatus(Status_Aborted);
}

void ParseJob::setNotifyWhenReady(const QList<QPointer< QObject > >& notify
) {
    d->notify = notify;
}

void ParseJob::setStaticMinimumFeatures(const IndexedString& url, TopDUContext::Features features) {
    QMutexLocker lock(&minimumFeaturesMutex);
    ::staticMinimumFeatures[url].append(features);
}

void ParseJob::unsetStaticMinimumFeatures(const IndexedString& url, TopDUContext::Features features) {
    QMutexLocker lock(&minimumFeaturesMutex);
    ::staticMinimumFeatures[url].removeOne(features);
    if(::staticMinimumFeatures[url].isEmpty())
      ::staticMinimumFeatures.remove(url);
}

KDevelop::ProblemPointer ParseJob::readContents()
{
    Q_ASSERT(!d->hasReadContents);
    d->hasReadContents = true;

    QString localFile(document().toUrl().toLocalFile());
    QFileInfo fileInfo( localFile );

    QDateTime lastModified = fileInfo.lastModified();

    d->tracker = ICore::self()->languageController()->backgroundParser()->trackerForUrl(document());

    //Try using an artificial code-representation, which overrides everything else
    if(artificialCodeRepresentationExists(document())) {
        CodeRepresentation::Ptr repr = createCodeRepresentation(document());
        d->contents.contents = repr->text().toUtf8();
        kDebug() << "took contents for " << document().str() << " from artificial code-representation";
        return KDevelop::ProblemPointer();
    }

    bool hadTracker = false;
    if(d->tracker)
    {
        ForegroundLock lock;
        if(DocumentChangeTracker* t = d->tracker.data())
        {
            // The file is open in an editor
            d->previousRevision = t->revisionAtLastReset();

            t->reset(); // Reset the tracker to the current revision
            Q_ASSERT(t->revisionAtLastReset());

            d->contents.contents = t->textAtLastReset().toUtf8();
            d->contents.modification = KDevelop::ModificationRevision( lastModified, t->revisionAtLastReset()->revision() );

            d->revision = t->acquireRevision(d->contents.modification.revision);
            hadTracker = true;
        }
    }
    if (!hadTracker) {
        // We have to load the file from disk

        static const int maximumFileSize = 5 * 1024 * 1024; // 5 MB
        if (fileInfo.size() > maximumFileSize) {
            KFormat f;

            KDevelop::ProblemPointer p(new Problem());
            p->setSource(KDevelop::ProblemData::Disk);
            p->setDescription(i18nc("%1: filename", "Skipped file that is too large: '%1'", localFile ));
            p->setExplanation(i18nc("%1: file size, %2: limit file size",
                                    "The file is %1 and exceeds the limit of %2.",
                                    f.formatByteSize(fileInfo.size()),
                                    f.formatByteSize(maximumFileSize)));
            p->setFinalLocation(DocumentRange(document(), KTextEditor::Range::invalid()));
            kWarning( 9007 ) << p->description() << p->explanation();
            return p;
        }
        QFile file( localFile );

        if ( !file.open( QIODevice::ReadOnly ) )
        {
            KDevelop::ProblemPointer p(new Problem());
            p->setSource(KDevelop::ProblemData::Disk);
            p->setDescription(i18n( "Could not open file '%1'", localFile ));
            switch (file.error()) {
              case QFile::ReadError:
                  p->setExplanation(i18n("File could not be read from disk."));
                  break;
              case QFile::OpenError:
                  p->setExplanation(i18n("File could not be opened."));
                  break;
              case QFile::PermissionsError:
                  p->setExplanation(i18n("File could not be read from disk due to permissions."));
                  break;
              default:
                  break;
            }
            p->setFinalLocation(DocumentRange(document(), KTextEditor::Range::invalid()));

            kWarning( 9007 ) << "Could not open file" << document().str() << "(path" << localFile << ")" ;

            return p;
        }

        d->contents.contents = file.readAll(); ///@todo Convert from local encoding to utf-8 if they don't match
        d->contents.modification = KDevelop::ModificationRevision(lastModified);

        file.close();
    }

    // To make the parsing more robust, we add some zeroes to the end of the buffer.
    d->contents.contents.push_back((char)0);
    d->contents.contents.push_back((char)0);
    d->contents.contents.push_back((char)0);
    d->contents.contents.push_back((char)0);

    return KDevelop::ProblemPointer();
}

const KDevelop::ParseJob::Contents& ParseJob::contents() const
{
    Q_ASSERT(d->hasReadContents);
    return d->contents;
}

struct MovingRangeTranslator : public DUChainVisitor
{
    MovingRangeTranslator(qint64 _source, qint64 _target, MovingInterface* _moving) : source(_source), target(_target), moving(_moving) {
    }

    virtual void visit(DUContext* context) {
        translateRange(context);
        ///@todo Also map import-positions
        // Translate uses
        uint usesCount = context->usesCount();
        for(uint u = 0; u < usesCount; ++u)
        {
            RangeInRevision r = context->uses()[u].m_range;
            translateRange(r);
            context->changeUseRange(u, r);
        }
    }

    virtual void visit(Declaration* declaration) {
        translateRange(declaration);
    }

    void translateRange(DUChainBase* object)
    {
        RangeInRevision r = object->range();
        translateRange(r);
        object->setRange(r);
    }

    void translateRange(RangeInRevision& r)
    {
        // PHP and python use top contexts that start at (0, 0) end at INT_MAX, so make sure that doesn't overflow
        // or translate the start of the top context away from (0, 0)
        if ( r.start.line != 0 || r.start.column != 0 ) {
            moving->transformCursor(r.start.line, r.start.column, MovingCursor::MoveOnInsert, source, target);
        }
        if ( r.end.line != std::numeric_limits<int>::max() || r.end.column != std::numeric_limits<int>::max() ) {
            moving->transformCursor(r.end.line, r.end.column, MovingCursor::StayOnInsert, source, target);
        }
    }

    KTextEditor::Range range;
    qint64 source;
    qint64 target;
    MovingInterface* moving;
};

void ParseJob::translateDUChainToRevision(TopDUContext* context)
{
    qint64 targetRevision = d->contents.modification.revision;

    if(targetRevision == -1)
    {
        kDebug() << "invalid target revision" << targetRevision;
        return;
    }

    qint64 sourceRevision;

    {
        DUChainReadLocker duChainLock;

        Q_ASSERT(context->parsingEnvironmentFile());

        // Cannot map if there is no source revision
        sourceRevision = context->parsingEnvironmentFile()->modificationRevision().revision;

        if(sourceRevision == -1)
        {
            kDebug() << "invalid source revision" << sourceRevision;
            return;
        }
    }

    if(sourceRevision > targetRevision)
    {
        kDebug() << "for document" << document().str() << ": source revision is higher than target revision:" << sourceRevision << " > " << targetRevision;
        return;
    }

    ForegroundLock lock;
    if(DocumentChangeTracker* t = d->tracker.data())
    {
        if(!d->previousRevision)
        {
            kDebug() << "not translating because there is no valid predecessor-revision";
            return;
        }

        if(sourceRevision != d->previousRevision->revision() || !d->previousRevision->valid())
        {
            kDebug() << "not translating because the document revision does not match the tracker start revision (maybe the document was cleared)";
            return;
        }

        if(!t->holdingRevision(sourceRevision) || !t->holdingRevision(targetRevision))
        {
            kDebug() << "lost one of the translation revisions, not doing the map";
            return;
        }

        // Perform translation
        MovingInterface* moving = t->documentMovingInterface();

        DUChainWriteLocker wLock;

        MovingRangeTranslator translator(sourceRevision, targetRevision, moving);
        context->visit(translator);

        QList< ProblemPointer > problems = context->problems();
        for(QList< ProblemPointer >::iterator problem = problems.begin(); problem != problems.end(); ++problem)
        {
            RangeInRevision r = (*problem)->range();
            translator.translateRange(r);
            (*problem)->setRange(r);
        }

        // Update the modification revision in the meta-data
        ModificationRevision modRev = context->parsingEnvironmentFile()->modificationRevision();
        modRev.revision = targetRevision;
        context->parsingEnvironmentFile()->setModificationRevision(modRev);
    }
}

bool ParseJob::isUpdateRequired(const IndexedString& languageString)
{
    if (abortRequested()) {
        return false;
    }

    if (minimumFeatures() & TopDUContext::ForceUpdate) {
        return true;
    }

    DUChainReadLocker lock;
    if (abortRequested()) {
        return false;
    }
    foreach(const ParsingEnvironmentFilePointer &file, DUChain::self()->allEnvironmentFiles(document())) {
        if (file->language() != languageString) {
            continue;
        }
        if (!file->needsUpdate(environment()) && file->featuresSatisfied(minimumFeatures())) {
            kDebug() << "Already up to date" << document().str();
            setDuChain(file->topContext());
            lock.unlock();
            highlightDUChain();
            return false;
        }
        break;
    }
    return !abortRequested();
}

const ParsingEnvironment* ParseJob::environment() const
{
    return nullptr;
}

void ParseJob::highlightDUChain()
{
    ENSURE_CHAIN_NOT_LOCKED
    if (!d->languageSupport->codeHighlighting() || !duChain() || abortRequested()) {
        // language doesn't support highlighting
        return;
    }
    if (!d->hasReadContents && !d->tracker) {
        d->tracker = ICore::self()->languageController()->backgroundParser()->trackerForUrl(document());
    }
    if (d->tracker) {
        d->languageSupport->codeHighlighting()->highlightDUChain(duChain());
    }
}

ControlFlowGraph* ParseJob::controlFlowGraph()
{
    return nullptr;
}

DataAccessRepository* ParseJob::dataAccessInformation()
{
    return nullptr;
}

bool ParseJob::hasTracker() const
{
    return d->tracker;
}

ThreadWeaver::QObjectDecorator* ParseJob::decorator() const
{
    return d->decorator;
}

}

