/*
 * KDevelop xUnit testing support
 *
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
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

#ifndef VERITASCPP_METHODSKELETON_INCLUDED
#define VERITASCPP_METHODSKELETON_INCLUDED

#include "veritascppexport.h"
#include <QtCore/QString>
#include <language/duchain/types/functiontype.h>

namespace Veritas
{
enum Access { Public, Protected, Private };

/*! Value class which stores a simplified AST for member functions */
class VERITASCPP_EXPORT MethodSkeleton
{
public:
    MethodSkeleton();
    virtual ~MethodSkeleton();
    bool isEmpty() const;

    void setFunctionType(KDevelop::FunctionType::Ptr);
    void setName(const QString&);
    void setArguments(const QString&);
    void setReturnType(const QString&);
    void setBody(const QString&);
    void setAccess(Access) {}
    void setVirtual() {}
    void setConst(bool);

    QString name() const;
    QString arguments() const;
    QString body() const;
    QString returnType() const;
    bool isConst() const;

private:
    bool m_empty;
    QString m_name;
    QString m_body;
    QString m_returnType;
    QString m_arguments;
    bool m_isConst;
};

}

#endif // VERITASCPP_METHODSKELETON_INCLUDED
