/* This file is part of KDevelop
    Copyright (C) 2004 Roberto Raggi <roberto@kdevelop.org>
    Copyright (C) 2007 Andreas Pakulat <apaku@gmx.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef MAKEBUILDER_H
#define MAKEBUILDER_H

#include <iplugin.h>
#include "imakebuilder.h"
#include <QtCore/QList>
#include <QtCore/QPair>


class QStringList;
class KDialog;
class QString;

namespace KDevelop {
class ProjectBaseItem;
}

/**
@author Roberto Raggi
*/
class MakeBuilder: public KDevelop::IPlugin, public IMakeBuilder
{
    Q_OBJECT
    Q_INTERFACES( IMakeBuilder )
    Q_INTERFACES( KDevelop::IProjectBuilder )
public:
    MakeBuilder(QObject *parent = 0, const QStringList &args = QStringList());
    virtual ~MakeBuilder();

    virtual bool build(KDevelop::ProjectBaseItem *dom);
    virtual bool clean(KDevelop::ProjectBaseItem *dom);

Q_SIGNALS:
    void built( KDevelop::ProjectBaseItem* );
    void failed( KDevelop::ProjectBaseItem* );

private Q_SLOTS:
    void commandFinished(const QStringList &command);
    void commandFailed(const QStringList &command);

private:
    QStringList buildCommand(KDevelop::ProjectBaseItem *dom, const QString &target = QString::null);

private:
    QList<QPair< QStringList, KDevelop::ProjectBaseItem* > > m_queue;
};

#endif // KDEVMAKEBUILDER_H

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
