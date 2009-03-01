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

#ifndef ABSTRACTDECLARATIONNAVIGATIONCONTEXT_H
#define ABSTRACTDECLARATIONNAVIGATIONCONTEXT_H

#include "abstractnavigationcontext.h"
#include "../duchainpointer.h"
#include "../types/structuretype.h"
#include <language/duchain/declaration.h>

namespace KDevelop {

class KDEVPLATFORMLANGUAGE_EXPORT AbstractDeclarationNavigationContext : public AbstractNavigationContext
{
  public:
    AbstractDeclarationNavigationContext( DeclarationPointer decl, KDevelop::TopDUContextPointer topContext, AbstractNavigationContext* previousContext = 0 );

    virtual QString name() const;
    virtual QString html(bool shorten = false);

    DeclarationPointer declaration() const;

  protected:
    DeclarationPointer m_declaration;

    
    static QString stringFromAccess(Declaration::AccessPolicy access);
    static QString stringFromAccess(DeclarationPointer decl);
    static QString declarationName( DeclarationPointer decl );
    static QStringList declarationDetails(DeclarationPointer decl);

    
    ///Print the function-signature in a way that return-type and argument can be jumped to
    virtual void htmlFunction();
    ///Navigation for additional less important links, like what function was overloaded etc.
    virtual void htmlAdditionalNavigation();

    virtual void htmlClass();
    virtual void htmlIdentifiedType(AbstractType::Ptr type, const IdentifiedType* idType);

    ///Creates and registers a link for the given type that jumps to its declaration and to the template-argument declarations
    virtual void eventuallyMakeTypeLinks( KDevelop::AbstractType::Ptr type );

    virtual NavigationContextPointer executeKeyAction(QString key);

    ///Creates a link that triggers a recomputation of this context with m_fullBackwardSearch set to true
    void createFullBackwardSearchLink(QString string);
    
    bool m_fullBackwardSearch;
};

}

#endif
