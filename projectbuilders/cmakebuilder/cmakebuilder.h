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

#include <interfaces/iplugin.h>
#include <QtCore/QList>
#include <QtCore/QVariant>
#include <QtCore/QPair>
#include <QtCore/QSet>
#include <KUrl>
#include <project/interfaces/iprojectbuilder.h>

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
class CMakeBuilder : public KDevelop::IPlugin, public KDevelop::IProjectBuilder
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IProjectBuilder )

public:
    explicit CMakeBuilder(QObject *parent = 0, const QVariantList &args = QVariantList());
    virtual ~CMakeBuilder();

    virtual KJob* build(KDevelop::ProjectBaseItem *dom);
    virtual KJob* install(KDevelop::ProjectBaseItem *dom);
    virtual KJob* clean(KDevelop::ProjectBaseItem *dom);
    virtual KJob* configure(KDevelop::IProject*);
    virtual KJob* prune(KDevelop::IProject*);

//     bool updateConfig( KDevelop::IProject* project );
private Q_SLOTS:
    void buildFinished(KDevelop::ProjectBaseItem*);
    
Q_SIGNALS:
    void built(KDevelop::ProjectBaseItem*);
    void failed(KDevelop::ProjectBaseItem*);
    void installed(KDevelop::ProjectBaseItem*);
    void cleaned(KDevelop::ProjectBaseItem*);
    void pruned(KDevelop::IProject*);

private:
    IMakeBuilder* m_builder;
    QSet<KDevelop::ProjectBaseItem*> m_deleteWhenDone;
};

#endif // CMAKEBUILDER_H

