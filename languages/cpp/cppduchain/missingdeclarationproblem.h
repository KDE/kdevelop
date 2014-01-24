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

#ifndef CPP_MISSINGDECLARATIONPROBLEM_H
#define CPP_MISSINGDECLARATIONPROBLEM_H

#include <language/duchain/problem.h>
#include <language/duchain/types/abstracttype.h>
#include "missingdeclarationtype.h"
#include "cppduchainexport.h"

namespace Cpp {

class KDEVCPPDUCHAIN_EXPORT MissingDeclarationProblem : public KDevelop::Problem
{
public:
  typedef KSharedPtr<MissingDeclarationProblem> Ptr;

  ///@param type Must not be zero!
  MissingDeclarationProblem(MissingDeclarationType::Ptr type);

  virtual KSharedPtr< KDevelop::IAssistant > solutionAssistant() const;

  MissingDeclarationType::Ptr type;
};

}

#endif // CPP_MISSINGDECLARATIONPROBLEM_H
