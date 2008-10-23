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

#include "abstractnavigationcontext.h"

#include <QtGui/QTextDocument>
#include <klocale.h>

#include "abstractdeclarationnavigationcontext.h"
#include "../../../interfaces/icore.h"
#include "../../../interfaces/idocumentcontroller.h"
#include "../functiondeclaration.h"
#include "../functiondefinition.h"
#include "../namespacealiasdeclaration.h"
#include "../classmemberdeclaration.h"
#include "../classfunctiondeclaration.h"
#include "../forwarddeclaration.h"
#include "../duchainutils.h"
#include "../types/functiontype.h"
#include "../types/enumeratortype.h"
#include "../types/enumerationtype.h"
#include "../types/referencetype.h"
#include "../types/pointertype.h"


namespace KDevelop {

AbstractNavigationContext::AbstractNavigationContext( KDevelop::TopDUContextPointer topContext, AbstractNavigationContext* previousContext)
  : m_selectedLink(0), m_linkCount(-1),
    m_previousContext(previousContext), m_topContext(topContext)
{
}

void AbstractNavigationContext::addExternalHtml( const QString& text )
{
  int lastPos = 0;
  int pos = 0;
  QString fileMark = "KDEV_FILE_LINK{";
  while( pos < text.length() && (pos = text.indexOf( fileMark, pos)) != -1 ) {
    m_currentText += text.mid(lastPos, pos-lastPos);

    pos += fileMark.length();

    if( pos != text.length() ) {
      int fileEnd = text.indexOf('}', pos);
      if( fileEnd != -1 ) {
        QString file = text.mid( pos, fileEnd - pos );
        pos = fileEnd + 1;
        makeLink( KUrl(file).fileName(), file, NavigationAction( KUrl(file), KTextEditor::Cursor() ) );
      }
    }

    lastPos = pos;
  }

  m_currentText += text.mid(lastPos, text.length()-lastPos);
}

void AbstractNavigationContext::makeLink( const QString& name, DeclarationPointer declaration, NavigationAction::Type actionType )
{
  NavigationAction action( declaration, actionType );
  QString targetId = QString("%1").arg((quint64)declaration.data());
  makeLink(name, targetId, action);
}

void AbstractNavigationContext::makeLink( const QString& name, QString targetId, const NavigationAction& action)
{
  m_links[ targetId ] = action;
  m_intLinks[ m_linkCount ] = action;

  QString str = Qt::escape(name);
  if( m_linkCount == m_selectedLink ) ///@todo Change background-color here instead of foreground-color
    str = "<font color=\"#880088\">" + str + "</font>";

  m_currentText += "<a href=\"" + targetId + "\"" + (m_linkCount == m_selectedLink ? QString(" name = \"selectedItem\"") : QString()) + ">" + str + "</a>";

  if( m_selectedLink == m_linkCount )
    m_selectedLinkAction = action;

  ++m_linkCount;
}

NavigationContextPointer AbstractNavigationContext::execute(NavigationAction& action)
{
  if(action.targetContext)
    return NavigationContextPointer(action.targetContext);

  if( !action.decl && (action.type != NavigationAction::JumpToSource || action.document.isEmpty()) ) {
      kDebug(9007) << "Navigation-action has invalid declaration" << endl;
      return NavigationContextPointer(this);
  }
  qRegisterMetaType<KUrl>("KUrl");
  qRegisterMetaType<KTextEditor::Cursor>("KTextEditor::Cursor");

  switch( action.type ) {
    case NavigationAction::None:
      kDebug(9007) << "Tried to execute an invalid action in navigation-widget" << endl;
      break;
    case NavigationAction::NavigateDeclaration:
    {
      AbstractDeclarationNavigationContext* ctx = dynamic_cast<AbstractDeclarationNavigationContext*>(m_previousContext);
      if( ctx && ctx->declaration() == action.decl )
        return NavigationContextPointer( m_previousContext );
      return registerChild(action.decl);
    } break;
  case NavigationAction::JumpToSource:
      {
        KUrl doc = action.document;
        KTextEditor::Cursor cursor = action.cursor;
        {
          DUChainReadLocker lock(DUChain::lock());
          if(action.decl) {
            if(doc.isEmpty()) {
              doc = KUrl(action.decl->url().str());
    /*          if(action.decl->internalContext())
                cursor = action.decl->internalContext()->range().textRange().start() + KTextEditor::Cursor(0, 1);
              else*/
                cursor = action.decl->range().textRange().start();
            }
    
            action.decl->activateSpecialization();
          }
        }

        //This is used to execute the slot delayed in the event-loop, so crashes are avoided
        QMetaObject::invokeMethod( ICore::self()->documentController(), "openDocument", Qt::QueuedConnection, Q_ARG(KUrl, doc), Q_ARG(KTextEditor::Cursor, cursor) );
        break;
      }
  }

  return NavigationContextPointer( this );
}

NavigationContextPointer AbstractNavigationContext::registerChild( AbstractNavigationContext* context ) {
  m_children << NavigationContextPointer(context);
  return m_children.last();
}

void AbstractNavigationContext::nextLink()
{
  //Make sure link-count is valid
  if( m_linkCount == -1 )
    html();

  if( m_linkCount > 0 )
    m_selectedLink = (m_selectedLink+1) % m_linkCount;
}

void AbstractNavigationContext::previousLink()
{
  //Make sure link-count is valid
  if( m_linkCount == -1 )
    html();

  if( m_linkCount > 0 ) {
    --m_selectedLink;
    if( m_selectedLink <  0 )
      m_selectedLink += m_linkCount;
  }

  Q_ASSERT(m_selectedLink >= 0);
}

void AbstractNavigationContext::setPrefixSuffix( const QString& prefix, const QString& suffix ) {
  m_prefix = prefix;
  m_suffix = suffix;
}

NavigationContextPointer AbstractNavigationContext::accept() {
  if( m_selectedLink >= 0 &&  m_selectedLink < m_linkCount )
  {
    NavigationAction action = m_intLinks[m_selectedLink];
    return execute(action);
  }
  return NavigationContextPointer(this);
}

NavigationContextPointer AbstractNavigationContext::acceptLink(const QString& link) {
  if( !m_links.contains(link) ) {
    kDebug(9007) << "Executed unregistered link " << link << endl;
    return NavigationContextPointer(this);
  }

  return execute(m_links[link]);
}


NavigationAction AbstractNavigationContext::currentAction() const {
  return m_selectedLinkAction;
}


QString AbstractNavigationContext::declarationKind(DeclarationPointer decl)
{
  const AbstractFunctionDeclaration* function = dynamic_cast<const AbstractFunctionDeclaration*>(decl.data());

  QString kind;

  if( decl->isTypeAlias() )
    kind = i18n("Typedef");
  else if( decl->kind() == Declaration::Type ) {
    if( decl->type<StructureType>() )
      kind = i18n("Class");
  }

  if( decl->kind() == Declaration::Instance )
    kind = i18n("Variable");

  if( NamespaceAliasDeclaration* alias = dynamic_cast<NamespaceAliasDeclaration*>(decl.data()) ) {
    if( alias->identifier().isEmpty() )
      kind = i18n("Namespace import");
    else
      kind = i18n("Namespace alias");
  }

  if(function)
    kind = i18n("Function");

  if( decl->isForwardDeclaration() )
    kind = i18n("Forward Declaration");

  return kind;
}

const Colorizer AbstractNavigationContext::errorHighlight("990000");
const Colorizer AbstractNavigationContext::labelHighlight("000035");
const Colorizer AbstractNavigationContext::codeHighlight("005000");
const Colorizer AbstractNavigationContext::propertyHighlight("009900");
const Colorizer AbstractNavigationContext::navigationHighlight("000099");
const Colorizer AbstractNavigationContext::importantHighlight("000000", true, true);
const Colorizer AbstractNavigationContext::commentHighlight("000000", false, true);
const Colorizer AbstractNavigationContext::nameHighlight("000000", true, false);

}
