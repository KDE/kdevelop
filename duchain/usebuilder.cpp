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

#include "usebuilder.h"

#include "helper.h"
#include "parsesession.h"

using namespace KDevelop;

UseBuilder::UseBuilder(ParseSession* session)
: UseBuilderBase()
{
    m_session = session;
}

bool UseBuilder::visit(QmlJS::AST::IdentifierExpression* node)
{
    const RangeInRevision range(m_session->locationToRange(node->identifierToken));
    const QualifiedIdentifier id(node->name.toString());
    const DeclarationPointer decl(QmlJS::getDeclaration(id, currentContext()));

    newUse(node, range, decl);
    const bool ret = UseBuilderBase::visit(node);

    return ret;
}
