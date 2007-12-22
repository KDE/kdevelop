/* 
   Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>

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

#include "identifiedtype.h"
#include "typesystem.h"

namespace KDevelop {
  class TopDUContext;
  /**
   * A forward-declaration type is generally nothing more than an identified type.
   * */
class KDEVPLATFORMLANGUAGE_EXPORT ForwardDeclarationType : public AbstractType, public IdentifiedType {
  public:
  ForwardDeclarationType(const ForwardDeclarationType& rhs);
  ForwardDeclarationType();

  ///Returns the resolved type, or this if it could not be resolved.
  AbstractType::Ptr resolve(const TopDUContext* topContext) const;
  
  virtual bool equals(const AbstractType* rhs) const;
  virtual QString toString() const;
  virtual AbstractType* clone() const;
  virtual void accept0 (TypeVisitor *v) const;
};
}
