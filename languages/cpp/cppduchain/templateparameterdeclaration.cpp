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

#include "templateparameterdeclaration.h"
#include <duchain/identifier.h>
#include <duchain/declaration.h>


using namespace KDevelop;

class TemplateParameterDeclarationPrivate
{
public:
  QualifiedIdentifier m_defaultParameter;
};

TemplateParameterDeclaration::TemplateParameterDeclaration(const HashedString& url, const KDevelop::SimpleRange& range, DUContext* context)
  : Declaration(url, range, context), d_ptr(new TemplateParameterDeclarationPrivate)
{
  if(context)
    setContext(context);
}

TemplateParameterDeclaration::~TemplateParameterDeclaration()
{
}

QualifiedIdentifier TemplateParameterDeclaration::defaultParameter() const {
  return d_func()->m_defaultParameter;
}

void TemplateParameterDeclaration::setDefaultParameter(const QualifiedIdentifier& str) {
  d_func()->m_defaultParameter = str;
}

TemplateParameterDeclaration::TemplateParameterDeclaration(const TemplateParameterDeclaration& rhs) : Declaration(rhs), 
        d_ptr(new TemplateParameterDeclarationPrivate) {
  d_func()->m_defaultParameter = rhs.d_func()->m_defaultParameter;
  setIsTypeAlias(true);
}

Declaration* TemplateParameterDeclaration::clone() const {
  return new TemplateParameterDeclaration(*this);
}


