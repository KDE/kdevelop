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

/*

Some mindmapping about how the template-system works:

While construction:
- Simplify: Template-parameters are types
- Within template-contexts, do not resolve any types. Instead create "virtual types" that will resolve the types when template-parameters are given.
 (DelayedType) - ready

 
 Later:
 - Searching instantiated template-class:
        - return virtual declaration
        - return virtual context (Change template-parameter-context to given template-arguments)
 - Searching IN instantiated template-class:
       - When searching local declarations:
         - Check whether they are already in the instantiated context, if yes return them
         - If not, Search in non-instantiated context(only for local declarations), then:
           - Copy & Change returned objects:
             - Resolve virtual types (DelayedType)
             - Change parent-context to virtual context
             - Change internal context, (create virtual, move set parent)

 - How template-parameters are resolved:
    - The DUContext's with type DUContext::Template get their template-parameter declarations instantiated and added locally. Then they will be found when resolving virtual types.
    - 

*/

#define ifDebug(x)
//#define ifDebug(x) x

#ifndef CPPDUCONTEXT_H
#define CPPDUCONTEXT_H

/** @todo Remove this. It is currently needed because CppDUContext<KDevelop::DUContext> cannot call protected members of KDevelop::DUContext,
 *  which is wrong because KDevelop::DUContext is a base-class of CppDUContext<KDevelop::DUContext>. Find out why this happens and then remove this.
 * */
#include <duchain/ducontext.h>

#include <QSet>
#include <QMutex>

#include <duchain/abstractfunctiondeclaration.h>
#include <duchain/declaration.h>
#include <duchain/duchainlock.h>
#include <duchain/duchain.h>
#include <duchain/topducontext.h>
#include <duchain/classfunctiondeclaration.h>
#include <duchain/namespacealiasdeclaration.h>
#include "typeutils.h"
#include "cpptypes.h"
#include "templatedeclaration.h"
#include "expressionparser.h"

using namespace KDevelop;

namespace Cpp {
extern QMutex cppDuContextInstantiationsMutex;

    ///This class breaks up the logic of searching a declaration in C++, so QualifiedIdentifiers as well as AST-based lookup mechanisms can be used for searching
    class FindDeclaration {
      public:
        FindDeclaration( const DUContext* ctx, const ImportTrace& trace, DUContext::SearchFlags flags, const SimpleCursor& position, AbstractType::Ptr dataType = AbstractType::Ptr() ) : m_context(ctx), m_trace(trace), m_flags(flags), m_position(position), m_dataType(dataType) {
        }
        void openQualifiedIdentifier( bool isExplicitlyGlobal ) {
          QualifiedIdentifier i;
          i.setExplicitlyGlobal( isExplicitlyGlobal );
          State s;
          s.identifier = i;
          m_states << s;
        }

        ///Can be used to just append a result that was computed outside. closeQualifiedIdentifier(...) must still be called.
        void openQualifiedIdentifier( const ExpressionEvaluationResult& result ) {
          State s;
          s.expressionResult = result;
          s.result = result.allDeclarations;
          
          m_states << s;
        }
        

        /**
         * After this was called, lastDeclarations(..) can be used to retrieve declarations of the qualified identifier.
         * The DUChain needs to be locked when this is called.
         * */
        void closeQualifiedIdentifier() {
          State s = m_states.top();
          m_lastDeclarations = s.result;
          m_states.pop();
          if( !m_states.isEmpty() ) {
            //Append template-parameter to parent
            if( s.expressionResult.isValid() ) {
              m_states.top().templateParameters << s.expressionResult;
            } else {
              ExpressionEvaluationResult res;
              if( !s.result.isEmpty() ) {
                res.allDeclarations = s.result;
                res.type = s.result[0]->abstractType();
                res.instance = s.result[0]->kind() != Declaration::Type;
              }
              m_states.top().templateParameters << res;
            }
          }
        }
        /**
         * The identifier must not have template identifiers, those need to be added using openQualifiedIdentifier(..) and closeQualifiedIdentifier(..)
         * */
        void openIdentifier( const Identifier& identifier ) {
#ifdef DEBUG
         for( int a = 0; a < identifier.count(); ++a )
           Q_ASSERT(identifier.at(a).templateIdentifiers().count() == 0);
#endif
         m_states.top().identifier.push(identifier);
        }
        /**
         * When closeIdentifier() is called, the last opened identifier is searched, and can be retrieved using lastDeclarations().
         * Returns false when the search should be stopped.
         * The DUChain needs to be locked when this is called.
         * */
        bool closeIdentifier() {
          State& s = m_states.top();
          QualifiedIdentifier lookup = s.identifier;
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
            
            if( !scopeContext ) {
              s.result.clear();
              return false;
            }
          }
          
          /// Look up Declarations
//           DUContext::SearchFlags flags = (num != (identifier.count()-1)) ? BaseContext::OnlyContainerTypes : BaseContext::NoSearchFlags;
//           flags |= m_flags;

          QList<Declaration*> tempDecls;
          if( !scopeContext ) {
            m_context->findDeclarationsInternal( toList(lookup), m_position, m_dataType, tempDecls, m_trace, m_flags | DUContext::DirectQualifiedLookup );
            if( tempDecls.isEmpty() ) {
              ///If we have a m_trace, walk the m_trace up so we're able to find the item in earlier imported contexts.
              //To simulate a search starting at searchContext->scopIdentifier, we must search the identifier with all partial scopes prepended
              QList<QualifiedIdentifier> allIdentifiers;
              QualifiedIdentifier prepend = m_context->scopeIdentifier(true);
              while( !prepend.isEmpty() ) {
                allIdentifiers << prepend + lookup;
                prepend.pop();
              }
              allIdentifiers << lookup;

              for( int a = m_trace.count()-1; a >= 0; --a ) {
                const ImportTraceItem& traceItem(m_trace[a]);
                QList<Declaration*> decls;
                ///@todo Give a correctly modified m_trace(without the used items)
                traceItem.ctx->findDeclarationsInternal( allIdentifiers, traceItem.position.isValid() ? traceItem.position : traceItem.ctx->range().end, AbstractType::Ptr(), decls, m_trace.mid(0,a), KDevelop::DUContext::NoUndefinedTemplateParams );
                if( !decls.isEmpty() ) {
                  tempDecls = decls;
                  break;
                }
              }
            }
          } else {
            scopeContext->findDeclarationsInternal( toList(lookup), scopeContext->url() == m_context->url() ? m_position : scopeContext->range().end, m_dataType, tempDecls, m_trace, m_flags | DUContext::DontSearchInParent | DUContext::DirectQualifiedLookup );
          }

          if( !tempDecls.isEmpty() ) {
            //We need to build the inclusion-m_trace here, so template-parameters can be resolved nicely
            TopDUContext* previousTopContext = scopeContext ? scopeContext->topContext() : m_context->topContext();
            
            if( previousTopContext != tempDecls.last()->topContext() ) {
              /*The top-context has changed, so we need to append the inclusion-path
                from previousTopContext to tempDecls.last()->topContext() to newTrace */
              ///@todo which item from tempDecls to choose here?
              m_trace += previousTopContext->importTrace(tempDecls.last()->topContext());
            }
            
            s.result.clear();
            //instantiate template declarations
            foreach(Declaration* decl, tempDecls) {
              if( s.templateParameters.isEmpty() ) {
                s.result << DeclarationPointer(decl);
              }else{
                Declaration* dec = instantiateDeclaration(decl, s.templateParameters);
                if( dec )
                  s.result << DeclarationPointer(dec);
              }
            }

            s.templateParameters.clear();


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
            s.templateParameters.clear();
            //Nothing was found 
              //This is ok in the case that currentLookup stands for a namespace, because namespaces do not have a declaration.
              if( s.templateParameters.count() != 0 ) {
                kDebug(9007) << "CppDUContext::findDeclarationsInternal: Template in scope could not be located: " << lookup.toString();
                return false; //If one of the parts has a template-identifier, it cannot be a namespace
              }
          }

          m_lastDeclarations = s.result;
          
          return true;
        }

        /**
         * Returns the Declarations found for the last closed qualified identifier.
         * 
         * */
        QList<DeclarationPointer> lastDeclarations() const {
          return m_lastDeclarations;
        }

        const ImportTrace& trace() const {
          return m_trace;
        }
      private:

        TopDUContext* topContext() const {
          if( !m_trace.isEmpty() )
            return m_trace.front().ctx->topContext();
          else
            return m_context->topContext();
        }
        
        QList<QualifiedIdentifier> toList( const QualifiedIdentifier& id ) const {
          QList<QualifiedIdentifier> ret;
          ret << id;
          return ret;
        }

        Declaration* instantiateDeclaration( Declaration* decl, const QList<Cpp::ExpressionEvaluationResult>& templateArguments ) const
        {
          if( templateArguments.isEmpty() )
            return decl;
          
          TemplateDeclaration* templateDecl = dynamic_cast<TemplateDeclaration*>(decl);
          if( !templateDecl ) {
            ifDebug( kDebug(9007) << "Tried to instantiate a non-template declaration" << decl->toString(); )
            return 0;
          }

          return templateDecl->instantiate( templateArguments, m_trace );
        }
        
        struct State {
          State() {
          }
          QualifiedIdentifier identifier; //identifier including eventual namespace prefix
          QList<ExpressionEvaluationResult> templateParameters;

          ///One of the following is filled
          QList<DeclarationPointer> result;
          ExpressionEvaluationResult expressionResult;
        };
        QStack<State> m_states;
        const DUContext* m_context;
        ImportTrace m_trace;
        DUContext::SearchFlags m_flags;
        QList<DeclarationPointer> m_lastDeclarations;
        SimpleCursor m_position;
        AbstractType::Ptr m_dataType;
    };

/**
 * This is a du-context template that wraps the c++-specific logic around existing DUContext-derived classes.
 * In practice this means DUContext and TopDUContext.
 * */
template<class BaseContext>
class CppDUContext : public BaseContext {
  public:
    ///Parameters will be reached to the base-class
    template<class Param1, class Param2, class Param3>
    CppDUContext( const Param3& document, const Param1& p1, const Param2& p2, bool isInstantiationContext ) : BaseContext(document, p1, p2, isInstantiationContext), m_instantiatedFrom(0) {
    }

    ///Both parameters will be reached to the base-class. This fits TopDUContext.
    template<class Param1, class Param2, class Param3>
    CppDUContext( const Param3& document, const Param1& p1, const Param2& p2) : BaseContext(document, p1, p2), m_instantiatedFrom(0) {
    }

    ~CppDUContext() {
      //Specializations will be destroyed the same time this is destroyed
      QSet<CppDUContext<BaseContext>*> instatiations;
      {
        QMutexLocker l(&cppDuContextInstantiationsMutex);
        instatiations = m_instatiations;
      }
      foreach( CppDUContext<BaseContext>* instatiation, instatiations )
        delete instatiation;
    }
    
    virtual bool findDeclarationsInternal(const QList<KDevelop::QualifiedIdentifier>& identifiers, const SimpleCursor& position, const AbstractType::Ptr& dataType, QList<KDevelop::Declaration*>& ret, const ImportTrace& trace, typename BaseContext::SearchFlags basicFlags ) const
    {
      if( basicFlags & BaseContext::DirectQualifiedLookup ) {
        //ifDebug( kDebug(9007) << "redirecting findDeclarationsInternal in " << this << "(" << this->scopeIdentifier() <<") for \"" << identifier.toString() << "\""; )
        //We use DirectQualifiedLookup to signalize that we don't need to do the whole scope-search, template-resolution etc. logic.
        return BaseContext::findDeclarationsInternal(identifiers, position, dataType, ret, trace, basicFlags );
      }
      
      for( QList<QualifiedIdentifier>::const_iterator it = identifiers.begin(); it != identifiers.end(); it++ )
        if( !findDeclarationsInternal(*it, position, dataType, ret, trace, basicFlags) )
          return false;

      //Remove all foward-declarations if there is a real declaration in the list

      bool haveForwardDeclaration = false;
      bool haveNonForwardDeclaration = false;
      
      QList<KDevelop::Declaration*>::iterator it = ret.begin();
      QList<KDevelop::Declaration*>::iterator end = ret.end();
      for( ; it != end; ++it)
      {
        if((*it)->isForwardDeclaration())
          haveForwardDeclaration = true;
        else
          haveNonForwardDeclaration = true;
      }

      if(haveForwardDeclaration && haveNonForwardDeclaration) {
        QList<KDevelop::Declaration*> oldRet = ret;
        ret.clear();
        foreach(KDevelop::Declaration* decl, oldRet)
          if(!decl->isForwardDeclaration())
            ret << decl;
      }
      return true;
    }

    ///Overridden to take care of templates and other c++ specific things
    bool findDeclarationsInternal(const QualifiedIdentifier& identifier, const SimpleCursor& position, const AbstractType::Ptr& dataType, QList<Declaration*>& ret, const ImportTrace& trace, typename BaseContext::SearchFlags basicFlags ) const
    {
      ifDebug( kDebug(9007) << "findDeclarationsInternal in " << this << "(" << this->scopeIdentifier() <<") for \"" << identifier.toString() << "\""; )

      FindDeclaration find( this, trace, basicFlags, position, dataType );

      find.openQualifiedIdentifier( identifier.explicitlyGlobal() );

      for( int num = 0; num < identifier.count(); num++ )
      {
        {
          Identifier current = identifier.at(num);
          current.clearTemplateIdentifiers();
          find.openIdentifier(current);
        }
        
        {
          Identifier currentIdentifier = identifier.at(num);

          ///Step 1: Resolve the template-arguments
          //Since there may be non-type template-parameters, represent them as ExpressionEvaluationResult's

          for( int a = 0; a < currentIdentifier.templateIdentifiers().size(); a++ ) {
            //Use the already available mechanism for resolving delayed types
            Cpp::ExpressionEvaluationResult res;
            QualifiedIdentifier i = currentIdentifier.templateIdentifiers().at(a);
            //If the identifier is empty, it is probably just a mark that a template should be instantiated, but without explicit paremeters.
            if( !i.isEmpty() ) {
              DelayedType::Ptr delayed( new DelayedType() );
              delayed->setQualifiedIdentifier( currentIdentifier.templateIdentifiers().at(a) );
              
              res.type = Cpp::resolveDelayedTypes( AbstractType::Ptr( delayed.data() ), this, trace, basicFlags & KDevelop::DUContext::NoUndefinedTemplateParams ? DUContext::NoUndefinedTemplateParams : DUContext::NoSearchFlags );
              
              if( (basicFlags & KDevelop::DUContext::NoUndefinedTemplateParams) && (dynamic_cast<CppTemplateParameterType*>(res.type.data()) || dynamic_cast<DelayedType*>(res.type.data())) ) {
                return false;
              }

              if( !res.isValid() )
                kDebug(9007) << "Could not resolve template-parameter \"" << currentIdentifier.templateIdentifiers().at(a).toString() << "\" in \"" << identifier.toString() << "resolved:" << res.toString();
            }
          
            find.openQualifiedIdentifier( res );
            find.closeQualifiedIdentifier();
          }
        }

        if( !find.closeIdentifier() )
          return false;
      }
      find.closeQualifiedIdentifier();
      
      foreach( const DeclarationPointer& decl, find.lastDeclarations() )
        ret << decl.data();
      
      return true;
    }

    virtual void findLocalDeclarationsInternal( const QualifiedIdentifier& identifier, const SimpleCursor & position, const AbstractType::Ptr& dataType, bool allowUnqualifiedMatch, QList<Declaration*>& ret, const ImportTrace& trace, typename BaseContext::SearchFlags flags ) const
    {
      ifDebug( kDebug(9007) << "findLocalDeclarationsInternal in " << this << "with parent" << this->parentContext() << "(" << this->scopeIdentifier() <<") for \"" << identifier.toString() << "\""; )
      ifDebug( if( BaseContext::owner() && BaseContext::owner()->asDeclaration() ) kDebug(9007) << "in declaration: " << "(" << BaseContext::owner()->asDeclaration()->toString(); )
      /**
        - When searching local declarations:
         - Check whether they are already in the instantiated context, if yes return them
         - If not, Search in non-instantiated context(only for local declarations), then:
           - Copy & Change returned objects:
             - Resolve virtual types (DelayedType)
             - Change parent-context to virtual context
             - Change internal context, (create virtual, move set parent)
      * */

        int retCount = ret.count();
      
        BaseContext::findLocalDeclarationsInternal(identifier, position, dataType, allowUnqualifiedMatch, ret, trace, flags );

        ifDebug( kDebug(9007) << "basically found:" << ret.count() - retCount << "containing" << BaseContext::localDeclarations().count() << "searching-position" << position; )
        
        if( !(flags & DUContext::NoFiltering) ) {
          //Filter out constructors and if needed unresolved template-params
          QList<Declaration*>::iterator it = ret.begin();
          while( it != ret.end() ) {
            if( ( (flags & KDevelop::DUContext::NoUndefinedTemplateParams) && dynamic_cast<const CppTemplateParameterType*>((*it)->abstractType().data()) )
                || ( (!(flags & BaseContext::OnlyFunctions)) &&  (dynamic_cast<ClassFunctionDeclaration*>(*it) && static_cast<ClassFunctionDeclaration*>(*it)->isConstructor() ) ) ) { //Maybe this filtering should be done in the du-chain?
              it = ret.erase(it);
              //kDebug(9007) << "filtered out 1 declaration";
            } else {
              ++it;
            }
          }
        }

        ifDebug( if( BaseContext::owner() && BaseContext::owner()->asDeclaration() ) kDebug(9007) << "in declaration: " << "(" << BaseContext::owner()->asDeclaration()->toString(); )
        ifDebug( kDebug(9007) << "instantiated from:" << m_instantiatedFrom; )
        
        if( m_instantiatedFrom && ret.count() == retCount ) {
          ///Search in the context this one was instantiated from
          QList<Declaration*> decls;
          ifDebug( kDebug(9007) << "searching base"; )
          m_instantiatedFrom->findLocalDeclarationsInternal( identifier, position, dataType, allowUnqualifiedMatch, decls, trace, flags );
          
          ifDebug( if( BaseContext::owner() && BaseContext::owner()->asDeclaration() ) kDebug(9007) << "in declaration: " << "(" << BaseContext::owner()->asDeclaration()->toString(); )
          ifDebug( kDebug(9007) << "found" << decls.count() << "in base"; )
          foreach( Declaration* decl, decls ) {
            Declaration* copy = decl->clone();
#ifdef DEBUG
            if(decl->internalContext())
              kDebug(9007) << "declaration" << decl->toString() << "has internal context";
#endif
            
            instantiateDeclarationContext( const_cast<CppDUContext*>(this), trace, decl->internalContext(), QList<Cpp::ExpressionEvaluationResult>(), copy, decl );

            ///instantiateDeclarationContext moved the declaration into this context anonymously, but we want to be able to find it
            copy->setContext(const_cast<CppDUContext*>(this));

            ifDebug( kDebug(9007) << "returning instantiation" << copy->toString(); )
            
            ret << copy;
          }
        }
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

    /**
     * Set the context which this is instantiated from. This context will be strictly attached to that context, and will be deleted once the other is deleted.
     * */
    void setInstantiatedFrom( CppDUContext<BaseContext>* context, const QList<ExpressionEvaluationResult>& templateArguments )
    {
      //Change the identifier so it contains the template-parameters
      QualifiedIdentifier totalId = this->localScopeIdentifier();
      KDevelop::Identifier id;
      if( !totalId.isEmpty() ) {
        id = totalId.last();
        totalId.pop();
      }
      
      id.clearTemplateIdentifiers();
      for( QList<ExpressionEvaluationResult>::const_iterator it = templateArguments.begin(); it != templateArguments.end(); it++ )
        id.appendTemplateIdentifier( QualifiedIdentifier(it->toShortString()) );

      totalId.push(id);
      
      this->setLocalScopeIdentifier(totalId);
      
      QMutexLocker l(&cppDuContextInstantiationsMutex);
      
      if( m_instantiatedFrom )
        m_instantiatedFrom->m_instatiations.remove( this );
      m_instantiatedFrom = context;
      m_instantiatedFrom->m_instatiations.insert( this );
    }

    /**
     * If this returns nonzero value, this context is a instatiation of some other context, and that other context will be returned here.
     * */
    CppDUContext<BaseContext>* instantiatedFrom() const {
      return m_instantiatedFrom;
    }

    virtual bool inDUChain() const {
      return m_instantiatedFrom || BaseContext::inDUChain();
    }

    virtual QWidget* createNavigationWidget(Declaration* decl, TopDUContext* topContext, const QString& htmlPrefix, const QString& htmlSuffix) const;
    
  private:

    virtual void mergeDeclarationsInternal(QList< QPair<Declaration*, int> >& definitions, const SimpleCursor& position, QHash<const DUContext*, bool>& hadContexts, const ImportTrace& trace,  bool searchInParents, int currentDepth) const
    {
      if( m_instantiatedFrom )
      {
        //We need to make sure that all declarations from the specialization-base are instantiated, so they are returned.
        QList<Declaration*> baseDecls = m_instantiatedFrom->localDeclarations();
        QList<Declaration*> temp;

        //This requests all declarations, so they all will be instantiated and instances of them added into this context.
        //DUContext::mergeDeclarationsInternal will then get them.
        
        QList<Declaration*> decls = m_instantiatedFrom->localDeclarations();

        foreach( Declaration* baseDecls, decls )
          this->findLocalDeclarationsInternal( QualifiedIdentifier(baseDecls->identifier()), SimpleCursor::invalid(), AbstractType::Ptr(), true, temp, trace, DUContext::NoFiltering );
      }

      return BaseContext::mergeDeclarationsInternal(definitions, position, hadContexts, trace, searchInParents, currentDepth);
    }

    CppDUContext<BaseContext>* m_instantiatedFrom;

    ///Every access to m_instatiations must be serialized through instatiationsMutex, because they may be written without a write-lock
    QSet<CppDUContext<BaseContext>* > m_instatiations;
};

}

#endif
