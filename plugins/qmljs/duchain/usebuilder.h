/*************************************************************************************
 *  Copyright (C) 2013 by Andrea Scarpino <scarpino@kde.org>                         *
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

#ifndef USEBUILDER_H
#define USEBUILDER_H

#include <language/duchain/builders/abstractusebuilder.h>
#include <util/stack.h>

#include "contextbuilder.h"

using UseBuilderBase = KDevelop::AbstractUseBuilder<QmlJS::AST::Node, QmlJS::AST::IdentifierPropertyName, ContextBuilder>;

class KDEVQMLJSDUCHAIN_EXPORT UseBuilder : public UseBuilderBase
{
public:
    explicit UseBuilder(ParseSession* session);

protected:
    using Visitor::visit;
    using Visitor::preVisit;
    using Visitor::postVisit;

    bool preVisit(QmlJS::AST::Node* node) override;
    void postVisit(QmlJS::AST::Node* node) override;

    bool visit(QmlJS::AST::FieldMemberExpression* node) override;
    bool visit(QmlJS::AST::IdentifierExpression* node) override;
    bool visit(QmlJS::AST::UiQualifiedId* node) override;
    bool visit(QmlJS::AST::UiImport* node) override;
    bool visit(QmlJS::AST::UiPublicMember* node) override;
    bool visit(QmlJS::AST::UiScriptBinding* node) override;

private:
    void useForExpression(QmlJS::AST::Node* node, const KDevelop::RangeInRevision &range = KDevelop::RangeInRevision::invalid());

private:
    KDevelop::Stack<QmlJS::AST::Node*> m_nodesThatOpenedContexts;
};

#endif // USEBUILDER_H
