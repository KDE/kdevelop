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

#include "includenavigationcontext.h"

#include <QtGui/QTextDocument>

#include <klocale.h>

#include <language/duchain/duchain.h>
#include <language/duchain/parsingenvironment.h>

#include "../environmentmanager.h"

namespace Cpp {
using namespace KDevelop;

IncludeNavigationContext::IncludeNavigationContext(const IncludeItem& item, KDevelop::TopDUContextPointer topContext)
    : AbstractNavigationContext(topContext), m_item(item)
{}

TopDUContext* pickContextWithData(QList<TopDUContext*> duchains, uint maxDepth, bool forcePick = true) {
  TopDUContext* duchain = 0;

  foreach(TopDUContext* ctx, duchains) {
    if(!ctx->parsingEnvironmentFile() || ctx->parsingEnvironmentFile()->type() != KDevelop::CppParsingEnvironment)
      continue;

    if(ctx->childContexts().count() != 0 && (duchain == 0 || ctx->childContexts().count() > duchain->childContexts().count())) {
      duchain = ctx;
    }
    if(ctx->localDeclarations().count() != 0 && (duchain == 0 || ctx->localDeclarations().count() > duchain->localDeclarations().count())) {
      duchain = ctx;
    }
  }
  
  if(!duchain && maxDepth != 0) {
    if(maxDepth != 0) {
      foreach(TopDUContext* ctx, duchains) {
        QList<TopDUContext*> children;
        foreach(DUContext::Import import, ctx->importedParentContexts())
          if(import.context(0))
            children << import.context(0)->topContext();
        
        duchain = pickContextWithData(children, maxDepth-1, false);
        if(duchain)
          break;
      }
    }
  }
  
  if(!duchain && !duchains.isEmpty() && forcePick)
    duchain = duchains.first();
  
  return duchain;
}

QString IncludeNavigationContext::html(bool shorten)
{
  m_currentText  = "<html><body><p><small><small>";
  m_linkCount = 0;
  addExternalHtml(m_prefix);

  KUrl u(m_item.url());
  NavigationAction action(u, KTextEditor::Cursor(0,0));
  makeLink(u.fileName(), u.pathOrUrl(), action);
  QList<TopDUContext*> duchains = DUChain::self()->chainsForDocument(u);
  m_currentText += "<br />";
  m_currentText += "path: " + u.pathOrUrl();

  //Pick the one duchain for this document that has the most child-contexts/declarations.
  //This prevents picking a context that is empty due to header-guards.
  TopDUContext* duchain = pickContextWithData(duchains, 2);

  m_currentText += "<br />";

  if(duchain) {
    const Cpp::EnvironmentFile* f = dynamic_cast<const Cpp::EnvironmentFile*>(duchain->parsingEnvironmentFile().data());
    Q_ASSERT(f); //Should always be for c++
    m_currentText += QString("%1: %2 %3: %4 %5: %6").arg(labelHighlight(i18nc("Headers included into this header", "Included"))).arg(duchain->importedParentContexts().count()).arg(i18nc("Count of files this header was included into", "Included by")).arg(duchain->importers().count()).arg(i18nc("Count of macros defined in this header", "Defined macros")).arg(f->definedMacros().set().count());
    m_currentText += "<br />";
    if(!shorten) {
      m_currentText += labelHighlight(i18n("Declarations:")) + "<br />";
      bool first = true;
      addDeclarationsFromContext(duchain, first);
    }
  }else if(duchains.isEmpty()) {
    m_currentText += i18n("not parsed yet");
  }

  addExternalHtml(m_suffix);

  m_currentText += "</small></small></p></body></html>";
  return m_currentText;
}

QString IncludeNavigationContext::name() const
{
  return m_item.name;
}

void IncludeNavigationContext::addDeclarationsFromContext(KDevelop::DUContext* ctx, bool& first, QString indent)
{
  //m_currentText += indent + ctx->localScopeIdentifier().toString() + "{<br />";
  QVector<DUContext*> children = ctx->childContexts();
  QVector<Declaration*> declarations = ctx->localDeclarations();

  QVector<DUContext*>::const_iterator childIterator = children.begin();
  QVector<Declaration*>::const_iterator declarationIterator = declarations.begin();

  while(childIterator != children.end() || declarationIterator != declarations.end()) {

    //Show declarations/contexts in the order they appear in the file
    int currentDeclarationLine = -1;
    int currentContextLine = -1;
    if(declarationIterator != declarations.end())
      currentDeclarationLine = (*declarationIterator)->range().textRange().start().line();

    if(childIterator != children.end())
      currentDeclarationLine = (*childIterator)->range().textRange().start().line();

    if((currentDeclarationLine <= currentContextLine || currentContextLine == -1 || childIterator == children.end()) && declarationIterator != declarations.end() )
    {
      if(!(*declarationIterator)->qualifiedIdentifier().toString().isEmpty() && !(*declarationIterator)->range().isEmpty() && !(*declarationIterator)->isForwardDeclaration()) {
        //Show the declaration
        if(!first)
          m_currentText += Qt::escape(", ");
        else
          first = false;

        m_currentText += Qt::escape(indent + declarationKind(DeclarationPointer(*declarationIterator)) + " ");
        makeLink((*declarationIterator)->qualifiedIdentifier().toString(), DeclarationPointer(*declarationIterator), NavigationAction::NavigateDeclaration);
      }
      ++declarationIterator;
    } else {
      //Eventually Recurse into the context
      if((*childIterator)->type() == DUContext::Global || (*childIterator)->type() == DUContext::Namespace /*|| (*childIterator)->type() == DUContext::Class*/)
        addDeclarationsFromContext(*childIterator, first, indent + " ");
      ++childIterator;
    }
  }
  //m_currentText += "}<br />";
}

}
