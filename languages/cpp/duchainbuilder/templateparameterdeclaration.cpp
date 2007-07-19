/* This  is part of KDevelop
    Copyright (C) 2007 David Nolden <david.nolden@kdevelop.org>

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

using namespace KDevelop;

class TemplateParameterDeclarationPrivate
{
public:
  QString m_defaultParameter;
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

QString TemplateParameterDeclaration::defaultParameter() const {
  return d->m_defaultParameter;
}

void TemplateParameterDeclaration::setDefaultParameter(const QString& str) {
  d->m_defaultParameter = str;
}

// kate: space-indent on; indent-width 2; tab-width: 4; replace-tabs on; auto-insert-doxygen on
