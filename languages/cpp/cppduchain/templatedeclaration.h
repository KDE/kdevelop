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

#include <duchain/declaration.h>
#include <duchain/declarationid.h>
#include <duchain/duchainpointer.h>
#include <duchain/ducontext.h>
#include "expressionparser.h"
#include "cppduchainexport.h"
#include <appendedlist.h>
#include "expressionevaluationresult.h"


namespace KTextEditor {
  class Range;
}
namespace  KDevelop {
  class DUContext;
  class Declaration;
}

namespace Cpp {
  class IndexedInstantiationInformation;
  
  DECLARE_LIST_MEMBER_HASH(InstantiationInformation, templateParameters, IndexedExpressionEvaluationResult);
  
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
    
    QString toString() const;
    
    ///Instantiation-information for the surrounding context(see IndexedInstantiationInformation), or zero.
    uint previousInstantiationInformation;
    
    START_APPENDED_LISTS(InstantiationInformation);
    
    ///templateParameters contains the template-parameters used for the instantiation
    APPENDED_LIST_FIRST(InstantiationInformation, IndexedExpressionEvaluationResult, templateParameters);
    
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
       * */
      KDevelop::Declaration* instantiate( const InstantiationInformation& templateArguments, const KDevelop::TopDUContext* source, bool visible = false );

      ///Returns true if this class is either is a direct instantiation of the given class. Not if it is an instantiation of a specialization of the given class.
      bool isInstantiatedFrom(const TemplateDeclaration* other) const;

      void setSpecializedFrom(TemplateDeclaration* other);

      ///@return Zero, or a non-specialized(thus also non-instantiated) template-class, which this was explicitly specialized from.
      TemplateDeclaration* specializedFrom() const;

      IndexedInstantiationInformation instantiatedWith() const;

      ///Returns all current instantiations of this declaration
      InstantiationsHash instantiations() const;
      
      uint specialization() const;
      
      DeclarationId id() const;
      
      Declaration* specialize(uint specialization, TopDUContext* topContext);
    
    private:

      //The context in which the template-parameters are declared
      KDevelop::DUContextPointer m_parameterContext;
      TemplateDeclaration* m_instantiatedFrom;
      TemplateDeclaration* m_specializedFrom; 
      IndexedInstantiationInformation m_instantiatedWith;

      static QMutex instantiationsMutex;
      ///Every access to m_instantiations must be serialized through instantiationsMutex!
      
      InstantiationsHash m_instantiations; ///Every declaration nested within a template declaration knows all its instantiations.
      QList<TemplateDeclaration*> m_specializations; ///Explicit specializations
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
    ///Copy-constructor for cloning
    SpecialTemplateDeclaration(const SpecialTemplateDeclaration<BaseDeclaration>& rhs) : BaseDeclaration(rhs), TemplateDeclaration(rhs) {
    }
    ///Arguments are passed to the base
    SpecialTemplateDeclaration( const HashedString& url, const KDevelop::SimpleRange& range, KDevelop::DUContext* context ) : BaseDeclaration(url, range, context) {
    }

    virtual bool inDUChain() const {
      return instantiatedFrom() || BaseDeclaration::inDUChain();
    }
    
    virtual KDevelop::Declaration* clone() const {
      return new SpecialTemplateDeclaration(*this);
    }

    virtual uint additionalIdentity() const {
      return BaseDeclaration::additionalIdentity() + 101;
    }
    virtual Declaration* specialize(uint specialization, TopDUContext* topContext) {
      return TemplateDeclaration::specialize(specialization, topContext);
    }
    
    virtual uint specialization() const {
      return TemplateDeclaration::specialization();
    }
    
    virtual DeclarationId id() const {
      return TemplateDeclaration::id();
    }
    
    //Is specialized for ForwardDeclaration in templatedeclaration.cpp to actively instantiate template forward declarations
    virtual Declaration* resolve(const TopDUContext* /*topContext*/) const {
      Q_ASSERT(0);
      return 0;
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
  CppDUContext<KDevelop::DUContext>* instantiateDeclarationAndContext( KDevelop::DUContext* parentContext, const KDevelop::TopDUContext* source, KDevelop::DUContext* context, const InstantiationInformation& templateArguments, KDevelop::Declaration* instantiatedDeclaration, KDevelop::Declaration* instantiatedFrom , bool visible = false );

  /**
   * Eventually creates a copy of the given type, where all DelayedTypes that can be resolved in the given context are resolved.
   * */
  AbstractType::Ptr resolveDelayedTypes( AbstractType::Ptr type, const KDevelop::DUContext* context, const KDevelop::TopDUContext* source, KDevelop::DUContext::SearchFlags searchFlags = KDevelop::DUContext::NoUndefinedTemplateParams );
}

#endif

