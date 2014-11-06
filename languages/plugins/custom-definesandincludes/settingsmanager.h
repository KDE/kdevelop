/*
 * This file is part of KDevelop
 *
 * Copyright 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <language/interfaces/idefinesandincludesmanager.h>

#include "definesandincludesexport.h"

#include "compilerprovider/icompiler.h"

class KConfig;
namespace KDevelop
{
class IProject;
}

class ICompilerProvider;
class KDEVDEFINESANDINCLUDESMANAGER_EXPORT SettingsManager
{
public:
    SettingsManager();
    ~SettingsManager(){}

    void setProvider(const ICompilerProvider* provider);

    QList<KDevelop::ConfigEntry> readPaths(KConfig* cfg) const;

    void writePaths(KConfig* cfg, const QList<KDevelop::ConfigEntry>& paths);

    void writeCurrentCompiler(KConfig* cfg, const CompilerPointer& compiler);

    /// @param defaultCompiler the compiler that will be returned, if the @see ICompilerProvider doesn't have a factory to create the needed type of compiler
    /// @return stored compiler or nullptr if the project is opened for the first time.
    CompilerPointer currentCompiler(KConfig* cfg, const CompilerPointer& defaultCompiler = {}) const;

    void writeUserDefinedCompilers(const QVector<CompilerPointer>& compilers);

    QVector<CompilerPointer> userDefinedCompilers() const;

    bool needToReparseCurrentProject( KConfig* cfg ) const;

private:
    const ICompilerProvider* m_provider;
};

#endif // SETTINGSMANAGER_H
