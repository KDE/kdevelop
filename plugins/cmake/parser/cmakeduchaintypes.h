/* KDevelop CMake Support
 *
 * Copyright 2007-2012 Aleix Pol <aleixpol@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef CMAKEDUCHAINTYPES_H
#define CMAKEDUCHAINTYPES_H

#include <language/duchain/types/abstracttype.h>
#include <language/duchain/types/typesystemdata.h>
#include "cmaketypes.h"

struct KDEVCMAKECOMMON_EXPORT TargetTypeData : public KDevelop::AbstractTypeData {
  TargetTypeData() {}
  ~TargetTypeData() {}
   
  TargetTypeData(const TargetTypeData& rhs) : AbstractTypeData(rhs) {}
  TargetTypeData& operator=(const TargetTypeData& rhs) = delete;
};

class KDEVCMAKECOMMON_EXPORT TargetType : public KDevelop::AbstractType
{
    public:
        TargetType();
        TargetType(const TargetType& rhs);
        explicit TargetType(KDevelop::AbstractTypeData& dd);
        TargetType& operator=(const TargetType& rhs) = delete;

        void accept0(KDevelop::TypeVisitor* v) const override;
        AbstractType* clone() const override;
        bool equals(const AbstractType* rhs) const override;
        
    private:
        TYPE_DECLARE_DATA(TargetType)
};

#endif
