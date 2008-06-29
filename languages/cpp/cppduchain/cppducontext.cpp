/* This file is part of KDevelop
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

#include "cppducontext.h"
#include "navigationwidget.h"

namespace Cpp {

QMutex cppDuContextInstantiationsMutex(QMutex::Recursive);

template<>
QWidget* CppDUContext<TopDUContext>::createNavigationWidget( Declaration* decl, TopDUContext* topContext, const QString& htmlPrefix, const QString& htmlSuffix ) const {
  if( decl == 0 ) {
    KUrl u( url().str() );
    IncludeItem i;
    i.pathNumber = -1;
    i.name = u.fileName();
    i.isDirectory = false;
    i.basePath = u.upUrl();
    
    return new NavigationWidget( i, TopDUContextPointer(topContext), htmlPrefix, htmlSuffix );
  } else {
    return new NavigationWidget( DeclarationPointer(decl), TopDUContextPointer(topContext), htmlPrefix, htmlSuffix );
  }
}

template<>
QWidget* CppDUContext<DUContext>::createNavigationWidget(Declaration* decl, TopDUContext* topContext, const QString& htmlPrefix, const QString& htmlSuffix) const {
  if( decl == 0 ) {
    if( owner() )
      return new NavigationWidget( DeclarationPointer(owner()), TopDUContextPointer(topContext), htmlPrefix, htmlSuffix );
    else
      return 0;
  } else {
    return new NavigationWidget( DeclarationPointer(decl), TopDUContextPointer(topContext), htmlPrefix, htmlSuffix );
  }
}

Declaration* FindDeclaration::instantiateDeclaration( Declaration* decl, const InstantiationInformation& templateArguments ) const
{
  if( !templateArguments.isValid() )
    return decl;
  
  TemplateDeclaration* templateDecl = dynamic_cast<TemplateDeclaration*>(decl);
  if( !templateDecl ) {
    ifDebug( kDebug(9007) << "Tried to instantiate a non-template declaration" << decl->toString(); )
    return 0;
  }
  InstantiationInformation info(templateArguments);
  CppDUContext<DUContext>* context = dynamic_cast<CppDUContext<DUContext>*>(decl->context());
  
  if(context && context->instantiatedWith().isValid())
    info.previousInstantiationInformation = context->instantiatedWith().index();


  return templateDecl->instantiate( info, m_source );
}

void FindDeclaration::closeQualifiedIdentifier() {
  State s = m_states.top();
  m_lastDeclarations = s.result;
  m_states.pop();
  if( !m_states.isEmpty() ) {
    //Append template-parameter to parent
    if( s.expressionResult.isValid() ) {
      m_states.top().templateParameters.templateParametersList().append(s.expressionResult.indexed());
    } else {
      ExpressionEvaluationResult res;
      if( !s.result.isEmpty() ) {
        res.allDeclarationsList().clear();
        foreach(DeclarationPointer decl, s.result)
          if(decl)
            res.allDeclarationsList().append(decl->id()); ///@todo prevent unneeded conversions here
        if(s.result[0]->abstractType())
          res.type = s.result[0]->abstractType()->indexed();
        res.isInstance = s.result[0]->kind() != Declaration::Type;
      }
      m_states.top().templateParameters.templateParametersList().append(res.indexed());
    }
  }
}

bool FindDeclaration::closeIdentifier(bool isFinalIdentifier) {
  State& s = m_states.top();
  QualifiedIdentifier lookup = s.identifier;
  
  DUContext::SearchItem::PtrList allIdentifiers;
  allIdentifiers.append( DUContext::SearchItem::Ptr( new DUContext::SearchItem(lookup) ) );
  
  ///Search a Declaration of the identifier

  DUContext* scopeContext = 0;

  if( !s.result.isEmpty() && lookup.count() == 1 ) { //When we are searching within a scope-context, no namespaces are involved any more, so we look up exactly one item at a time.

    //Eventually extract a scope context
    foreach( DeclarationPointer decl, s.result ) {
      if( !decl )
        continue;
      
      scopeContext = decl->logicalInternalContext(topContext());

      if( !scopeContext || scopeContext->type() == DUContext::Template )
        if( IdentifiedType* idType = dynamic_cast<IdentifiedType*>(decl->abstractType().data()) ) //Try to get the context from the type, maybe it is a typedef.
          if( idType->declaration() )
            scopeContext = idType->declaration()->logicalInternalContext(topContext());

#ifdef DEBUG
        kDebug(9007) << decl->toString() << ": scope-context" << scopeContext;
        if(scopeContext)
          kDebug(9007) << "scope-context-type" << scopeContext->type();
#endif

      if( scopeContext && scopeContext->type() == DUContext::Class )
        break;
    }
    
    if( scopeContext && scopeContext->owner() && scopeContext->owner()->isForwardDeclaration() ) {
      kDebug(9007) << "Tried to search in forward-declaration of " << scopeContext->owner()->identifier().toString();
      m_lastScopeContext = DUContextPointer(scopeContext);
      scopeContext = 0;
    }
    
    if( !scopeContext ) {
      s.result.clear();
      return false;
    }
  }

  m_lastScopeContext = DUContextPointer(scopeContext);
  
  /// Look up Declarations

  DUContext::SearchFlags basicFlags = isFinalIdentifier ? m_flags : DUContext::OnlyContainerTypes;
  
  DUContext::DeclarationList tempDecls;
  if( !scopeContext ) {
    m_context->findDeclarationsInternal( allIdentifiers, m_position, m_dataType, tempDecls, m_source, basicFlags | DUContext::DirectQualifiedLookup );
    if( tempDecls.isEmpty() ) {
      //To simulate a search starting at searchContext->scopIdentifier, we must search the identifier with all partial scopes prepended

      DUContext::SearchItem::Ptr prependedSearch( new DUContext::SearchItem(m_context->scopeIdentifier(false)) );
      prependedSearch->addToEachNode( allIdentifiers[0] );
      
      if(!prependedSearch->next.isEmpty()) //Can happen when explicitly global is set
        insertToArray(allIdentifiers, prependedSearch, 0);

      //If we have a trace, walk the trace up so we're able to find the item in earlier imported contexts.
      ImportTrace trace;
      m_source->importTrace(m_context->topContext(), trace);
      for( int a = trace.count()-1; a >= 0; --a ) {
        const ImportTraceItem& traceItem(trace[a]);
        DUContext::DeclarationList decls;
        ///@todo Give a correctly modified trace(without the used items)
        traceItem.ctx->findDeclarationsInternal( allIdentifiers, traceItem.position.isValid() ? traceItem.position : traceItem.ctx->range().end, AbstractType::Ptr(), decls, m_source, (KDevelop::DUContext::SearchFlag)(KDevelop::DUContext::NoUndefinedTemplateParams | KDevelop::DUContext::DirectQualifiedLookup) );
        if( !decls.isEmpty() ) {
          tempDecls = decls;
          break;
        }
      }
    }
  } else { //Create a new trace, so template-parameters can be resolved globally
    scopeContext->findDeclarationsInternal( allIdentifiers, scopeContext->url() == m_context->url() ? m_position : scopeContext->range().end, m_dataType, tempDecls, topContext(), basicFlags | DUContext::DontSearchInParent | DUContext::DirectQualifiedLookup );
  }

  if( !tempDecls.isEmpty() ) {
    s.result.clear();
    //instantiate template declarations
    FOREACH_ARRAY(Declaration* decl, tempDecls) {
      if( !s.templateParameters.isValid() ) {
        s.result << DeclarationPointer(decl);
      }else{
        Declaration* dec = instantiateDeclaration(decl, s.templateParameters);
        if( dec )
          s.result << DeclarationPointer(dec);
      }
    }

    s.templateParameters = InstantiationInformation();

    ///@todo When there is a namespace-alias and a class, what to prefer?
    
    ///Namespace-aliases are treated elsewhere, and should not screw our search, so simply remove them
    bool hadNamespaceAlias = false;
    for(QList<DeclarationPointer>::iterator it = s.result.begin(); it != s.result.end(); ++it) {
      if( dynamic_cast<NamespaceAliasDeclaration*>(it->data()) )
        hadNamespaceAlias = true;
    }

    if(!hadNamespaceAlias)
      s.identifier.clear();
  } else {
    s.result.clear();
    s.templateParameters = InstantiationInformation();
    //Nothing was found 
      //This is ok in the case that currentLookup stands for a namespace, because namespaces do not have a declaration.
      if( s.templateParameters.isValid() ) {
        kDebug(9007) << "CppDUContext::findDeclarationsInternal: Template in scope could not be located: " << lookup.toString();
        return false; //If one of the parts has a template-identifier, it cannot be a namespace
      }
  }

  m_lastDeclarations = s.result;
  
  return true;
}
}
