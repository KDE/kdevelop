/*
 * This file is part of KDevelop
 *
 * Copyright 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ICOMPILERFACTORY_H
#define ICOMPILERFACTORY_H

#include "icompiler.h"

class CompilerProvider;

/// Interface that represents a factory for creating compilers
class ICompilerFactory
{
public:
    virtual QString name() const = 0;

    ///@return new compiler
    ///@see ICompiler
    virtual CompilerPointer createCompiler( const QString& name, const QString& path, bool editable = true ) const = 0;

    /**
     * registers default compilers for the @p provider
     * E.g. for gcc default compilers could be "gcc c99" and "gcc c++11"
     */
    virtual void registerDefaultCompilers(CompilerProvider* provider) const = 0;

    virtual ~ICompilerFactory() = default;
};

typedef QSharedPointer<ICompilerFactory> CompilerFactoryPointer;

#endif // ICOMPILERFACTORY_H
