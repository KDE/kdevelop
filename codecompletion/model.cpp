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
#include "context.h"
#include <duchain/parsesession.h>

#include <language/codecompletion/codecompletionworker.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/duchainlock.h>
#include <KTextEditor/View>
#include <KTextEditor/Document>

using namespace KDevelop;

namespace {
class ClangCodeCompletionWorker : public CodeCompletionWorker
{
    Q_OBJECT
public:
    ClangCodeCompletionWorker(CodeCompletionModel* model)
        : CodeCompletionWorker(model)
    {}
    virtual ~ClangCodeCompletionWorker() = default;

public slots:
    void completionRequested(const KUrl& url, const KTextEditor::Cursor& position, const QString& text)
    {
        aborting() = false;

        DUChainReadLocker lock;
        if (aborting()) {
            failed();
            return;
        }

        auto top = DUChainUtils::standardContextForUrl(url);
        if (!top) {
            kWarning() << "No context found for" << url;
            return;
        }

        // We hold DUChain lock, and ask for ParseSession, but TUDUChain indirectly holds ParseSession lock.
        lock.unlock();

        const ParseSessionData::Ptr sessionData(dynamic_cast<ParseSessionData*>(top->ast().data()));
        if (!sessionData) {
            // TODO: trigger reparse and re-request code completion
            kWarning() << "No parse session / AST attached to context for url" << url;
            return;
        }

        if (aborting()) {
            failed();
            return;
        }

        lock.lock();
        ClangCodeCompletionContext completionContext(DUContextPointer(top), sessionData, position, text);

        if (aborting()) {
            failed();
            return;
        }

        bool abort = false;
        // NOTE: cursor might be wrong here, but shouldn't matter much I hope...
        //       when the document changed significantly, then the cache is off anyways and we don't get anything sensible
        //       the position here is just a "optimization" to only search up to that position
        const auto& items = completionContext.completionItems(abort);

        if (aborting()) {
            failed();
            return;
        }

        auto tree = computeGroups( items, {} );

        if (aborting()) {
            failed();
            return;
        }

        tree += completionContext.ungroupedElements();

        foundDeclarations( tree, {} );
    }
};
}

ClangCodeCompletionModel::ClangCodeCompletionModel(QObject* parent)
    : CodeCompletionModel(parent)
{
    qRegisterMetaType<KTextEditor::Cursor>();
}

ClangCodeCompletionModel::~ClangCodeCompletionModel()
{

}

CodeCompletionWorker* ClangCodeCompletionModel::createCompletionWorker()
{
    auto worker = new ClangCodeCompletionWorker(this);
    connect(this, SIGNAL(requestCompletion(KUrl,KTextEditor::Cursor,QString)),
            worker, SLOT(completionRequested(KUrl,KTextEditor::Cursor,QString)));
    return worker;
}

void ClangCodeCompletionModel::completionInvokedInternal(KTextEditor::View* view, const KTextEditor::Range& range,
                                                         CodeCompletionModel::InvocationType /*invocationType*/, const KUrl& url)
{
    // get text before this range so we can parse this version with clang
    auto text = view->document()->text({0, 0, range.start().line(), range.start().column()});
    emit requestCompletion(url, KTextEditor::Cursor(range.start()), text);
}

#include "model.moc"
#include "moc_model.cpp"
