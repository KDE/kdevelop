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
#include <QtCore/QStack>

#include "contextbuilder.h"

typedef KDevelop::AbstractUseBuilder<QmlJS::AST::Node, QmlJS::AST::IdentifierPropertyName, ContextBuilder> UseBuilderBase;

class KDEVQMLJSDUCHAIN_EXPORT UseBuilder : public UseBuilderBase
{
public:
    UseBuilder(ParseSession* session);

protected:
    using Visitor::visit;
    using Visitor::preVisit;
    using Visitor::postVisit;

    virtual bool preVisit(QmlJS::AST::Node* node);
    virtual void postVisit(QmlJS::AST::Node* node);

    virtual bool visit(QmlJS::AST::FieldMemberExpression* node);
    virtual bool visit(QmlJS::AST::IdentifierExpression* node);
    virtual bool visit(QmlJS::AST::UiQualifiedId* node);
    virtual bool visit(QmlJS::AST::UiImport* node);
    virtual bool visit(QmlJS::AST::UiPublicMember* node);

private:
    void useForExpression(QmlJS::AST::Node* node, const KDevelop::RangeInRevision &range = KDevelop::RangeInRevision::invalid());

private:
    QStack<QmlJS::AST::Node*> m_nodesThatOpenedContexts;
};

#endif // USEBUILDER_H
