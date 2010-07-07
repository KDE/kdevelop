/*
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

#include "qtfunctiondeclaration.h"
#include <language/duchain/duchainregister.h>

using namespace KDevelop;

namespace Cpp {

REGISTER_DUCHAIN_ITEM(QtFunctionDeclaration);

QtFunctionDeclaration::QtFunctionDeclaration(const QtFunctionDeclaration& rhs) : KDevelop::ClassFunctionDeclaration(*new QtFunctionDeclarationData(*rhs.d_func())) {
  d_func_dynamic()->setClassId(this);
}

QtFunctionDeclaration::QtFunctionDeclaration(QtFunctionDeclarationData& data) : KDevelop::ClassFunctionDeclaration(data) {
}

QtFunctionDeclaration::QtFunctionDeclaration(const KDevelop::RangeInRevision& range, KDevelop::DUContext* context) : KDevelop::ClassFunctionDeclaration(*new QtFunctionDeclarationData, range, context) {
  d_func_dynamic()->setClassId(this);
}

KDevelop::Declaration* QtFunctionDeclaration::clonePrivate() const {
  return new QtFunctionDeclaration(*this);
}

KDevelop::IndexedString QtFunctionDeclaration::normalizedSignature() const {
  return d_func()->normalizedSignature;
}

void QtFunctionDeclaration::setNormalizedSignature(KDevelop::IndexedString signature) {
  d_func_dynamic()->normalizedSignature = signature;
}
}
