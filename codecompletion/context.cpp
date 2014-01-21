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

#include "context.h"

#include <language/duchain/duchainlock.h>
#include <language/duchain/ducontext.h>
#include <language/duchain/topducontext.h>
#include <language/interfaces/iastcontainer.h>
#include <language/codecompletion/codecompletionitem.h>
#include <language/codecompletion/codecompletionmodel.h>

#include "../duchain/parsesession.h"
#include "../duchain/clangtypes.h"

#include <memory>
#include <KTextEditor/Document>

using namespace KDevelop;

namespace {

class Item : public CompletionTreeItem
{
public:
    Item(const CXCompletionResult& result);
    virtual ~Item() = default;

    virtual QVariant data(const QModelIndex& index, int role, const CodeCompletionModel* model) const;

    virtual void execute(KTextEditor::Document* document, const KTextEditor::Range& word);

private:
    QString m_typed;
    QString m_text;
    QString m_result;
    QString m_currParam;
};

Item::Item(const CXCompletionResult& result)
{
    const uint chunks = clang_getNumCompletionChunks(result.CompletionString);
    for (uint i = 0; i < chunks; ++i) {
        const auto kind = clang_getCompletionChunkKind(result.CompletionString, i);
        const QString string = ClangString(clang_getCompletionChunkText(result.CompletionString, i)).toString();
        switch (kind) {
            case CXCompletionChunk_TypedText:
                m_typed = string;
                break;
            case CXCompletionChunk_CurrentParameter:
                m_currParam = string;
                continue;
            case CXCompletionChunk_ResultType:
                m_result = string;
                continue;
            case CXCompletionChunk_Informative:
            case CXCompletionChunk_Optional:
                continue;
            case CXCompletionChunk_Placeholder:
                m_text += "/*" + string + "*/";
                continue;
        }
        m_text += string;
    }
}

QVariant Item::data(const QModelIndex& index, int role, const CodeCompletionModel* /*model*/) const
{
    if (role != Qt::DisplayRole) {
        return {};
    }
    switch (index.column()) {
        case CodeCompletionModel::Name:
            return m_typed;
        case CodeCompletionModel::Prefix:
            return m_result;
    }
    return {};
}

void Item::execute(KTextEditor::Document* document, const KTextEditor::Range& word)
{
    document->replaceText(word, m_text);
}

}

ClangCodeCompletionContext::ClangCodeCompletionContext(const DUContextPointer& context, const QString& text,
                                                       const CursorInRevision& position, int depth)
    : CodeCompletionContext(context, text, position, depth)
{
}

ClangCodeCompletionContext::~ClangCodeCompletionContext()
{
}

QList<CompletionTreeItemPointer> ClangCodeCompletionContext::completionItems(bool& abort, bool /*fullCompletion*/)
{
    // FIXME: ensure we don't access the TU from this thread and the parse thread simultaneously!
    DUChainReadLocker lock;
    if (abort) {
        return {};
    }
    const auto session = KSharedPtr<ParseSession>::dynamicCast(m_duContext->topContext()->ast());
    if (!session) {
        qDebug() << "no session found for file" << m_duContext->url();
        return {};
    }

    ClangString file(clang_getFileName(session->file()));

    /* FIXME: m_text only contains the part of the current context which is not enough for the clang use-case
    CXUnsavedFile unsaved;
    const auto contents = m_text.toUtf8();
    unsaved.Contents = contents.constData();
    unsaved.Length = contents.size();
    unsaved.Filename = file;
    */

    const auto results = std::unique_ptr<CXCodeCompleteResults, void(*)(CXCodeCompleteResults*)>(
        clang_codeCompleteAt(session->unit(), file, m_position.line + 1, m_position.column + 1, nullptr, 0,
                             clang_defaultCodeCompleteOptions()),
        clang_disposeCodeCompleteResults);

    QList<CompletionTreeItemPointer> ret;
    ret.reserve(results->NumResults);
    for (uint i = 0; i < results->NumResults; ++i) {
        ret.append(CompletionTreeItemPointer(new Item(results->Results[i])));
    }

    return ret;
}
