/* This file is part of KDevelop
    Copyright 2012 Andrew Fuller <mactalla.obair@gmail.com>

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

#ifndef KDEVPLATFORM_PLUGIN_PROJECTMODELSAVER_H
#define KDEVPLATFORM_PLUGIN_PROJECTMODELSAVER_H

#include <kviewstatesaver.h>

namespace KDevelop
{

class IProject;

class ProjectModelSaver: public KViewStateSaver
{
    Q_OBJECT
public:
    ProjectModelSaver();

    /**
     * If @p project is non-null, only files from that project will
     * be handled by @code index{From,To}ConfigString @endcode.
     *
     * For other files, an invalid index or empty string is be returned.
     */
    void setProject(IProject* project);

    virtual QModelIndex indexFromConfigString(const QAbstractItemModel *model, const QString &key) const override;

    virtual QString indexToConfigString(const QModelIndex &index) const override;

private:
    IProject* m_project;
};

}

#endif // KDEVPLATFORM_PLUGIN_PROJECTMODELSAVER_H
