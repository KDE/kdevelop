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
            templateArgumentTypes << res;
          }else{
            ///@todo Let the expression-parser handle the thing. This will allow evaluating integral expressions like "1 - 1" and such
            ///problem: the du-chain is already locked
            templateArgumentTypes << ExpressionEvaluationResult();
          }
        }

        currentIdentifier.clearTemplateIdentifiers();
        
        currentLookup.push(currentIdentifier);

        ///Step 2: Find the type
        typename BaseContext::SearchFlags flags = (num != (identifier.count()-1)) ? BaseContext::OnlyContainerTypes : BaseContext::NoSearchFlags;
        flags |= basicFlags;
        
        QList<Declaration*> tempDecls;
        if( !scopeContext )
          BaseContext::findDeclarationsInternal( currentLookup, position, dataType, usingNamespaces, tempDecls, flags );
        else
          scopeContext->findDeclarationsInternal( currentLookup, position, dataType, usingNamespaces, tempDecls, flags | BaseContext::DontSearchInParent );

        if( !tempDecls.isEmpty() ) {
          //We have found a part of the scope
          if( num == identifier.count()-1 ) {
            //Last part of the scope found -> target found
            ret += tempDecls;
          }else{
            //Only a part of the scope found, keep on searching
            currentLookup.clear();
            if( tempDecls.size() == 1 ) {
            } else {
              kDebug() << "Cpp::DUContext::findDeclarationsInternal: found " << tempDecls.size() << " multiple ambiguous declarations for scope " << currentLookup.toString() << endl;
            }
            //Extract a context, maybe it would be enough only testing the first found declaration
            foreach( Declaration* decl, tempDecls ) {
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
        return false; //All forward-declarations must be collected

      if( dynamic_cast<const KDevelop::AbstractFunctionDeclaration*>(decls.front()) )
        return false; //Collect overloaded function-declarations

      return true;
    }
};


}

#endif
