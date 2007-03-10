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
#ifndef IPROJECTBUILDER_H
#define IPROJECTBUILDER_H

#include "iextension.h"
#include <QtDesigner/QAbstractExtensionFactory>
#include "kdevexport.h"

namespace KDevelop
{
class IProject;
class ProjectBaseItem;

/**
@author Roberto Raggi

@short KDevProjectBuilder Base class for the Project Builders

Describes a <b>Project Builder</b> to KDevelop's Project Manager.
*/
class KDEVPLATFORM_EXPORT IProjectBuilder
{
public:

    virtual ~IProjectBuilder() {}

    virtual bool build(ProjectBaseItem *dom) = 0;
    virtual bool clean(ProjectBaseItem *dom) = 0;

Q_SIGNALS:
    void built(ProjectBaseItem *dom);
    void failed(ProjectBaseItem *dom);
};

}

KDEV_DECLARE_EXTENSION_INTERFACE( KDevelop, IProjectBuilder, "org.kdevelop.IProjectBuilder" )
Q_DECLARE_INTERFACE( KDevelop::IProjectBuilder, "org.kdevelop.IProjectBuilder" )

#endif
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
