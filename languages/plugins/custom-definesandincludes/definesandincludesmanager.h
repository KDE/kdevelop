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

#include "compilerprovider/settingsmanager.h"

class CompilerProvider;

/// @brief: Class for retrieving custom defines and includes.
class DefinesAndIncludesManager : public KDevelop::IPlugin, public KDevelop::IDefinesAndIncludesManager
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IDefinesAndIncludesManager )

public:
    explicit DefinesAndIncludesManager( QObject* parent, const QVariantList& args = QVariantList() );
    // NOTE: Part of a fix for build failures on <GCC-4.7
    virtual ~DefinesAndIncludesManager() noexcept;

    ///@return list of all custom defines for @p item
    KDevelop::Defines defines( KDevelop::ProjectBaseItem* item, Type type ) const override;
    ///@return list of all custom includes for @p item
    KDevelop::Path::List includes( KDevelop::ProjectBaseItem* item, Type type  ) const override;

    virtual void registerProvider( Provider* provider ) override;
    virtual bool unregisterProvider( Provider* provider ) override;

private:
    QVector<Provider*> m_providers;
    SettingsManager m_settings;
};

#endif // CUSTOMDEFINESANDINCLUDESMANAGER_H
