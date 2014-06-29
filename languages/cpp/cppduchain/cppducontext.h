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

#include <language/duchain/ducontext.h>

#include <QMutex>

#include <language/duchain/abstractfunctiondeclaration.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/classfunctiondeclaration.h>
#include <language/duchain/namespacealiasdeclaration.h>
#include "typeutils.h"
#include "templatedeclaration.h"
#include "expressionevaluationresult.h"
#include "cppduchain.h"
#include "cpptypes.h"
#include "templatedeclaration.h"
#include "cppdebughelper.h"

using namespace KDevelop;

namespace Cpp {
extern QMutex cppDuContextInstantiationsMutex;

    ///This class breaks up the logic of searching a declaration in C++, so QualifiedIdentifiers as well as AST-based lookup mechanisms can be used for searching
    class FindDeclaration {
      public:
        FindDeclaration( const DUContext* ctx, const TopDUContext* source, DUContext::SearchFlags flags, const CursorInRevision& position, AbstractType::Ptr dataType = AbstractType::Ptr() ) : m_context(ctx), m_source(source), m_flags(flags), m_position(position), m_dataType(dataType) {
          Q_ASSERT(m_source);
        }
        void openQualifiedIdentifier( bool isExplicitlyGlobal ) {
          StatePtr s(new State);
          s->identifier.setExplicitlyGlobal( isExplicitlyGlobal );
          m_states << s;
        }

        ///Can be used to just append a result that was computed outside. closeQualifiedIdentifier(...) must still be called.
        void openQualifiedIdentifier( const ExpressionEvaluationResult& result ) {
          StatePtr s(new State);
          s->expressionResult = result;
          s->result.clear();
          foreach(const DeclarationId& decl, result.allDeclarations)
          s->result << DeclarationPointer( decl.getDeclaration(const_cast<TopDUContext*>(topContext())) );
          
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
         m_states.top()->identifier.push(identifier);
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
        
        uint currentLength() const {
          return m_states.size();
        }

      private:

        ///Uses the instantiation-information from the context of decl as parent of templateArguments.
        Declaration* instantiateDeclaration( Declaration* decl, const InstantiationInformation& templateArguments ) const;
        
        struct State : public QSharedData {
          State() {
          }
          QualifiedIdentifier identifier; //identifier including eventual namespace prefix
          InstantiationInformation templateParameters;

          ///One of the following is filled
          QList<DeclarationPointer> result;
          ExpressionEvaluationResult expressionResult;
        };
        
        typedef KSharedPtr<State> StatePtr;
        
        QStack<StatePtr> m_states;
        const DUContext* m_context;
        const TopDUContext* m_source;
        DUContext::SearchFlags m_flags;
        QList<DeclarationPointer> m_lastDeclarations;
        CursorInRevision m_position;
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
    template<class Data>
    CppDUContext(Data& data) : BaseContext(data), m_instantiatedFrom(0) {
    }

    ///Parameters will be reached to the base-class
    template<class Param1, class Param2>
    CppDUContext( const Param1& p1, const Param2& p2, bool isInstantiationContext ) : BaseContext(p1, p2, isInstantiationContext), m_instantiatedFrom(0) {
      static_cast<DUChainBase*>(this)->d_func_dynamic()->setClassId(this);
    }

    ///Both parameters will be reached to the base-class. This fits TopDUContext.
    template<class Param1, class Param2, class Param3>
    CppDUContext( const Param1& p1, const Param2& p2, const Param3& p3) : BaseContext(p1, p2, p3), m_instantiatedFrom(0) {
      static_cast<DUChainBase*>(this)->d_func_dynamic()->setClassId(this);
    }
    template<class Param1, class Param2>
    CppDUContext( const Param1& p1, const Param2& p2) : BaseContext(p1, p2), m_instantiatedFrom(0) {
      static_cast<DUChainBase*>(this)->d_func_dynamic()->setClassId(this);
    }
    
    ///Matches the qualified identifier represented by the search item to the tail of the context's scope identfier
    ///Also matches searches without template-parameters to fully instantiated contexts
    ///Returns true if they match
    inline bool matchSearchItem(DUContext::SearchItem::Ptr item, const DUContext* ctx) const {
        DUContext::SearchItem::PtrList items;
        while(1) {
          items << item;
          if(!item->next.isEmpty())
            item = item->next[0];
          else
            break;
        }
        
        while(ctx && !items.isEmpty()) {
          QualifiedIdentifier localId = ctx->localScopeIdentifier();
          
          if(localId.isEmpty())
            return false;
        
          int matchPos = localId.count()-1;
          while(!items.isEmpty() && matchPos >= 0) {
            
            if(items.back()->identifier.templateIdentifiersCount())
              return false; //Don't match when there is template parameters, as that needs other mechanisms
            
            if((!items.back()->identifier.templateIdentifiersCount() && items.back()->identifier.identifier() == localId.at(matchPos).identifier()) ||
                items.back()->identifier == localId.at(matchPos)) {
              --matchPos;
              items.resize(items.size()-1);
            }else{
              return false;
            }
          }
          
          if(items.isEmpty())
            return true;
          
          ctx = ctx->parentContext();
        }
        
        return false;
    }

    ///Overridden to take care of templates and other c++ specific things
    virtual bool findDeclarationsInternal(const DUContext::SearchItem::PtrList& identifiers, const CursorInRevision& position, const AbstractType::Ptr& dataType, DUContext::DeclarationList& ret, const TopDUContext* source, typename BaseContext::SearchFlags basicFlags, uint depth ) const
    {
      if(this->type() == DUContext::Class && identifiers.count() == 1 &&
        !(basicFlags & DUContext::NoSelfLookUp) && !(basicFlags & DUContext::OnlyFunctions) && this->localScopeIdentifier().count() &&
        !identifiers[0]->isExplicitlyGlobal) {

        //Check whether we're searching for just the name of this context's class. If yes, return this context's owner.
        if(matchSearchItem(identifiers[0], this)) {
          
          Declaration* owner = this->owner();
          if(owner) {
            if(basicFlags & DUContext::NoUndefinedTemplateParams) {
              //If no undefined template parameters are allowed, make sure this template has all parameters assigned.
              TemplateDeclaration* templateOwner = dynamic_cast<TemplateDeclaration*>(this->owner());
              if(templateOwner) {
                if(!templateOwner->instantiatedFrom())
                  return false;
                DUContext* templateContext = templateOwner->templateContext(source);
                if(templateContext) {
                  foreach(Declaration* decl, templateContext->localDeclarations()) {
                    if(decl->type<CppTemplateParameterType>()) {
                      return false;
                    }
                  }
                }
              }
            }
            
            ret << this->owner();
            return true;
          }
        }
      }
      
      if( basicFlags & BaseContext::DirectQualifiedLookup ) {
        //ifDebug( kDebug(9007) << "redirecting findDeclarationsInternal in " << this << "(" << this->scopeIdentifier() <<") for \"" << identifier.toString() << "\""; )
        //We use DirectQualifiedLookup to signal that we don't need to do the whole scope-search, template-resolution etc. logic.
        return BaseContext::findDeclarationsInternal(identifiers, position, dataType, ret, source, basicFlags, depth );
      }
      
      FOREACH_ARRAY( const DUContext::SearchItem::Ptr& item, identifiers )
        foreach( const QualifiedIdentifier& id, item->toList() )
          if(!findDeclarationsInternal(id, position, dataType, ret, source, basicFlags))
            return false;

      // Remove all forward-declarations if there is a real declaration in the list

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

    bool findDeclarationsInternal(const QualifiedIdentifier& identifier, const CursorInRevision& position, const AbstractType::Ptr& dataType, DUContext::DeclarationList& ret, const TopDUContext* source, typename BaseContext::SearchFlags basicFlags) const
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
            IndexedTypeIdentifier i = currentIdentifier.templateIdentifier(a);
            //If the identifier is empty, it is probably just a mark that a template should be instantiated, but without explicit paremeters.
            QualifiedIdentifier qid(i.identifier().identifier());
            if( !qid.isEmpty() ) {
              DelayedType::Ptr delayed( new DelayedType() );
              delayed->setIdentifier( i );
              
              res.type = Cpp::resolveDelayedTypes( delayed.cast<AbstractType>(), this, source, basicFlags & KDevelop::DUContext::NoUndefinedTemplateParams ? DUContext::NoUndefinedTemplateParams : DUContext::NoSearchFlags )->indexed();
              
              if( basicFlags & KDevelop::DUContext::NoUndefinedTemplateParams) {
                AbstractType::Ptr targetTypePtr = TypeUtils::unAliasedType(TypeUtils::targetType(res.type.abstractType(), 0));
                if (targetTypePtr.cast<CppTemplateParameterType>() || (targetTypePtr.cast<DelayedType>() && targetTypePtr.cast<DelayedType>()->kind() == DelayedType::Delayed)) {
                  ifDebug( kDebug() << "stopping because the type" << targetTypePtr->toString() << "of" << i.toString() << "is bad"; )
                  return false;
                }
              }

              ifDebug( if( !res.isValid() ) kDebug(9007) << "Could not resolve template-parameter \"" << currentIdentifier.templateIdentifier(a).toString() << "\" in \"" << identifier.toString() << "resolved:" << res.toString(); )
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

    virtual void findLocalDeclarationsInternal( const Identifier& identifier, const CursorInRevision & position, const AbstractType::Ptr& dataType, DUContext::DeclarationList& ret, const TopDUContext* source, typename BaseContext::SearchFlags flags ) const
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

        ifDebug( kDebug(9007) << "basically found:" << ret.count() - retCount << "containing" << BaseContext::localDeclarations().count() << "searching-position" << position.castToSimpleCursor().textCursor(); )

        if( !(flags & DUContext::NoFiltering) ) {
          //Filter out constructors and if needed unresolved template-params
          for(int a = 0; a < ret.size(); ) {
            
            AbstractType::Ptr retAbstractTypePtr = ret[a]->abstractType();
            if( ( (flags & KDevelop::DUContext::NoUndefinedTemplateParams) && retAbstractTypePtr.cast<CppTemplateParameterType>() )
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
          memberInstantiationInformation.previousInstantiationInformation = m_instantiatedWith;
          
          FOREACH_ARRAY( Declaration* decl, decls ) {
            TemplateDeclaration* templateDecl = dynamic_cast<TemplateDeclaration*>(decl);
            if(!templateDecl) {
              kDebug() << "problem";
            } else {
              Declaration* copy;
              
              DUContext* current = decl->context();
              while(current != m_instantiatedFrom && current)
              {
                // The declaration has been propagated up from a sub-context like an enumerator, add more empty instantiation information
                // so the depth is matched correctly by the information
                InstantiationInformation i;
                i.previousInstantiationInformation = memberInstantiationInformation.indexed();
                memberInstantiationInformation = i;
                current = current->parentContext();
              }
              Q_ASSERT(source);
              copy = templateDecl->instantiate(memberInstantiationInformation, source);
              //This can happen in case of explicit specializations
//               if(copy->context() != this)
//                 kWarning() << "serious problem: Instatiation is in wrong context, should be in this one";

              if(copy)
                ret.append(copy);
            }
          }
        }
    }
    
    virtual void visit(DUChainVisitor& visitor) {
      QMutexLocker l(&cppDuContextInstantiationsMutex);
      foreach(CppDUContext<BaseContext>* ctx, m_instatiations)
        ctx->visit(visitor);
      
      BaseContext::visit(visitor);
    }
    
    virtual void deleteUses() {
      QMutexLocker l(&cppDuContextInstantiationsMutex);
      foreach(CppDUContext<BaseContext>* ctx, m_instatiations)
        ctx->deleteUses();
      BaseContext::deleteUses();
    }
    
    virtual bool foundEnough( const DUContext::DeclarationList& decls, DUContext::SearchFlags flags ) const
    {
      if(flags & DUContext::NoFiltering)
        return false;
      
      if( decls.isEmpty() )
        return false;

      if( dynamic_cast<const KDevelop::AbstractFunctionDeclaration*>(decls[0]) && BaseContext::type() != DUContext::Class ) //In classes, one function hides all overloads
        return false; //Collect overloaded function-declarations

      return true;
    }

    /**
     * Set the context which this is instantiated from. This context will be strictly attached to that context, and will be deleted once the other is deleted.
     * */
    void setInstantiatedFrom( CppDUContext<BaseContext>* context, const InstantiationInformation& templateArguments )
    {
      Q_ASSERT(!dynamic_cast<TopDUContext*>(this));
      if(context && context->m_instantiatedFrom) {
        setInstantiatedFrom(context->m_instantiatedFrom, templateArguments);
        return;
      }
      QMutexLocker l(&cppDuContextInstantiationsMutex);
      
      if( m_instantiatedFrom ) {
        Q_ASSERT(m_instantiatedFrom->m_instatiations[m_instantiatedWith] == this);
        m_instantiatedFrom->m_instatiations.remove( m_instantiatedWith );
      }
      
      m_instantiatedWith = templateArguments.indexed();
      if(context) {
        //Change the identifier so it contains the template-parameters
        QualifiedIdentifier totalId = this->localScopeIdentifier();
        KDevelop::Identifier id;
        if( !totalId.isEmpty() ) {
          id = totalId.last();
          totalId.pop();
        }
        
        id.clearTemplateIdentifiers();
        FOREACH_FUNCTION(const IndexedType& arg, templateArguments.templateParameters) {
          AbstractType::Ptr type(arg.abstractType());
          IdentifiedType* identified = dynamic_cast<IdentifiedType*>(type.unsafeData());
          if(identified)
            id.appendTemplateIdentifier( IndexedTypeIdentifier(identified->qualifiedIdentifier()) );
          else if(type)
            id.appendTemplateIdentifier( IndexedTypeIdentifier(type->toString(), true) );
          else
            id.appendTemplateIdentifier( IndexedTypeIdentifier("no type") );
        }

        totalId.push(id);
        
        this->setLocalScopeIdentifier(totalId);
      }
      
      m_instantiatedFrom = context;
      Q_ASSERT(m_instantiatedFrom != this);
      if(m_instantiatedFrom) {
        if(!m_instantiatedFrom->m_instatiations.contains(m_instantiatedWith)) {
          m_instantiatedFrom->m_instatiations.insert( m_instantiatedWith, this );
        }else{
          kDebug(9007) << "created orphaned instantiation for" << context->m_instatiations[m_instantiatedWith]->scopeIdentifier(true).toString();
          m_instantiatedFrom = 0;
        }
      }
    }
    
    virtual void applyUpwardsAliases(DUContext::SearchItem::PtrList& identifiers, const TopDUContext* source) const
    {
      BaseContext::applyUpwardsAliases(identifiers, source);
      ///@see Iso C++ 3.4.1 : Unqualified name lookup: 
      ///We need to make sure that when leaving a function definition, the namespace components are searched
      DUContext::ContextType type = BaseContext::type();
      
      if(type == DUContext::Function || type == DUContext::Other || type == DUContext::Helper)
      {
        QualifiedIdentifier prefix = BaseContext::localScopeIdentifier();
        if(prefix.count() > 1) {
          prefix = Cpp::namespaceScopeComponentFromContext(prefix, this, source);
          
          if(!prefix.isEmpty()) {
            prefix.setExplicitlyGlobal(true);
            
            DUContext::SearchItem::Ptr newItem(new DUContext::SearchItem(prefix));
            newItem->addToEachNode(identifiers);
            
            if(!newItem->next.isEmpty()) //Can happen if the identifier was explicitly global
              identifiers.insert(0, newItem);
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
      return (BaseContext::parentContext() && BaseContext::parentContext()->type() == DUContext::Class) || BaseContext::shouldSearchInParent(flags);
    }

    virtual DUContext* specialize(const IndexedInstantiationInformation& specialization,
                                  const TopDUContext* topContext, int upDistance) {
      if(specialization.index() == 0)
        return this;
      else {
        InstantiationInformation information = specialization.information();
        
        //Add empty elements until the specified depth
        for(int a = 0; a < upDistance; ++a) {
          InstantiationInformation nextInformation;
          nextInformation.previousInstantiationInformation = information.indexed();
          information = nextInformation;
        }
        
        return instantiate(information, topContext);
      }
    }

    ///@see TemplateDeclaration::instantiate
    DUContext* instantiate(const InstantiationInformation& info, const TopDUContext* source) {
      if(!info.isValid() || m_instantiatedWith == info.indexed() || !this->parentContext())
        return this;

      if(m_instantiatedFrom)
        return m_instantiatedFrom->instantiate(info, source);
      
      {
        typename QHash<IndexedInstantiationInformation, CppDUContext<BaseContext>* >::const_iterator it = m_instatiations.constFind(info.indexed());
        if(it != m_instatiations.constEnd())
          return *it;
      }
      
      if(this->owner()) {
        TemplateDeclaration* templ = dynamic_cast<TemplateDeclaration*>(this->owner());
        if(templ) {
          Declaration* instantiatedDecl = templ->instantiate(info, source);
          if(!instantiatedDecl)
            return 0;
          return instantiatedDecl->internalContext();
        }
      }
      
      DUContext* surroundingContext = this->parentContext();
      Q_ASSERT(surroundingContext);
      {
        //This context does not have an attached declaration, but it needs to be instantiated.
        CppDUContext<DUContext>* parent = dynamic_cast<CppDUContext<DUContext>* >(this->parentContext());
        if(parent)
          surroundingContext = parent->instantiate(info.previousInstantiationInformation.information(), source);
      }
      
      if(!surroundingContext)
        return 0;
      
      return instantiateDeclarationAndContext( surroundingContext, source, this, info, 0, 0 );
    }
    
    virtual QWidget* createNavigationWidget(Declaration* decl, TopDUContext* topContext, const QString& htmlPrefix, const QString& htmlSuffix) const;
    
    enum {
      Identity = BaseContext::Identity + 50
    };
    
    ///Duchain must be write-locked
    void deleteAllInstantiations() {
      //Specializations will be destroyed the same time this is destroyed
      CppDUContext<BaseContext>* oldFirst = 0;
      QMutexLocker l(&cppDuContextInstantiationsMutex);
      while(!m_instatiations.isEmpty()) {
        CppDUContext<BaseContext>* first = 0;
        first = *m_instatiations.begin();
        
        Q_ASSERT(first != oldFirst);
        Q_UNUSED(oldFirst);
        oldFirst = first;
        
        l.unlock();
        
        ///TODO: anonymous contexts should get deleted but that is crashy
        ///      see also declarationbuilder which also encountered this
        ///      issue before and also removed the context deletion...
        Q_ASSERT(first->m_instantiatedFrom == this);
        first->setInstantiatedFrom(0, InstantiationInformation());
        Q_ASSERT(first->m_instantiatedFrom == 0);
        
        l.relock();
      }      
    }
    
    //Overridden to instantiate all not yet instantiated local declarations
    virtual QVector<Declaration*> localDeclarations(const TopDUContext* source) const {
      
      if(m_instantiatedFrom && source && BaseContext::type() != DUContext::Template) {
        QVector<Declaration*> decls = m_instantiatedFrom->localDeclarations(source);
//         DUContext::DeclarationList temp;

        InstantiationInformation inf;
        inf.previousInstantiationInformation = m_instantiatedWith;

        foreach( Declaration* baseDecl, decls ) {
          TemplateDeclaration* tempDecl = dynamic_cast<TemplateDeclaration*>(baseDecl);
          if(tempDecl) {
            tempDecl->instantiate(inf, source);
          }else{
            kDebug() << "Strange: non-template within template context";
            KDevVarLengthArray<Declaration*, 40> temp;
            this->findLocalDeclarationsInternal( baseDecl->identifier(), CursorInRevision::invalid(), AbstractType::Ptr(), temp, source, DUContext::NoFiltering );
          }
        }
      }
        
        return BaseContext::localDeclarations(source);
    }
    
  private:
    ~CppDUContext() {

      if(m_instantiatedFrom)
        setInstantiatedFrom(0, InstantiationInformation());
      
      deleteAllInstantiations();
    }

    virtual void mergeDeclarationsInternal(QList< QPair<Declaration*, int> >& definitions, const CursorInRevision& position, QHash<const DUContext*, bool>& hadContexts, const TopDUContext* source,  bool searchInParents, int currentDepth) const
    {
      Q_ASSERT(source);
//       kDebug() << "checking in" << this->scopeIdentifier(true).toString();
      if( m_instantiatedFrom )
      {
        //We need to make sure that all declarations from the specialization-base are instantiated, so they are returned.

        //This requests all declarations, so they all will be instantiated and instances of them added into this context.
        //DUContext::mergeDeclarationsInternal will then get them.
        
        //Calling localDeclarations() instantiates all not yet instantiated member declarations
        localDeclarations(source);
        
//         kDebug() << "final count of local declarations:" << this->localDeclarations().count();
        
        //Instantiate up-propagating child-contexts with the correct same instantiation-information
        //This for examples makes unnamed enums accessible
        InstantiationInformation inf;
        inf.previousInstantiationInformation = m_instantiatedWith;
        
        foreach(DUContext* child, m_instantiatedFrom->childContexts()) {
//           kDebug() << "checking child-context" << child->isPropagateDeclarations();
          
          if(child->isPropagateDeclarations())
            static_cast<CppDUContext<BaseContext>*>(static_cast<CppDUContext<BaseContext>*>(child)->instantiate(inf, source))->mergeDeclarationsInternal(definitions, position, hadContexts, source, searchInParents, currentDepth);
        }
      }

      BaseContext::mergeDeclarationsInternal(definitions, position, hadContexts, source, searchInParents, currentDepth);
    }

    CppDUContext<BaseContext>* m_instantiatedFrom;

    ///Every access to m_instatiations must be serialized through instatiationsMutex, because they may be written without a write-lock
    QHash<IndexedInstantiationInformation, CppDUContext<BaseContext>* > m_instatiations;
    IndexedInstantiationInformation m_instantiatedWith;
};

///Returns whether the given declaration depends on a missing template-parameter
bool isTemplateDependent(const Declaration* decl);
bool isTemplateDependent(const DUContext* context);

}

#endif
