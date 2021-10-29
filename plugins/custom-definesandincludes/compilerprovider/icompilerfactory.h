/*
    SPDX-FileCopyrightText: 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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

    /** @returns whether @p path is a compiler supported by the factory */
    virtual bool isSupported(const KDevelop::Path &path) const = 0;

    virtual ~ICompilerFactory() = default;
};

using CompilerFactoryPointer = QSharedPointer<ICompilerFactory>;

#endif // ICOMPILERFACTORY_H
