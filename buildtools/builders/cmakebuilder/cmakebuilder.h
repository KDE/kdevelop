/* KDevelop CMake Support
 *
 * Copyright 2006-2007 Aleix Pol <aleixpol@gmail.com>
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

#ifndef CMAKEBUILDER_H
#define CMAKEBUILDER_H

#include <iplugin.h>
#include "icmakebuilder.h"
#include <QtCore/QList>
#include <QtCore/QVariant>
#include <QtCore/QPair>
#include <KUrl>

class QStringList;
class QSignalMapper;
class KDialog;
class IMakeBuilder;
namespace KDevelop{
    class ProjectBaseItem;
    class CommandExecutor;
    class OutputModel;
}

/**
 * @author Aleix Pol
*/
class CMakeBuilder : public KDevelop::IPlugin, public ICMakeBuilder
{
    Q_OBJECT
    Q_INTERFACES( ICMakeBuilder )
    Q_INTERFACES( KDevelop::IProjectBuilder )

public:
    explicit CMakeBuilder(QObject *parent = 0, const QVariantList &args = QVariantList());
    virtual ~CMakeBuilder();

    virtual bool build(KDevelop::ProjectBaseItem *dom);
    virtual bool install(KDevelop::ProjectBaseItem *dom);
    virtual bool clean(KDevelop::ProjectBaseItem *dom);

    bool updateConfig( KDevelop::IProject* project );

Q_SIGNALS:
    void built(KDevelop::ProjectBaseItem*);
    void failed(KDevelop::ProjectBaseItem*);
    void installed(KDevelop::ProjectBaseItem*);
    void cleaned(KDevelop::ProjectBaseItem*);
private Q_SLOTS:
    void completed( int id );
    void errored( int id );
    void cleanupModel( int id );
private:
    QMap< KDevelop::IProject*, int > m_ids;
    QMap< int, KDevelop::CommandExecutor* > m_cmds;
    QMap< int, KDevelop::ProjectBaseItem* > m_items;
    QMap< int, KDevelop::OutputModel* > m_models;
    QSignalMapper* m_failedMapper;
    QSignalMapper* m_completedMapper;

    KUrl m_buildDirectory;
    KUrl m_cmakeBinary;
    KUrl m_installPrefix;
    QString m_buildType;
    bool m_dirty;
    IMakeBuilder* m_builder;
};

#endif // CMAKEBUILDER_H

