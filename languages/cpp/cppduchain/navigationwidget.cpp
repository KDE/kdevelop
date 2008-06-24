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

#include <QMap>
#include <QStringList>
#include <QMetaObject>
#include <QScrollBar>

#include <klocale.h>

#include <duchain/declaration.h>
#include <duchain/ducontext.h>
#include <duchain/duchainlock.h>
#include <duchain/typesystem.h>
#include <duchain/functiondeclaration.h>
#include <duchain/forwarddeclaration.h>
#include <duchain/namespacealiasdeclaration.h>
#include <duchain/classfunctiondeclaration.h>
#include <duchain/classmemberdeclaration.h>
#include <duchain/topducontext.h>

#include <icore.h>
#include <idocumentcontroller.h>

#include <rpp/pp-macro.h>

#include "cpplanguagesupport.h"
#include "cpptypes.h"
#include "cppduchain.h"
#include "templatedeclaration.h"
#include "typeutils.h"
#include "environmentmanager.h"
#include "completionhelpers.h"
#include "parser/rpp/chartools.h"
#include "parser/rpp/macrorepository.h"

using namespace KDevelop;
using namespace rpp;

namespace Cpp {

QString htmlColor(const QString& text, const QString& color = "880088") {
  return "<font color=\"#" + color + "\">" + text + "</font>";
}

/** A helper-class for elegant colorization of html-strings .
 *
 * Initialize it with a html-color like "990000". and colorize strings
 * using operator()
 */
struct Colorizer
{
  Colorizer(const QString& color, bool bold=false, bool italic=false) : m_color(color), m_bold(bold), m_italic(italic) {
  }

  QString operator()(const QString& str) const
  {
    QString ret = htmlColor(str, m_color);
    if( m_bold )
      ret = "<b>"+ret+"</b>";

    if( m_italic )
      ret = "<i>"+ret+"</i>";
    return ret;
  }

  QString m_color;
  bool m_bold, m_italic;
};

const Colorizer errorHighlight("990000");
const Colorizer labelHighlight("000035");
const Colorizer codeHighlight("005000");
const Colorizer propertyHighlight("009900");
const Colorizer navigationHighlight("000099");
const Colorizer importantHighlight("000000", true, true);
const Colorizer commentHighlight("000000", false, true);
const Colorizer nameHighlight("000000", true, false);

QString stringFromAccess(Declaration::AccessPolicy access) {
  switch(access) {
    case Declaration::Private:
      return "private";
    case Declaration::Protected:
      return "protected";
    case Declaration::Public:
      return "public";
  }
  return "";
}
    
QString declarationName( Declaration* decl ) {

  if( NamespaceAliasDeclaration* alias = dynamic_cast<NamespaceAliasDeclaration*>(decl) ) {
    if( alias->identifier().isEmpty() )
      return "using namespace " + alias->importIdentifier().toString();
    else
      return "namespace " + alias->identifier().toString() + " = " + alias->importIdentifier().toString();
  }
  
  if( !decl )
    return i18nc("An unknown c++ declaration that is unknown", "Unknown");
  else
    return decl->identifier().toString();
}

struct NavigationAction {
  enum Type {
    None,
    NavigateDeclaration,
    JumpToSource //If this is set, the action jumps to document and cursor if they are valid, else to the declaration-position of decl
  };

  NavigationAction() : targetContext(0), type(None) {
  }

  NavigationAction( DeclarationPointer decl_, Type type_ ) : targetContext(0), decl(decl_), type(type_) {
  }

  NavigationAction( const KUrl& _document, const KTextEditor::Cursor& _cursor) : targetContext(0), document(_document), cursor(_cursor) {
    type = JumpToSource;
  }

  NavigationAction(NavigationContext* _targetContext) : targetContext(_targetContext) {
  }

  NavigationContext* targetContext; //If this is set, this action does nothing else than jumping to that context
  
  DeclarationPointer decl;
  Type type;
  
  KUrl document;
  KTextEditor::Cursor cursor;
};


  
class NavigationContext : public KShared {
  public:
    NavigationContext( DeclarationPointer decl, KDevelop::TopDUContextPointer topContext, NavigationContext* previousContext = 0 ) : m_declaration(decl), m_selectedLink(0), m_linkCount(-1), m_previousContext(previousContext), m_topContext(topContext) {
      //Jump from definition to declaration if possible
      if(m_declaration && m_declaration->isDefinition() && m_declaration->declaration())
        m_declaration = DeclarationPointer(m_declaration->declaration()); 
    }
    virtual ~NavigationContext() {
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

    void setPrefixSuffix( const QString& prefix, const QString& suffix ) {
      m_prefix = prefix;
      m_suffix = suffix;
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

    static QString declarationKind(Declaration* decl) {
      const AbstractFunctionDeclaration* function = dynamic_cast<const AbstractFunctionDeclaration*>(decl);

      QString kind;

      if( decl->isTypeAlias() )
        kind = "Typedef";
      else if( decl->kind() == Declaration::Type ) {
        if( decl->type<CppClassType>() )
          kind = i18n("Class");
        else
          i18n("Class");
      }
      
      if( decl->kind() == Declaration::Instance )
        kind = i18n("Variable");

      if( NamespaceAliasDeclaration* alias = dynamic_cast<NamespaceAliasDeclaration*>(decl) ) {
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

  virtual QString name() const {
    return declarationName(m_declaration.data());
  }
  
    virtual QString html(bool shorten = false) {

      m_linkCount = 0;
      m_currentText  = "<html><body><p><small><small>";

      addExternalHtml(m_prefix);
      
      if( shorten && !m_declaration->comment().isEmpty() ) {
        QString comment = m_declaration->comment();
        if( comment.length() > 60 ) {
          comment.truncate(60);
          comment += "...";
        }
        comment.replace('\n', ' ');
        comment.replace("<br />", " ");
        comment.replace("<br/>", " ");
        m_currentText += commentHighlight(Qt::escape(comment)) + "   ";
      }
      
      
      if( m_previousContext ) {
        m_currentText += navigationHighlight("Back to ");
        makeLink( m_previousContext->name(), m_previousContext->name(), NavigationAction(m_previousContext) );
        m_currentText += "<br>";
      }

      if( m_declaration )
      {
        QStringList details;
        QString kind = declarationKind(m_declaration.data());
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

        if( function ) {

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

          if( !shorten ) {
            if( type && function ) {
              if( !classFunDecl || !classFunDecl->isConstructor() || !classFunDecl->isDestructor() ) {
                eventuallyMakeTypeLinks( type->returnType().data() );
                m_currentText += ' ' + nameHighlight(Qt::escape(m_declaration->identifier().toString()));
              }

              if( type->arguments().count() == 0 )
              {
                m_currentText += "()";
              } else {
                m_currentText += "( ";
                bool first = true;

                KDevelop::DUContext* argumentContext = Cpp::getArgumentContext(m_declaration.data());
                
                if(argumentContext) {
                  QList<QString>::const_iterator defaultIt = function->defaultParameters().begin();
                  int firstDefaultParam = argumentContext->localDeclarations().count() - function->defaultParameters().count();
                  int currentArgNum = 0;

                  foreach(Declaration* argument, argumentContext->localDeclarations()) {
                    if( !first )
                      m_currentText += ", ";
                    first = false;

                    AbstractType::Ptr argType = argument->abstractType();
                    
                    eventuallyMakeTypeLinks( argType.data() );
                    m_currentText += " " + nameHighlight(Qt::escape(argument->identifier().toString()));

                    if( currentArgNum >= firstDefaultParam )
                      m_currentText += " = " + Qt::escape(function->defaultParameters()[ currentArgNum - firstDefaultParam ]);

                    ++currentArgNum;
                  }
                }

                m_currentText += " )";
              }
              m_currentText += "<br>";
            }else {
              m_currentText += errorHighlight("Invalid type<br>");
            }
          }
        } else if( !shorten ) {
          
          if( m_declaration->isTypeAlias() || m_declaration->kind() == Declaration::Instance ) {
            if( m_declaration->isTypeAlias() )
              m_currentText += importantHighlight("typedef ");
            
            if(m_declaration->type<CppEnumeratorType>())
              m_currentText += i18n("enumerator") + " ";

            eventuallyMakeTypeLinks( m_declaration->abstractType().data() );

            m_currentText += " " + nameHighlight(Qt::escape(declarationName(m_declaration.data()))) + "<br>";
          }else{
            CppClassType* klass = dynamic_cast<CppClassType*>(m_declaration->abstractType().data());
            if( m_declaration->kind() == Declaration::Type && klass ) {
              m_currentText += "class ";
              eventuallyMakeTypeLinks( klass );

              foreach( const CppClassType::BaseClassInstance& base, klass->baseClasses() ) {
                m_currentText += ", " + stringFromAccess(base.access) + " " + (base.virtualInheritance ? QString("virtual") : QString()) + " ";
                eventuallyMakeTypeLinks(base.baseClass.data());
              }
              m_currentText += " ";
            }

            if(m_declaration->type<CppEnumerationType>()) {
              CppEnumerationType::Ptr enumeration = m_declaration->type<CppEnumerationType>();
              m_currentText += i18n("enumeration") + " " + Qt::escape(m_declaration->identifier().toString()) + "<br>";
            }

            if(m_declaration->isForwardDeclaration()) {
              ForwardDeclaration* forwardDec = static_cast<ForwardDeclaration*>(m_declaration.data());
              Declaration* resolved = forwardDec->resolve(m_topContext.data());
              if(resolved) {
                m_currentText += "(" + i18n("resolved forward-declaration") + ": ";
                makeLink(resolved->identifier().toString(), KDevelop::DeclarationPointer(resolved), NavigationAction::NavigateDeclaration );
                m_currentText += ") ";
              }else{
                m_currentText += i18n("(unresolved forward-declaration)") + " ";
              }
            }
          }
        }

        QualifiedIdentifier identifier = m_declaration->qualifiedIdentifier();
        if( identifier.count() > 1 ) {
          if( m_declaration->context() && m_declaration->context()->owner() )
          {
            Declaration* decl = m_declaration->context()->owner();
            if(decl->isDefinition() && decl->declaration())
              decl = decl->declaration();
            
            if(dynamic_cast<CppEnumerationType*>(decl->abstractType().data()))
              m_currentText += labelHighlight(i18n("Enum: "));
            else
              m_currentText += labelHighlight(i18n("Container: "));
            
            makeLink( declarationName(decl), DeclarationPointer(decl), NavigationAction::NavigateDeclaration );
            m_currentText += " ";
          } else {
            QualifiedIdentifier parent = identifier;
            parent.pop();
            m_currentText += labelHighlight(i18n("Scope: ")) + Qt::escape(parent.toString()) + " ";
          }
        }

        if( !access.isEmpty() )
          m_currentText += labelHighlight(i18n("Access: ")) + propertyHighlight(Qt::escape(access)) + " ";
        
        
        ///@todo Enumerations
        
        QString detailsHtml;

        if( !details.isEmpty() ) {
          bool first = true;
          foreach( QString str, details ) {
            if( !first )
              detailsHtml += ", ";
            first = false;
            detailsHtml += propertyHighlight(str);
          }
        }

        if( !kind.isEmpty() ) {
          if( !detailsHtml.isEmpty() )
            m_currentText += labelHighlight(i18n("Kind: ")) + importantHighlight(Qt::escape(kind)) + " " + detailsHtml + " ";
          else
            m_currentText += labelHighlight(i18n("Kind: ")) + importantHighlight(Qt::escape(kind)) + " ";
        } else if( !detailsHtml.isEmpty() ) {
          m_currentText += labelHighlight(i18n("Modifiers: ")) +  importantHighlight(Qt::escape(kind)) + " ";
        }

        m_currentText += "<br />";

        if( !shorten && m_declaration && !m_declaration->comment().isEmpty() ) {
          QString comment = m_declaration->comment();
          comment.replace("<br />", "\n"); //do not escape html newlines within the comment
          comment.replace("<br/>", "\n");
          comment = Qt::escape(comment);
          comment.replace("\n", "<br />"); //Replicate newlines in html
          m_currentText += commentHighlight(comment);
          m_currentText += "<br />";
        }

        if( !shorten ) {
          if(m_declaration->isDefinition())
            m_currentText += labelHighlight(i18n( "Def.: " ));
          else
            m_currentText += labelHighlight(i18n( "Decl.: " ));

          makeLink( QString("%1 :%2").arg( KUrl(m_declaration->url().str()).fileName() ).arg( m_declaration->range().textRange().start().line() ), m_declaration, NavigationAction::JumpToSource );
          m_currentText += " ";
          //m_currentText += "<br />";
          if( m_declaration->definition() ) {
            m_currentText += labelHighlight(i18n( "Def.: " ));
            makeLink( QString("%1 :%2").arg( KUrl(m_declaration->definition()->url().str()).fileName() ).arg( m_declaration->definition()->range().textRange().start().line() ), DeclarationPointer(m_declaration->definition()), NavigationAction::JumpToSource );
          }

          if( m_declaration->declaration() ) {
            m_currentText += labelHighlight(i18n( "Decl.: " ));
            makeLink( QString("%1 :%2").arg( KUrl(m_declaration->declaration()->url().str()).fileName() ).arg( m_declaration->declaration()->range().textRange().start().line() ), DeclarationPointer(m_declaration->declaration()), NavigationAction::JumpToSource );
          }

          QMap<HashedString, QList<SimpleRange> > uses = m_declaration->logicalDeclaration(m_topContext.data())->uses();

          if(!uses.isEmpty()) {
            m_currentText += labelHighlight(i18n("<br />Uses:<br />"));
            for(QMap<HashedString, QList<SimpleRange> >::const_iterator it = uses.begin(); it != uses.end(); ++it) {
              m_currentText += " " + Qt::escape(KUrl(it.key().str()).fileName()) + "<br />";
              foreach(const SimpleRange& range, *it) {
                m_currentText += "  ";
                makeLink( QString("%1:%2").arg(range.start.line).arg(range.start.column), QString("%1").arg(qHash(range) + it.key().hash()), NavigationAction(KUrl(it.key().str()), range.start.textCursor()) );
              }
              m_currentText += "<br/>";
            }
          }
        }
        //m_currentText += "<br />";
      }

      addExternalHtml(m_suffix);
      
      m_currentText += "</small></small></p></body></html>";

      return m_currentText;
    }
  protected:
    DeclarationPointer m_declaration;

    NavigationContextPointer execute(NavigationAction& action);

    void addExternalHtml( const QString& text ) {
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
  
    ///Creates and registers a link for the given type that jumps to its declaration and to the template-argument declarations
    void eventuallyMakeTypeLinks( const AbstractType* type ) {
      if( !type ) {
        m_currentText += Qt::escape("<no type>");
        return;
      }
        const AbstractType* target = TypeUtils::targetType( type, m_topContext.data() );
        const IdentifiedType* idType = dynamic_cast<const IdentifiedType*>( target );
        
        ///@todo handle const etc. correctly
        const CppPointerType* pointer = dynamic_cast<const CppPointerType*>(type);
        const CppReferenceType* ref = dynamic_cast<const CppReferenceType*>(type);
        
        if(pointer && pointer->cv() & Declaration::Const)
          m_currentText += "const ";
        if(ref && ref->cv() & Declaration::Const)
          m_currentText += "const ";
        
        if( idType ) {
          if( idType->declaration() ) {

            //Remove the last template-identifiers, because we create those directly
            QualifiedIdentifier id = idType->identifier();
            Identifier lastId = id.last();
            id.pop();
            lastId.clearTemplateIdentifiers();
            id.push(lastId);

            //We leave out the * and & reference and pointer signs, those are added to the end
            makeLink(id.toString() , DeclarationPointer(idType->declaration()), NavigationAction::NavigateDeclaration );
          } else {
            m_currentText += Qt::escape(type->toString());
          }

          if( const TemplateDeclaration* templ = dynamic_cast<const TemplateDeclaration*>(idType->declaration()) ) {
            if( templ->instantiatedFrom() ) {
              m_currentText += Qt::escape("  <");

              const QList<ExpressionEvaluationResult>& params = templ->instantiatedWith();
              bool first = true;
              foreach( const ExpressionEvaluationResult& result, params ) {
                if( first )
                  first = false;
                else
                  m_currentText += ", ";

                if( result.type ) {
                  eventuallyMakeTypeLinks(result.type.data());
                }else{
                  m_currentText += Qt::escape(result.toShortString());
                }
              }
              
              m_currentText += Qt::escape(" >");
            }
          }
          if(idType) {
            //Add reference and pointer
            ///@todo correct const handling
            while(pointer || ref) {
              if(pointer) {
                m_currentText += Qt::escape("*");
                ref = dynamic_cast<const CppReferenceType*>(pointer->baseType().data());
                pointer = dynamic_cast<const CppPointerType*>(pointer->baseType().data());
              }
              if(ref) {
                m_currentText += Qt::escape("&");
                pointer = dynamic_cast<const CppPointerType*>(ref->baseType().data());
                ref = dynamic_cast<const CppReferenceType*>(ref->baseType().data());
              }
            }
            
          }
        }else if( type ) {
          m_currentText += Qt::escape(type->toString());
        }
    }
    
    ///Creates and registers a link to the given declaration, labeled by the given name
    void makeLink( const QString& name, DeclarationPointer declaration, NavigationAction::Type actionType )
    {
      NavigationAction action( declaration, actionType );
      QString targetId = QString("%1").arg((quint64)declaration.data());
      makeLink(name, targetId, action);
    }

    ///Creates a link that executes the given action
    void makeLink( const QString& name, QString targetId, const NavigationAction& action)
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
  
    int m_selectedLink; //The link currently selected
    NavigationAction m_selectedLinkAction; //Target of the currently selected link

    NavigationContextPointer registerChild( NavigationContext* context ) {
      m_children << NavigationContextPointer(context);
      return m_children.last();
    }
    
    QList<NavigationContextPointer> m_children; //Useed to keep alive all children until this is deleted

    //A counter used while building the html-code to count the used links.
    int m_linkCount;
    QString m_currentText; //Here the text is built
    QMap<QString, NavigationAction> m_links;
    QMap<int, NavigationAction> m_intLinks;
    NavigationContext* m_previousContext;
    QString m_prefix, m_suffix;
    KDevelop::TopDUContextPointer m_topContext;
};

NavigationContextPointer NavigationContext::execute(NavigationAction& action)
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
      if( m_previousContext && m_previousContext->m_declaration == action.decl )
        return NavigationContextPointer( m_previousContext );
      
      return registerChild( new NavigationContext(action.decl, m_topContext, this) );
    } break;
  case NavigationAction::JumpToSource:
      {
        KUrl doc = action.document;
        KTextEditor::Cursor cursor = action.cursor;
        if(doc.isEmpty()) {
          doc = KUrl(action.decl->url().str());
/*          if(action.decl->internalContext())
            cursor = action.decl->internalContext()->range().textRange().start() + KTextEditor::Cursor(0, 1);
          else*/
            cursor = action.decl->range().textRange().start();
        }
        
        //This is used to execute the slot delayed in the event-loop, so crashes are avoided
        QMetaObject::invokeMethod( ICore::self()->documentController(), "openDocument", Qt::QueuedConnection, Q_ARG(KUrl, doc), Q_ARG(KTextEditor::Cursor, cursor) );
        break;
      }
  }
  
  return NavigationContextPointer( this );
}

class IncludeNavigationContext : public NavigationContext {
public:
  IncludeNavigationContext(const IncludeItem& item, KDevelop::TopDUContextPointer topContext) : NavigationContext(DeclarationPointer(0), topContext), m_item(item) {
  }
  virtual QString html(bool shorten) {
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
      if(!duchain)
        duchain = ctx; //Any is better than none
    }
    
    m_currentText += "<br />";
    
    if(duchain) {
      const Cpp::EnvironmentFile* f = dynamic_cast<const Cpp::EnvironmentFile*>(duchain->parsingEnvironmentFile().data());
      Q_ASSERT(f); //Should always be for c++
      m_currentText += QString("%1: %2 %3: %4 %5: %6").arg(labelHighlight(i18nc("Headers included into this header", "Included"))).arg(duchain->importedParentContexts().count()).arg(i18nc("Count of files this header was included into", "Included by")).arg(duchain->importedChildContexts().count()).arg(i18nc("Count of macros defined in this header", "Defined macros")).arg(f->definedMacros().set().count());
      m_currentText += "<br />";
      if(!shorten) {
        m_currentText += labelHighlight(i18n("Declarations:")) + "<br />";
        addDeclarationsFromContext(duchain);
      }
    }else if(duchains.isEmpty()) {
      m_currentText += i18n("not parsed yet");
    }

    addExternalHtml(m_suffix);
    
    m_currentText += "</small></small></p></body></html>";
    return m_currentText;
  }

  virtual QString name() const {
    return m_item.name;
  }
  
private:

  void addDeclarationsFromContext(DUContext* ctx, QString indent = "", bool first = true) {
    //m_currentText += indent + ctx->localScopeIdentifier().toString() + "{<br />";
    QVector<DUContext*>::const_iterator childIterator = ctx->childContexts().begin();
    QVector<Declaration*>::const_iterator declarationIterator = ctx->localDeclarations().begin();

    while(childIterator != ctx->childContexts().end() || declarationIterator != ctx->localDeclarations().end()) {

      //Show declarations/contexts in the order they appear in the file
      int currentDeclarationLine = -1;
      int currentContextLine = -1;
      if(declarationIterator != ctx->localDeclarations().end())
        currentDeclarationLine = (*declarationIterator)->range().textRange().start().line();
      
      if(childIterator != ctx->childContexts().end())
        currentDeclarationLine = (*childIterator)->range().textRange().start().line();

      if((currentDeclarationLine <= currentContextLine || currentContextLine == -1 || childIterator == ctx->childContexts().end()) && declarationIterator != ctx->localDeclarations().end() )
      {
        if(!(*declarationIterator)->qualifiedIdentifier().toString().isEmpty() && !(*declarationIterator)->range().isEmpty() && !(*declarationIterator)->isForwardDeclaration()) {
          //Show the declaration
          if(!first)
            m_currentText += Qt::escape(", ");
          else
            first = false;
          
          m_currentText += Qt::escape(indent + declarationKind(*declarationIterator) + " ");
          makeLink((*declarationIterator)->qualifiedIdentifier().toString(), DeclarationPointer(*declarationIterator), NavigationAction::NavigateDeclaration);
        }
        ++declarationIterator;
      } else {
        //Eventually Recurse into the context
        if((*childIterator)->type() == DUContext::Global || (*childIterator)->type() == DUContext::Namespace /*|| (*childIterator)->type() == DUContext::Class*/)
          addDeclarationsFromContext(*childIterator, indent + " ", first);
        ++childIterator;
        first = false;
      }
    }
    //m_currentText += "}<br />";
  }
  
  IncludeItem m_item;
};

class MacroNavigationContext : public NavigationContext {
public:
  MacroNavigationContext(const pp_macro& macro, QString preprocessedBody) : NavigationContext(DeclarationPointer(0), TopDUContextPointer(0)), m_macro(copyConstantMacro(&macro)), m_body(preprocessedBody) {
  }

~MacroNavigationContext() {
    delete m_macro;
  }
  
  virtual QString html(bool shorten) {
    m_currentText  = "<html><body><p><small><small>";
    m_linkCount = 0;
    addExternalHtml(m_prefix);

    QString args;
    
    if(m_macro->formalsSize()) {
      args = "(";

      bool first = true;
      FOREACH_CUSTOM(uint b, m_macro->formals(), m_macro->formalsSize()) {
        if(!first)
          args += ", ";
        first = false;
        args += IndexedString(b).str();
      }

      args += ")";
    }
    
    m_currentText += (m_macro->function_like ? i18n("Function macro") : i18n("Macro")) + " " + importantHighlight(m_macro->name.str()) + " " + args +  "<br />";
    
    KUrl u(m_macro->file.str());
    NavigationAction action(u, KTextEditor::Cursor(m_macro->sourceLine,0));
    QList<TopDUContext*> duchains = DUChain::self()->chainsForDocument(u);

    if(!shorten) {
      m_currentText += "<br />";

      if(!m_body.isEmpty()) {
      m_currentText += labelHighlight(i18n("Preprocessed body:")) + "<br />";
      m_currentText += codeHighlight(Qt::escape(m_body));
      m_currentText += "<br />";
      }

      
      m_currentText += labelHighlight(i18n("Body:")) + "<br />";

      m_currentText += codeHighlight(Qt::escape(QString::fromUtf8(stringFromContents(m_macro->definition(), m_macro->definitionSize()))));
      m_currentText += "<br />";
    }

    makeLink(u.pathOrUrl(), u.pathOrUrl(), action);
    
    m_currentText += "</small></small></p></body></html>";
    return m_currentText;
  }
  
  virtual QString name() const {
    return m_macro->name.str();
  }
  
private:

  pp_macro* m_macro;
  QString m_body;
};

NavigationWidget::NavigationWidget(KDevelop::DeclarationPointer declaration, KDevelop::TopDUContextPointer topContext, const QString& htmlPrefix, const QString& htmlSuffix) : m_topContext(topContext), m_declaration(declaration)
{
  initBrowser(400);

  //The first context is registered so it is kept alive by the shared-pointer mechanism
  m_startContext = NavigationContextPointer(new NavigationContext(declaration, m_topContext));
  m_startContext->setPrefixSuffix( htmlPrefix, htmlSuffix );
  setContext( m_startContext );
}

NavigationWidget::NavigationWidget(const IncludeItem& includeItem, KDevelop::TopDUContextPointer topContext, const QString& htmlPrefix, const QString& htmlSuffix) : m_topContext(topContext) {
  initBrowser(200);
  
//The first context is registered so it is kept alive by the shared-pointer mechanism
  m_startContext = NavigationContextPointer(new IncludeNavigationContext(includeItem, m_topContext));
  m_startContext->setPrefixSuffix( htmlPrefix, htmlSuffix );
  setContext( m_startContext );
}

NavigationWidget::NavigationWidget(const rpp::pp_macro& macro, const QString& preprocessedBody, const QString& htmlPrefix, const QString& htmlSuffix) : m_topContext(0) {
  initBrowser(200);
  
//The first context is registered so it is kept alive by the shared-pointer mechanism
  m_startContext = NavigationContextPointer(new MacroNavigationContext(macro, preprocessedBody));
  m_startContext->setPrefixSuffix( htmlPrefix, htmlSuffix );
  setContext( m_startContext );
}

void NavigationWidget::initBrowser(int height) {
  setOpenLinks(false);
  setOpenExternalLinks(false);
  resize(height, 100);
  setNotifyClick(true);

  connect( this, SIGNAL(anchorClicked(const QUrl&)), this, SLOT(anchorClicked(const QUrl&)) );
}

NavigationWidget::~NavigationWidget() {
}

void NavigationWidget::setContext(NavigationContextPointer context)
{
  m_context = context;
  update();
}

void NavigationWidget::update() {
  Q_ASSERT( m_context );
  int scrollPos = verticalScrollBar()->value();
  setHtml( m_context->html() );
  verticalScrollBar()->setValue(scrollPos);
  scrollToAnchor("selectedItem");
}

void NavigationWidget::anchorClicked(const QUrl& url) {
  DUChainReadLocker lock( DUChain::lock() );
  setContext( m_context->acceptLink(url.toString()) );
}

void NavigationWidget::next() {
  DUChainReadLocker lock( DUChain::lock() );
  Q_ASSERT( m_context );
  m_context->nextLink();
  update();
}

void NavigationWidget::previous() {
  DUChainReadLocker lock( DUChain::lock() );
  Q_ASSERT( m_context );
  m_context->previousLink();
  update();
}

void NavigationWidget::accept() {
  DUChainReadLocker lock( DUChain::lock() );
  Q_ASSERT( m_context );
  setContext( m_context->accept() );
}

void NavigationWidget::up() {
  verticalScrollBar()->triggerAction( QAbstractSlider::SliderSingleStepSub );
}

void NavigationWidget::down() {
  verticalScrollBar()->triggerAction( QAbstractSlider::SliderSingleStepAdd );
}

QString NavigationWidget::shortDescription(KDevelop::Declaration* declaration) {
  NavigationContextPointer ctx(new NavigationContext(DeclarationPointer(declaration), TopDUContextPointer())); ///@todo give correct top-context
  return ctx->html(true);
}

QString NavigationWidget::shortDescription(const IncludeItem& includeItem) {
  NavigationContextPointer ctx(new IncludeNavigationContext(includeItem, TopDUContextPointer())); ///@todo give correct top-context
  return ctx->html(true);
}

}

#include "navigationwidget.moc"
