/*************************************************************************************
 *  Copyright (C) 2012 by Aleix Pol <aleixpol@kde.org>                               *
 *  Copyright (C) 2012 by Milian Wolff <mail@milianw.de>                             *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include "declarationbuilder.h"

#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>

#include "parsesession.h"

using namespace KDevelop;

DeclarationBuilder::DeclarationBuilder(ParseSession* session)
{
    m_session = session;
}

ReferencedTopDUContext DeclarationBuilder::build(const IndexedString& url,
                                                 QmlJS::AST::Node* node,
                                                 ReferencedTopDUContext updateContext)
{
    ///TODO: cleanup
    Q_ASSERT(m_session->url() == url);
    return DeclarationBuilderBase::build(url, node, updateContext);
}

bool DeclarationBuilder::visit(QmlJS::AST::FunctionDeclaration* node)
{
    const QualifiedIdentifier name(node->name.toString());
    const RangeInRevision range = ParseSession::locationToRange(node->identifierToken);
    {
        DUChainWriteLocker lock;
        openDeclaration<FunctionDeclaration>(name, range);
    }

    const bool ret = DeclarationBuilderBase::visit(node);

    closeDeclaration();

    return ret;
}

bool DeclarationBuilder::visit(QmlJS::AST::FormalParameterList* node)
{
    for (QmlJS::AST::FormalParameterList *plist = node; plist; plist = plist->next) {
        const QualifiedIdentifier name(plist->name.toString());
        const RangeInRevision range = ParseSession::locationToRange(plist->identifierToken);
        DUChainWriteLocker lock;
        openDeclaration<Declaration>(name, range);
        closeDeclaration();
    }

    return true;
}

void DeclarationBuilder::closeContext()
{
    DUChainWriteLocker lock;
    if (FunctionDeclaration* function = dynamic_cast<FunctionDeclaration*>(currentDeclaration())) {
        DUContext* ctx = currentContext();
        if (ctx->type() == DUContext::Function) {
            function->setInternalFunctionContext(ctx);
        } else {
            Q_ASSERT(ctx->type() == DUContext::Other);
            function->setInternalContext(ctx);
        }
    }
    DeclarationBuilderBase::closeContext();
}
