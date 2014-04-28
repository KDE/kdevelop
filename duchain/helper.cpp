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

#include "helper.h"

#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/types/unsuretype.h>
#include <language/duchain/types/integraltype.h>

#include <qmljs/parser/qmljsast_p.h>

namespace QmlJS
{
using namespace KDevelop;

DeclarationPointer getDeclaration(const QualifiedIdentifier& id, const DUContext* context)
{
    DUChainReadLocker lock;
    if (context) {
        foreach (Declaration* dec, context->findDeclarations(id)) {
            return DeclarationPointer(dec);
        }
    }
    return DeclarationPointer();
}

QString getQMLAttribute(QmlJS::AST::UiObjectMemberList* members, const QString& attribute)
{
    for (QmlJS::AST::UiObjectMemberList *it = members; it; it = it->next) {
        // The member needs to be a script binding whose name matches attribute
        QmlJS::AST::UiScriptBinding* binding = QmlJS::AST::cast<QmlJS::AST::UiScriptBinding*>(it->member);

        if (!binding || !binding->qualifiedId || binding->qualifiedId->name != attribute) {
            continue;
        }

        // The value of the binding must be an expression
        QmlJS::AST::ExpressionStatement* statement = QmlJS::AST::cast<QmlJS::AST::ExpressionStatement*>(binding->statement);
        if (!statement) {
            continue;
        }

        // The expression must be an identifier or a string litera
        QmlJS::AST::IdentifierExpression* identifier = QmlJS::AST::cast<QmlJS::AST::IdentifierExpression*>(statement->expression);
        QmlJS::AST::StringLiteral* string = QmlJS::AST::cast<QmlJS::AST::StringLiteral*>(statement->expression);

        if (identifier) {
            return identifier->name.toString();
        } else if (string) {
            return string->value.toString();
        }
    }

    return QString();
}

} // End of namespace QmlJS
