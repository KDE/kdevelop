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

#ifndef MISSINGDECLARATIONASSISTANT_H
#define MISSINGDECLARATIONASSISTANT_H

#include <interfaces/iassistant.h>
#include <language/duchain/types/abstracttype.h>
#include "missingdeclarationtype.h"
#include "cppduchainexport.h"
#include "missingdeclarationproblem.h"

namespace Cpp {

class KDEVCPPDUCHAIN_EXPORT MissingDeclarationAssistant : public KDevelop::IAssistant
{
public:
  MissingDeclarationAssistant(const MissingDeclarationProblem::Ptr& p);
  MissingDeclarationProblem::Ptr problem;
  MissingDeclarationType::Ptr type;
  QString title() const override { return m_title; }
private:
  bool canCreateLocal(KDevelop::DUContext* searchFrom) const;
  bool canAddTo(KDevelop::Declaration* toClass, KDevelop::Declaration* fromClass) const;
  QString m_title;
};

}
#endif // MISSINGDECLARATIONASSISTANT_H
