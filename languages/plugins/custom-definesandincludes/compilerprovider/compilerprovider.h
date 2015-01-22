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

#ifndef COMPILERSPROVIDER_H
#define COMPILERSPROVIDER_H

#include "icompilerfactory.h"

#include <QVector>

class SettingsManager;

class CompilerProvider : public QObject, public KDevelop::IDefinesAndIncludesManager::Provider
{
    Q_OBJECT

public:
    explicit CompilerProvider( SettingsManager* settings, QObject* parent = nullptr );
    ~CompilerProvider();

    KDevelop::Defines defines( KDevelop::ProjectBaseItem* item ) const override;
    KDevelop::Path::List includes( KDevelop::ProjectBaseItem* item ) const override;
    KDevelop::IDefinesAndIncludesManager::Type type() const override;

    /// @return current compiler for the @p item
    CompilerPointer compilerForItem( KDevelop::ProjectBaseItem* item ) const;

    /// @return list of all available compilers
    QVector<CompilerPointer> compilers() const;
    /**
     * Adds compiler to the list of available compilers
     * @return true on success (if there is no compiler with the same name registered).
     */
    bool registerCompiler(const CompilerPointer& compiler);
    /// Removes compiler from the list of available compilers
    void unregisterCompiler( const CompilerPointer& compiler );

    /// @return All available factories
    QVector<CompilerFactoryPointer> compilerFactories() const;

    /// Checks wheter the @p compiler exist, if so returns it. Otherwise returns default compiler
    CompilerPointer checkCompilerExists( const CompilerPointer& compiler ) const;

private Q_SLOTS:
    void retrieveUserDefinedCompilers();

private:
    QVector<CompilerPointer> m_compilers;
    QVector<CompilerFactoryPointer> m_factories;

    SettingsManager* m_settings;
};

#endif // COMPILERSPROVIDER_H
