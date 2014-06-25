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

#ifndef CUSTOMDEFINESANDINCLUDESMANAGER_H
#define CUSTOMDEFINESANDINCLUDESMANAGER_H

#include <QVariantList>
#include <QVector>

#include <interfaces/iplugin.h>

#include <language/interfaces/idefinesandincludesmanager.h>

#include "settingsmanager.h"

namespace KDevelop
{
/// @brief: Class for retrieving custom defines and includes.
class DefinesAndIncludesManager : public IPlugin, public IDefinesAndIncludesManager, public SettingsManager
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IDefinesAndIncludesManager )
public :
    explicit DefinesAndIncludesManager( QObject* parent, const QVariantList& args = QVariantList() );
    ///@return list of all custom defines for @p item
    QHash<QString, QString> defines( ProjectBaseItem* item, Type type ) const override;

    ///@return list of all custom includes for @p item
    Path::List includes( ProjectBaseItem* item, Type type  ) const override;

    virtual QHash<QString, QString> defines( const Path& ) const override;

    virtual Path::List includes( const Path& ) const override;

    virtual void registerProvider( Provider* provider ) override;

    virtual bool unregisterProvider( Provider* provider ) override;

private:
    QVector<Provider*> m_providers;
};
}
#endif // CUSTOMDEFINESANDINCLUDESMANAGER_H
