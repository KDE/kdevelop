/* This  is part of KDevelop
    Copyright 2007 David Nolden <david.nolden@kdevelop.org>

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

#include <duchain/identifier.h>
#include "templateparameterdeclaration.h"

using namespace KDevelop;

class TemplateParameterDeclarationPrivate
{
public:
  QualifiedIdentifier m_defaultParameter;
};

TemplateParameterDeclaration::TemplateParameterDeclaration(KTextEditor::Range * range, DUContext* context)
  : Declaration(range, LocalScope, context)
  , d(new TemplateParameterDeclarationPrivate)
{
}

TemplateParameterDeclaration::~TemplateParameterDeclaration()
{
  delete d;
}

QualifiedIdentifier TemplateParameterDeclaration::defaultParameter() const {
  return d->m_defaultParameter;
}

void TemplateParameterDeclaration::setDefaultParameter(const QualifiedIdentifier& str) {
  d->m_defaultParameter = str;
}

TemplateParameterDeclaration::TemplateParameterDeclaration(const TemplateParameterDeclaration& rhs) : Declaration(rhs), d(new TemplateParameterDeclarationPrivate) {
  d->m_defaultParameter = rhs.d->m_defaultParameter;
  setIsTypeAlias(true);
}

Declaration* TemplateParameterDeclaration::clone() const {
  return new TemplateParameterDeclaration(*this);
}


// kate: space-indent on; indent-width 2; tab-width: 4; replace-tabs on; auto-insert-doxygen on
