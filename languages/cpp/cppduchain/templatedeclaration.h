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

#ifndef TEMPLATEDECLARATION_H
#define TEMPLATEDECLARATION_H

#include <QList>
#include <QMutex>

#include <language/duchain/declaration.h>
#include <language/duchain/declarationid.h>
#include <language/duchain/duchainpointer.h>
#include <language/duchain/ducontext.h>
#include <language/duchain/topducontext.h>
#include "cppduchainexport.h"
#include <language/duchain/appendedlist.h>
#include "expressionevaluationresult.h"
#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <language/backgroundparser/backgroundparser.h>


namespace KTextEditor {
  class Range;
}
namespace  KDevelop {
  class DUContext;
  class Declaration;
}

namespace Cpp {
  class IndexedInstantiationInformation;
  
  DECLARE_LIST_MEMBER_HASH(InstantiationInformation, templateParameters, IndexedType)
  
  struct InstantiationInformation {
    InstantiationInformation();
    ///@todo include some information for instantiation only with default parameters
    InstantiationInformation(const InstantiationInformation& rhs);
    
    ~InstantiationInformation();
    
    InstantiationInformation& operator=(const InstantiationInformation& rhs);

    bool operator==(const InstantiationInformation& rhs) const;
    
    uint hash() const;
    
    bool isValid() const {
      return previousInstantiationInformation || templateParametersSize();
    }
    
    ///Applies this instantiation information to the given QualifiedIdentifier.
    ///The template parameters of the qualified identifier will be marked as expressions, thus the qualified identifier is not "clean".
    ///Should only be used for displaying.
    ///This only adds template-parameters in places where none are known yet.
    QualifiedIdentifier applyToIdentifier(const QualifiedIdentifier& id) const;
    
    ///@param local If this is true, only the template-parameters of this scope are printed, but not the parent ones
    QString toString(bool local = false) const;
    
    ///Instantiation-information for the surrounding context(see IndexedInstantiationInformation), or zero.
    uint previousInstantiationInformation;
    
    START_APPENDED_LISTS(InstantiationInformation)
    
    static size_t classSize() {
      return sizeof(InstantiationInformation);
    }
    
    short unsigned int itemSize() const {
      return dynamicSize();
    }
    
    ///templateParameters contains the template-parameters used for the instantiation
    APPENDED_LIST_FIRST(InstantiationInformation, IndexedType, templateParameters);
    
    END_APPENDED_LISTS(InstantiationInformation, templateParameters);
    
    IndexedInstantiationInformation indexed() const;
  };
  
  class IndexedInstantiationInformation {
    public:
      IndexedInstantiationInformation();
      IndexedInstantiationInformation(uint index);
      
      const InstantiationInformation& information() const;
      
      uint hash() const {
        return m_index * 73;
      }
      
      //Is always zero for the empty information
      uint index() const {
        return m_index;
      }
      
      bool operator==(const IndexedInstantiationInformation& rhs) const {
        return m_index == rhs.m_index;
      }
      
      //Returns true if one of the values represented by this information is non-default
      bool isValid() const;
    
    private:
      uint m_index;
  };
  
  template<class Base>
  class CppDUContext;
  
  struct TemplateDeclarationData {
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
       * */
      KDevelop::Declaration* instantiate( const InstantiationInformation& templateArguments, const KDevelop::TopDUContext* source );

      ///Returns true if this class is either is a direct instantiation of the given class. Not if it is an instantiation of a specialization of the given class.
      bool isInstantiatedFrom(const TemplateDeclaration* other) const;

      void setSpecializedFrom(TemplateDeclaration* other);

      ///@return Zero, or a non-specialized(thus also non-instantiated) template-class, which this was explicitly specialized from.
      TemplateDeclaration* specializedFrom() const;

      IndexedInstantiationInformation instantiatedWith() const;
      
      ///Marks that currently a declaration is instantiated with the given instantiation-information. A zero instantiation is inserted into
      ///the cache to make sure that we don't instantiate the same declaration again in the meantime.
      void reserveInstantiation(const IndexedInstantiationInformation& info);

      ///Returns all current instantiations of this declaration
      ///@warning Some instantiations may have the value zero when an instantiation is currently happening.
       InstantiationsHash instantiations() const;
      
      uint specialization() const;
      
      DeclarationId id(bool forceDirect) const;
      
      Declaration* specialize(uint specialization, const TopDUContext* topContext);
    
    private:
      virtual TemplateDeclarationData* dynamicTemplateData() = 0;
      virtual const TemplateDeclarationData* templateData() const = 0;

      TemplateDeclaration* m_instantiatedFrom;
      TemplateDeclaration* m_specializedFrom; 
      IndexedInstantiationInformation m_instantiatedWith;

      static QMutex instantiationsMutex;
      ///Every access to m_instantiations must be serialized through instantiationsMutex!
      
      InstantiationsHash m_instantiations; ///Every declaration nested within a template declaration knows all its instantiations.
      QList<TemplateDeclaration*> m_specializations; ///Explicit specializations
  };
  
  
    template<class Base>
    class SpecialTemplateDeclarationData : public Base, public TemplateDeclarationData
    {
    public:
    SpecialTemplateDeclarationData() {
    }
    
    ~SpecialTemplateDeclarationData() {
    }
    
    SpecialTemplateDeclarationData(const SpecialTemplateDeclarationData& rhs) : Base(rhs), TemplateDeclarationData(rhs) {
    }
    };

  /**
   * Use this to merge any type of declaration with a TemplateDeclaration.
   * This allows wrapping template-declaration's over any kind of declaration, without changing the basic structure of types.
   * Necessary because we have at least 3 differnt declaration-classes that are not derived from each other, and that can
   * be a TemplateDeclaration.
   * */
  template<class BaseDeclaration>
  class KDEVCPPDUCHAIN_EXPORT SpecialTemplateDeclaration : public BaseDeclaration, public TemplateDeclaration {
    public:
    template<class Data>
    SpecialTemplateDeclaration(Data& data) : BaseDeclaration(data) {
    }
    ///Copy-constructor for cloning
    SpecialTemplateDeclaration(const SpecialTemplateDeclaration<BaseDeclaration>& rhs) : BaseDeclaration(*new SpecialTemplateDeclarationData<typename BaseDeclaration::Data>(*rhs.d_func())), TemplateDeclaration(rhs) {
      static_cast<DUChainBase*>(this)->d_func_dynamic()->setClassId(this);
    }
    ///Arguments are passed to the base
    SpecialTemplateDeclaration( const KDevelop::SimpleRange& range, KDevelop::DUContext* context ) : BaseDeclaration(*new SpecialTemplateDeclarationData<typename BaseDeclaration::Data>()) {
      static_cast<DUChainBase*>(this)->d_func_dynamic()->setClassId(this);
      
      this->setRange(range);
      
      if(context)
        this->setContext(context);
    }

    virtual bool inDUChain() const {
      return instantiatedFrom() || BaseDeclaration::inDUChain();
    }
    
    virtual uint additionalIdentity() const {
      return BaseDeclaration::additionalIdentity() + 101;
    }
    virtual Declaration* specialize(uint specialization, const TopDUContext* topContext) {
      return TemplateDeclaration::specialize(specialization, topContext);
    }
    
    virtual uint specialization() const {
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
    virtual TemplateDeclarationData* dynamicTemplateData() {
        return d_func_dynamic();
    }
    
    virtual const TemplateDeclarationData* templateData() const {
        return d_func();
    }

    virtual void activateSpecialization()
    {
      BaseDeclaration::activateSpecialization();
      
      if(specialization()) {
        //Also register parents
        DUContext* context = this->context();
        if(context->owner() && context->owner()->specialization()) {
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
   * @param instantiatedFrom The declaration instantiatedDeclaration should be/is instantiated from. This is needed to eventually change the declaration of in IdentifiedType
   * @param visible Whether the created declaration should be non-anonymous in its parent context(thus findable)
   *
   * The DU-Context must be read-locked but not write-locked when this is called.
   * */
  CppDUContext<KDevelop::DUContext>* instantiateDeclarationAndContext( KDevelop::DUContext* parentContext, const KDevelop::TopDUContext* source, KDevelop::DUContext* context, const InstantiationInformation& templateArguments, KDevelop::Declaration* instantiatedDeclaration, KDevelop::Declaration* instantiatedFrom );

  /**
   * Eventually creates a copy of the given type, where all DelayedTypes that can be resolved in the given context are resolved.
   * */
  AbstractType::Ptr resolveDelayedTypes( AbstractType::Ptr type, const KDevelop::DUContext* context, const KDevelop::TopDUContext* source, KDevelop::DUContext::SearchFlags searchFlags = KDevelop::DUContext::NoUndefinedTemplateParams );

inline uint qHash(const Cpp::IndexedInstantiationInformation& info) {
  return info.hash();
}
}

#endif

