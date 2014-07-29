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
#include "../persistentsymboltable.h"
#include <interfaces/icore.h>
#include <interfaces/idocumentationcontroller.h>
#include <duchain/types/typealiastype.h>
#include <duchain/types/structuretype.h>
#include <duchain/classdeclaration.h>
#include <typeinfo>

namespace KDevelop {
AbstractDeclarationNavigationContext::AbstractDeclarationNavigationContext( DeclarationPointer decl, KDevelop::TopDUContextPointer topContext, AbstractNavigationContext* previousContext)
  : AbstractNavigationContext((topContext ? topContext : TopDUContextPointer(decl ? decl->topContext() : 0)), previousContext), m_declaration(decl), m_fullBackwardSearch(false)
{
  //Jump from definition to declaration if possible
  FunctionDefinition* definition = dynamic_cast<FunctionDefinition*>(m_declaration.data());
  if(definition && definition->declaration())
    m_declaration = DeclarationPointer(definition->declaration());
}

QString AbstractDeclarationNavigationContext::name() const
{
  if(m_declaration.data())
    return prettyQualifiedIdentifier(m_declaration).toString();
  else
    return declarationName(m_declaration);
}

QString AbstractDeclarationNavigationContext::html(bool shorten)
{
  clear();
  m_shorten = shorten;
  modifyHtml()  += "<html><body><p>" + fontSizePrefix(shorten);

  addExternalHtml(m_prefix);

  if(!m_declaration.data()) {
    modifyHtml() += i18n("<br /> lost declaration <br />");
    return currentHtml();
  }
  
  if( m_previousContext ) {
    QString link = createLink( m_previousContext->name(), m_previousContext->name(), NavigationAction(m_previousContext) );
    modifyHtml() += navigationHighlight(i18n("Back to %1<br />", link));
  }
  
  KSharedPtr<IDocumentation> doc;
  
  if( !shorten ) {
    doc = ICore::self()->documentationController()->documentationForDeclaration(m_declaration.data());

    const AbstractFunctionDeclaration* function = dynamic_cast<const AbstractFunctionDeclaration*>(m_declaration.data());
    if( function ) {
      htmlFunction();
    } else if( m_declaration->isTypeAlias() || m_declaration->type<EnumeratorType>() || m_declaration->kind() == Declaration::Instance ) {
      if( m_declaration->isTypeAlias() )
        modifyHtml() += importantHighlight("typedef ");

      if(m_declaration->type<EnumeratorType>())
        modifyHtml() += i18n("enumerator ");

      AbstractType::Ptr useType = m_declaration->abstractType();
      if(m_declaration->isTypeAlias()) {
        //Do not show the own name as type of typedefs
        if(useType.cast<TypeAliasType>())
          useType = useType.cast<TypeAliasType>()->type();
      }

      eventuallyMakeTypeLinks( useType );

      modifyHtml() += ' ' + identifierHighlight(Qt::escape(declarationName(m_declaration)), m_declaration);

      if(auto integralType = m_declaration->type<ConstantIntegralType>()) {
        const QString plainValue = integralType->valueAsString();
        if (!plainValue.isEmpty()) {
          modifyHtml() += QString(" = %1").arg(plainValue);
        }
      }

      modifyHtml() += "<br>";
    }else{
      if( m_declaration->kind() == Declaration::Type && m_declaration->abstractType().cast<StructureType>() ) {
        htmlClass();
      }
      if ( m_declaration->kind() == Declaration::Namespace ) {
        modifyHtml() += i18n("namespace %1 ", identifierHighlight(Qt::escape(m_declaration->qualifiedIdentifier().toString()), m_declaration));
      }

      if(m_declaration->type<EnumerationType>()) {
        EnumerationType::Ptr enumeration = m_declaration->type<EnumerationType>();
        modifyHtml() += i18n("enumeration %1 ", identifierHighlight(Qt::escape(m_declaration->identifier().toString()), m_declaration));
      }

      if(m_declaration->isForwardDeclaration()) {
        ForwardDeclaration* forwardDec = static_cast<ForwardDeclaration*>(m_declaration.data());
        Declaration* resolved = forwardDec->resolve(m_topContext.data());
        if(resolved) {
          modifyHtml() += i18n("( resolved forward-declaration: ");
          makeLink(resolved->identifier().toString(), KDevelop::DeclarationPointer(resolved), NavigationAction::NavigateDeclaration );
          modifyHtml() += i18n(") ");
        }else{
          modifyHtml() += i18n("(unresolved forward-declaration) ");
          QualifiedIdentifier id = forwardDec->qualifiedIdentifier();
          uint count;
          const IndexedDeclaration* decls;
          PersistentSymbolTable::self().declarations(id, count, decls);
          for(uint a = 0; a < count; ++a) {
            if(decls[a].isValid() && !decls[a].data()->isForwardDeclaration()) {
              modifyHtml() += "<br />";
              makeLink(i18n("possible resolution from"), KDevelop::DeclarationPointer(decls[a].data()), NavigationAction::NavigateDeclaration);
              modifyHtml() += ' ' + decls[a].data()->url().str();
            }
          }
        }
      }
      modifyHtml() += "<br />";
    }
  }else{
    AbstractType::Ptr showType = m_declaration->abstractType();
    if(showType && showType.cast<FunctionType>()) {
      showType = showType.cast<FunctionType>()->returnType();
      if(showType)
        modifyHtml() += labelHighlight(i18n("Returns: "));
    }else  if(showType) {
      modifyHtml() += labelHighlight(i18n("Type: "));
    }
    
    if(showType) {
      eventuallyMakeTypeLinks(showType);
      modifyHtml() += " ";
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
        modifyHtml() += labelHighlight(i18n("Enum: "));
      else
        modifyHtml() += labelHighlight(i18n("Container: "));

      makeLink( declarationName(DeclarationPointer(decl)), DeclarationPointer(decl), NavigationAction::NavigateDeclaration );
      modifyHtml() += " ";
    } else {
      QualifiedIdentifier parent = identifier;
      parent.pop();
      modifyHtml() += labelHighlight(i18n("Scope: %1 ", typeHighlight(Qt::escape(parent.toString()))));
    }
  }
  
  if( shorten && !m_declaration->comment().isEmpty() ) {
    QString comment = QString::fromUtf8(m_declaration->comment());
    if( comment.length() > 60 ) {
      comment.truncate(60);
      comment += "...";
    }
    comment.replace('\n', " ");
    comment.replace("<br />", " ");
    comment.replace("<br/>", " ");
    modifyHtml() += commentHighlight(Qt::escape(comment)) + "   ";
  }
  

  QString access = stringFromAccess(m_declaration);
  if( !access.isEmpty() )
    modifyHtml() += labelHighlight(i18n("Access: %1 ", propertyHighlight(Qt::escape(access))));


  ///@todo Enumerations

  QString detailsHtml;
  QStringList details = declarationDetails(m_declaration);
  if( !details.isEmpty() ) {
    bool first = true;
    foreach( const QString &str, details ) {
      if( !first )
        detailsHtml += ", ";
      first = false;
      detailsHtml += propertyHighlight(str);
    }
  }

  QString kind = declarationKind(m_declaration);
  if( !kind.isEmpty() ) {
    if( !detailsHtml.isEmpty() )
      modifyHtml() += labelHighlight(i18n("Kind: %1 %2 ", importantHighlight(Qt::escape(kind)), detailsHtml));
    else
      modifyHtml() += labelHighlight(i18n("Kind: %1 ", importantHighlight(Qt::escape(kind))));
  }

  if (m_declaration->isDeprecated()) {
    modifyHtml() += labelHighlight(i18n("Status: %1 ", propertyHighlight(i18n("Deprecated"))));
  }

  modifyHtml() += "<br />";

  if(!shorten)
    htmlAdditionalNavigation();
  
  if( !shorten ) {
    if(dynamic_cast<FunctionDefinition*>(m_declaration.data()))
      modifyHtml() += labelHighlight(i18n( "Def.: " ));
    else
      modifyHtml() += labelHighlight(i18n( "Decl.: " ));

    makeLink( QString("%1 :%2").arg( KUrl(m_declaration->url().str()).fileName() ).arg( m_declaration->rangeInCurrentRevision().textRange().start().line()+1 ), m_declaration, NavigationAction::JumpToSource );
    modifyHtml() += " ";
    //modifyHtml() += "<br />";
    if(!dynamic_cast<FunctionDefinition*>(m_declaration.data())) {
      if( FunctionDefinition* definition = FunctionDefinition::definition(m_declaration.data()) ) {
        modifyHtml() += labelHighlight(i18n( " Def.: " ));
        makeLink( QString("%1 :%2").arg( KUrl(definition->url().str()).fileName() ).arg( definition->rangeInCurrentRevision().textRange().start().line()+1 ), DeclarationPointer(definition), NavigationAction::JumpToSource );
      }
    }

    if( FunctionDefinition* definition = dynamic_cast<FunctionDefinition*>(m_declaration.data()) ) {
      if(definition->declaration()) {
        modifyHtml() += labelHighlight(i18n( " Decl.: " ));
        makeLink( QString("%1 :%2").arg( KUrl(definition->declaration()->url().str()).fileName() ).arg( definition->declaration()->rangeInCurrentRevision().textRange().start().line()+1 ), DeclarationPointer(definition->declaration()), NavigationAction::JumpToSource );
      }
    }
    
    modifyHtml() += " "; //The action name _must_ stay "show_uses", since that is also used from outside
    makeLink(i18n("Show uses"), "show_uses", NavigationAction(m_declaration, NavigationAction::NavigateUses));
  }
  
  if( !shorten && (!m_declaration->comment().isEmpty() || doc) ) {
    modifyHtml() += "<br />";
    QString comment = QString::fromUtf8(m_declaration->comment());
    if(comment.isEmpty() && doc) {
      comment = doc->description();
      if(!comment.isEmpty()) {
        connect(doc.data(), SIGNAL(descriptionChanged()), this, SIGNAL(contentsChanged()));
        modifyHtml() += "<br />" + commentHighlight(comment);
      }
    } else if(!comment.isEmpty()) {
      // if the first paragraph does not contain a tag, we assume that this is a plain-text comment
      if (!Qt::mightBeRichText(comment)) {
        // still might contain extra html tags for line breaks (this is the case for doxygen-style comments sometimes)
        // let's protect them from being removed completely
        comment.replace(QRegExp("<br */>"), "\n");
        comment = Qt::escape(comment);
        comment.replace('\n', "<br />"); //Replicate newlines in html
      }
      modifyHtml() += commentHighlight(comment);
      modifyHtml() += "<br />";
    }
  }
  
    if(!shorten && doc) {
      modifyHtml() += "<br />" + i18n("Show documentation for ");
      makeLink( prettyQualifiedIdentifier(m_declaration).toString(), m_declaration, NavigationAction::ShowDocumentation );
    }
  
  
    //modifyHtml() += "<br />";

  addExternalHtml(m_suffix);

  modifyHtml() += fontSizeSuffix(shorten) + "</p></body></html>";

  return currentHtml();
}

KDevelop::AbstractType::Ptr AbstractDeclarationNavigationContext::typeToShow(KDevelop::AbstractType::Ptr type) {
  return type;
}

void AbstractDeclarationNavigationContext::htmlFunction()
{
  const AbstractFunctionDeclaration* function = dynamic_cast<const AbstractFunctionDeclaration*>(m_declaration.data());
  Q_ASSERT(function);

  const ClassFunctionDeclaration* classFunDecl = dynamic_cast<const ClassFunctionDeclaration*>(m_declaration.data());
  const FunctionType::Ptr type = m_declaration->abstractType().cast<FunctionType>();
  if( !type ) {
    modifyHtml() += errorHighlight("Invalid type<br />");
    return;
  }

  if( !classFunDecl || (!classFunDecl->isConstructor() && !classFunDecl->isDestructor()) ) {
    // only print return type for global functions and non-ctor/dtor methods
    eventuallyMakeTypeLinks( type->returnType() );
  }

  modifyHtml() += ' ' + identifierHighlight(Qt::escape(prettyIdentifier(m_declaration).toString()), m_declaration);

  if( type->indexedArgumentsSize() == 0 )
  {
    modifyHtml() += "()";
  } else {
    modifyHtml() += "( ";

    bool first = true;
    int firstDefaultParam = type->indexedArgumentsSize() - function->defaultParametersSize();
    int currentArgNum = 0;

    QVector<Declaration*> decls;
    if (KDevelop::DUContext* argumentContext = DUChainUtils::getArgumentContext(m_declaration.data())) {
      decls = argumentContext->localDeclarations(m_topContext.data());
    }
    foreach(const AbstractType::Ptr& argType, type->arguments()) {
      if( !first )
        modifyHtml() += ", ";
      first = false;

      eventuallyMakeTypeLinks( argType );
      if (currentArgNum < decls.size()) {
        modifyHtml() += ' ' + identifierHighlight(Qt::escape(decls[currentArgNum]->identifier().toString()), m_declaration);
      }

      if( currentArgNum >= firstDefaultParam )
        modifyHtml() += " = " + Qt::escape(function->defaultParameters()[ currentArgNum - firstDefaultParam ].str());

      ++currentArgNum;
    }

    modifyHtml() += " )";
  }
  modifyHtml() += "<br />";
}


Identifier AbstractDeclarationNavigationContext::prettyIdentifier(DeclarationPointer decl) const
{
  Identifier ret;
  QualifiedIdentifier q = prettyQualifiedIdentifier(decl);
  if(!q.isEmpty())
    ret = q.last();
  
  return ret;
}

QualifiedIdentifier AbstractDeclarationNavigationContext::prettyQualifiedIdentifier(DeclarationPointer decl) const
{
  if(decl)
    return decl->qualifiedIdentifier();
  else
    return QualifiedIdentifier();
}

void AbstractDeclarationNavigationContext::htmlAdditionalNavigation()
{
  ///Check if the function overrides or hides another one
  const ClassFunctionDeclaration* classFunDecl = dynamic_cast<const ClassFunctionDeclaration*>(m_declaration.data());
  if(classFunDecl) {
    
    Declaration* overridden = DUChainUtils::getOverridden(m_declaration.data());

    if(overridden) {
        modifyHtml() += i18n("Overrides a ");
        makeLink(i18n("function"), QString("jump_to_overridden"), NavigationAction(DeclarationPointer(overridden), KDevelop::NavigationAction::NavigateDeclaration));
        modifyHtml() += i18n(" from ");
        makeLink(prettyQualifiedIdentifier(DeclarationPointer(overridden->context()->owner())).toString(), QString("jump_to_overridden_container"), NavigationAction(DeclarationPointer(overridden->context()->owner()), KDevelop::NavigationAction::NavigateDeclaration));
        
        modifyHtml() += "<br />";
    }else{
      //Check if this declarations hides other declarations
      QList<Declaration*> decls;
      foreach(const DUContext::Import &import, m_declaration->context()->importedParentContexts())
        if(import.context(m_topContext.data()))
          decls += import.context(m_topContext.data())->findDeclarations(QualifiedIdentifier(m_declaration->identifier()), 
                                                CursorInRevision::invalid(), AbstractType::Ptr(), m_topContext.data(), DUContext::DontSearchInParent);
      uint num = 0;
      foreach(Declaration* decl, decls) {
        modifyHtml() += i18n("Hides a ");
        makeLink(i18n("function"), QString("jump_to_hide_%1").arg(num), NavigationAction(DeclarationPointer(decl), KDevelop::NavigationAction::NavigateDeclaration));
        modifyHtml() += i18n(" from ");
        makeLink(prettyQualifiedIdentifier(DeclarationPointer(decl->context()->owner())).toString(), QString("jump_to_hide_container_%1").arg(num), NavigationAction(DeclarationPointer(decl->context()->owner()), KDevelop::NavigationAction::NavigateDeclaration));
        
        modifyHtml() += "<br />";
        ++num;
      }
    }
    
    ///Show all places where this function is overridden
    if(classFunDecl->isVirtual()) {
      Declaration* classDecl = m_declaration->context()->owner();
      if(classDecl) {
        uint maxAllowedSteps = m_fullBackwardSearch ? (uint)-1 : 10;
        QList<Declaration*> overriders = DUChainUtils::getOverriders(classDecl, classFunDecl, maxAllowedSteps);
        
        if(!overriders.isEmpty()) {
          modifyHtml() += i18n("Overridden in ");
          bool first = true;
          foreach(Declaration* overrider, overriders) {
            if(!first)
              modifyHtml() += ", ";
            first = false;
            
            QString name = prettyQualifiedIdentifier(DeclarationPointer(overrider->context()->owner())).toString();
            makeLink(name, name, NavigationAction(DeclarationPointer(overrider), NavigationAction::NavigateDeclaration));
          }
          modifyHtml() += "<br />";
        }
        if(maxAllowedSteps == 0)
          createFullBackwardSearchLink(overriders.isEmpty() ? i18n("Overriders possible, show all") : i18n("More overriders possible, show all"));
      }
    }
  }
  
  ///Show all classes that inherit this one
  uint maxAllowedSteps = m_fullBackwardSearch ? (uint)-1 : 10;
  QList<Declaration*> inheriters = DUChainUtils::getInheriters(m_declaration.data(), maxAllowedSteps);
  
  if(!inheriters.isEmpty()) {
      modifyHtml() += i18n("Inherited by ");
      bool first = true;
      foreach(Declaration* importer, inheriters) {
        if(!first)
          modifyHtml() += ", ";
        first = false;
        
        QString importerName = prettyQualifiedIdentifier(DeclarationPointer(importer)).toString();
        makeLink(importerName, importerName, NavigationAction(DeclarationPointer(importer), KDevelop::NavigationAction::NavigateDeclaration));
      }
      modifyHtml() += "<br />";
  }
  if(maxAllowedSteps == 0)
    createFullBackwardSearchLink(inheriters.isEmpty() ? i18n("Inheriters possible, show all") : i18n("More inheriters possible, show all"));
}

void AbstractDeclarationNavigationContext::createFullBackwardSearchLink(QString string)
{
  makeLink(string, "m_fullBackwardSearch=true", NavigationAction("m_fullBackwardSearch=true"));
  modifyHtml() += "<br />";
}

NavigationContextPointer AbstractDeclarationNavigationContext::executeKeyAction( QString key )
{
  if(key == "m_fullBackwardSearch=true") {
    m_fullBackwardSearch = true;
    clear();
  }
  return NavigationContextPointer(this);
}

void AbstractDeclarationNavigationContext::htmlClass()
{
  StructureType::Ptr klass = m_declaration->abstractType().cast<StructureType>();
  Q_ASSERT(klass);
  
  ClassDeclaration* classDecl = dynamic_cast<ClassDeclaration*>(klass->declaration(m_topContext.data()));
  if(classDecl) {
    switch ( classDecl->classType() ) {
      case ClassDeclarationData::Class:
        modifyHtml() += "class ";
        break;
      case ClassDeclarationData::Struct:
        modifyHtml() += "struct ";
        break;
      case ClassDeclarationData::Union:
        modifyHtml() += "union ";
        break;
      case ClassDeclarationData::Interface:
        modifyHtml() += "interface ";
        break;
      case ClassDeclarationData::Trait:
        modifyHtml() += "trait ";
        break;
      default:
        modifyHtml() += "<unknown type> ";
        break;
    }
    eventuallyMakeTypeLinks( klass.cast<AbstractType>() );
    
    FOREACH_FUNCTION( const KDevelop::BaseClassInstance& base, classDecl->baseClasses ) {
      modifyHtml() += ", " + stringFromAccess(base.access) + " " + (base.virtualInheritance ? QString("virtual") : QString()) + " ";
      eventuallyMakeTypeLinks(base.baseClass.abstractType());
    }
  } else {
    /// @todo How can we get here? and should this really be a class?
    modifyHtml() += "class ";
    eventuallyMakeTypeLinks( klass.cast<AbstractType>() );
  }
  modifyHtml() += " ";
}

void AbstractDeclarationNavigationContext::htmlIdentifiedType(AbstractType::Ptr type, const IdentifiedType* idType)
{
  Q_ASSERT(type);
  Q_ASSERT(idType);

  if( Declaration* decl = idType->declaration(m_topContext.data()) ) {
    
    //Remove the last template-identifiers, because we create those directly
    QualifiedIdentifier id = prettyQualifiedIdentifier(DeclarationPointer(decl));
    Identifier lastId = id.last();
    id.pop();
    lastId.clearTemplateIdentifiers();
    id.push(lastId);
    
    if(decl->context() && decl->context()->owner()) {
      
      //Also create full type-links for the context around
      AbstractType::Ptr contextType = decl->context()->owner()->abstractType();
      IdentifiedType* contextIdType = dynamic_cast<IdentifiedType*>(contextType.unsafeData());
      if(contextIdType && !contextIdType->equals(idType)) {
        //Create full type information for the context
        if(!id.isEmpty())
          id = id.mid(id.count()-1);
        htmlIdentifiedType(contextType, contextIdType);
        modifyHtml() += Qt::escape("::");
      }
    }

    //We leave out the * and & reference and pointer signs, those are added to the end
    makeLink(id.toString() , DeclarationPointer(idType->declaration(m_topContext.data())), NavigationAction::NavigateDeclaration );
  } else {
    kDebug() << "could not resolve declaration:" << idType->declarationId().isDirect() << idType->qualifiedIdentifier().toString() << "in top-context" << m_topContext->url().str();
    modifyHtml() += typeHighlight(Qt::escape(type->toString()));
  }
}

void AbstractDeclarationNavigationContext::eventuallyMakeTypeLinks( AbstractType::Ptr type )
{
  type = typeToShow(type);
  
  if( !type ) {
    modifyHtml() += typeHighlight(Qt::escape("<no type>"));
    return;
  }

  AbstractType::Ptr target = TypeUtils::targetTypeKeepAliases( type, m_topContext.data() );
  const IdentifiedType* idType = dynamic_cast<const IdentifiedType*>( target.unsafeData() );

  kDebug() << "making type-links for" << type->toString() << typeid(*type).name();
  
  if( idType && idType->declaration(m_topContext.data()) ) {
    ///@todo This is C++ specific, move into subclass
    
    if(target->modifiers() & AbstractType::ConstModifier)
      modifyHtml() += typeHighlight("const ");
    
    htmlIdentifiedType(target, idType);

    //We need to exchange the target type, else template-parameters may confuse this
    SimpleTypeExchanger exchangeTarget(target, AbstractType::Ptr());
    
    AbstractType::Ptr exchanged = exchangeTarget.exchange(type);
    
    if(exchanged) {
      QString typeSuffixString = exchanged->toString();
      QRegExp suffixExp("\\&|\\*");
      int suffixPos = typeSuffixString.indexOf(suffixExp);
      if(suffixPos != -1)
        modifyHtml() += typeHighlight(typeSuffixString.mid(suffixPos));
    }

  } else {
    if(idType) {
      kDebug() << "identified type could not be resolved:" << idType->qualifiedIdentifier() << idType->declarationId().isValid() << idType->declarationId().isDirect();
    }
    modifyHtml() += typeHighlight(Qt::escape(type->toString()));
  }
}

DeclarationPointer AbstractDeclarationNavigationContext::declaration() const
{
  return m_declaration;
}

QString AbstractDeclarationNavigationContext::identifierHighlight(const QString& identifier, const DeclarationPointer& decl) const
{
  QString ret = nameHighlight(identifier);
  if (!decl) {
    return ret;
  }

  if (decl->isDeprecated()) {
    ret = QString("<s>%1</s>").arg(ret);
  }
  return ret;
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
    default:
      break;
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

QString AbstractDeclarationNavigationContext::declarationName( DeclarationPointer decl ) const
{
  if( NamespaceAliasDeclaration* alias = dynamic_cast<NamespaceAliasDeclaration*>(decl.data()) ) {
    if( alias->identifier().isEmpty() )
      return "using namespace " + alias->importIdentifier().toString();
    else
      return "namespace " + alias->identifier().toString() + " = " + alias->importIdentifier().toString();
  }

  if( !decl )
    return i18nc("A declaration that is unknown", "Unknown");
  else
    return prettyIdentifier(decl).toString();
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
    details << i18nc("tells if a declaration is defining the variable's value", "definition");
  if( decl->isExplicitlyDeleted() )
    details << "deleted";

  if( memberDecl && memberDecl->isForwardDeclaration() )
    details << i18nc("as in c++ forward declaration", "forward");

  AbstractType::Ptr t(decl->abstractType());
  if( t ) {
    if( t->modifiers() & AbstractType::ConstModifier )
      details << i18nc("a variable that won't change, const", "constant");
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
      if( classFunDecl->isSignal() )
        details << "signal";
      if( classFunDecl->isSlot() )
        details << "slot";
      if( classFunDecl->isConstructor() )
        details << "constructor";
      if( classFunDecl->isDestructor() )
        details << "destructor";
      if( classFunDecl->isConversionFunction() )
        details << "conversion-function";
      if( classFunDecl->isAbstract() )
        details << "abstract";
    }
  }
  
  return details;
}

}
