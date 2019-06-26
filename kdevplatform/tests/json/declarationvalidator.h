/* This file is part of KDevelop
   Copyright 2012 Olivier de Gaalon <olivier.jg@gmail.com>

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

#ifndef KDEVPLATFORM_DECLARATIONVALIDATOR_H
#define KDEVPLATFORM_DECLARATIONVALIDATOR_H

#include <tests/testsexport.h>

#include "language/duchain/ducontext.h"

namespace KDevelop {
class DeclarationValidatorPrivate;

class KDEVPLATFORMTESTS_EXPORT DeclarationValidator
    : public DUChainVisitor
{
public:
    DeclarationValidator();
    ~DeclarationValidator() override;

    virtual bool testsPassed() const;
    void visit(DUContext*) override;
    void visit(Declaration* declaration) override;

private:
    Q_DISABLE_COPY(DeclarationValidator)
    const QScopedPointer<class DeclarationValidatorPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DeclarationValidator)
};
}
#endif //KDEVPLATFORM_DECLARATIONVALIDATOR_H
