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
#include "completionitem.h"

#include <language/codecompletion/codecompletionitem.h>
#include <language/codecompletion/normaldeclarationcompletionitem.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/classdeclaration.h>
#include <language/duchain/namespacealiasdeclaration.h>
#include <language/duchain/codemodel.h>

using namespace KDevelop;

typedef QPair<Declaration*, int> DeclarationDepthPair;

namespace QmlJS {

CodeCompletionContext::CodeCompletionContext(const DUContextPointer& context, const QString& text,
                                             const CursorInRevision& position, int depth)
    : KDevelop::CodeCompletionContext(context, text, position, depth)
{
    // Determine which kind of completion should be offered
    // ...
}

QList<CompletionTreeItemPointer> CodeCompletionContext::completionItems(bool& abort, bool fullCompletion)
{
    Q_UNUSED (fullCompletion);

    // Compute the completion items, based on the surrounding text and the
    // type of completion to offer in the constructor

    QList<CompletionTreeItemPointer> items;

    if (abort) {
        return items;
    }

    items << completionsInContext(m_duContext);
    items << globalCompletions();

    return items;
}

QList<CompletionTreeItemPointer> CodeCompletionContext::completionsInContext(const DUContextPointer& context)
{
    QList<CompletionTreeItemPointer> items;
    DUChainReadLocker lock;

    if (context) {
        const QList<DeclarationDepthPair>& declarations = context->allDeclarations(
            context == m_duContext ? m_position : CursorInRevision::invalid(),
            context->topContext()
        );

        foreach (const DeclarationDepthPair& decl, declarations) {
            DeclarationPointer declaration(decl.first);

            if (decl.first->kind() == Declaration::NamespaceAlias) {
                continue;
            } else if (decl.first->qualifiedIdentifier().isEmpty()) {
                continue;
            }

            items << CompletionTreeItemPointer(new CompletionItem(declaration, decl.second));
        }
    }

    return items;
}

QList<CompletionTreeItemPointer> CodeCompletionContext::globalCompletions()
{
    QList<CompletionTreeItemPointer> items;

    // Iterate over all the imported namespaces and add their definitions
    DUChainReadLocker lock;
    QList<Declaration*> imports = m_duContext->findDeclarations(globalImportIdentifier());
    QList<Declaration*> realImports;

    foreach (Declaration* import, imports) {
        if (import->kind() != Declaration::NamespaceAlias) {
            continue;
        }

        NamespaceAliasDeclaration* decl = static_cast<NamespaceAliasDeclaration *>(import);
        realImports << m_duContext->findDeclarations(decl->importIdentifier());
    }

    lock.unlock();

    foreach (Declaration* import, realImports) {
        items << completionsInContext(DUContextPointer(import->internalContext()));
    }

    return items;
}

}
