/*************************************************************************************
*  Copyright (C) 2012 by Aleix Pol <aleixpol@kde.org>                               *
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

#ifndef DECLARATIONBUILDER_H
#define DECLARATIONBUILDER_H

#include <language/duchain/builders/abstractdeclarationbuilder.h>
#include <qmljs/parser/qmljsast_p.h>
#include <qmljs/qmljsdocument.h>
#include "typebuilder.h"

class DeclarationBuilder : public KDevelop::AbstractDeclarationBuilder<QmlJS::AST::Node, QmlJS::AST::IdentifierPropertyName, TypeBuilder>
{
    public:
        DeclarationBuilder(QmlJS::Document::MutablePtr ptr);
        
        virtual KDevelop::QualifiedIdentifier identifierForNode(QmlJS::AST::IdentifierPropertyName* node);
        virtual KDevelop::RangeInRevision editorFindRange(QmlJS::AST::Node* fromNode, QmlJS::AST::Node* toNode);

    private:
        QmlJS::Document::MutablePtr m_doc;
};

#endif // DECLARATIONBUILDER_H
