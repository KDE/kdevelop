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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KDEVMAKEBUILDER_H
#define KDEVMAKEBUILDER_H

#include <kdevprojectbuilder.h>
#include <qvaluelist.h>
#include <qpair.h>

class KDialogBase;

/**
@author Roberto Raggi
*/
class KDevMakeBuilder: public KDevProjectBuilder
{
    Q_OBJECT
public:
    KDevMakeBuilder(QObject *parent = 0, const char *name = 0, const QStringList &args = QStringList());
    virtual ~KDevMakeBuilder();
    
    virtual KDevProject *project() const;
    
    virtual bool isExecutable(ProjectItemDom dom) const;

    virtual ProjectItemDom defaultExecutable() const;
    virtual void setDefaultExecutable(ProjectItemDom dom);
    
    virtual bool configure(ProjectItemDom dom = ProjectItemDom());
    virtual bool build(ProjectItemDom dom = ProjectItemDom());
    virtual bool clean(ProjectItemDom dom = ProjectItemDom());
    virtual bool execute(ProjectItemDom dom = ProjectItemDom());
    
private slots:
    void projectConfigWidget(KDialogBase *dialog);
    void commandFinished(const QString &command);
    void commandFailed(const QString &command);
    
private:
    QString buildCommand(ProjectItemDom dom);
    QString makeEnvironment() const;
    
private:
    KDevProject *m_project;
    ProjectItemDom m_defaultExecutable;
    QValueList< QPair<QString, ProjectItemDom> > m_commands;
    
    static const QString &builder;
    static const QString &makeTool;
    static const QString &priority;
    static const QString &abortOnError;
    static const QString &numberOfJobs;
    static const QString &dontAct; 
    static const QString &environment; 
};

#endif // KDEVMAKEBUILDER_H

