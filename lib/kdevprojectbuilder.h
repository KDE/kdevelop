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
#ifndef KDEVPROJECTBUILDER_H
#define KDEVPROJECTBUILDER_H

#include <QObject>
#include "kdevexport.h"
class KDevProject;
class KDevProjectItem;

/**
@author Roberto Raggi

@short KDevProjectBuilder Base class for the Project Builders

Describes a <b>Project Builder</b> to KDevelop's Project Manager.
*/
class KDEVINTERFACES_EXPORT KDevProjectBuilder: public QObject
{
    Q_OBJECT
public:
    KDevProjectBuilder(QObject *parent = 0);
    virtual ~KDevProjectBuilder();

    virtual KDevProject *project() const = 0;

    virtual bool build(KDevProjectItem *dom) = 0;
    virtual bool clean(KDevProjectItem *dom) = 0;

signals:
    void built(KDevProjectItem *dom);
    void failed();
};

#endif
