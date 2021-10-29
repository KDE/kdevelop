/*
    SPDX-FileCopyrightText: 2007-2012 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
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
