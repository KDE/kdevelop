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
#include "cppduchain.h"
#include "templatedeclaration.h"
#include "expressionparser.h"
#include "cppdebughelper.h"

using namespace KDevelop;

namespace Cpp {
extern QMutex cppDuContextInstantiationsMutex;

    ///This class breaks up the logic of searching a declaration in C++, so QualifiedIdentifiers as well as AST-based lookup mechanisms can be used for searching
    class FindDeclaration {
      public:
        FindDeclaration( const DUContext* ctx, const TopDUContext* source, DUContext::SearchFlags flags, const SimpleCursor& position, AbstractType::Ptr dataType = AbstractType::Ptr() ) : m_context(ctx), m_source(source), m_flags(flags), m_position(position), m_dataType(dataType) {
          Q_ASSERT(m_source);
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
          s.result.clear();
          FOREACH_FUNCTION(const DeclarationId& decl, result.allDeclarations)
          s.result << DeclarationPointer( decl.getDeclaration(const_cast<TopDUContext*>(topContext())) );
          
          m_states << s;
        }

        /**
         * After this was called, lastDeclarations(..) can be used to retrieve declarations of the qualified identifier.
         * The DUChain needs to be locked when this is called.
         * */
        void closeQualifiedIdentifier();
        /**
         * The identifier must not have template identifiers, those need to be added using openQualifiedIdentifier(..) and closeQualifiedIdentifier(..)
         * */
        void openIdentifier( const Identifier& identifier ) {
         m_states.top().identifier.push(identifier);
        }
        /**
         * When closeIdentifier() is called, the last opened identifier is searched, and can be retrieved using lastDeclarations().
         * Returns false when the search should be stopped.
         * The DUChain needs to be locked when this is called.
         * @param isFinalIdentifier Whether the closed identifier the last one. Needed to know when to apply what search-flags.
         * */
        bool closeIdentifier(bool isFinalIdentifier);

        ///For debugging. Describes the last search context.
        QString describeLastContext() const {
          if( m_lastScopeContext ) {
            return "Context " + m_lastScopeContext->scopeIdentifier(true).toString() + QString(" from %1:%2").arg(m_lastScopeContext->url().str()).arg(m_lastScopeContext->range().start.line);
          } else {
            return QString("Global search with top-context %2").arg(topContext()->url().str());
          }
        }

        /**
         * Returns the Declarations found for the last closed qualified identifier.
         * 
         * */
        QList<DeclarationPointer> lastDeclarations() const {
          return m_lastDeclarations;
        }

        const TopDUContext* topContext() const {
          return m_source;
        }

      private:

        ///Uses the instantiation-information from the context of decl as parent of templateArguments.
        Declaration* instantiateDeclaration( Declaration* decl, const InstantiationInformation& templateArguments ) const;
        
        struct State {
          State() {
          }
          QualifiedIdentifier identifier; //identifier including eventual namespace prefix
          InstantiationInformation templateParameters;

          ///One of the following is filled
          QList<DeclarationPointer> result;
          ExpressionEvaluationResult expressionResult;
        };
        QStack<State> m_states;
        const DUContext* m_context;
        const TopDUContext* m_source;
        DUContext::SearchFlags m_flags;
        QList<DeclarationPointer> m_lastDeclarations;
        SimpleCursor m_position;
        AbstractType::Ptr m_dataType;

        DUContextPointer m_lastScopeContext; //For debugging, last context in which we searched
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
      //Delete all the local declarations first, so they also delete their instantiations
      BaseContext::deleteLocalDeclarations();
      //Specializations will be destroyed the same time this is destroyed
      QSet<CppDUContext<BaseContext>*> instatiations;
      {
        QMutexLocker l(&cppDuContextInstantiationsMutex);
        instatiations = m_instatiations;
      }
      foreach( CppDUContext<BaseContext>* instatiation, instatiations )
        delete instatiation;
    }
    
    ///Overridden to take care of templates and other c++ specific things
    virtual bool findDeclarationsInternal(const DUContext::SearchItem::PtrList& identifiers, const SimpleCursor& position, const AbstractType::Ptr& dataType, DUContext::DeclarationList& ret, const TopDUContext* source, typename BaseContext::SearchFlags basicFlags ) const
    {
      if( basicFlags & BaseContext::DirectQualifiedLookup ) {
        //ifDebug( kDebug(9007) << "redirecting findDeclarationsInternal in " << this << "(" << this->scopeIdentifier() <<") for \"" << identifier.toString() << "\""; )
        //We use DirectQualifiedLookup to signalize that we don't need to do the whole scope-search, template-resolution etc. logic.
        return BaseContext::findDeclarationsInternal(identifiers, position, dataType, ret, source, basicFlags );
      }
      
      FOREACH_ARRAY( const DUContext::SearchItem::Ptr& item, identifiers )
        foreach( const QualifiedIdentifier& id, item->toList() )
          if(!findDeclarationsInternal(id, position, dataType, ret, source, basicFlags))
            return false;

      //Remove all foward-declarations if there is a real declaration in the list

      bool haveForwardDeclaration = false;
      bool haveNonForwardDeclaration = false;
      
      for(int a = 0; a < ret.size(); ++a) {
        if(ret[a]->isForwardDeclaration())
          haveForwardDeclaration = true;
        else
          haveNonForwardDeclaration = true;
      }

      if(haveForwardDeclaration && haveNonForwardDeclaration) {
        DUContext::DeclarationList oldRet = ret;
        ret.clear();
        for(int a = 0; a < oldRet.size(); ++a)
          if(!oldRet[a]->isForwardDeclaration())
            ret.append(oldRet[a]);
      }
      return true;
    }

    bool findDeclarationsInternal(const QualifiedIdentifier& identifier, const SimpleCursor& position, const AbstractType::Ptr& dataType, DUContext::DeclarationList& ret, const TopDUContext* source, typename BaseContext::SearchFlags basicFlags ) const
    {
      ifDebug( kDebug(9007) << "findDeclarationsInternal in " << this << "(" << this->scopeIdentifier() <<") for \"" << identifier.toString() << "\""; )

      FindDeclaration find( this, source, basicFlags, position, dataType );

      find.openQualifiedIdentifier( identifier.explicitlyGlobal() );

      int idCount = identifier.count();
      for( int num = 0; num < idCount; num++ )
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

          int tempCount = currentIdentifier.templateIdentifiersCount();
          for( int a = 0; a < tempCount; a++ ) {
            //Use the already available mechanism for resolving delayed types
            Cpp::ExpressionEvaluationResult res;
            TypeIdentifier i = currentIdentifier.templateIdentifier(a);
            //If the identifier is empty, it is probably just a mark that a template should be instantiated, but without explicit paremeters.
            if( !i.isEmpty() ) {
              DelayedType::Ptr delayed( new DelayedType() );
              delayed->setIdentifier( i );
              
              res.type = Cpp::resolveDelayedTypes( AbstractType::Ptr( delayed.data() ), this, source, basicFlags & KDevelop::DUContext::NoUndefinedTemplateParams ? DUContext::NoUndefinedTemplateParams : DUContext::NoSearchFlags )->indexed();
              
              if( (basicFlags & KDevelop::DUContext::NoUndefinedTemplateParams) && (dynamic_cast<CppTemplateParameterType*>(TypeUtils::targetType(res.type.type().data(), 0)) || dynamic_cast<DelayedType*>(TypeUtils::targetType(res.type.type().data(), 0))) ) {
                return false;
              }

              ifDebug( if( !res.isValid() ) kDebug(9007) << "Could not resolve template-parameter \"" << currentIdentifier.templateIdentifiers().at(a).toString() << "\" in \"" << identifier.toString() << "resolved:" << res.toString(); )
            }
          
            find.openQualifiedIdentifier( res );
            find.closeQualifiedIdentifier();
          }
        }

        if( !find.closeIdentifier( num == idCount-1 ) )
          return false;
      }
      find.closeQualifiedIdentifier();
      
      foreach( const DeclarationPointer& decl, find.lastDeclarations() )
        ret.append(decl.data());
      
      return true;
    }

    virtual void findLocalDeclarationsInternal( const Identifier& identifier, const SimpleCursor & position, const AbstractType::Ptr& dataType, DUContext::DeclarationList& ret, const TopDUContext* source, typename BaseContext::SearchFlags flags ) const
    {
      ifDebug( kDebug(9007) << "findLocalDeclarationsInternal in " << this << "with parent" << this->parentContext() << "(" << this->scopeIdentifier() <<") for \"" << identifier.toString() << "\""; )
      ifDebug( if( BaseContext::owner() && BaseContext::owner() ) kDebug(9007) << "in declaration: " << "(" << BaseContext::owner()->toString(); )
      /**
        - When searching local declarations:
         - Check whether they are already in the instantiated context, if yes return them
         - If not, Search in non-instantiated context(only for local declarations), then:
           - Copy & Change returned objects:
             - Resolve virtual types (DelayedType)
             - Change parent-context to virtual context
             - Change internal context, (create virtual, move set parent)
      * */

        int retCount = ret.size();
      
        BaseContext::findLocalDeclarationsInternal(identifier, position, dataType, ret, source, flags );

        ifDebug( kDebug(9007) << "basically found:" << ret.count() - retCount << "containing" << BaseContext::localDeclarations().count() << "searching-position" << position.textCursor(); )

        if( !(flags & DUContext::NoFiltering) ) {
          //Filter out constructors and if needed unresolved template-params
          for(int a = 0; a < ret.size(); ) {
            if( ( (flags & KDevelop::DUContext::NoUndefinedTemplateParams) && dynamic_cast<const CppTemplateParameterType*>((ret[a])->abstractType().data()) )
                || ( (!(flags & BaseContext::OnlyFunctions)) &&  (dynamic_cast<ClassFunctionDeclaration*>(ret[a]) && static_cast<ClassFunctionDeclaration*>(ret[a])->isConstructor() ) ) ) { //Maybe this filtering should be done in the du-chain?
                
              //Erase the item
              for(int b = a+1; b < ret.size(); ++b)
                ret[b-1] = ret[b];
              ret.resize(ret.size()-1);
              //kDebug(9007) << "filtered out 1 declaration";
            } else {
              ++a;
            }
          }
        }

        ifDebug( if( BaseContext::owner() && BaseContext::owner() ) kDebug(9007) << "in declaration: " << "(" << BaseContext::owner()->toString(); )
        ifDebug( kDebug(9007) << "instantiated from:" << m_instantiatedFrom; )
        
        if( m_instantiatedFrom && ret.size() == retCount ) {
          ///Search in the context this one was instantiated from
          DUContext::DeclarationList decls;
          ifDebug( kDebug(9007) << "searching base"; )
          m_instantiatedFrom->findLocalDeclarationsInternal( identifier, position, dataType, decls, source, flags );
          
          ifDebug( if( BaseContext::owner() && BaseContext::owner() ) kDebug(9007) << "in declaration: " << "(" << BaseContext::owner()->toString(); )
          ifDebug( kDebug(9007) << "found" << decls.count() << "in base"; )
          
          InstantiationInformation memberInstantiationInformation;
          memberInstantiationInformation.previousInstantiationInformation = m_instantiatedWith.index();
          
          FOREACH_ARRAY( Declaration* decl, decls ) {
            TemplateDeclaration* templateDecl = dynamic_cast<TemplateDeclaration*>(decl);
            if(!templateDecl) {
              kDebug() << "problem";
            } else {
              Declaration* copy;
              if(decl->context() != m_instantiatedFrom) {
                //The declaration has been propagated up from a sub-context like an enumerator, add more empty instantiation informations
                InstantiationInformation i;
                i.previousInstantiationInformation = memberInstantiationInformation.indexed().index(); //Currently we don't propagate higher then 1 level
                copy = templateDecl->instantiate(i, source, true);
              }else{
                copy = templateDecl->instantiate(memberInstantiationInformation, source);
              }
              ret.append(copy);
            }
          }
        }
    }
    
    virtual bool foundEnough( const DUContext::DeclarationList& decls ) const
    {
      if( decls.isEmpty() )
        return false;

      if( decls[0]->isForwardDeclaration() )
        return false; //All forward-declarations must be collected(for implementation-reasons)

      if( dynamic_cast<const KDevelop::AbstractFunctionDeclaration*>(decls[0]) && BaseContext::type() != DUContext::Class ) //In classes, one function hides all overloads
        return false; //Collect overloaded function-declarations

      return true;
    }

    /**
     * Set the context which this is instantiated from. This context will be strictly attached to that context, and will be deleted once the other is deleted.
     * */
    void setInstantiatedFrom( CppDUContext<BaseContext>* context, const InstantiationInformation& templateArguments )
    {
      if(context->m_instantiatedFrom) {
        
        setInstantiatedFrom(context->m_instantiatedFrom, templateArguments);
      }
      m_instantiatedWith = templateArguments.indexed();
      //Change the identifier so it contains the template-parameters
      QualifiedIdentifier totalId = this->localScopeIdentifier();
      KDevelop::Identifier id;
      if( !totalId.isEmpty() ) {
        id = totalId.last();
        totalId.pop();
      }
      
      id.clearTemplateIdentifiers();
      FOREACH_FUNCTION(IndexedExpressionEvaluationResult arg, templateArguments.templateParameters)
        id.appendTemplateIdentifier( arg.result().identifier() );

      totalId.push(id);
      
      this->setLocalScopeIdentifier(totalId);
      
      QMutexLocker l(&cppDuContextInstantiationsMutex);
      
      if( m_instantiatedFrom )
        m_instantiatedFrom->m_instatiations.remove( this );
      m_instantiatedFrom = context;
      m_instantiatedFrom->m_instatiations.insert( this );
    }

    virtual void applyUpwardsAliases(DUContext::SearchItem::PtrList& identifiers) const
    {
      BaseContext::applyUpwardsAliases(identifiers);
      ///@see Iso C++ 3.4.1 : Unqualified name lookup: 
      ///We need to make sure that when leaving a function definition, the namespace components are searched
      DUContext::ContextType type = BaseContext::type();
      
      if(type == DUContext::Function || type == DUContext::Other || type == DUContext::Helper)
      {
        QualifiedIdentifier prefix = BaseContext::localScopeIdentifier();
        if(prefix.count() > 1) {
          //This must be a function-definition, like void A::B::test() {}
          KDevelop::DUContext* classContext  = 0;
          
          if(type == DUContext::Helper) {
            if(!BaseContext::importedParentContexts().isEmpty())
              classContext = BaseContext::importedParentContexts()[0].data();
          } else {
            Declaration* classDeclaration = Cpp::localClassFromCodeContext(const_cast<BaseContext*>((const BaseContext*)this));
            if(classDeclaration && classDeclaration->internalContext())
              classContext = classDeclaration->internalContext();
          }
          
          if(classContext) {
            //If this is a definition of a class member, only add aliases for the namespace elements(The class scope will be
            //searched using then normal import logic)
            prefix = classContext->scopeIdentifier(false);
          }else{
            prefix.pop();
          }
          
          if(!prefix.isEmpty()) {
            prefix.setExplicitlyGlobal(true);
            
            DUContext::SearchItem::Ptr newItem(new DUContext::SearchItem(prefix));
            newItem->addToEachNode(identifiers);
            
            if(!newItem->next.isEmpty()) //Can happen if the identifier was explicitly global
              insertToArray(identifiers, newItem, 0);
          }
        }
      }
    }

    /**
     * If this returns nonzero value, this context is a instatiation of some other context, and that other context will be returned here.
     * */
    CppDUContext<BaseContext>* instantiatedFrom() const {
      return m_instantiatedFrom;
    }

    virtual bool inDUChain() const {
      ///There must be no changes from the moment m_instantiatedFrom is set, because then it can be found as an instantiation by other threads
      return m_instantiatedFrom || BaseContext::inDUChain();
    }
    
    IndexedInstantiationInformation instantiatedWith() const {
      return m_instantiatedWith;
    }
    
    virtual bool shouldSearchInParent(typename BaseContext::SearchFlags flags) const {
      //If the parent context is a class context, we should even search it from an import
      return !(flags & DUContext::InImportedParentContext) || (BaseContext::parentContext() && BaseContext::parentContext()->type() == DUContext::Class);
    }
    
    virtual QWidget* createNavigationWidget(Declaration* decl, TopDUContext* topContext, const QString& htmlPrefix, const QString& htmlSuffix) const;
    
  private:

    virtual void mergeDeclarationsInternal(QList< QPair<Declaration*, int> >& definitions, const SimpleCursor& position, QHash<const DUContext*, bool>& hadContexts, const TopDUContext* source,  bool searchInParents, int currentDepth) const
    {
      if( m_instantiatedFrom )
      {
        //We need to make sure that all declarations from the specialization-base are instantiated, so they are returned.
        QVector<Declaration*> baseDecls = m_instantiatedFrom->localDeclarations();
        DUContext::DeclarationList temp;

        //This requests all declarations, so they all will be instantiated and instances of them added into this context.
        //DUContext::mergeDeclarationsInternal will then get them.
        
        QVector<Declaration*> decls = m_instantiatedFrom->localDeclarations();

        foreach( Declaration* baseDecls, decls )
          this->findLocalDeclarationsInternal( baseDecls->identifier(), SimpleCursor::invalid(), AbstractType::Ptr(), temp, source, DUContext::NoFiltering );
      }

      return BaseContext::mergeDeclarationsInternal(definitions, position, hadContexts, source, searchInParents, currentDepth);
    }

    CppDUContext<BaseContext>* m_instantiatedFrom;

    ///Every access to m_instatiations must be serialized through instatiationsMutex, because they may be written without a write-lock
    QSet<CppDUContext<BaseContext>* > m_instatiations;
    IndexedInstantiationInformation m_instantiatedWith;
};

}

#endif
