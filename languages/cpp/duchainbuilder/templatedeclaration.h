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

#ifndef TEMPLATEDECLARATION_H
#define TEMPLATEDECLARATION_H

#include "cppduchainbuilderexport.h"
#include <duchain/declaration.h>
#include <QList>


namespace KTextEditor {
  class Range;
}
namespace  KDevelop {
  class DUContext;
  class Declaration;
}

namespace Cpp {
  class ExpressionEvaluationResult;
  
  //Represents the template-part of a template-class'es or template-function's template-part
  class KDEVCPPDUCHAINBUILDER_EXPORT TemplateDeclaration {
    public:
      ///Copy-constructor for cloning
      TemplateDeclaration(const TemplateDeclaration& rhs);
      TemplateDeclaration();
      virtual ~TemplateDeclaration();

      void setTemplateParameterContext(KDevelop::DUContext* context);
      KDevelop::DUContext* templateParameterContext() const;

      /**
       * Either finds the existing instance instantiated with the given template-arguments, or creates a new one.
       * */
      KDevelop::Declaration* instantiate( const QList<ExpressionEvaluationResult>& templateArguments );
      
    private:
      //The context in which the template-parameters are declared
      KDevelop::DUContext* m_parameterContext;
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

    virtual KDevelop::Declaration* clone() const {
      return new SpecialTemplateDeclaration(*this);
    }
  };

  bool KDEVCPPDUCHAINBUILDER_EXPORT isTemplateDeclaration(const KDevelop::Declaration*);
}

#endif

