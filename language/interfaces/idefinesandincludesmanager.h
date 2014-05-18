/*
 * This file is part of KDevelop
 *
 * Copyright 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef IDEFINESANDINCLUDES_H
#define IDEFINESANDINCLUDES_H

#include <QHash>
#include <QString>
#include <QStringList>

#include <KConfigGroup>

#include <util/path.h>

#include "../languageexport.h"

namespace KDevelop
{
class ProjectBaseItem;
class IProject;

typedef QHash<QString, QVariant> Defines;
struct ConfigEntry
{
    QString path;
    QStringList includes;
    Defines defines;

    ConfigEntry( const QString& path = QString() ) : path( path ) {}
};

/** An interface that provides language plugins with include directories/files and defines.
* Call IDefinesAndIncludesManager::manager() to get the instance of the plugin.
**/
class KDEVPLATFORMLANGUAGE_EXPORT IDefinesAndIncludesManager
{
public:
    /// The type of includes/defines
    enum Type {
        CompilerSpecific = 1, ///< Those that compiler provides
        ProjectSpecific = 2, ///< Those that project manager provides
        UserDefined = 4,    ///< Those that user defines
        All = CompilerSpecific | ProjectSpecific | UserDefined
     };

    /// Class that actually does all the work of calculating i/d.
    /// Implement one in e.g. project manager and register it with @see registerProvider
    /// To unregister it use @see unregisterProvider
    class Provider
    {
    public:
        virtual ~Provider() = default;

        virtual QHash<QString, QString> defines( ProjectBaseItem* item ) const = 0;

        virtual Path::List includes( ProjectBaseItem* item ) const = 0;

        /// @return the type of i/d this provider provides
        virtual Type type() const = 0;
    };

    ///@param item project item, use nullptr for files without project
    ///@return list of defines for @p item
    ///NOTE: call it from the foreground thread only.
    virtual QHash<QString, QString> defines( ProjectBaseItem* item, Type type = All ) const = 0;

    ///@param item project item, use nullptr for files without project
    ///@return list of include directories/files for @p item
    ///NOTE: call it from the foreground thread only.
    virtual Path::List includes( ProjectBaseItem* item, Type type = All ) const = 0;

    ///@return the instance of the plugin.
    static IDefinesAndIncludesManager* manager();

    virtual ~IDefinesAndIncludesManager();

    /// Register the @p provider
    virtual void registerProvider(Provider* provider) = 0;

     /**
     * Unregister the provider
     *
     * @return true on success, false otherwise (e.g. if not registered)
     */
    virtual bool unregisterProvider(Provider* provider) = 0;
};
}

Q_DECLARE_INTERFACE( KDevelop::IDefinesAndIncludesManager, "org.kdevelop.IDefinesAndIncludesManager" )

#endif
