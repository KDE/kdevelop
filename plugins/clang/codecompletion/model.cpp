/*
    SPDX-FileCopyrightText: 2014 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "model.h"

#include "util/clangdebug.h"
#include "context.h"
#include "includepathcompletioncontext.h"

#include "duchain/parsesession.h"
#include "duchain/clangindex.h"
#include "duchain/duchainutils.h"

#include <language/codecompletion/codecompletionworker.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/duchainlock.h>

#include <KTextEditor/View>
#include <KTextEditor/Document>

#include <QTimer>

using namespace KDevelop;

namespace {

bool isSpaceOnly(const QString& string)
{
    return std::find_if(string.begin(), string.end(), [] (const QChar c) { return !c.isSpace(); }) == string.end();
}

bool includePathCompletionRequired(const QString& text)
{
    const auto properties = IncludePathProperties::parseText(text);
    return properties.valid;
}

QSharedPointer<CodeCompletionContext> createCompletionContext(const DUContextPointer& context,
                                                              const ParseSessionData::Ptr& session,
                                                              const QUrl& url,
                                                              const KTextEditor::Cursor& position,
                                                              const QString& text,
                                                              const QString& followingText)
{
    if (includePathCompletionRequired(text)) {
        return QSharedPointer<IncludePathCompletionContext>::create(context, session, url, position, text);
    } else {
        return QSharedPointer<ClangCodeCompletionContext>::create(context, session, url, position, text, followingText);
    }
}

class ClangCodeCompletionWorker : public CodeCompletionWorker
{
    Q_OBJECT
public:
    ClangCodeCompletionWorker(ClangIndex* index, CodeCompletionModel* model)
        : CodeCompletionWorker(model)
        , m_index(index)
    {}
    ~ClangCodeCompletionWorker() override = default;

public Q_SLOTS:
    void completionRequested(const QUrl &url, const KTextEditor::Cursor& position, const QString& text, const QString& followingText)
    {
        // group requests and only handle the latest one
        m_url = url;
        m_position = position;
        m_text = text;
        m_followingText = followingText;

        if (!m_timer) {
            // lazy-load the timer to initialize it in the background thread
            m_timer = new QTimer(this);
            m_timer->setInterval(0);
            m_timer->setSingleShot(true);
            connect(m_timer, &QTimer::timeout, this, &ClangCodeCompletionWorker::run);
        }
        m_timer->start();
    }

private:
    void run()
    {
        aborting() = false;

        DUChainReadLocker lock;
        if (aborting()) {
            failed();
            return;
        }

        auto top = DUChainUtils::standardContextForUrl(m_url);
        if (!top) {
            qCWarning(KDEV_CLANG) << "No context found for" << m_url;
            return;
        }

        ParseSessionData::Ptr sessionData(ClangIntegration::DUChainUtils::findParseSessionData(top->url(), m_index->translationUnitForUrl(top->url())));

        if (!sessionData) {
            // TODO: trigger reparse and re-request code completion
            qCWarning(KDEV_CLANG) << "No parse session / AST attached to context for url" << m_url;
            return;
        }

        if (aborting()) {
            failed();
            return;
        }

        // We hold DUChain lock, and ask for ParseSession, but TUDUChain indirectly holds ParseSession lock.
        lock.unlock();

        auto completionContext = ::createCompletionContext(DUContextPointer(top), sessionData, m_url,
                                                           m_position, m_text, m_followingText);

        lock.lock();
        if (aborting()) {
            failed();
            return;
        }

        bool abort = false;
        // NOTE: cursor might be wrong here, but shouldn't matter much I hope...
        //       when the document changed significantly, then the cache is off anyways and we don't get anything sensible
        //       the position here is just a "optimization" to only search up to that position
        const auto& items = completionContext->completionItems(abort);

        if (aborting()) {
            failed();
            return;
        }

        auto tree = computeGroups( items, {} );

        if (aborting()) {
            failed();
            return;
        }

        tree += completionContext->ungroupedElements();

        foundDeclarations( tree, {} );
    }
private:
    ClangIndex* m_index;
    QTimer* m_timer = nullptr;
    QUrl m_url;
    KTextEditor::Cursor m_position;
    QString m_text;
    QString m_followingText;
};
}

ClangCodeCompletionModel::ClangCodeCompletionModel(ClangIndex* index, QObject* parent)
    : CodeCompletionModel(parent)
    , m_index(index)
{
    qRegisterMetaType<KTextEditor::Cursor>();
}

ClangCodeCompletionModel::~ClangCodeCompletionModel()
{

}

bool ClangCodeCompletionModel::shouldStartCompletion(KTextEditor::View* view, const QString& inserted,
                                                     bool userInsertion, const KTextEditor::Cursor& position)
{
    const QString noCompletionAfter = QStringLiteral(";{}]) ");

    if (inserted.isEmpty() || isSpaceOnly(inserted)) {
        return false;
    }
    const auto lastChar = inserted.at(inserted.size() - 1);
    if (noCompletionAfter.contains(lastChar)) {
        return false;
    }
    const auto wordAtPosition = view->document()->wordAt(position);
    if (!wordAtPosition.isEmpty() && wordAtPosition.at(0).isDigit()) {
        return false;
    }
    // also show include path completion after dashes
    if (userInsertion && lastChar == QLatin1Char('-') && includePathCompletionRequired(view->document()->line(position.line()))) {
        return true;
    }
    if (userInsertion && inserted.endsWith(QLatin1String("::"))) {
        return true;
    }

    return KDevelop::CodeCompletionModel::shouldStartCompletion(view, inserted, userInsertion, position);
}

KTextEditor::Range ClangCodeCompletionModel::completionRange(KTextEditor::View* view, const KTextEditor::Cursor& position)
{
    auto range = KDevelop::CodeCompletionModel::completionRange(view, position);
    const auto includeProperties = IncludePathProperties::parseText(view->document()->line(position.line()), position.column());
    if (includeProperties.valid && includeProperties.inputFrom != -1) {
        // expand include path range to include e.g. dashes
        range.setStart({position.line(), includeProperties.inputFrom});
    }
    return range;
}

bool ClangCodeCompletionModel::shouldAbortCompletion(KTextEditor::View* view, const KTextEditor::Range& range, const QString& currentCompletion)
{
    const auto shouldAbort = KDevelop::CodeCompletionModel::shouldAbortCompletion(view, range, currentCompletion);
    if (shouldAbort && includePathCompletionRequired(view->document()->line(range.end().line()))) {
        // don't abort include path completion which can contain dashes
        return false;
    }
    return shouldAbort;
}

CodeCompletionWorker* ClangCodeCompletionModel::createCompletionWorker()
{
    auto worker = new ClangCodeCompletionWorker(m_index, this);
    connect(this, &ClangCodeCompletionModel::requestCompletion,
            worker, &ClangCodeCompletionWorker::completionRequested);
    return worker;
}

void ClangCodeCompletionModel::completionInvokedInternal(KTextEditor::View* view, const KTextEditor::Range& range,
                                                         CodeCompletionModel::InvocationType /*invocationType*/, const QUrl &url)
{
    auto text = view->document()->text({0, 0, range.start().line(), range.start().column()});
    auto followingText = view->document()->text({{range.start().line(), range.start().column()}, view->document()->documentEnd()});
    emit requestCompletion(url, KTextEditor::Cursor(range.start()), text, followingText);
}

#include "model.moc"
#include "moc_model.cpp"
