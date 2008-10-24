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

#include "abstractdeclarationnavigationcontext.h"

#include <QtGui/QTextDocument>

#include <klocale.h>

#include "../functiondeclaration.h"
#include "../functiondefinition.h"
#include "../classfunctiondeclaration.h"
#include "../namespacealiasdeclaration.h"
#include "../forwarddeclaration.h"
#include "../types/enumeratortype.h"
#include "../types/enumerationtype.h"
#include "../types/functiontype.h"
#include "../duchainutils.h"
#include "../types/pointertype.h"
#include "../types/referencetype.h"
#include "../types/typeutils.h"

namespace KDevelop {
AbstractDeclarationNavigationContext::AbstractDeclarationNavigationContext( DeclarationPointer decl, KDevelop::TopDUContextPointer topContext, AbstractNavigationContext* previousContext)
  : AbstractNavigationContext(topContext, previousContext), m_declaration(decl)
{
  //Jump from definition to declaration if possible
  FunctionDefinition* definition = dynamic_cast<FunctionDefinition*>(m_declaration.data());
  if(definition && definition->declaration())
    m_declaration = DeclarationPointer(definition->declaration());
}

QString AbstractDeclarationNavigationContext::name() const
{
  return declarationName(m_declaration);
}

QString AbstractDeclarationNavigationContext::html(bool shorten)
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

  if( !shorten ) {
    const AbstractFunctionDeclaration* function = dynamic_cast<const AbstractFunctionDeclaration*>(m_declaration.data());
    if( function ) {
      htmlFunction();
    } else if( m_declaration->isTypeAlias() || m_declaration->kind() == Declaration::Instance ) {
      if( m_declaration->isTypeAlias() )
        m_currentText += importantHighlight("typedef ");

      if(m_declaration->type<EnumeratorType>())
        m_currentText += i18n("enumerator") + " ";

      eventuallyMakeTypeLinks( m_declaration->abstractType() );

      m_currentText += " " + nameHighlight(Qt::escape(declarationName(m_declaration))) + "<br>";
    }else{
      if( m_declaration->kind() == Declaration::Type && m_declaration->abstractType().cast<StructureType>() ) {
        qDebug() << "xxx" << m_declaration;
        qDebug() << "xxx" << m_declaration->abstractType().cast<StructureType>();
        htmlClass();
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

        makeLink( declarationName(DeclarationPointer(decl)), DeclarationPointer(decl), NavigationAction::NavigateDeclaration );
        m_currentText += " ";
      } else {
        QualifiedIdentifier parent = identifier;
        parent.pop();
        m_currentText += labelHighlight(i18n("Scope: ")) + Qt::escape(parent.toString()) + " ";
      }
    }
  }

  QString access = stringFromAccess(m_declaration);
  if( !access.isEmpty() )
    m_currentText += labelHighlight(i18n("Access: ")) + propertyHighlight(Qt::escape(access)) + " ";


  ///@todo Enumerations

  QString detailsHtml;
  QStringList details = declarationDetails(m_declaration);
  if( !details.isEmpty() ) {
    bool first = true;
    foreach( QString str, details ) {
      if( !first )
        detailsHtml += ", ";
      first = false;
      detailsHtml += propertyHighlight(str);
    }
  }

  QString kind = declarationKind(m_declaration);
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
    
    m_currentText += " ";
    makeLink(i18n("Show uses"), "show_uses", NavigationAction(m_declaration, NavigationAction::NavigateUses));
  }
    //m_currentText += "<br />";

  addExternalHtml(m_suffix);

  m_currentText += "</small></small></p></body></html>";

  return m_currentText;
}

void AbstractDeclarationNavigationContext::htmlFunction()
{
  const AbstractFunctionDeclaration* function = dynamic_cast<const AbstractFunctionDeclaration*>(m_declaration.data());
  Q_ASSERT(function);

  const ClassFunctionDeclaration* classFunDecl = dynamic_cast<const ClassFunctionDeclaration*>(m_declaration.data());
  const FunctionType::Ptr type = m_declaration->abstractType().cast<FunctionType>();
  if( !type ) {
    m_currentText += errorHighlight("Invalid type<br>");
    return;
  }
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
}

void AbstractDeclarationNavigationContext::htmlClass()
{
  StructureType::Ptr klass = m_declaration->abstractType().cast<StructureType>();
  Q_ASSERT(klass);

  m_currentText += "class ";
  eventuallyMakeTypeLinks( klass.cast<AbstractType>() );
}

void AbstractDeclarationNavigationContext::htmlIdentifiedType(AbstractType::Ptr type, const IdentifiedType* idType)
{
  Q_ASSERT(type);
  Q_ASSERT(idType);

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
}

void AbstractDeclarationNavigationContext::eventuallyMakeTypeLinks( AbstractType::Ptr type )
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

    htmlIdentifiedType(type, idType);

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

  } else {
    m_currentText += Qt::escape(type->toString());
  }
}

DeclarationPointer AbstractDeclarationNavigationContext::declaration() const
{
  return m_declaration;
}

QString AbstractDeclarationNavigationContext::stringFromAccess(Declaration::AccessPolicy access)
{
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

QString AbstractDeclarationNavigationContext::stringFromAccess(DeclarationPointer decl)
{
  const ClassMemberDeclaration* memberDecl = dynamic_cast<const ClassMemberDeclaration*>(decl.data());
  if( memberDecl ) {
    return stringFromAccess(memberDecl->accessPolicy());
  }
  return QString();
}

QString AbstractDeclarationNavigationContext::declarationName( DeclarationPointer decl )
{
  if( NamespaceAliasDeclaration* alias = dynamic_cast<NamespaceAliasDeclaration*>(decl.data()) ) {
    if( alias->identifier().isEmpty() )
      return "using namespace " + alias->importIdentifier().toString();
    else
      return "namespace " + alias->identifier().toString() + " = " + alias->importIdentifier().toString();
  }

  if( !decl )
    return i18nc("An unknown declaration that is unknown", "Unknown");
  else
    return decl->identifier().toString();
}

QStringList AbstractDeclarationNavigationContext::declarationDetails(DeclarationPointer decl)
{
  QStringList details;
  const AbstractFunctionDeclaration* function = dynamic_cast<const AbstractFunctionDeclaration*>(decl.data());
  const ClassMemberDeclaration* memberDecl = dynamic_cast<const ClassMemberDeclaration*>(decl.data());
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
  }

  if( decl->isDefinition() )
    details << "definition";

  if( memberDecl && memberDecl->isForwardDeclaration() )
    details << "forward";

  AbstractType::Ptr t(decl->abstractType());
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

    const ClassFunctionDeclaration* classFunDecl = dynamic_cast<const ClassFunctionDeclaration*>(decl.data());

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
  }
  return details;
}

}
