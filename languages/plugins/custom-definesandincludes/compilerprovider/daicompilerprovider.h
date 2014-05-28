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

#include "idaicompilerprovider.h"

#include "../definesandincludesmanager.h"
#include "icompilerprovider.h"

#include <QVariantList>
#include <QVector>

#include <interfaces/iplugin.h>

#include <util/path.h>

#include <QScopedPointer>

using KDevelop::Path;

using namespace KDevelop;

class DAICompilerProvider : public IPlugin, public IDAICompilerProvider, public IDefinesAndIncludesManager::Provider
{
    Q_OBJECT
    Q_INTERFACES( IDAICompilerProvider )
public :
    explicit DAICompilerProvider( QObject* parent, const QVariantList& args = QVariantList() );

    ~DAICompilerProvider();

    virtual QHash<QString, QString> defines( ProjectBaseItem* item ) const override;

    virtual Path::List includes( ProjectBaseItem* item ) const override;

    virtual IDefinesAndIncludesManager::Type type() const override;

    virtual bool setCompiler( IProject* project, const QString& name, const QString& path ) override;

private:
    struct Compiler {
        ProviderPointer compiler;
        QString path;
    };

    Compiler compilerForItem( ProjectBaseItem* item ) const;

    ///Goes through the list of available compilers and automatically selects an appropriate one.
    Compiler selectCompiler( const QString& compilerName, const QString& path ) const;

    void addPoject( IProject* project, Compiler compiler );

    void removePoject( IProject* project );

private Q_SLOTS:
    void projectOpened( KDevelop::IProject* );
    void projectClosed( KDevelop::IProject* );

private:
    //list of compilers for each projects
    QHash<IProject*, Compiler> m_projects;
    QVector<ProviderPointer> m_providers;
};

#endif // COMPILERSPROVIDER_H
