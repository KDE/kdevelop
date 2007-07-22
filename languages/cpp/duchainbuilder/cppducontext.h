/* This file is part of KDevelop
    Copyright (C) 2007 David Nolden [david.nolden.kdevelop  art-master.de]

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

/*

Some mindmapping about how the template-system works:

While construction:
- Simplify: Template-parameters are types
- Within template-contexts, do not resolve any types. Instead create "virtual types" that will resolve the types when specializations are given.
 (CppDelayedType) - ready

 
 Later:
 - Searching specialized template-class: 
        - return virtual declaration 
        - return virtual context (Change template-parameter-context to given template-arguments)
 - Searching IN specialized template-class:
       - Search in non-specialized context, then:
       - Copy & Change returned objects:
         - Resolve virtual types (CppDelayedType)
         - Change parent-context to virtual context
         - Change internal context, (create virtual, move set parent)

*/

#ifndef CPPDUCONTEXT_H
#define CPPDUCONTEXT_H

/** @todo Remove this. It is currently needed because Cpp::DUContext<KDevelop::DUContext> cannot call protected members of KDevelop::DUContext,
 *  which is wrong because KDevelop::DUContext is a base-class of Cpp::DUContext<KDevelop::DUContext>. Find out why this happens and then remove this.
 * */
#define protected public
#include <duchain/ducontext.h>
#undef protected

#include <duchain/abstractfunctiondeclaration.h>
#include <duchain/declaration.h>
#include "typeutils.h"
#include "cpptypes.h"
#include "templatedeclaration.h"
#include "expressionparser/expressionparser.h"

namespace Cpp{
using namespace KDevelop;

/**
 * This is a du-context template that wraps the c++-specific logic around existing DUContext specializations.
 * In practice this means DUContext and TopDUContext.
 * */
template<class BaseContext>
class DUContext : public BaseContext {
  public:
    ///Both parameters will be reached to the base-class
    template<class Param1, class Param2>
    DUContext( Param1* p1, Param2* p2 ) : BaseContext(p1, p2) {
    }

    ///Overridden to take care of templates
    virtual void findDeclarationsInternal(const QualifiedIdentifier& identifier, const KTextEditor::Cursor& position, const AbstractType::Ptr& dataType, QList<KDevelop::DUContext::UsingNS*>& usingNamespaces, QList<Declaration*>& ret, typename BaseContext::SearchFlags basicFlags ) const
    {
      ///@todo maybe move parts of this logic directly into the du-chain

      ///Iso c++ 3.4.3.1 and 3.4.3.2 say that identifiers should be looked up part by part
      ///Since we cannot directly locate namespaces becase A) they have no declaration and b) they may be declared in multiple positions,
      ///we put qualified identifiers in the form of Namespace::...::Identifier together in currentLookup.
      QualifiedIdentifier currentLookup;
      KDevelop::DUContext* scopeContext = 0; //The scope(class) we are searching in
      

      for( int num = 0; num < identifier.count(); num++ )
      {
        Identifier currentIdentifier = identifier.at(num);
        
        ///Step 1: Resolve the template-arguments
        //Since there may be non-type template-parameters, represent them as ExpressionEvaluationResult's
        QList<ExpressionEvaluationResult> templateArgumentTypes;

        for( int a = 0; a < currentIdentifier.templateIdentifiers().size(); a++ ) {
          QList<KDevelop::Declaration*> decls = BaseContext::findDeclarations( currentIdentifier.templateIdentifiers().at(a) );
          if( !decls.isEmpty() ) {
            ExpressionEvaluationResult res;
            res.type = decls.front()->abstractType();

            ///If the type is CppTemplateParameterType, this means that an unresolved template-parameter is refernced.
            ///That should fail, so return here.
            if( (basicFlags & KDevelop::DUContext::NoUndefinedTemplateParams) && dynamic_cast<CppTemplateParameterType*>(res.type.data()) )
              return;
            
            templateArgumentTypes << res;
          }else{
            ///@todo Let the expression-parser handle the thing. This will allow evaluating integral expressions like "1 - 1" and such
            ///problem: the du-chain is already locked
            templateArgumentTypes << ExpressionEvaluationResult();
            kDebug() << "Could not resolve template-parameter \"" << currentIdentifier.templateIdentifiers().at(a).toString() << "\" in \"" << identifier.toString() << endl;
          }
        }

        currentIdentifier.clearTemplateIdentifiers();
        
        currentLookup.push(currentIdentifier);

        ///Step 2: Find the type
        typename BaseContext::SearchFlags flags = (num != (identifier.count()-1)) ? BaseContext::OnlyContainerTypes : BaseContext::NoSearchFlags;
        flags |= basicFlags;
        
        QList<Declaration*> tempDecls;
        if( !scopeContext ) {
          BaseContext::findDeclarationsInternal( currentLookup, position, dataType, usingNamespaces, tempDecls, flags );

          //If the use of unresolved template-parameters is not allowed, filter them out
          if( (basicFlags & KDevelop::DUContext::NoUndefinedTemplateParams) ) {
            QList<Declaration*>::iterator it = tempDecls.begin();
            while( it != tempDecls.end() ) {
              if( dynamic_cast<CppTemplateParameterType*>(*it) ) {
                //Unresolved template-paramers are not allowed, so remove the item from the list
                it = tempDecls.erase(it);
              } else {
                ++it;
              }
            }
          }
          
        } else ///@todo Check whether it is the same file, if yes keep the position-cursor
          scopeContext->findDeclarationsInternal( currentLookup, KTextEditor::Cursor(), dataType, usingNamespaces, tempDecls, flags | BaseContext::DontSearchInParent );

        
        if( !tempDecls.isEmpty() ) {
          //We have found a part of the scope
          if( num == identifier.count()-1 ) {
            //Last part of the scope found -> target found
            if( templateArgumentTypes.isEmpty() ) {
              ret += tempDecls;
            } else {
              foreach( Declaration* decl, tempDecls ) {
                Declaration* dec = specializeDeclaration(decl, templateArgumentTypes);
                if( dec )
                  ret << dec;
                else
                  kDebug() << "Could not specialize template-declaration" << endl;
              }
            }
          }else{
            //Only a part of the scope found, keep on searching
            currentLookup.clear();
            if( tempDecls.size() == 1 ) {
            } else {
              kDebug() << "Cpp::DUContext::findDeclarationsInternal: found " << tempDecls.size() << " multiple ambiguous declarations for scope " << currentLookup.toString() << endl;
            }
            //Extract a context, maybe it would be enough only testing the first found declaration
            foreach( Declaration* decl, tempDecls ) {
              Declaration* dec = decl;
              
              if( !templateArgumentTypes.isEmpty() )
                dec = specializeDeclaration(decl, templateArgumentTypes);
              
              if( !dec ) {
                kDebug() << "Could not specialize context-declaration" << endl;
                continue;
              }
              
              scopeContext = TypeUtils::getInternalContext(decl);
              if( scopeContext )
                break;
            }
            if( !scopeContext || scopeContext->type() != DUContext::Class ) {
              kDebug() << "Cpp::DUContext::findDeclarationsInternal: could not get a class-context from " << tempDecls.size() << " declarations for scope " << currentLookup.toString() << endl;
              return;
              
            }
          }
        } else {
          //Nothing was found for currentLookup.
          if( num != identifier.count() - 1 ) {
            //This is ok in the case that currentLookup stands for a namespace, because namespaces do not have a declaration.
            for( int a = 0; a < currentLookup.count(); a++ ) {
              if( currentLookup.at(a).templateIdentifiers().count() != 0 ) {
                kDebug() << "Cpp::DUContext::findDeclarationsInternal: while searching " << identifier.toString() << " Template in scope could not be located: " << currentLookup.toString() << endl;
                return; //If one of the parts has a template-identifier, it cannot be a namespace
              }
            }
          } else {
            //Final part of the scope not found
            return;
          }
        }
      }
    }

    virtual void findLocalDeclarationsInternal( const QualifiedIdentifier& identifier, const KTextEditor::Cursor & position, const AbstractType::Ptr& dataType, bool allowUnqualifiedMatch, QList<Declaration*>& ret ) const
    {
        return findLocalDeclarationsInternal(identifier, position, dataType, allowUnqualifiedMatch, ret);
    }
    
    virtual bool foundEnough( const QList<Declaration*>& decls ) const
    {
      if( decls.isEmpty() )
        return false;
      
      if( decls.front()->isForwardDeclaration() )
        return false; //All forward-declarations must be collected(for implementation-reasons)

      if( dynamic_cast<const KDevelop::AbstractFunctionDeclaration*>(decls.front()) )
        return false; //Collect overloaded function-declarations

      return true;
    }

  private:
    Declaration* specializeDeclaration( Declaration* decl, const QList<ExpressionEvaluationResult>& templateArguments ) const
    {
      TemplateDeclaration* templateDecl = dynamic_cast<TemplateDeclaration*>(decl);
      if( !templateDecl ) {
        kDebug() << "Tried to specialize a non-template declaration" << endl;
        return 0;
      }

      return templateDecl->instantiate( templateArguments );
    }
};


}

#endif
