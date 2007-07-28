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
#include "expressionparser.h"
#include "cppduchainbuilderexport.h"


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

uint qHash( const Cpp::ExpressionEvaluationResult& key );
uint qHash( const QList<Cpp::ExpressionEvaluationResult>& key );

namespace Cpp {
  template<class Base>
  class CppDUContext;
  
  //Represents the template-part of a template-class'es or template-function's template-part
  class KDEVCPPDUCHAINBUILDER_EXPORT TemplateDeclaration {
    public:
      ///Copy-constructor for cloning
      TemplateDeclaration(const TemplateDeclaration& rhs);
      TemplateDeclaration();
      virtual ~TemplateDeclaration();

      void setTemplateParameterContext(KDevelop::DUContext* context);
      KDevelop::DUContext* templateParameterContext() const;

      TemplateDeclaration* specializedFrom() const;

      ///Marks this template-declaration as a specialization of the given one. This also means that this declaration will be deleted when the here given is deleted.
      void setSpecializedFrom(TemplateDeclaration* from, const QList<ExpressionEvaluationResult>& templateArguments);
      
      /**
       * Either finds the existing instance specialized with the given template-arguments, or creates a new one.
       * */
      KDevelop::Declaration* specialize( const QList<ExpressionEvaluationResult>& templateArguments );

      bool isSpecializedFrom(const TemplateDeclaration* other) const;

    private:

      //The context in which the template-parameters are declared
      KDevelop::DUContextPointer m_parameterContext;
      TemplateDeclaration* m_specializedFrom;
      QList<ExpressionEvaluationResult> m_specializedWith;
      typedef QHash<QList<ExpressionEvaluationResult>, TemplateDeclaration*> SpecializationHash;

      static QMutex specializationsMutex;
      ///Every access to m_specializations must be serialized through specializationsMutex!
      SpecializationHash m_specializations;
      
  };

  /**
   * Use this to merge any type of declaration with a TemplateDeclaration.
   * This allows wrapping template-declaration's over any kind of declaration, without changing the basic structure of types.
   * Necessary because we have at least 3 differnt declaration-classes that are not derived from each other, and that can
   * be a TemplateDeclaration.
   * */
  template<class BaseDeclaration>
  class KDEVCPPDUCHAINBUILDER_EXPORT SpecialTemplateDeclaration : public BaseDeclaration, public TemplateDeclaration {
    public:
    ///Copy-constructor for cloning
    SpecialTemplateDeclaration(const SpecialTemplateDeclaration<BaseDeclaration>& rhs) : BaseDeclaration(rhs), TemplateDeclaration(rhs) {
    }
    ///Arguments are passed to the base
    SpecialTemplateDeclaration( KTextEditor::Range* range, KDevelop::Declaration::Scope scope, KDevelop::DUContext* context ) : BaseDeclaration(range, scope, context) {
    }
    ///Arguments are passed to the base
    SpecialTemplateDeclaration( KTextEditor::Range* range, KDevelop::DUContext* context ) : BaseDeclaration(range, context) {
    }

    virtual bool inDUChain() const {
      return specializedFrom() || BaseDeclaration::inDUChain();
    }
    
    virtual KDevelop::Declaration* clone() const {
      return new SpecialTemplateDeclaration(*this);
    }
  };

  bool KDEVCPPDUCHAINBUILDER_EXPORT isTemplateDeclaration(const KDevelop::Declaration*);

  /**
   * If this is used to specialize a declaration, it should be called BEFORE the Declaration's types DelayedType's are resolved,
   * because those are needed to resolve missing base-classes.
   * 
   * The given context should be one that, on some level, imports a template-parameter-declaration context.
   * The given declaration will be registered anonymously, the same for the created contexts.
   * @param parentContext he parent-context everything should be created in(specializedDeclaration will be moved into that context anonymously)
   * @param context A du-context that will be copied and used as internal context for declaration. If this is zero, no context will be copied.
   * @param templateArguments The template-arguments that will be used to specialize the input-context. If this is empty, the intersting context will be only copied without specialization.
   * @param specializedDeclaration The copied declaration this should belong to. If this is set, the created context will be made the given declaration's internal-context, and it's parent-context will be set to the given context's parent-context. Also delayed types in the declaration will be resolved(The declaration will be changed)
   * @param specializedFrom The declaration specializedDeclaration should be/is specialized from. This is needed to eventually change the declaration of in IdentifiedType
   *
   * The DU-Context must be read-locked but not write-locked when this is called.
   * */
  CppDUContext<KDevelop::DUContext>* specializeDeclarationContext( KDevelop::DUContext* parentContext, KDevelop::DUContext* context, const QList<Cpp::ExpressionEvaluationResult>& templateArguments, KDevelop::Declaration* specializedDeclaration, KDevelop::Declaration* specializedFrom  );

  /**
   * Eventually creates a copy of the given type, where all DelayedTypes that can be resolved in the given context are resolved.
   * */
  AbstractType::Ptr resolveDelayedTypes( AbstractType::Ptr type, const KDevelop::DUContext* context );
}

#endif

