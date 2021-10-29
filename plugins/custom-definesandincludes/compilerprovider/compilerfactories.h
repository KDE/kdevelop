/*
    SPDX-FileCopyrightText: 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef COMPILERFACTORIES_H
#define COMPILERFACTORIES_H

#include "icompilerfactory.h"

class ClangFactory : public ICompilerFactory
{
public:
    CompilerPointer createCompiler( const QString& name, const QString& path, bool editable = true ) const override;

    QString name() const override;
    bool isSupported(const KDevelop::Path& path) const override;

    void registerDefaultCompilers(CompilerProvider* provider) const override;
};

class GccFactory : public ICompilerFactory
{
public:
    CompilerPointer createCompiler( const QString& name, const QString& path, bool editable = true ) const override;

    QString name() const override;
    bool isSupported(const KDevelop::Path& path) const override;

    void registerDefaultCompilers(CompilerProvider* provider) const override;
};

class MsvcFactory : public ICompilerFactory
{
public:
    CompilerPointer createCompiler( const QString& name, const QString& path, bool editable = true ) const override;

    QString name() const override;
    bool isSupported(const KDevelop::Path& path) const override;

    void registerDefaultCompilers(CompilerProvider* provider) const override;
};

#endif // COMPILERFACTORIES_H
