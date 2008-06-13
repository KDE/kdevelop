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
#include <duchain/duchainpointer.h>
#include <duchain/ducontext.h>
#include "expressionparser.h"
#include "cppduchainexport.h"


namespace KTextEditor {
  class Range;
}
namespace  KDevelop {
  class DUContext;
  class Declaration;
}

namespace Cpp {
  class ExpressionEvaluationResult;
}

struct InstantiationKey {
  ///Template-arguments
  InstantiationKey() {
  }
  explicit InstantiationKey( const QList<Cpp::ExpressionEvaluationResult>& _args ) : args(_args) {
  }
  QList<Cpp::ExpressionEvaluationResult> args;
  bool operator==(const InstantiationKey & rhs) const;
};

uint qHash( const InstantiationKey& );

namespace Cpp {
  template<class Base>
  class CppDUContext;
  
  //Represents the template-part of a template-class'es or template-function's template-part
  class KDEVCPPDUCHAIN_EXPORT TemplateDeclaration {
    public:
      typedef QHash<InstantiationKey, TemplateDeclaration*> InstantiationsHash;
      
      ///Copy-constructor for cloning
      TemplateDeclaration(const TemplateDeclaration& rhs);
      TemplateDeclaration();
      virtual ~TemplateDeclaration();

      void setTemplateParameterContext(KDevelop::DUContext* context);
      KDevelop::DUContext* templateParameterContext() const;

      TemplateDeclaration* instantiatedFrom() const;

      ///Marks this template-declaration as a instantiation of the given one. This also means that this declaration will be deleted when the here given is deleted.
      void setInstantiatedFrom(TemplateDeclaration* from);
      
      /**
       * Either finds the existing instance instantiated with the given template-arguments, or creates a new one.
       * */
      KDevelop::Declaration* instantiate( const QList<ExpressionEvaluationResult>& templateArguments, const KDevelop::TopDUContext* source );

      ///Returns true if this class is either is a direct instantiation of the given class. Not if it is an instantiation of a specialization of the given class.
      bool isInstantiatedFrom(const TemplateDeclaration* other) const;

      void setSpecializedFrom(TemplateDeclaration* other);

      ///@return Zero, or a non-specialized(thus also non-instantiated) template-class, which this was explicitly specialized from.
      TemplateDeclaration* specializedFrom() const;

      const QList<ExpressionEvaluationResult>& instantiatedWith() const;

      ///Must not be called once setInstantiatedFrom was called
      void setInstantiatedWith(const QList<ExpressionEvaluationResult>& templateParams);

      ///Returns all current instantiations of this declaration
      InstantiationsHash instantiations() const;
      
    private:

      //The context in which the template-parameters are declared
      KDevelop::DUContextPointer m_parameterContext;
      TemplateDeclaration* m_instantiatedFrom;
      TemplateDeclaration* m_specializedFrom; 
      InstantiationKey m_instantiatedWith;

      static QMutex instantiationsMutex;
      ///Every access to m_instantiations must be serialized through instantiationsMutex!
      InstantiationsHash m_instantiations;
      QList<TemplateDeclaration*> m_specializations;
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
    SpecialTemplateDeclaration( const HashedString& url, const KDevelop::SimpleRange& range, KDevelop::Declaration::Scope scope, KDevelop::DUContext* context ) : BaseDeclaration(url, range, scope, context) {
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
   *
   * The DU-Context must be read-locked but not write-locked when this is called.
   * */
  CppDUContext<KDevelop::DUContext>* instantiateDeclarationContext( KDevelop::DUContext* parentContext, const KDevelop::TopDUContext* source, KDevelop::DUContext* context, const QList<Cpp::ExpressionEvaluationResult>& templateArguments, KDevelop::Declaration* instantiatedDeclaration, KDevelop::Declaration* instantiatedFrom  );

  /**
   * Eventually creates a copy of the given type, where all DelayedTypes that can be resolved in the given context are resolved.
   * */
  AbstractType::Ptr resolveDelayedTypes( AbstractType::Ptr type, const KDevelop::DUContext* context, const KDevelop::TopDUContext* source, KDevelop::DUContext::SearchFlags searchFlags = KDevelop::DUContext::NoUndefinedTemplateParams );
}

#endif

