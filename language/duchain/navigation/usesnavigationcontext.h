/*
   Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef KDEVPLATFORM_USESNAVIGATIONCONTEXT_H
#define KDEVPLATFORM_USESNAVIGATIONCONTEXT_H

#include "abstractnavigationwidget.h"
#include "languageexport.h"

namespace KDevelop {
  class UsesWidget;
  class KDEVPLATFORMLANGUAGE_EXPORT UsesNavigationContext : public AbstractNavigationContext {
    public:
      UsesNavigationContext( KDevelop::IndexedDeclaration declaration, AbstractNavigationContext* previousContext = 0 );
      
      ~UsesNavigationContext();
      
      virtual QString name() const;
      virtual QWidget* widget() const;
      virtual QString html(bool shorten);
    private:
      KDevelop::IndexedDeclaration m_declaration;
      UsesWidget* m_widget;
  };
}

#endif
