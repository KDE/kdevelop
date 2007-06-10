/* KDevelop QMake Support
 *
 * Copyright 2006 Andreas Pakulat <apaku@gmx.de>
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

#ifndef QMAKEBUILDER_H
#define QMAKEBUILDER_H

#include <iplugin.h>
#include "iqmakebuilder.h"
#include <QtCore/QList>
#include <QtCore/QPair>

class QStringList;
class QSignalMapper;
class KDialog;
namespace KDevelop{
    class ProjectBaseItem;
    class CommandExecutor;
    class OutputModel;
}

/**
@author Andreas Pakulat
*/
class QMakeBuilder : public KDevelop::IPlugin, public IQMakeBuilder
{
    Q_OBJECT
    Q_INTERFACES( IQMakeBuilder )
    Q_INTERFACES( KDevelop::IProjectBuilder )

public:
    explicit QMakeBuilder(QObject *parent = 0, const QStringList &args = QStringList());
    virtual ~QMakeBuilder();

    virtual bool build(KDevelop::ProjectBaseItem *dom);
    virtual bool clean(KDevelop::ProjectBaseItem *dom);

Q_SIGNALS:
    void built(KDevelop::ProjectBaseItem*);
    void failed(KDevelop::ProjectBaseItem*);
private Q_SLOTS:
    void completed(const QString &id);
    void errored(const QString &id);

private:
    QStringList m_ids;
    QMap< QString, KDevelop::CommandExecutor* > m_cmds;
    QMap< QString, KDevelop::ProjectBaseItem* > m_items;
    QMap< QString, KDevelop::OutputModel* > m_models;
    QSignalMapper* m_failedMapper;
    QSignalMapper* m_completedMapper;
};

#endif // QMAKEBUILDER_H

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
