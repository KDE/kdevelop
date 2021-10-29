/*
    SPDX-FileCopyrightText: 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef COMPILERPROVIDER_H
#define COMPILERPROVIDER_H

#include "icompilerfactory.h"

#include <QVector>

class SettingsManager;

class CompilerProvider : public QObject, public KDevelop::IDefinesAndIncludesManager::Provider
{
    Q_OBJECT

public:
    explicit CompilerProvider( SettingsManager* settings, QObject* parent = nullptr );
    ~CompilerProvider() override;

    KDevelop::Defines defines( const QString& path ) const override;
    KDevelop::Defines defines( KDevelop::ProjectBaseItem* item ) const override;
    KDevelop::Path::List includes( const QString& path ) const override;
    KDevelop::Path::List includes( KDevelop::ProjectBaseItem* item ) const override;
    KDevelop::Path::List frameworkDirectories( const QString& path ) const override;
    KDevelop::Path::List frameworkDirectories( KDevelop::ProjectBaseItem* item ) const override;
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

    /// @returns a default compiler
    CompilerPointer defaultCompiler() const;

private Q_SLOTS:
    void retrieveUserDefinedCompilers();
    void projectChanged(KDevelop::IProject* p);

private:
    mutable CompilerPointer m_defaultProvider;

    QVector<CompilerPointer> m_compilers;
    QVector<CompilerFactoryPointer> m_factories;

    SettingsManager* m_settings;
};

#endif // COMPILERSPROVIDER_H
