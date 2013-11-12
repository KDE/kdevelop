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

#ifndef EXPRESSIONVISITOR_H
#define EXPRESSIONVISITOR_H

#include <QStack>

#include <language/duchain/types/integraltype.h>

#include <qmljs/parser/qmljsast_p.h>

#include "duchainexport.h"

KDevelop::AbstractType::Ptr findType(QmlJS::AST::Node* node);

class KDEVQMLJSDUCHAIN_EXPORT ExpressionVisitor : public QmlJS::AST::Visitor
{
public:
    using Visitor::visit;
    using Visitor::endVisit;

    KDevelop::AbstractType::Ptr lastType();

protected:
    virtual void endVisit(QmlJS::AST::ArrayLiteral*);
    virtual void endVisit(QmlJS::AST::NumericLiteral*);
    virtual void endVisit(QmlJS::AST::StringLiteral*);

private:
    QStack<KDevelop::AbstractType::Ptr> m_lastType;

};

#endif // EXPRESSIONVISITOR_H
