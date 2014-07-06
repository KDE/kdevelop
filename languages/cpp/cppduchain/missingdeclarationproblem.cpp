/*
   Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>
   
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

#include "missingdeclarationproblem.h"
#include "missingdeclarationtype.h"
#include "missingdeclarationassistant.h"
#include <klocalizedstring.h>

namespace Cpp {

MissingDeclarationProblem::MissingDeclarationProblem(Cpp::MissingDeclarationType::Ptr _type) : type(_type) {
  setDescription(i18n("Declaration not found: %1", type->identifier().toString()));
  setSeverity(ProblemData::Warning);
}

QExplicitlySharedDataPointer< KDevelop::IAssistant > Cpp::MissingDeclarationProblem::solutionAssistant() const {
  return QExplicitlySharedDataPointer< KDevelop::IAssistant >(new MissingDeclarationAssistant(QExplicitlySharedDataPointer<MissingDeclarationProblem>(const_cast<MissingDeclarationProblem*>(this))));
}

}
