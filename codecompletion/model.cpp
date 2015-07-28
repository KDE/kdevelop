/*
 * This file is part of KDevelop
 * Copyright 2014 Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "model.h"

#include "util/clangdebug.h"
#include "context.h"
#include "includepathcompletioncontext.h"

#include "duchain/parsesession.h"
#include "duchain/clangindex.h"

#include <language/codecompletion/codecompletionworker.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/duchainlock.h>

#include <QRegularExpression>

#include <KTextEditor/View>
#include <KTextEditor/Document>

using namespace KDevelop;

namespace {

bool includePathCompletionRequired(const QString& text)
{
    QString line;
    const int idx = text.lastIndexOf(QLatin1Char('\n'));
    if (idx != -1) {
        line = text.mid(idx + 1).trimmed();
    } else {
        line = text.trimmed();
    }

    const static QRegularExpression includeRegExp(QStringLiteral("^\\s*#\\s*include"));
    if (!line.contains(includeRegExp)) {
        return false;
    }

    return true;
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
        return QSharedPointer<ClangCodeCompletionContext>::create(context, session, url, position, text + followingText);
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
    virtual ~ClangCodeCompletionWorker() = default;

public slots:
    void completionRequested(const QUrl &url, const KTextEditor::Cursor& position, const QString& text, const QString& followingText)
    {
        aborting() = false;

        DUChainReadLocker lock;
        if (aborting()) {
            failed();
            return;
        }

        auto top = DUChainUtils::standardContextForUrl(url);
        if (!top) {
            qCWarning(KDEV_CLANG) << "No context found for" << url;
            return;
        }

        // We hold DUChain lock, and ask for ParseSession, but TUDUChain indirectly holds ParseSession lock.
        lock.unlock();

        ParseSessionData::Ptr sessionData(dynamic_cast<ParseSessionData*>(top->ast().data()));
        if (!sessionData) {
            // ask the TU to which we are pinned, if available
            const auto tuUrl = m_index->translationUnitForUrl(top->url());
            if (tuUrl != top->url()) {
                auto tu = DUChainUtils::standardContextForUrl(tuUrl.toUrl());
                if (tu) {
                    sessionData = dynamic_cast<ParseSessionData*>(tu->ast().data());
                }
            }
        }
        if (!sessionData) {
            // TODO: trigger reparse and re-request code completion
            qCWarning(KDEV_CLANG) << "No parse session / AST attached to context for url" << url;
            return;
        }

        if (aborting()) {
            failed();
            return;
        }

        auto completionContext = ::createCompletionContext(DUContextPointer(top), sessionData, url, position, text, followingText);

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
