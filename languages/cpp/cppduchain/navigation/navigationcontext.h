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
#ifndef CPP_NAVIGATIONCONTEXT_H
#define CPP_NAVIGATIONCONTEXT_H

#include <language/duchain/navigation/abstractnavigationcontext.h>
#include <language/duchain/types/abstracttype.h>

namespace Cpp {

class NavigationContext : public KDevelop::AbstractNavigationContext
{
  public:
    NavigationContext( KDevelop::DeclarationPointer decl, KDevelop::TopDUContextPointer topContext, KDevelop::AbstractNavigationContext* previousContext = 0 );

   virtual QString name() const;
   virtual QString html(bool shorten = false);

  protected:
    ///Creates and registers a link for the given type that jumps to its declaration and to the template-argument declarations
    void eventuallyMakeTypeLinks( KDevelop::AbstractType::Ptr type );

    virtual KDevelop::NavigationContextPointer registerChild(KDevelop::DeclarationPointer declaration);

};

}

#endif
