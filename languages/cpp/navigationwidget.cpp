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

#include <ktextbrowser.h>
#include <QMap>
#include <QStringList>
#include <QMetaObject>

#include <klocale.h>

#include <duchain/declaration.h>
#include <duchain/ducontext.h>
#include <duchain/typesystem.h>
#include <duchain/functiondeclaration.h>
#include <duchain/classfunctiondeclaration.h>
#include <duchain/classmemberdeclaration.h>

#include <icore.h>
#include <idocumentcontroller.h>

#include "cpplanguagesupport.h"
#include "cpptypes.h"
#include "typeutils.h"


using namespace KDevelop;

namespace Cpp {

QString declarationName( Declaration* decl ) {
  if( !decl )
    return i18nc("An unkown c++ declaration that is unknown", "Unknown");
  else
    return decl->qualifiedIdentifier().toString();
}

struct NavigationAction {
  enum Type {
    None,
    NavigateDeclaration,
    JumpToSource
  };

  NavigationAction() : type(None) {
  }

  NavigationAction( DeclarationPointer decl_, Type type_ ) : decl(decl_), type(type_) {
  }

  DeclarationPointer decl;
  Type type;
};
  
class NavigationContext : public KShared {
  public:
    NavigationContext( DeclarationPointer decl, NavigationContext* previousContext = 0 ) : m_declaration(decl), m_selectedLink(0), m_linkCount(-1), m_previousContext(previousContext) {
    }

    void nextLink()
    {
      //Make sure link-count is valid
      if( m_linkCount == -1 )
        html();
      
      if( m_linkCount > 0 )
        m_selectedLink = (m_selectedLink+1) % m_linkCount;
    }
    
    void previousLink()
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

    NavigationContextPointer accept() {
      if( m_selectedLink >= 0 &&  m_selectedLink < m_linkCount )
      {
        NavigationAction action = m_intLinks[m_selectedLink];
        return execute(action);
      }
      return NavigationContextPointer(this);
    }

    NavigationContextPointer acceptLink(const QString& link) {
      if( !m_links.contains(link) ) {
        kDebug(9007) << "Executed unregistered link " << link << endl;
        return NavigationContextPointer(this);
      }

      return execute(m_links[link]);
    }
    

    NavigationAction currentAction() const {
      return m_selectedLinkAction;
    }

    QString html() {
      m_linkCount = 0;
      m_currentText  = "<html><body><p><small><small>";
      if( m_previousContext ) {
        m_currentText += "Back to ";
        makeLink( declarationName(m_previousContext->m_declaration.data()), m_previousContext->m_declaration, NavigationAction::NavigateDeclaration );
        m_currentText += "<br>";
      }

      if( m_declaration )
      {
        QStringList details;
        QString kind;
        QString access;
        
        const AbstractFunctionDeclaration* function = dynamic_cast<const AbstractFunctionDeclaration*>(m_declaration.data());

        const ClassMemberDeclaration* memberDecl = dynamic_cast<const ClassMemberDeclaration*>(m_declaration.data());

        if( memberDecl ) {
          if( memberDecl->isMutable() )
            details << "mutable";
          if( memberDecl->isRegister() )
            details << "register";
          if( memberDecl->isStatic() )
            details << "static";
          if( memberDecl->isAuto() )
            details << "auto";
          if( memberDecl->isExtern() )
            details << "extern";
          if( memberDecl->isFriend() )
            details << "friend";

          switch( memberDecl->accessPolicy() ) {
            case ClassMemberDeclaration::Private:
              access = "private";
              break;
            case ClassMemberDeclaration::Public:
              access = "public";
              break;
            case ClassMemberDeclaration::Protected:
              access = "protected";
              break;
          }
        }

        if( m_declaration->isTypeAlias() )
          kind = "Typedef";
        else if( m_declaration->kind() == Declaration::Type )
          kind = "Type";

        

        if( m_declaration->isDefinition() )
          details << "definition";

        if( memberDecl && memberDecl->isForwardDeclaration() )
          details << "forward";

        const CppCVType* cvType = dynamic_cast<const CppCVType*>( m_declaration->abstractType().data() );
        if( cvType ) {
          if( cvType->isConstant() )
            details << "constant";
          if( cvType->isVolatile() )
            details << "volatile";
        }

        if( m_declaration->kind() == Declaration::Instance )
          kind = "Variable";

        if( function ) {
          kind = "Function";

          if( function->isInline() )
            details << "inline";
          
          if( function->isExplicit() )
            details << "explicit";

          if( function->isVirtual() )
            details << "virtual";

          const ClassFunctionDeclaration* classFunDecl = dynamic_cast<const ClassFunctionDeclaration*>(m_declaration.data());

          if( classFunDecl ) {
            if( classFunDecl->functionType() == ClassFunctionDeclaration::Signal )
              details << "signal";
            if( classFunDecl->functionType() == ClassFunctionDeclaration::Slot )
              details << "slot";
            if( classFunDecl->isConstructor() )
              details << "constructor";
            if( classFunDecl->isDestructor() )
              details << "destructor";
            if( classFunDecl->isConversionFunction() )
              details << "conversion-function";
          }

          //Print the function-signature in a way that return-type and argument can be jumped to

          const FunctionType* type = dynamic_cast<const FunctionType*>( m_declaration->abstractType().data() );

          if( type && function ) {
            if( !classFunDecl || !classFunDecl->isConstructor() || !classFunDecl->isDestructor() ) {
              eventuallyMakeTypeLink( type->returnType().data() );
              m_currentText += ' ' + m_declaration->identifier().toString();
            }

            if( type->arguments().count() == 0 )
            {
              m_currentText += "()";
            } else {
              m_currentText += "( ";
              bool first = true;

              QList<QString>::const_iterator defaultIt = function->defaultParameters().begin();
              int firstDefaultParam = type->arguments().count() - function->defaultParameters().count();
              int currentArgNum = 0;
              
              foreach( AbstractType::Ptr argType, type->arguments() )
              {
                if( !first )
                  m_currentText += ", ";
                first = false;
                
                eventuallyMakeTypeLink( argType.data() );

                if( currentArgNum >= firstDefaultParam )
                  m_currentText += " = " + function->defaultParameters()[ currentArgNum - firstDefaultParam ];

                ++currentArgNum;
              }
              
              m_currentText += " )";
            }
            m_currentText += "<br>";
          }else {
            m_currentText += "Invalid type<br>";
          }
        } else {
          if( m_declaration->isTypeAlias() || m_declaration->kind() == Declaration::Instance ) {
            if( m_declaration->isTypeAlias() )
              m_currentText += "typedef ";

            eventuallyMakeTypeLink( m_declaration->abstractType().data() );

            m_currentText += ' ' + declarationName(m_declaration.data()) + "<br>";
          }
        }

        QualifiedIdentifier identifier = m_declaration->qualifiedIdentifier();
        if( identifier.count() > 1 ) {
          if( m_declaration->context() && m_declaration->context()->declaration() ) {
            makeLink( i18n("Container: %1<br />", declarationName(m_declaration->context()->declaration()) ), m_declaration->context()->declaration(), NavigationAction::NavigateDeclaration );
          } else {
            QualifiedIdentifier parent = identifier;
            parent.pop();
            m_currentText += i18n("Scope: %1<br />", parent.toString());
          }
        }

        if( !access.isEmpty() )
          m_currentText += i18n("Access: %1<br />", access);
        
        
        ///@todo Enumerations
        
        QString detailsString;

        if( !details.isEmpty() ) {
          bool first = true;
          foreach( QString str, details ) {
            if( !first )
              detailsString += ", ";
            first = false;
            detailsString += str;
          }
        }

        if( !kind.isEmpty() ) {
          if( !detailsString.isEmpty() )
            m_currentText += i18n("Kind: %1 (%2)<br />", kind, detailsString);
          else
            m_currentText += i18n("Kind: %1<br>", kind);
        } else if( !detailsString.isEmpty() ) {
          m_currentText += i18n("Modifiers: %1<br />", kind);
        }
      }

      m_currentText += i18n( "Declaration: " );
      makeLink( QString("%1:%2:%3").arg( m_declaration->url().prettyUrl() ).arg( m_declaration->textRange().start().line() ).arg( m_declaration->textRange().start().column() ), m_declaration, NavigationAction::JumpToSource );
      m_currentText += "<br />";
      
      if( !m_declaration->comment().isEmpty() ) {
        QString comment = m_declaration->comment();
        comment.replace("\n", "<br />");
        m_currentText += comment;
      }
      m_currentText += "</small></small></p></body></html>";

      kDebug() << "printing " << m_currentText << endl;
      return m_currentText;
    }
  private:
    DeclarationPointer m_declaration;

    NavigationContextPointer execute(NavigationAction& action);

    ///Creates and registers a link for the given type that jumps to it's declaration
    void eventuallyMakeTypeLink( AbstractType* type ) {
        AbstractType* target = TypeUtils::targetType( type );
        IdentifiedType* idType = dynamic_cast<IdentifiedType*>( target );

        
        if( target && idType && idType->declaration() ) {
          makeLink( type->toString(), idType->declaration(), NavigationAction::NavigateDeclaration );
        } else {
          if( type )
            m_currentText += type->toString();
          else
            m_currentText += "<notype>";
        }
    }
    
    ///Creates and registers a link to the given declaration, labeled by the given name
    void makeLink( const QString& name, DeclarationPointer declaration, NavigationAction::Type actionType )
    {
      QString targetId = QString("%1").arg((unsigned long long)declaration.data());
      NavigationAction action( declaration, actionType );
      m_links[ targetId ] = action;
      m_intLinks[ m_linkCount ] = action;

      QString str = name;
      if( m_linkCount == m_selectedLink ) ///@todo Change background-color here instead of foreground-color
        str = "<font color=\"#880088\">" + str + "</font>";
      
      m_currentText += "<a href=\"" + targetId + "\">" + str + "</a>";


      if( m_selectedLink == m_linkCount )
        m_selectedLinkAction = action;

      ++m_linkCount;
    }

    int m_selectedLink; //The link currently selected
    NavigationAction m_selectedLinkAction; //Target of the currently selected link

    //A counter used while building the html-code to count the used links.
    int m_linkCount;
    QString m_currentText; //Here the text is built
    QMap<QString, NavigationAction> m_links;
    QMap<int, NavigationAction> m_intLinks;
    NavigationContext* m_previousContext;
};

NavigationContextPointer NavigationContext::execute(NavigationAction& action)
{
  if( !action.decl ) {
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
      if( m_previousContext && m_previousContext->m_declaration == action.decl )
        return NavigationContextPointer( m_previousContext );
      
      return NavigationContextPointer( new NavigationContext(action.decl, this) );
    } break;
    case NavigationAction::JumpToSource:
      //This is used to execute the slot delayed in the event-loop, so crashes are avoided
      QMetaObject::invokeMethod( CppLanguageSupport::self()->core()->documentController(), "openDocument", Qt::QueuedConnection, Q_ARG(KUrl, action.decl->url()), Q_ARG(KTextEditor::Cursor, action.decl->textRange().start()) );
      break;
  }
  
  return NavigationContextPointer( this );
}


NavigationWidget::NavigationWidget(KDevelop::DeclarationPointer declaration) : m_declaration(declaration)
{
    m_browser = new KTextBrowser();
    m_browser->setOpenLinks(false);
    m_browser->setOpenExternalLinks(false);
    m_browser->resize(500, 100);

    connect( m_browser, SIGNAL(destroyed(QObject*)), this, SLOT(targetDestroyed(QObject*)) );

    //The first context is registered so it is kept alive by the shared-pointer mechanism
    m_startContext = NavigationContextPointer(new NavigationContext(declaration));
    setContext( m_startContext );
}

NavigationWidget::~NavigationWidget() {
  delete m_browser;
}

QWidget* NavigationWidget::view() const {
  return m_browser;
}

void NavigationWidget::setContext(NavigationContextPointer context)
{
  m_context = context;
  update();
}

void NavigationWidget::update() {
  Q_ASSERT( m_context );
  m_browser->setHtml( m_context->html() );
}


void NavigationWidget::targetDestroyed(QObject*) {
  m_browser = 0;
  deleteLater();
}

void NavigationWidget::next() {
  Q_ASSERT( m_context );
  m_context->nextLink();
  update();
}

void NavigationWidget::previous() {
  Q_ASSERT( m_context );
  m_context->previousLink();
  update();
}

void NavigationWidget::accept() {
  Q_ASSERT( m_context );
  setContext( m_context->accept() );
}


}

#include "navigationwidget.moc"

