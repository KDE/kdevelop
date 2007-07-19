/* This file is part of KDevelop
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

#ifndef TEMPLATEPARAMETERDECLARATION_H
#define TEMPLATEPARAMETERDECLARATION_H

#include <declaration.h>
#include <cppduchainbuilderexport.h>

namespace KDevelop {
  class DUContext;
}

/**
 * Represents a single variable definition in a definition-use chain.
 */
class KDEVCPPDUCHAINBUILDER_EXPORT TemplateParameterDeclaration : public KDevelop::Declaration
{
public:
  TemplateParameterDeclaration(KTextEditor::Range* range, KDevelop::DUContext* context);
  ~TemplateParameterDeclaration();

  QString defaultParameter() const;

  void setDefaultParameter(const QString& str);

private:
  class TemplateParameterDeclarationPrivate* const d;
};

#endif // FUNCTIONDECLARATION_H

// kate: space-indent on; indent-width 2; tab-width: 4; replace-tabs on; auto-insert-doxygen on
