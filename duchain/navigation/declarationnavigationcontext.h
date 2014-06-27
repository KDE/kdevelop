/*
 * This file is part of qmljs, the QML/JS language support plugin for KDevelop
 * Copyright (c) 2014 Denis Steckelmacher <steckdenis@yahoo.fr>
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
#ifndef __DECLARATIONNAVIGATIONCONTEXT_H__
#define __DECLARATIONNAVIGATIONCONTEXT_H__

#include <language/duchain/navigation/abstractdeclarationnavigationcontext.h>
#include <language/duchain/types/identifiedtype.h>

namespace QmlJS {

class DeclarationNavigationContext : public KDevelop::AbstractDeclarationNavigationContext
{
public:
    DeclarationNavigationContext(KDevelop::DeclarationPointer decl,
                                 KDevelop::TopDUContextPointer topContext,
                                 KDevelop::AbstractNavigationContext* previousContext = 0);

protected:
    virtual void htmlIdentifiedType(KDevelop::AbstractType::Ptr type, const KDevelop::IdentifiedType* idType);
};

}

#endif