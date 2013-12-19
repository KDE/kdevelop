/* This file is part of KDevelop
    Copyright 2007-2009 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef TEMPLATEDECLARATION_H
#define TEMPLATEDECLARATION_H

#include <QMutex>

#include <language/duchain/forwarddeclaration.h>
#include <language/duchain/duchainbase.h>
#include <language/duchain/duchainpointer.h>
#include <language/duchain/ducontext.h>
#include "cppduchainexport.h"
#include <language/duchain/instantiationinformation.h>

using namespace KDevelop;

namespace KTextEditor {
  class Range;
}
namespace  KDevelop {
  class DUContext;
  class Declaration;
  class DeclarationId;
  class DelayedType;
  class TopDUContext;
}

namespace Cpp {
  using KDevelop::InstantiationInformation;
  using KDevelop::IndexedInstantiationInformation;
  template<class Base>
  class CppDUContext;
  
  struct KDEVCPPDUCHAIN_EXPORT TemplateDeclarationData {
    TemplateDeclarationData() {
    }
    TemplateDeclarationData(const TemplateDeclarationData& rhs) : m_parameterContext(rhs.m_parameterContext) {
    }
    //The context in which the template-parameters are declared
    KDevelop::IndexedDUContext m_parameterContext;
  };
  
  //Represents the template-part of a template-class'es or template-function's template-part
  class KDEVCPPDUCHAIN_EXPORT TemplateDeclaration {
    public:
      typedef QHash<IndexedInstantiationInformation, TemplateDeclaration*> InstantiationsHash;
      
      ///Copy-constructor for cloning
      TemplateDeclaration(const TemplateDeclaration& rhs);
      TemplateDeclaration();
      virtual ~TemplateDeclaration();

      void setTemplateParameterContext(KDevelop::DUContext* context);
      KDevelop::DUContext* templateParameterContext() const;

      TemplateDeclaration* instantiatedFrom() const;

      ///Marks this template-declaration as a instantiation of the given one. This also means that this declaration will be deleted when the here given is deleted.
      ///from can also be zero, then the declaration will just be marked as a specialization(instantiatedWith is set)
      void setInstantiatedFrom(TemplateDeclaration* from, const InstantiationInformation& instantiatedWith);
      
      /**
       * Either finds the existing instance instantiated with the given template-arguments, or creates a new one.
       * The template-arguments must chained up with the template-arguments of the parent, if the parent is a template class.
       * @param source the top-context from which ones perspective the instantiation should happen. If @p topContext is zero, only existing specializations are returned
       * @param forceLocal if this is true, it is this exact declaration that is instantiated. Else, this declaration is instantiated again.
       * */
      KDevelop::Declaration* instantiate(const InstantiationInformation& templateArguments,
                                         const KDevelop::TopDUContext* source, bool forceLocal = false );

      ///Returns true if this class is a direct instantiation of the given class. Not if it is an instantiation of a specialization of the given class.
      bool isInstantiatedFrom(const TemplateDeclaration* other) const;

      void setSpecializedFrom(TemplateDeclaration* other);

      IndexedInstantiationInformation instantiatedWith() const;
      
      ///Marks that a declaration is currently instantiated with the given instantiation-information.
      ///A zero instantiation is inserted into the cache to make sure that we don't instantiate the same declaration again in the meantime.
      void reserveInstantiation(const IndexedInstantiationInformation& info);

      ///An instantiation is an additional temporary version of this declaration that was created given some template-parameters.
      ///Opposed to that a specialization is a real declaration that was created by the user, that specializes this declaration.
      ///Returns all current instantiations of this declaration
      ///@warning Some instantiations may have the value zero when an instantiation is currently happening.
       InstantiationsHash instantiations() const;
      
      IndexedInstantiationInformation specialization() const;
      
      DeclarationId id(bool forceDirect) const;
      
      Declaration* specialize(const IndexedInstantiationInformation& specialization,
                              const TopDUContext* topContext, int upDistance);
    
      //Duchain must be write-locked
      void deleteAllInstantiations();
      
      ///Returns the template-context that belongs to this template declaration, or zero
      DUContext* templateContext(const TopDUContext* source) const;
      
      ///@return The declaration this one was explicitly specialized from.
      ///Zero, or a non-specialized (thus also non-instantiated) template-class, which this was explicitly specialized from.
      virtual IndexedDeclaration specializedFrom() = 0;
      virtual const IndexedDeclaration* specializations() const = 0;
      virtual uint specializationsSize() const = 0;
      
      ///These are internal, do not use them. They have to be public so they are visible from SpecialTemplateDeclaration.
      virtual void setSpecializedFromInternal(const IndexedDeclaration& decl) = 0;
      virtual void addSpecializationInternal(const IndexedDeclaration& decl) = 0;
      virtual void removeSpecializationInternal(const IndexedDeclaration& decl) = 0;
      
      virtual void setSpecializedWith(const IndexedInstantiationInformation& info) = 0;
      virtual IndexedInstantiationInformation specializedWith() const = 0;
      
    protected:
      //If the given info matches a specialization, returns the instantiated specialization
      TemplateDeclaration *instantiateSpecialization(const InstantiationInformation& info, const TopDUContext *source);
      //Matches the given instantiation-information to this declaration's specialization information and returns a score
      uint matchInstantiation(IndexedInstantiationInformation indexedInfo, const TopDUContext* topCtxt,
                              InstantiationInformation& instantiateWith, bool& instantiationRequired) const;
      
      virtual TemplateDeclarationData* dynamicTemplateData() = 0;
      virtual const TemplateDeclarationData* templateData() const = 0;

      TemplateDeclaration* m_instantiatedFrom;

      IndexedInstantiationInformation m_instantiatedWith;
      
      static QMutex instantiationsMutex;
      ///Every access to m_instantiations must be serialized through instantiationsMutex!
      typedef QHash<IndexedInstantiationInformation, IndexedInstantiationInformation> DefaultParameterInstantiationHash;
      DefaultParameterInstantiationHash m_defaultParameterInstantiations;
      InstantiationsHash m_instantiations; ///Every declaration nested within a template declaration knows all its instantiations.
      // recursion counter
      int m_instantiationDepth;
  };
  
  
    KDEVCPPDUCHAIN_EXPORT DECLARE_LIST_MEMBER_HASH(SpecialTemplateDeclarationData, m_specializations, IndexedDeclaration)
  
    template<class Base>
    class SpecialTemplateDeclarationData : public Base, public TemplateDeclarationData
    {
    public:
    SpecialTemplateDeclarationData() {
      initializeAppendedLists();
    }
    
    ~SpecialTemplateDeclarationData() {
      freeAppendedLists();
    }
    
    SpecialTemplateDeclarationData(const SpecialTemplateDeclarationData& rhs) : Base(rhs), TemplateDeclarationData(rhs) {
      initializeAppendedLists();
      copyListsFrom(rhs);
      m_specializedFrom = rhs.m_specializedFrom;
      m_specializedWith = rhs.m_specializedWith;
    }
    
    uint classSize() const {
      return Base::classSize();
    }

    IndexedDeclaration m_specializedFrom;
    IndexedInstantiationInformation m_specializedWith;

    START_APPENDED_LISTS_BASE(SpecialTemplateDeclarationData, Base);
    APPENDED_LIST_FIRST(SpecialTemplateDeclarationData, IndexedDeclaration, m_specializations);
    END_APPENDED_LISTS(SpecialTemplateDeclarationData, m_specializations);
    };

  /**
   * Use this to merge any type of declaration with a TemplateDeclaration.
   * This allows wrapping template-declaration's over any kind of declaration, without changing the basic structure of types.
   * Necessary because we have at least 3 different declaration-classes that are not derived from each other, and that can
   * be TemplateDeclarations.
   */
  template<class BaseDeclaration>
  class KDEVCPPDUCHAIN_EXPORT SpecialTemplateDeclaration : public BaseDeclaration, public TemplateDeclaration {
    public:
    template<class Data>
    SpecialTemplateDeclaration(Data& data) : BaseDeclaration(data) {
    }

    typedef SpecialTemplateDeclarationData<typename BaseDeclaration::Data> Data;
    
    ///Copy-constructor for cloning
    SpecialTemplateDeclaration(const SpecialTemplateDeclaration<BaseDeclaration>& rhs) : BaseDeclaration(*new SpecialTemplateDeclarationData<typename BaseDeclaration::Data>(*rhs.d_func())), TemplateDeclaration(rhs) {
      DUChainBaseData* data = static_cast<DUChainBase*>(this)->d_func_dynamic();
      data->setClassId(this);
      //To keep link-consistency these specialization links are cleared in copies/instantiations
      static_cast<Data*>(data)->m_specializedFrom = IndexedDeclaration();
      static_cast<Data*>(data)->m_specializationsList().clear();
    }
    ///Arguments are passed to the base
    SpecialTemplateDeclaration( const KDevelop::RangeInRevision& range, KDevelop::DUContext* context ) : BaseDeclaration(*new SpecialTemplateDeclarationData<typename BaseDeclaration::Data>()) {
      static_cast<DUChainBase*>(this)->d_func_dynamic()->setClassId(this);
      
      this->setRange(range);
      
      if(context)
        this->setContext(context);
    }
    
    ~SpecialTemplateDeclaration() {
      TopDUContext* top = this->topContext();
      Q_ASSERT(top);

      //This actually belongs into ~TemplateDeclaration, but we do it here because here we do not need to go through virtual
      //functions or dynamic casts, because those are not guaranteed to work from within destructors.
      if(!top->deleting() || !top->isOnDisk()) {
        ///When the declaration is being deleted, disconnect the links on both sides
        Declaration* specializedFromDeclaration = static_cast<const Data*>(this->DUChainBase::d_func())->m_specializedFrom.data();
        if(TemplateDeclaration* specializedFromTemplate = dynamic_cast<TemplateDeclaration*>(specializedFromDeclaration))
          specializedFromTemplate->removeSpecializationInternal(IndexedDeclaration(this));
        
        FOREACH_FUNCTION(IndexedDeclaration decl, static_cast<const Data*>(this->DUChainBase::d_func())->m_specializations) {
          TemplateDeclaration* tDecl = dynamic_cast<TemplateDeclaration*>(decl.data());
          if(tDecl)
            tDecl->setSpecializedFrom(0);
        }    
      }
    }

    virtual IndexedDeclaration specializedFrom() {
      return static_cast<const Data*>(this->DUChainBase::d_func())->m_specializedFrom;
    }
    virtual const IndexedDeclaration* specializations() const {
      return static_cast<const Data*>(this->DUChainBase::d_func())->m_specializations();
    }
    virtual uint specializationsSize() const {
      return static_cast<const Data*>(this->DUChainBase::d_func())->m_specializationsSize();
    }
    virtual IndexedInstantiationInformation specializedWith() const {
      return static_cast<const Data*>(this->DUChainBase::d_func())->m_specializedWith;
    }
    virtual bool inDUChain() const {
      return instantiatedFrom() || BaseDeclaration::inDUChain();
    }
    
    virtual uint additionalIdentity() const {
      return BaseDeclaration::additionalIdentity() + 101;
    }
    virtual Declaration* specialize(const IndexedInstantiationInformation& specialization,
                                    const TopDUContext* topContext, int upDistance) {
      return TemplateDeclaration::specialize(specialization, topContext, upDistance);
    }
    
    virtual IndexedInstantiationInformation specialization() const {
      return TemplateDeclaration::specialization();
    }
    
    virtual DeclarationId id(bool forceDirect) const {
      return TemplateDeclaration::id(forceDirect);
    }
    
    //Is specialized for ForwardDeclaration in templatedeclaration.cpp to actively instantiate template forward declarations
    virtual Declaration* resolve(const TopDUContext* /*topContext*/) const {
      Q_ASSERT(0);
      return 0;
    }
    
    enum {
      Identity = BaseDeclaration::Identity + 50
    };
    
    private:
    virtual void setSpecializedFromInternal(const IndexedDeclaration& decl) {
      static_cast<Data*>(this->DUChainBase::d_func_dynamic())->m_specializedFrom = decl;
    }
    virtual void addSpecializationInternal(const IndexedDeclaration& decl) {
      static_cast<Data*>(this->DUChainBase::d_func_dynamic())->m_specializationsList().append(decl);
    }
    virtual void removeSpecializationInternal(const IndexedDeclaration& decl) {
      bool result = static_cast<Data*>(this->DUChainBase::d_func_dynamic())->m_specializationsList().removeOne(decl);
      Q_ASSERT(result);
      Q_UNUSED(result);
    }
    virtual void setSpecializedWith(const IndexedInstantiationInformation& info) {
      static_cast<Data*>(this->DUChainBase::d_func_dynamic())->m_specializedWith = info;
    }
      
    virtual TemplateDeclarationData* dynamicTemplateData() {
        return d_func_dynamic();
    }
    
    virtual const TemplateDeclarationData* templateData() const {
        return d_func();
    }

    virtual void activateSpecialization()
    {
      BaseDeclaration::activateSpecialization();
      
      if(specialization().index()) {
        //Also register parents
        DUContext* context = this->context();
        if(context->owner() && context->owner()->specialization().index()) {
          context->owner()->activateSpecialization(); //will also add to the background-parser
        }else{
//           context->topContext()->setHasUses(false); //Force re-building of the uses
//           ICore::self()->languageController()->backgroundParser()->addDocument( KUrl(this->url().str()) );
        }
      }
    }

    inline SpecialTemplateDeclarationData<typename BaseDeclaration::Data>* d_func_dynamic() { this->makeDynamic(); return reinterpret_cast<SpecialTemplateDeclarationData<typename BaseDeclaration::Data>*>(this->d_ptr); }
    inline const SpecialTemplateDeclarationData<typename BaseDeclaration::Data>* d_func() const { return reinterpret_cast<const SpecialTemplateDeclarationData<typename BaseDeclaration::Data>*>(this->d_ptr); }
    virtual KDevelop::Declaration* clonePrivate() const {
      return new SpecialTemplateDeclaration(*this);
    }
  };

  bool KDEVCPPDUCHAIN_EXPORT isTemplateDeclaration(const KDevelop::Declaration*);

  /**
   * If this is used to instantiate a declaration, it should be called BEFORE the Declaration's types DelayedType's are resolved,
   * because those are needed to resolve missing base-classes.
   * 
   * The given context should be one that, on some level, imports a template-parameter-declaration context.
   * The given declaration will be registered anonymously, the same for the created contexts.
   * @param parentContext he parent-context everything should be created in(instantiatedDeclaration will be moved into that context anonymously)
   * @param inclusionTrace a trace as used in findDeclarationsInternal(..)
   * @param context A du-context that will be copied and used as internal context for declaration. If this is zero, no context will be copied.
   * @param templateArguments The template-arguments that will be used to instantiate the input-context. If this is empty, the intersting context will be only copied without specialization. If it contains exactly one argument, and that argument is invalid, the context is instantiated without arguments(default-arguments are used). Default-arguments will be used if needed.
   * @param instantiatedDeclaration The copied declaration this should belong to. If this is set, the created context will be made the given declaration's internal-context, and its parent-context will be set to the given context's parent-context. Also delayed types in the declaration will be resolved(The declaration will be changed)
   * @param instantiatedFrom The declaration instantiatedDeclaration should be/is instantiated from. This is needed to eventually change the declaration of in IdentifiedType. The instantiation has to be reserved before calling instantiateDeclarationAndContext!
   * @param doNotRegister If this is true, the context will not be registered anywhere, and can be deleted at will.
   *
   * The DU-Context must be read-locked but not write-locked when this is called.
   * */
  KDEVCPPDUCHAIN_EXPORT CppDUContext<KDevelop::DUContext>* instantiateDeclarationAndContext( KDevelop::DUContext* parentContext, const KDevelop::TopDUContext* source, KDevelop::DUContext* context, const InstantiationInformation& templateArguments, KDevelop::Declaration* instantiatedDeclaration, KDevelop::Declaration* instantiatedFrom, bool doNotRegister = false );

  /**
  * Returns whether any count of reference/pointer-types are followed by a delayed type
  * */
  KDEVCPPDUCHAIN_EXPORT TypePtr<DelayedType> containsDelayedType(AbstractType::Ptr type);
  
  /**
   * Eventually creates a copy of the given type, where all DelayedTypes that can be resolved in the given context are resolved.
   * */
  KDEVCPPDUCHAIN_EXPORT AbstractType::Ptr resolveDelayedTypes( AbstractType::Ptr type, const KDevelop::DUContext* context, const KDevelop::TopDUContext* source, KDevelop::DUContext::SearchFlags searchFlags = KDevelop::DUContext::NoUndefinedTemplateParams );

template<>
Declaration* SpecialTemplateDeclaration<ForwardDeclaration>::resolve(const TopDUContext* topContext) const;

}

#endif

