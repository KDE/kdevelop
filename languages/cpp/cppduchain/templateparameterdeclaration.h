/* This file is part of KDevelop
    Copyright 2007 David Nolden <david.nolden.kdevelop@kdevelop.org>

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

#ifndef TEMPLATEPARAMETERDECLARATION_H
#define TEMPLATEPARAMETERDECLARATION_H

#include <language/duchain/declaration.h>
#include <language/duchain/declarationdata.h>
#include <language/duchain/identifier.h>
#include "cppduchainexport.h"

namespace KDevelop {
  class QualifiedIdentifier;
}

namespace KDevelop {
  class DUContext;
}

class TemplateParameterDeclarationData : public KDevelop::DeclarationData
{
public:
  TemplateParameterDeclarationData() {
  }
  TemplateParameterDeclarationData(const TemplateParameterDeclarationData& rhs) : KDevelop::DeclarationData(rhs), m_defaultParameter(rhs.m_defaultParameter) {
  }
  KDevelop::IndexedQualifiedIdentifier m_defaultParameter;
};

/**
 * Represents a single template-parameter definition
 */
class KDEVCPPDUCHAIN_EXPORT TemplateParameterDeclaration : public KDevelop::Declaration
{
public:
  TemplateParameterDeclaration(TemplateParameterDeclarationData& data);
  TemplateParameterDeclaration(const TemplateParameterDeclaration& rhs);
  TemplateParameterDeclaration(const KDevelop::RangeInRevision& range, KDevelop::DUContext* context);
  ~TemplateParameterDeclaration();

  /**
   * The returned identifier may contain an expression
   * */
  KDevelop::QualifiedIdentifier defaultParameter() const;

  void setDefaultParameter(const KDevelop::QualifiedIdentifier& str);

  /**
   * @return true if this template parameter has a default value, false otherwise.
   */
  bool hasDefaultParameter() const;

  enum {
    Identity = 18
  };
  
private:
  virtual Declaration* clonePrivate() const override;
  DUCHAIN_DECLARE_DATA(TemplateParameterDeclaration)
};

#endif // FUNCTIONDECLARATION_H

