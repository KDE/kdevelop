/* This file is part of KDevelop
    Copyright 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2006 Adam Treat <treat@kde.org>
    Copyright 2006-2008 Hamish Rodda <rodda@kde.org>
    Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef KDEVPLATFORM_ENUMERATIONTYPE_H
#define KDEVPLATFORM_ENUMERATIONTYPE_H

#include "integraltype.h"
#include "identifiedtype.h"

namespace KDevelop
{

typedef KDevelop::MergeIdentifiedType<IntegralType> EnumerationTypeBase;

typedef EnumerationTypeBase::Data EnumerationTypeData;

class KDEVPLATFORMLANGUAGE_EXPORT EnumerationType : public EnumerationTypeBase
{
public:
  EnumerationType();

  EnumerationType(const EnumerationType& rhs);

  EnumerationType(EnumerationTypeData& data);
  typedef TypePtr<EnumerationType> Ptr;

  virtual uint hash() const override;

  virtual KDevelop::AbstractType* clone() const override;

  virtual bool equals(const KDevelop::AbstractType* rhs) const override;

  virtual QString toString() const override;

  virtual WhichType whichType() const override;

  enum {
    Identity = 21
  };

  typedef EnumerationTypeData Data;

protected:
  TYPE_DECLARE_DATA(EnumerationType);
};

template<>
inline EnumerationType* fastCast<EnumerationType*>(AbstractType* from) {
  if(!from || from->whichType() != KDevelop::AbstractType::TypeEnumeration)
    return 0;
  else
    return static_cast<EnumerationType*>(from);
}

}


#endif // KDEVPLATFORM_ENUMERATIONTYPE_H
