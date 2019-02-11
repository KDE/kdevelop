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

#include "compilerfactories.h"
#include "compilerprovider.h"

#include "gcclikecompiler.h"
#include "msvccompiler.h"

QString ClangFactory::name() const
{
    return QStringLiteral("Clang");
}

bool ClangFactory::isSupported(const KDevelop::Path& path) const
{
    const auto filename = path.lastPathSegment();
    return filename.contains(QLatin1String("clang")) && !filename.contains(QLatin1String("clang-cl"));
}

CompilerPointer ClangFactory::createCompiler(const QString& name, const QString& path, bool editable ) const
{
    return CompilerPointer(new GccLikeCompiler(name, path, editable, this->name()));
}

void ClangFactory::registerDefaultCompilers(CompilerProvider* provider) const
{
    const QString clang = QStringLiteral("clang");

    auto compiler = createCompiler(name(), clang, false);
    provider->registerCompiler(compiler);
}

QString GccFactory::name() const
{
    return QStringLiteral("GCC");
}

bool GccFactory::isSupported(const KDevelop::Path& path) const
{
    return path.lastPathSegment().contains(QLatin1String("gcc")) || path.lastPathSegment().contains(QLatin1String("g++"));
}

CompilerPointer GccFactory::createCompiler(const QString& name, const QString& path, bool editable ) const
{
    return CompilerPointer(new GccLikeCompiler(name, path, editable, this->name()));
}

void GccFactory::registerDefaultCompilers(CompilerProvider* provider) const
{
    const QString gcc = QStringLiteral("gcc");

    auto compiler = createCompiler(name(), gcc, false);
    provider->registerCompiler(compiler);
}

QString MsvcFactory::name() const
{
    return QStringLiteral("MSVC");
}

CompilerPointer MsvcFactory::createCompiler(const QString& name, const QString& path, bool editable ) const
{
   return CompilerPointer(new MsvcCompiler(name, path, editable, this->name()));
}

void MsvcFactory::registerDefaultCompilers(CompilerProvider* provider) const
{
    provider->registerCompiler(createCompiler(name(), QStringLiteral("cl.exe"), false));
}

bool MsvcFactory::isSupported(const KDevelop::Path& path) const
{
    return path.lastPathSegment() == QLatin1String("cl.exe") || path.lastPathSegment().contains(QLatin1String("clang-cl"));
}
