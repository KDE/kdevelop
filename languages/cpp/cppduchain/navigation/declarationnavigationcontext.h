/*
   Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef CPP_DECLARATIONNAVIGATIONCONTEXT_H
#define CPP_DECLARATIONNAVIGATIONCONTEXT_H

#include <language/duchain/navigation/abstractdeclarationnavigationcontext.h>
#include <language/duchain/types/abstracttype.h>

namespace Cpp {

class DeclarationNavigationContext : public KDevelop::AbstractDeclarationNavigationContext
{
  public:
    DeclarationNavigationContext( KDevelop::DeclarationPointer decl, KDevelop::TopDUContextPointer topContext, KDevelop::AbstractNavigationContext* previousContext = 0 );

  protected:
    virtual KDevelop::QualifiedIdentifier prettyQualifiedIdentifier(KDevelop::DeclarationPointer decl) const override;
    virtual KDevelop::AbstractType::Ptr typeToShow(KDevelop::AbstractType::Ptr type) override;
    virtual void setPreviousContext(AbstractNavigationContext* previous) override;
    virtual void htmlIdentifiedType(KDevelop::AbstractType::Ptr type, const KDevelop::IdentifiedType* idType) override;
};

}

#endif
