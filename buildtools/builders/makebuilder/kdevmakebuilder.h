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
class KDevMakeBuilder: public Koncrete::ProjectBuilder
{
    Q_OBJECT
public:
    KDevMakeBuilder(QObject *parent = 0, const QStringList &args = QStringList());
    virtual ~KDevMakeBuilder();

    virtual Koncrete::Project *project() const;

    virtual bool build(Koncrete::ProjectItem *dom);
    virtual bool clean(Koncrete::ProjectItem *dom);

private slots:
    void commandFinished(const QString &command);
    void commandFailed(const QString &command);

private:
    QString buildCommand(Koncrete::ProjectItem *dom, const QString &target = QString::null);

private:
    Koncrete::Project *m_project;
    QList< QPair<QString, Koncrete::ProjectItem *> > m_commands;
};

#endif // KDEVMAKEBUILDER_H

