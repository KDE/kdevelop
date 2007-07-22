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

#include "templatedeclaration.h"

using namespace KDevelop;
using namespace Cpp;


namespace Cpp {
TemplateDeclaration::TemplateDeclaration(const TemplateDeclaration& /*rhs*/) : m_parameterContext(0) {
}  
TemplateDeclaration::TemplateDeclaration() : m_parameterContext(0) {
}

TemplateDeclaration::~TemplateDeclaration() {
}

void TemplateDeclaration::setTemplateParameterContext(DUContext* context) {
  m_parameterContext = context;
}

DUContext* TemplateDeclaration::templateParameterContext() const {
  return m_parameterContext;
}


bool isTemplateDeclaration(const KDevelop::Declaration* decl) {
  return (bool)dynamic_cast<const TemplateDeclaration*>(decl);
}

Declaration* TemplateDeclaration::instantiate( const QList<ExpressionEvaluationResult>& templateArguments ) {
  
}

}

