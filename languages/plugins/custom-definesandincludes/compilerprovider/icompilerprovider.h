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

#ifndef ICOMPILERPROVIDER_H
#define ICOMPILERPROVIDER_H

#include <QHash>
#include <QString>
#include <QSharedPointer>

#include <util/path.h>

using KDevelop::Path;

class ICompilerProvider
{
public:
    virtual QHash<QString, QString> defines( const QString& path ) const = 0;
    virtual Path::List includes( const QString& path ) const = 0;

    virtual QString name() const = 0;

    virtual ~ICompilerProvider() = default;
protected:
    struct DefinesIncludes {
        QHash<QString, QString> definedMacros;
        Path::List includePaths;
    };
    // list of defines/includes for the compiler with selected path
    mutable QHash<QString, DefinesIncludes> m_definesIncludes;
};

typedef QSharedPointer<ICompilerProvider> ProviderPointer;

#endif // ICOMPILERPROVIDER_H
