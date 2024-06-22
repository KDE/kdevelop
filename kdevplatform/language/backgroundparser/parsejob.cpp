/*
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2006-2008 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "parsejob.h"

#include <QFile>
#include <QMutex>
#include <QMutexLocker>
#include <QStandardPaths>

#include <KLocalizedString>
#include <KFormat>
#include <KTextEditor/Document>

#include "backgroundparser.h"
#include <debug.h>
#include "duchain/topducontext.h"

#include "duchain/duchainlock.h"
#include "duchain/duchain.h"
#include "duchain/parsingenvironment.h"
#include "editor/documentrange.h"

#include <util/foregroundlock.h>
#include <util/kdevstringhandler.h>
#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <codegen/coderepresentation.h>
#include <duchain/declaration.h>
#include <duchain/use.h>
#include <interfaces/icodehighlighting.h>
#include <duchain/problem.h>

using namespace KTextEditor;

static QMutex minimumFeaturesMutex;
static QHash<KDevelop::IndexedString, QList<KDevelop::TopDUContext::Features>> staticMinimumFeatures;

namespace KDevelop {
class ParseJobPrivate
{
public:

    ParseJobPrivate(const IndexedString& url_, ILanguageSupport* languageSupport_) :
        url(url_)
        , languageSupport(languageSupport_)
        , abortRequested(0)
        , hasReadContents(false)
        , aborted(false)
        , features(TopDUContext::VisibleDeclarationsAndContexts)
        , parsePriority(0)
        , sequentialProcessingFlags(ParseJob::IgnoresSequentialProcessing)
        , maximumFileSize(5 * 1024 * 1024) // 5 MB
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
    QVector<QPointer<QObject>> notify;
    QPointer<DocumentChangeTracker> tracker;
    RevisionReference revision;
    RevisionReference previousRevision;

    int parsePriority;
    ParseJob::SequentialProcessingFlags sequentialProcessingFlags;
    qint64 maximumFileSize;
};

ParseJob::ParseJob(const IndexedString& url, KDevelop::ILanguageSupport* languageSupport)
    : ThreadWeaver::Sequence()
    , d_ptr(new ParseJobPrivate(url, languageSupport))
{
}

ParseJob::~ParseJob()
{
    Q_D(ParseJob);

    for (auto& p : std::as_const(d->notify)) {
        if (p) {
            QMetaObject::invokeMethod(p.data(), "updateReady", Qt::QueuedConnection,
                                      Q_ARG(KDevelop::IndexedString, d->url),
                                      Q_ARG(KDevelop::ReferencedTopDUContext, d->duContext));
        }
    }
}

ILanguageSupport* ParseJob::languageSupport() const
{
    Q_D(const ParseJob);

    return d->languageSupport;
}

void ParseJob::setParsePriority(int priority)
{
    Q_D(ParseJob);

    d->parsePriority = priority;
}

int ParseJob::parsePriority() const
{
    Q_D(const ParseJob);

    return d->parsePriority;
}

bool ParseJob::requiresSequentialProcessing() const
{
    Q_D(const ParseJob);

    return d->sequentialProcessingFlags & RequiresSequentialProcessing;
}

bool ParseJob::respectsSequentialProcessing() const
{
    Q_D(const ParseJob);

    return d->sequentialProcessingFlags & RespectsSequentialProcessing;
}

void ParseJob::setSequentialProcessingFlags(SequentialProcessingFlags flags)
{
    Q_D(ParseJob);

    d->sequentialProcessingFlags = flags;
}

qint64 ParseJob::maximumFileSize() const
{
    Q_D(const ParseJob);

    return d->maximumFileSize;
}

void ParseJob::setMaximumFileSize(qint64 value)
{
    Q_D(ParseJob);

    d->maximumFileSize = value;
}

IndexedString ParseJob::document() const
{
    Q_D(const ParseJob);

    return d->url;
}

bool ParseJob::success() const
{
    Q_D(const ParseJob);

    return !d->aborted;
}

void ParseJob::setMinimumFeatures(TopDUContext::Features features)
{
    Q_D(ParseJob);

    d->features = features;
}

bool ParseJob::hasStaticMinimumFeatures()
{
    QMutexLocker lock(&minimumFeaturesMutex);
    return !::staticMinimumFeatures.isEmpty();
}

TopDUContext::Features ParseJob::staticMinimumFeatures(const IndexedString& url)
{
    QMutexLocker lock(&minimumFeaturesMutex);
    TopDUContext::Features features{};

    const auto featuresIt = ::staticMinimumFeatures.constFind(url);
    if (featuresIt != ::staticMinimumFeatures.constEnd())
        for (const TopDUContext::Features f : *featuresIt)
            features |= f;

    return features;
}

TopDUContext::Features ParseJob::minimumFeatures() const
{
    Q_D(const ParseJob);

    return d->features | staticMinimumFeatures(d->url);
}

void ParseJob::setDuChain(const ReferencedTopDUContext& duChain)
{
    Q_D(ParseJob);

    d->duContext = duChain;
}

ReferencedTopDUContext ParseJob::duChain() const
{
    Q_D(const ParseJob);

    return d->duContext;
}

bool ParseJob::abortRequested() const
{
    Q_D(const ParseJob);
    return d->abortRequested.loadRelaxed() != 0;
}

void ParseJob::requestAbort()
{
    Q_D(ParseJob);

    d->abortRequested = 1;
}

void ParseJob::abortJob()
{
    Q_D(ParseJob);

    d->aborted = true;
    setStatus(Status_Aborted);
}

void ParseJob::setNotifyWhenReady(const QVector<QPointer<QObject>>& notify)
{
    Q_D(ParseJob);

    d->notify = notify;
}

void ParseJob::setStaticMinimumFeatures(const IndexedString& url, TopDUContext::Features features)
{
    QMutexLocker lock(&minimumFeaturesMutex);
    ::staticMinimumFeatures[url].append(features);
}

void ParseJob::unsetStaticMinimumFeatures(const IndexedString& url, TopDUContext::Features features)
{
    QMutexLocker lock(&minimumFeaturesMutex);
    ::staticMinimumFeatures[url].removeOne(features);
    if (::staticMinimumFeatures[url].isEmpty())
        ::staticMinimumFeatures.remove(url);
}

KDevelop::ProblemPointer ParseJob::readContents()
{
    Q_D(ParseJob);

    Q_ASSERT(!d->hasReadContents);
    d->hasReadContents = true;

    QString localFile(document().toUrl().toLocalFile());
    QFileInfo fileInfo(localFile);

    QDateTime lastModified = fileInfo.lastModified();

    d->tracker = ICore::self()->languageController()->backgroundParser()->trackerForUrl(document());

    //Try using an artificial code-representation, which overrides everything else
    if (artificialCodeRepresentationExists(document())) {
        CodeRepresentation::Ptr repr = createCodeRepresentation(document());
        d->contents.contents = repr->text().toUtf8();
        qCDebug(LANGUAGE) << "took contents for " << document().str() << " from artificial code-representation";
        return KDevelop::ProblemPointer();
    }

    bool hadTracker = false;
    if (d->tracker) {
        ForegroundLock lock;
        if (DocumentChangeTracker* t = d->tracker.data()) {
            // The file is open in an editor
            d->previousRevision = t->revisionAtLastReset();

            t->reset(); // Reset the tracker to the current revision
            Q_ASSERT(t->revisionAtLastReset());

            d->contents.contents = t->document()->text().toUtf8();
            d->contents.modification =
                KDevelop::ModificationRevision(lastModified, t->revisionAtLastReset()->revision());

            d->revision = t->acquireRevision(d->contents.modification.revision);
            hadTracker = true;
        }
    }
    if (!hadTracker) {
        // We have to load the file from disk

        if (fileInfo.size() > d->maximumFileSize) {
            KFormat f;

            KDevelop::ProblemPointer p(new Problem());
            p->setSource(IProblem::Disk);
            p->setDescription(i18nc("%1: filename", "Skipped file that is too large: '%1'", localFile));
            p->setExplanation(i18nc("%1: file size, %2: limit file size",
                                    "The file is %1 and exceeds the limit of %2.",
                                    f.formatByteSize(fileInfo.size()),
                                    f.formatByteSize(d->maximumFileSize)));
            p->setFinalLocation(DocumentRange(document(), KTextEditor::Range::invalid()));
            qCWarning(LANGUAGE) << p->description() << p->explanation();
            return p;
        }
        QFile file(localFile);

        if (!file.open(QIODevice::ReadOnly)) {
            KDevelop::ProblemPointer p(new Problem());
            p->setSource(IProblem::Disk);
            p->setDescription(i18n("Could not open file '%1'", localFile));
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

            qCWarning(LANGUAGE) << "Could not open file" << document().str() << "(path" << localFile << ")";

            return p;
        }

        d->contents.contents = file.readAll(); ///@todo Convert from local encoding to utf-8 if they don't match

        // This is consistent with KTextEditor::Document::text() as used for already-open files.
        normalizeLineEndings(d->contents.contents);
        d->contents.modification = KDevelop::ModificationRevision(lastModified);

        file.close();
    }

    return KDevelop::ProblemPointer();
}

const KDevelop::ParseJob::Contents& ParseJob::contents() const
{
    Q_D(const ParseJob);

    Q_ASSERT(d->hasReadContents);
    return d->contents;
}

struct MovingRangeTranslator
    : public DUChainVisitor
{
    MovingRangeTranslator(qint64 _source, qint64 _target, KTextEditor::Document* _document)
        : source(_source)
        , target(_target)
        , document(_document)
    {
    }

    void visit(DUContext* context) override
    {
        translateRange(context);
        ///@todo Also map import-positions
        // Translate uses
        uint usesCount = context->usesCount();
        for (uint u = 0; u < usesCount; ++u) {
            RangeInRevision r = context->uses()[u].m_range;
            translateRange(r);
            context->changeUseRange(u, r);
        }
    }

    void visit(Declaration* declaration) override
    {
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
        if (r.start.line != 0 || r.start.column != 0) {
            document->transformCursor(r.start.line, r.start.column, MovingCursor::MoveOnInsert, source, target);
        }
        if (r.end.line != std::numeric_limits<int>::max() || r.end.column != std::numeric_limits<int>::max()) {
            document->transformCursor(r.end.line, r.end.column, MovingCursor::StayOnInsert, source, target);
        }
    }

    KTextEditor::Range range;
    qint64 source;
    qint64 target;
    KTextEditor::Document* document;
};

void ParseJob::translateDUChainToRevision(TopDUContext* context)
{
    Q_D(ParseJob);

    qint64 targetRevision = d->contents.modification.revision;

    if (targetRevision == -1) {
        qCDebug(LANGUAGE) << "invalid target revision" << targetRevision;
        return;
    }

    qint64 sourceRevision;

    {
        DUChainReadLocker duChainLock;

        Q_ASSERT(context->parsingEnvironmentFile());

        // Cannot map if there is no source revision
        sourceRevision = context->parsingEnvironmentFile()->modificationRevision().revision;

        if (sourceRevision == -1) {
            qCDebug(LANGUAGE) << "invalid source revision" << sourceRevision;
            return;
        }
    }

    if (sourceRevision > targetRevision) {
        qCDebug(LANGUAGE) << "for document" << document().str() <<
            ": source revision is higher than target revision:" << sourceRevision << " > " << targetRevision;
        return;
    }

    ForegroundLock lock;
    if (DocumentChangeTracker* t = d->tracker.data()) {
        if (!d->previousRevision) {
            qCDebug(LANGUAGE) << "not translating because there is no valid predecessor-revision";
            return;
        }

        if (sourceRevision != d->previousRevision->revision() || !d->previousRevision->valid()) {
            qCDebug(LANGUAGE) <<
                "not translating because the document revision does not match the tracker start revision (maybe the document was cleared)";
            return;
        }

        if (!t->holdingRevision(sourceRevision) || !t->holdingRevision(targetRevision)) {
            qCDebug(LANGUAGE) << "lost one of the translation revisions, not doing the map";
            return;
        }

        // Perform translation
        DUChainWriteLocker wLock;

        MovingRangeTranslator translator(sourceRevision, targetRevision, t->document());
        context->visit(translator);

        const QList<ProblemPointer> problems = context->problems();
        for (auto& problem : problems) {
            RangeInRevision r = problem->range();
            translator.translateRange(r);
            problem->setRange(r);
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
    const auto files = DUChain::self()->allEnvironmentFiles(document());
    for (const ParsingEnvironmentFilePointer& file : files) {
        if (file->language() != languageString) {
            continue;
        }
        if (!file->needsUpdate(environment()) && file->featuresSatisfied(minimumFeatures())) {
            qCDebug(LANGUAGE) << "Already up to date" << document().str();
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
    Q_D(ParseJob);

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
    Q_D(const ParseJob);

    return d->tracker;
}
}

#include "moc_parsejob.cpp"
