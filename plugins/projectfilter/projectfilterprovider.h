/*
    This file is part of KDevelop

    Copyright 2013 Milian Wolff <mail@milianw.de>

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

#ifndef KDEVPLATFORM_PLUGIN_PROJECTFILTERPROVIDER_H
#define KDEVPLATFORM_PLUGIN_PROJECTFILTERPROVIDER_H

#include <interfaces/iplugin.h>
#include <project/interfaces/iprojectfilterprovider.h>

#include "projectfilter.h"

#include <QVariantList>

namespace KDevelop {

class ProjectFilterProvider: public IPlugin, public IProjectFilterProvider
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IProjectFilterProvider )

public:
    explicit ProjectFilterProvider( QObject* parent = 0, const QVariantList& args = QVariantList() );

    virtual QSharedPointer<IProjectFilter> createFilter(IProject* project) const;

    virtual ContextMenuExtension contextMenuExtension(Context* context);

signals:
    void filterChanged(KDevelop::IProjectFilterProvider*, KDevelop::IProject*);

private slots:
    void updateProjectFilters();
    void projectClosing(KDevelop::IProject*);
    void projectAboutToBeOpened(KDevelop::IProject*);

    void addFilterFromContextMenu();

private:
    QHash<KDevelop::IProject*, QVector<Filter> > m_filters;
};

}

#endif // KDEVPLATFORM_PLUGIN_PROJECTFILTERPROVIDER_H
