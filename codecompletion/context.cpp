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
#include <language/duchain/declaration.h>
#include <language/interfaces/iastcontainer.h>
#include <language/codecompletion/codecompletionitem.h>
#include <language/codecompletion/codecompletionmodel.h>
#include <language/codecompletion/normaldeclarationcompletionitem.h>

#include "../duchain/parsesession.h"
#include "../duchain/clangtypes.h"

#include "../debug.h"

#include <memory>
#include <KTextEditor/Document>

using namespace KDevelop;

namespace {

class SimpleItem : public CompletionTreeItem
{
public:
    SimpleItem(const QString& typed, const QString& result, const QString& text,
               CodeCompletionModel::CompletionProperties properties);
    virtual ~SimpleItem() = default;

    virtual QVariant data(const QModelIndex& index, int role, const CodeCompletionModel* model) const;

    virtual void execute(KTextEditor::Document* document, const KTextEditor::Range& word);

    virtual CodeCompletionModel::CompletionProperties completionProperties() const;

private:
    QString m_typed;
    QString m_result;
    QString m_text;
    CodeCompletionModel::CompletionProperties m_properties;
};

SimpleItem::SimpleItem(const QString& typed, const QString& result, const QString& text,
                       CodeCompletionModel::CompletionProperties properties)
    : m_typed(typed)
    , m_result(result)
    , m_text(text)
    , m_properties(properties)
{

}

QVariant SimpleItem::data(const QModelIndex& index, int role, const CodeCompletionModel* /*model*/) const
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

void SimpleItem::execute(KTextEditor::Document* document, const KTextEditor::Range& word)
{
    document->replaceText(word, m_text);
}

CodeCompletionModel::CompletionProperties SimpleItem::completionProperties() const
{
    return m_properties;
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
    if (abort || !m_duContext) {
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

    QSet<QualifiedIdentifier> handled;

    for (uint i = 0; i < results->NumResults; ++i) {
        auto result = results->Results[i];
        const uint chunks = clang_getNumCompletionChunks(result.CompletionString);

        QString typed;
        QString resultType;
        QString text;
        for (uint j = 0; j < chunks; ++j) {
            const auto kind = clang_getCompletionChunkKind(result.CompletionString, j);
            const QString string = ClangString(clang_getCompletionChunkText(result.CompletionString, j)).toString();
            switch (kind) {
                case CXCompletionChunk_TypedText:
                    typed = string;
                    break;
                case CXCompletionChunk_ResultType:
                    resultType = string;
                    continue;
                case CXCompletionChunk_CurrentParameter:
                case CXCompletionChunk_Informative:
                case CXCompletionChunk_Optional:
                    continue;
                case CXCompletionChunk_Placeholder:
                    text += "/*" + string + "*/";
                    continue;
                default:
                    break;
            }
            text += string;
        }
        if (result.CursorKind != CXCursor_MacroDefinition && result.CursorKind != CXCursor_NotImplemented) {
            const Identifier id(typed);
            QualifiedIdentifier qid;
            ClangString parent(clang_getCompletionParent(result.CompletionString, nullptr));
            if (parent.c_str() != nullptr) {
                qid = QualifiedIdentifier(parent.toString());
            }
            qid.push(id);

            if (handled.contains(qid)) {
                // TODO: support overload items - for now, just skip them
                continue;
            }
            handled.insert(qid);
            Declaration* found = 0;
            foreach(Declaration* dec, m_duContext->findDeclarations(qid, m_position)) {
                found = dec;
                break;
            }
            if (found) {
                ret.append(CompletionTreeItemPointer(new NormalDeclarationCompletionItem(DeclarationPointer(found))));
                continue;
            } else {
                debug() << "Could not find declaration for" << qid;
            }
        }

        // TODO: grouping of macros and built-in stuff
        ret.append(CompletionTreeItemPointer(new SimpleItem(typed, resultType, text, CodeCompletionModel::GlobalScope)));
    }

    return ret;
}
