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

#ifndef GCCLIKECOMPILER_H
#define GCCLIKECOMPILER_H

#include "icompiler.h"

class GccLikeCompiler : public QObject, public ICompiler
{
    Q_OBJECT
public:
    GccLikeCompiler( const QString& name, const QString& path, bool editable, const QString& factoryName );

    KDevelop::Defines defines(Utils::LanguageType type, const QString& arguments) const override;

    KDevelop::Path::List includes(Utils::LanguageType type, const QString& arguments) const override;

private:
    void invalidateCache();

    struct DefinesIncludes {
        KDevelop::Defines definedMacros;
        KDevelop::Path::List includePaths;
        bool hasCachedMacros = false;
        bool hasCachedIncludes = false;
    };

    /// List of defines/includes per arguments
    mutable QHash<Utils::LanguageType, QHash<QString, DefinesIncludes>> m_definesIncludes;
};

#endif // GCCLIKECOMPILER_H

