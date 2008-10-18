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

#include "navigationcontext.h"

#include <QtGui/QTextDocument>

#include <klocale.h>

#include <language/duchain/functiondefinition.h>
#include <language/duchain/namespacealiasdeclaration.h>
#include <language/duchain/classfunctiondeclaration.h>
#include <language/duchain/forwarddeclaration.h>
#include <language/duchain/duchainutils.h>

#include "../typeutils.h"
#include "../templatedeclaration.h"
#include "../classdeclaration.h"

namespace Cpp {

NavigationContext::NavigationContext( DeclarationPointer decl, KDevelop::TopDUContextPointer topContext, AbstractNavigationContext* previousContext)
  : AbstractNavigationContext( decl, topContext, previousContext )
{
}

QString NavigationContext::name() const
{
  return declarationName(m_declaration.data());
}

QString NavigationContext::html(bool shorten)
{
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

    AbstractType::Ptr t(m_declaration->abstractType());
    if( t ) {
      if( t->modifiers() & AbstractType::ConstModifier )
        details << "constant";
      if( t->modifiers() & AbstractType::VolatileModifier )
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

      const FunctionType::Ptr type = m_declaration->abstractType().cast<FunctionType>();

      if( !shorten ) {
        if( type && function ) {
          if( !classFunDecl || !classFunDecl->isConstructor() || !classFunDecl->isDestructor() ) {
            eventuallyMakeTypeLinks( type->returnType() );
            m_currentText += ' ' + nameHighlight(Qt::escape(m_declaration->identifier().toString()));
          }

          if( type->arguments().count() == 0 )
          {
            m_currentText += "()";
          } else {
            m_currentText += "( ";
            bool first = true;

            KDevelop::DUContext* argumentContext = DUChainUtils::getArgumentContext(m_declaration.data());

            if(argumentContext) {
              int firstDefaultParam = argumentContext->localDeclarations().count() - function->defaultParametersSize();
              int currentArgNum = 0;

              foreach(Declaration* argument, argumentContext->localDeclarations()) {
                if( !first )
                  m_currentText += ", ";
                first = false;

                AbstractType::Ptr argType = argument->abstractType();

                eventuallyMakeTypeLinks( argType );
                m_currentText += " " + nameHighlight(Qt::escape(argument->identifier().toString()));

                if( currentArgNum >= firstDefaultParam )
                  m_currentText += " = " + Qt::escape(function->defaultParameters()[ currentArgNum - firstDefaultParam ].str());

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

        if(m_declaration->type<EnumeratorType>())
          m_currentText += i18n("enumerator") + " ";

        eventuallyMakeTypeLinks( m_declaration->abstractType() );

        m_currentText += " " + nameHighlight(Qt::escape(declarationName(m_declaration.data()))) + "<br>";
      }else{
        //CppClassType::Ptr klass = m_declaration->abstractType().cast<CppClassType>();
        StructureType::Ptr klass = m_declaration->abstractType().cast<StructureType>();
        if( m_declaration->kind() == Declaration::Type && klass ) {
          m_currentText += "class ";
          eventuallyMakeTypeLinks( klass.cast<AbstractType>() );

          Cpp::ClassDeclaration* classDecl = dynamic_cast<Cpp::ClassDeclaration*>(klass->declaration(m_topContext.data()));

            if(classDecl) {
            FOREACH_FUNCTION( const Cpp::BaseClassInstance& base, classDecl->baseClasses ) {
              m_currentText += ", " + stringFromAccess(base.access) + " " + (base.virtualInheritance ? QString("virtual") : QString()) + " ";
              eventuallyMakeTypeLinks(base.baseClass.type());
            }
          }

          m_currentText += " ";
        }

        if(m_declaration->type<EnumerationType>()) {
          EnumerationType::Ptr enumeration = m_declaration->type<EnumerationType>();
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

        FunctionDefinition* definition = dynamic_cast<FunctionDefinition*>(decl);
        if(definition && definition->declaration())
          decl = definition->declaration();

        if(decl->abstractType().cast<EnumerationType>())
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

    if( !shorten && !m_declaration->comment().isEmpty() ) {
      QString comment = m_declaration->comment();
      comment.replace("<br />", "\n"); //do not escape html newlines within the comment
      comment.replace("<br/>", "\n");
      comment = Qt::escape(comment);
      comment.replace("\n", "<br />"); //Replicate newlines in html
      m_currentText += commentHighlight(comment);
      m_currentText += "<br />";
    }

    if( !shorten ) {
      if(dynamic_cast<FunctionDefinition*>(m_declaration.data()))
        m_currentText += labelHighlight(i18n( "Def.: " ));
      else
        m_currentText += labelHighlight(i18n( "Decl.: " ));

      makeLink( QString("%1 :%2").arg( KUrl(m_declaration->url().str()).fileName() ).arg( m_declaration->range().textRange().start().line()+1 ), m_declaration, NavigationAction::JumpToSource );
      m_currentText += " ";
      //m_currentText += "<br />";
      if(!dynamic_cast<FunctionDefinition*>(m_declaration.data())) {
        if( FunctionDefinition* definition = FunctionDefinition::definition(m_declaration.data()) ) {
          m_currentText += labelHighlight(i18n( " Def.: " ));
          makeLink( QString("%1 :%2").arg( KUrl(definition->url().str()).fileName() ).arg( definition->range().textRange().start().line()+1 ), DeclarationPointer(definition), NavigationAction::JumpToSource );
        }
      }

      if( FunctionDefinition* definition = dynamic_cast<FunctionDefinition*>(m_declaration.data()) ) {
        if(definition->declaration()) {
          m_currentText += labelHighlight(i18n( " Decl.: " ));
          makeLink( QString("%1 :%2").arg( KUrl(definition->declaration()->url().str()).fileName() ).arg( definition->declaration()->range().textRange().start().line()+1 ), DeclarationPointer(definition->declaration()), NavigationAction::JumpToSource );
        }
      }
    }
    //m_currentText += "<br />";
  }

  addExternalHtml(m_suffix);

  m_currentText += "</small></small></p></body></html>";

  return m_currentText;
}


void NavigationContext::eventuallyMakeTypeLinks( AbstractType::Ptr type )
{
  if( !type ) {
    m_currentText += Qt::escape("<no type>");
    return;
  }
    AbstractType::Ptr target = TypeUtils::targetType( type, m_topContext.data() );
    const IdentifiedType* idType = dynamic_cast<const IdentifiedType*>( target.unsafeData() );

    ///@todo handle const etc. correctly
    PointerType::Ptr pointer = type.cast<PointerType>();
    ReferenceType::Ptr ref = type.cast<ReferenceType>();

    if(pointer && pointer->modifiers() & AbstractType::ConstModifier)
      m_currentText += "const ";
    if(ref && ref->modifiers() & AbstractType::ConstModifier)
      m_currentText += "const ";

    if( idType ) {
      if( idType->declaration(m_topContext.data()) ) {

        //Remove the last template-identifiers, because we create those directly
        QualifiedIdentifier id = idType->qualifiedIdentifier();
        Identifier lastId = id.last();
        id.pop();
        lastId.clearTemplateIdentifiers();
        id.push(lastId);

        //We leave out the * and & reference and pointer signs, those are added to the end
        makeLink(id.toString() , DeclarationPointer(idType->declaration(m_topContext.data())), NavigationAction::NavigateDeclaration );
      } else {
        m_currentText += Qt::escape(type->toString());
      }

      if( const TemplateDeclaration* templ = dynamic_cast<const TemplateDeclaration*>(idType->declaration(m_topContext.data())) ) {
        if( templ->instantiatedFrom() ) {
          m_currentText += Qt::escape("  <");

          const Cpp::InstantiationInformation& params = templ->instantiatedWith().information();
          bool first = true;
          FOREACH_FUNCTION( const IndexedType& type, params.templateParameters ) {
            if( first )
              first = false;
            else
              m_currentText += ", ";

            if( type ) {
              AbstractType::Ptr t = type.type();
              eventuallyMakeTypeLinks(t);
            }else{
                m_currentText += "missing type";
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
            ref = pointer->baseType().cast<ReferenceType>();
            pointer = pointer->baseType().cast<PointerType>();
          }
          if(ref) {
            m_currentText += Qt::escape("&");
            pointer = ref->baseType().cast<PointerType>();
            ref = ref->baseType().cast<ReferenceType>();
          }
        }

      }
    }else if( type ) {
      m_currentText += Qt::escape(type->toString());
    }
}

NavigationContextPointer NavigationContext::registerChild(DeclarationPointer declaration)
{
  return AbstractNavigationContext::registerChild(new NavigationContext(declaration, m_topContext, this));
}

}
