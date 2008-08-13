/*
 * KDevelop xUnit test support
 * Copyright 2008 Manuel Breugelmans
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef VERITAS_STUBCLASS_INCLUDED_H
#define VERITAS_STUBCLASS_INCLUDED_H

#include <QtCore/QString>
#include "veritascppexport.h"
#include "constructorskeleton.h"
#include "methodskeleton.h"

namespace Veritas
{

/*! Lightweight struct which stores C++ Class information */
class VERITASCPP_EXPORT ClassSkeleton
{
public:
    ClassSkeleton();
    virtual ~ClassSkeleton();

    bool isEmpty() const;

    QString name() const;
    QString super() const;
    ConstructorSkeleton constructor() const;
    MethodSkeleton destructor() const;
    QList<MethodSkeleton> methods() const;

    void addSuper(const QString&);
    void setName(const QString&);
    void setConstructor(const ConstructorSkeleton&);
    void setDestructor(const MethodSkeleton&);
    void addMethod(const MethodSkeleton&);

    int memberCount() const;
    QString member(int i) const;
    void addMember(const QString&);

    bool hasMethods() const;
    bool hasMembers() const;
    bool hasConstructor() const;
    bool hasDestructor() const;
    bool hasSomething() const;

private:
    QString m_name;
    QList<MethodSkeleton> m_methods;
    bool m_empty;
    ConstructorSkeleton m_constructor;
    MethodSkeleton m_destructor;
    QList<QString> m_members;
    QString m_super;
};

}

#endif // VERITAS_STUBCLASS_INCLUDED_H
