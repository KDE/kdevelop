/* This file is part of KDevelop
    Copyright (C) 2004 Roberto Raggi <roberto@kdevelop.org>

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
#ifndef KDEVMAKEBUILDER_H
#define KDEVMAKEBUILDER_H

#include <kdevprojectbuilder.h>
#include <QtCore/QList>
#include <QtCore/QPair>

class QStringList;
class KDialog;

/**
@author Roberto Raggi
*/
class KDevMakeBuilder: public KDevProjectBuilder
{
    Q_OBJECT
public:
    KDevMakeBuilder(QObject *parent = 0, const QStringList &args = QStringList());
    virtual ~KDevMakeBuilder();

    virtual KDevProject *project() const;

    virtual bool isExecutable(KDevProjectItem *dom) const;

    virtual KDevProjectItem *defaultExecutable() const;
    virtual void setDefaultExecutable(KDevProjectItem *dom);

    virtual bool configure(KDevProjectItem *dom);
    virtual bool build(KDevProjectItem *dom);
    virtual bool clean(KDevProjectItem *dom);
    virtual bool execute(KDevProjectItem *dom);

private slots:
    void commandFinished(const QString &command);
    void commandFailed(const QString &command);

private:
    QString buildCommand(KDevProjectItem *dom, const QString &target = QString::null);
    QString makeEnvironment() const;

private:
    KDevProject *m_project;
    KDevProjectItem *m_defaultExecutable;
    QList< QPair<QString, KDevProjectItem *> > m_commands;
};

#endif // KDEVMAKEBUILDER_H

