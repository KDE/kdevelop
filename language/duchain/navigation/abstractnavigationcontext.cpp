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

#include <QTextDocument>
#include <klocale.h>

#include "abstractdeclarationnavigationcontext.h"
#include "abstractnavigationwidget.h"
#include "usesnavigationcontext.h"
#include "../../../interfaces/icore.h"
#include "../../../interfaces/idocumentcontroller.h"
#include "../functiondeclaration.h"
#include "../namespacealiasdeclaration.h"
#include "../types/functiontype.h"
#include "../types/structuretype.h"
#include <interfaces/icontextbrowser.h>
#include <interfaces/idocumentationcontroller.h>
#include <interfaces/iplugincontroller.h>


namespace KDevelop {
  
void AbstractNavigationContext::setTopContext(KDevelop::TopDUContextPointer context) {
  m_topContext = context;
}

KDevelop::TopDUContextPointer AbstractNavigationContext::topContext() const {
  return m_topContext;
}


AbstractNavigationContext::AbstractNavigationContext( KDevelop::TopDUContextPointer topContext, AbstractNavigationContext* previousContext)
  : m_selectedLink(0), m_shorten(false), m_linkCount(-1), m_currentPositionLine(0),
    m_previousContext(previousContext), m_topContext(topContext)
{
}

void AbstractNavigationContext::addExternalHtml( const QString& text )
{
  int lastPos = 0;
  int pos = 0;
  QString fileMark = "KDEV_FILE_LINK{";
  while( pos < text.length() && (pos = text.indexOf( fileMark, pos)) != -1 ) {
    modifyHtml() +=  text.mid(lastPos, pos-lastPos);

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

  modifyHtml() +=  text.mid(lastPos, text.length()-lastPos);
}

void AbstractNavigationContext::makeLink( const QString& name, DeclarationPointer declaration, NavigationAction::Type actionType )
{
  NavigationAction action( declaration, actionType );
  QString targetId = QString::number((quint64)declaration.data() * actionType);
  makeLink(name, targetId, action);
}

QString AbstractNavigationContext::createLink(const QString& name, QString targetId, const NavigationAction& action)
{
  if(m_shorten) {
    //Do not create links in shortened mode, it's only for viewing
    return typeHighlight(name.toHtmlEscaped());
  }
  
  m_links[ targetId ] = action;
  m_intLinks[ m_linkCount ] = action;
  m_linkLines[ m_linkCount ] = m_currentLine;
  if(m_currentPositionLine == m_currentLine) {
    m_currentPositionLine = -1;
    m_selectedLink = m_linkCount;
  }
  
  QString str = name.toHtmlEscaped();
  if( m_linkCount == m_selectedLink )
    str = "<font style=\"background-color:#f1f1f1;\" color=\"#880088\">" + str + "</font>";

  QString ret =  "<a href=\"" + targetId + "\"" + ((m_linkCount == m_selectedLink && m_currentPositionLine == -1) ? QString(" name = \"currentPosition\"") : QString()) + ">" + str + "</a>";

  if( m_selectedLink == m_linkCount )
    m_selectedLinkAction = action;

  ++m_linkCount;
  return ret;
}

void AbstractNavigationContext::makeLink( const QString& name, QString targetId, const NavigationAction& action)
{
  modifyHtml() += createLink(name, targetId, action);
}

void AbstractNavigationContext::clear() {
    m_linkCount = 0;
    m_currentLine = 0;
    m_currentText.clear();
    m_links.clear();
    m_intLinks.clear();
    m_linkLines.clear();
}


NavigationContextPointer AbstractNavigationContext::executeLink (QString link)
{
  if(!m_links.contains(link))
    return NavigationContextPointer(this);

  return execute(m_links[link]);
}

NavigationContextPointer AbstractNavigationContext::executeKeyAction(QString key) {
  Q_UNUSED(key);
  return NavigationContextPointer(this);
}

NavigationContextPointer AbstractNavigationContext::execute(const NavigationAction& action)
{
  if(action.targetContext)
    return NavigationContextPointer(action.targetContext);

  if(action.type == NavigationAction::ExecuteKey)
    return executeKeyAction(action.key);


  if( !action.decl && (action.type != NavigationAction::JumpToSource || action.document.isEmpty()) ) {
      kDebug() << "Navigation-action has invalid declaration" << endl;
      return NavigationContextPointer(this);
  }
  qRegisterMetaType<KUrl>("KUrl");
  qRegisterMetaType<KTextEditor::Cursor>("KTextEditor::Cursor");

  switch( action.type ) {
    case NavigationAction::ExecuteKey:
      break;
    case NavigationAction::None:
      kDebug() << "Tried to execute an invalid action in navigation-widget" << endl;
      break;
    case NavigationAction::NavigateDeclaration:
    {
      AbstractDeclarationNavigationContext* ctx = dynamic_cast<AbstractDeclarationNavigationContext*>(m_previousContext);
      if( ctx && ctx->declaration() == action.decl )
        return NavigationContextPointer( m_previousContext );
      return AbstractNavigationContext::registerChild(action.decl);
    } break;
    case NavigationAction::NavigateUses:
    {
      IContextBrowser* browser = ICore::self()->pluginController()->extensionForPlugin<IContextBrowser>();
      if (browser) {
        browser->showUses(action.decl);
        return NavigationContextPointer(this);
      }
      // fall-through
    }
    case NavigationAction::ShowUses:
      return registerChild(new UsesNavigationContext(action.decl.data(), this));
    case NavigationAction::JumpToSource:
      {
        KUrl doc = action.document;
        KTextEditor::Cursor cursor = action.cursor;
        {
          DUChainReadLocker lock(DUChain::lock());
          if(action.decl) {
            if(doc.isEmpty()) {
              doc = action.decl->url().toUrl();
    /*          if(action.decl->internalContext())
                cursor = action.decl->internalContext()->range().start() + KTextEditor::Cursor(0, 1);
              else*/
                cursor = action.decl->rangeInCurrentRevision().start();
            }

            action.decl->activateSpecialization();
          }
        }

        //This is used to execute the slot delayed in the event-loop, so crashes are avoided
        QMetaObject::invokeMethod( ICore::self()->documentController(), "openDocument", Qt::QueuedConnection, Q_ARG(KUrl, doc), Q_ARG(KTextEditor::Cursor, cursor) );
        break;
      }
    case NavigationAction::ShowDocumentation: {
        QExplicitlySharedDataPointer<IDocumentation> doc=ICore::self()->documentationController()->documentationForDeclaration(action.decl.data());
        ICore::self()->documentationController()->showDocumentation(doc);
      }
      break;
  }

  return NavigationContextPointer( this );
}

void AbstractNavigationContext::setPreviousContext(KDevelop::AbstractNavigationContext* previous) {
  m_previousContext = previous;
}

NavigationContextPointer AbstractNavigationContext::registerChild( AbstractNavigationContext* context ) {
  m_children << NavigationContextPointer(context);
  return m_children.last();
}

NavigationContextPointer AbstractNavigationContext::registerChild(DeclarationPointer declaration) {
  //We create a navigation-widget here, and steal its context.. evil ;)
  QWidget* navigationWidget = declaration->context()->createNavigationWidget(declaration.data());
  NavigationContextPointer ret;
  AbstractNavigationWidget* abstractNavigationWidget = dynamic_cast<AbstractNavigationWidget*>(navigationWidget);
  if(abstractNavigationWidget)
    ret = abstractNavigationWidget->context();
  delete navigationWidget;
  ret->setPreviousContext(this);
  m_children << ret;
  return ret;
}

const int lineJump = 3;

void AbstractNavigationContext::down() {
  //Make sure link-count is valid
  if( m_linkCount == -1 )
    html();

  int fromLine = m_currentPositionLine;

  if(m_selectedLink >= 0 && m_selectedLink < m_linkCount) {

    if(fromLine == -1)
      fromLine = m_linkLines[m_selectedLink];

    for(int newSelectedLink = m_selectedLink+1; newSelectedLink < m_linkCount; ++newSelectedLink) {
      if(m_linkLines[newSelectedLink] > fromLine && m_linkLines[newSelectedLink] - fromLine <= lineJump) {
        m_selectedLink = newSelectedLink;
        m_currentPositionLine = -1;
        return;
      }
    }
  }
  if(fromLine == -1)
    fromLine = 0;

  m_currentPositionLine = fromLine + lineJump;

  if(m_currentPositionLine > m_currentLine)
    m_currentPositionLine = m_currentLine;
}

void AbstractNavigationContext::up() {
  //Make sure link-count is valid
  if( m_linkCount == -1 )
    html();

  int fromLine = m_currentPositionLine;

  if(m_selectedLink >= 0 && m_selectedLink < m_linkCount) {

    if(fromLine == -1)
      fromLine = m_linkLines[m_selectedLink];

    for(int newSelectedLink = m_selectedLink-1; newSelectedLink >= 0; --newSelectedLink) {
      if(m_linkLines[newSelectedLink] < fromLine && fromLine - m_linkLines[newSelectedLink] <= lineJump) {
        m_selectedLink = newSelectedLink;
        m_currentPositionLine = -1;
        return;
      }
    }
  }

  if(fromLine == -1)
    fromLine = m_currentLine;

  m_currentPositionLine = fromLine - lineJump;
  if(m_currentPositionLine < 0)
    m_currentPositionLine = 0;
}

void AbstractNavigationContext::nextLink()
{
  //Make sure link-count is valid
  if( m_linkCount == -1 )
    html();

  m_currentPositionLine = -1;

  if( m_linkCount > 0 )
    m_selectedLink = (m_selectedLink+1) % m_linkCount;
}

void AbstractNavigationContext::previousLink()
{
  //Make sure link-count is valid
  if( m_linkCount == -1 )
    html();

  m_currentPositionLine = -1;

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

NavigationContextPointer AbstractNavigationContext::back() {
  if(m_previousContext)
    return NavigationContextPointer(m_previousContext);
  else
    return NavigationContextPointer(this);
}

NavigationContextPointer AbstractNavigationContext::accept() {
  if( m_selectedLink >= 0 &&  m_selectedLink < m_linkCount )
  {
    NavigationAction action = m_intLinks[m_selectedLink];
    return execute(action);
  }
  return NavigationContextPointer(this);
}

NavigationContextPointer AbstractNavigationContext::accept(IndexedDeclaration decl) {
  if(decl.data()) {
    NavigationAction action(DeclarationPointer(decl.data()), NavigationAction::NavigateDeclaration);
    return execute(action);
  }else{
    return NavigationContextPointer(this);
  }
}

NavigationContextPointer AbstractNavigationContext::acceptLink(const QString& link) {
  if( !m_links.contains(link) ) {
    kDebug() << "Executed unregistered link " << link << endl;
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
  } else if( decl->kind() == Declaration::Instance ) {
    kind = i18n("Variable");
  } else if ( decl->kind() == Declaration::Namespace ) {
    kind = i18n("Namespace");
  }

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

QString AbstractNavigationContext::html(bool shorten) {
  m_shorten = shorten;
  return QString();
}

bool AbstractNavigationContext::alreadyComputed() const {
  return !m_currentText.isEmpty();
}

bool AbstractNavigationContext::isWidgetMaximized() const
{
  return true;
}

QWidget* AbstractNavigationContext::widget() const {
  return 0;
}

///Splits the string by the given regular expression, but keeps the split-matches at the end of each line
static QStringList splitAndKeep(QString str, QRegExp regExp) {
  QStringList ret;
  int place = regExp.indexIn(str);
  while(place != -1) {
    ret << str.left(place + regExp.matchedLength());
    str = str.mid(place + regExp.matchedLength());
    place = regExp.indexIn(str);
  }
  ret << str;
  return ret;
}

void AbstractNavigationContext::addHtml(QString html) {
  QRegExp newLineRegExp("<br>|<br */>");
  foreach(const QString& line, splitAndKeep(html, newLineRegExp)) {
    m_currentText +=  line;
    if(line.indexOf(newLineRegExp) != -1) {
      ++m_currentLine;
      if(m_currentLine == m_currentPositionLine) {
        m_currentText += "<font color=\"#880088\"> <a name = \"currentPosition\" >" + QString("<->").toHtmlEscaped() + "</a> </font>";
      }
    }
  }
}

QString AbstractNavigationContext::currentHtml() const {
  return m_currentText;
}

QString AbstractNavigationContext::fontSizePrefix(bool /*shorten*/) const
{
  return QString();
}

QString AbstractNavigationContext::fontSizeSuffix(bool /*shorten*/) const
{
  return QString();
}

QString Colorizer::operator() ( const QString& str ) const
{
  QString ret = "<font color=\"#" + m_color + "\">" + str + "</font>";

  if( m_formatting & Fixed )
    ret = "<tt>"+ret+"</tt>";
  if ( m_formatting & Bold )
    ret = "<b>"+ret+"</b>";
  if ( m_formatting & Italic )
    ret = "<i>"+ret+"</i>";

  return ret;
}

const Colorizer AbstractNavigationContext::typeHighlight("006000");
const Colorizer AbstractNavigationContext::errorHighlight("990000");
const Colorizer AbstractNavigationContext::labelHighlight("000000");
const Colorizer AbstractNavigationContext::codeHighlight("005000");
const Colorizer AbstractNavigationContext::propertyHighlight("009900");
const Colorizer AbstractNavigationContext::navigationHighlight("000099");
const Colorizer AbstractNavigationContext::importantHighlight("000000", Colorizer::Bold | Colorizer::Italic);
const Colorizer AbstractNavigationContext::commentHighlight("303030");
const Colorizer AbstractNavigationContext::nameHighlight("000000", Colorizer::Bold);

}
