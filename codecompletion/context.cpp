/*
 * This file is part of qmljs, the QML/JS language support plugin for KDevelop
 * Copyright (c) 2013 Sven Brauch <svenbrauch@googlemail.com>
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
 *
 */

#include "context.h"

#include <language/codecompletion/codecompletionitem.h>
#include <language/codecompletion/normaldeclarationcompletionitem.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/classdeclaration.h>
#include <language/duchain/codemodel.h>

using namespace KDevelop;

typedef QPair<Declaration*, int> DeclarationDepthPair;

namespace QmlJS {

class UiObjectDefinitionItem : public NormalDeclarationCompletionItem
{
public:
    UiObjectDefinitionItem(const DeclarationPointer& decl)
    : NormalDeclarationCompletionItem(decl)
    {
    }
    virtual KTextEditor::CodeCompletionModel::CompletionProperties completionProperties() const
    {
        return KTextEditor::CodeCompletionModel::GlobalScope | KTextEditor::CodeCompletionModel::Class;
    }
};

CodeCompletionContext::CodeCompletionContext(const DUContextPointer& context, const QString& text,
                                             const CursorInRevision& position, int depth)
    : KDevelop::CodeCompletionContext(context, text, position, depth)
{
    // Determine which kind of completion should be offered
    // ...
}

QList< CompletionTreeItemPointer > CodeCompletionContext::completionItems(bool& abort, bool fullCompletion)
{
    Q_UNUSED (fullCompletion);

    // Compute the completion items, based on the surrounding text and the
    // type of completion to offer in the constructor

    QList<CompletionTreeItemPointer> items;

    if ( abort ) {
        return items;
    }

    DUChainReadLocker lock;
    if ( m_duContext ) {
        items += globalItems();

        const QList<DeclarationDepthPair>& locals = m_duContext->allDeclarations(m_position, duContext()->topContext());
        foreach ( const DeclarationDepthPair& decl, locals ) {
            if (dynamic_cast<ClassDeclaration*>(decl.first)) {
                continue;
            }
            DeclarationPointer declaration(decl.first);
            items << CompletionTreeItemPointer(new NormalDeclarationCompletionItem(declaration));
        }
    }
    return items;
}

QList<CompletionTreeItemPointer> CodeCompletionContext::globalItems() const
{
    QList<CompletionTreeItemPointer> ret;

    uint itemCount = 0;
    const CodeModelItem* items = 0;
    CodeModel::self().items(m_duContext->url(), itemCount, items);
    for (uint i = 0; i < itemCount; ++i) {
        const CodeModelItem& item = items[i];
        if (item.kind & CodeModelItem::Class && item.id.isValid()) {
            foreach (Declaration* dec, m_duContext->findDeclarations(item.id.identifier())) {
                if (dynamic_cast<ClassDeclaration*>(dec)) {
                    ret << CompletionTreeItemPointer(new UiObjectDefinitionItem(DeclarationPointer(dec)));
                    break;
                }
            }
        }
    }

    return ret;
}


}
