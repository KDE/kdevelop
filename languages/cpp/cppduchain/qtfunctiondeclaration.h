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

#ifndef QTFUNCTIONDECLARATION_H
#define QTFUNCTIONDECLARATION_H

#include <language/duchain/classfunctiondeclaration.h>
#include "cppduchainexport.h"

namespace Cpp {

class KDEVCPPDUCHAIN_EXPORT QtFunctionDeclarationData : public KDevelop::ClassFunctionDeclarationData {
public:

  KDevelop::IndexedString normalizedSignature;
};
  
class KDEVCPPDUCHAIN_EXPORT QtFunctionDeclaration : public KDevelop::ClassFunctionDeclaration
{
public:
  QtFunctionDeclaration(const QtFunctionDeclaration& rhs);
  QtFunctionDeclaration(QtFunctionDeclarationData& data);
  QtFunctionDeclaration(const KDevelop::RangeInRevision& range, KDevelop::DUContext* context);

  KDevelop::IndexedString normalizedSignature() const;
  void setNormalizedSignature(KDevelop::IndexedString);
  
  enum {
    Identity = 74
  };
  
private:
  virtual KDevelop::Declaration* clonePrivate () const override;
  DUCHAIN_DECLARE_DATA(QtFunctionDeclaration)
};

}

#endif // QTFUNCTIONDECLARATION_H
