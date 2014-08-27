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
#include "navigationwidget.h"
#include "declarationnavigationcontext.h"
#include "includenavigationcontext.h"
#include "macronavigationcontext.h"

namespace Cpp {
using namespace KDevelop;

NavigationWidget::NavigationWidget(KDevelop::DeclarationPointer declaration, KDevelop::TopDUContextPointer topContext, const QString& htmlPrefix, const QString& htmlSuffix)
  : m_declaration(declaration)
{
  m_topContext = topContext;

  initBrowser(400);

  //The first context is registered so it is kept alive by the shared-pointer mechanism
  m_startContext = NavigationContextPointer(new DeclarationNavigationContext(declaration, m_topContext));
  m_startContext->setPrefixSuffix( htmlPrefix, htmlSuffix );
  setContext( m_startContext );
}

NavigationWidget::NavigationWidget(const IncludeItem& includeItem, KDevelop::TopDUContextPointer topContext, const QString& htmlPrefix, const QString& htmlSuffix)
{
  m_topContext = topContext;

  initBrowser(200);

  //The first context is registered so it is kept alive by the shared-pointer mechanism
  m_startContext = NavigationContextPointer(new IncludeNavigationContext(includeItem, m_topContext));
  m_startContext->setPrefixSuffix( htmlPrefix, htmlSuffix );
  setContext( m_startContext );
}

NavigationWidget::NavigationWidget(const rpp::pp_macro& macro, const QString& preprocessedBody, const QString& htmlPrefix, const QString& htmlSuffix)
{
  initBrowser(200);

  //The first context is registered so it is kept alive by the shared-pointer mechanism
  m_startContext = NavigationContextPointer(new MacroNavigationContext(macro, preprocessedBody));
  m_startContext->setPrefixSuffix( htmlPrefix, htmlSuffix );
  setContext( m_startContext );
}



QString NavigationWidget::shortDescription(KDevelop::Declaration* declaration) {
  NavigationContextPointer ctx(new DeclarationNavigationContext(DeclarationPointer(declaration), TopDUContextPointer())); ///@todo give correct top-context
  return ctx->html(true);
}

QString NavigationWidget::shortDescription(const IncludeItem& includeItem) {
  NavigationContextPointer ctx(new IncludeNavigationContext(includeItem, TopDUContextPointer())); ///@todo give correct top-context
  return ctx->html(true);
}

}

