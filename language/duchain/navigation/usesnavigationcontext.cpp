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

#include "usesnavigationcontext.h"

#include "useswidget.h"
#include <klocale.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>

using namespace KDevelop;

UsesNavigationContext::UsesNavigationContext( KDevelop::IndexedDeclaration declaration, AbstractNavigationContext* previousContext ) : AbstractNavigationContext(TopDUContextPointer(), previousContext), m_declaration(declaration) {
  m_widget = new UsesWidget(m_declaration);
}

UsesNavigationContext::~UsesNavigationContext() {
  delete m_widget;
}

QString UsesNavigationContext::name() const {
  return "Uses";
}

QString UsesNavigationContext::html(bool shorten) {
  clear();
  modifyHtml()  += "<html><body><p>" + fontSizePrefix(shorten);
  
  if( m_previousContext ) {
    modifyHtml() += navigationHighlight(i18n("Uses of "));
    makeLink( m_previousContext->name(), m_previousContext->name(), NavigationAction(m_previousContext) );
  }else{
    KDevelop::DUChainReadLocker lock(DUChain::lock());
    if(Declaration* decl = m_declaration.data()) {
      makeLink( i18n("Uses of %1", decl->toString()), DeclarationPointer(decl), NavigationAction::NavigateDeclaration);
    }
  }
  
  modifyHtml() += fontSizeSuffix(shorten) + "</p></body></html>";

  return currentHtml();
}

QWidget* UsesNavigationContext::widget() const {
  return m_widget;
}

