/* This file is part of KDevelop
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

#include "cppducontext.h"
#include "navigationwidget.h"

namespace Cpp {

QMutex cppDuContextInstantiationsMutex(QMutex::Recursive);

template<>
QWidget* CppDUContext<TopDUContext>::createNavigationWidget( Declaration* decl, const QString& htmlPrefix, const QString& htmlSuffix ) const {
  if( decl == 0 ) {
    IncludeItem i;
    i.pathNumber = -1;
    i.name = url().fileName();
    i.isDirectory = false;
    i.basePath = url().upUrl();
    
    return new NavigationWidget( i, htmlPrefix, htmlSuffix );
  } else {
    return new NavigationWidget( DeclarationPointer(decl), htmlPrefix, htmlSuffix );
  }
}

template<>
QWidget* CppDUContext<DUContext>::createNavigationWidget(Declaration* decl, const QString& htmlPrefix, const QString& htmlSuffix) const {
  if( decl == 0 ) {
    if( owner() && owner()->asDeclaration() )
      return new NavigationWidget( DeclarationPointer(owner()->asDeclaration()), htmlPrefix, htmlSuffix );
    else
      return 0;
  } else {
    return new NavigationWidget( DeclarationPointer(decl), htmlPrefix, htmlSuffix );
  }
}

}
