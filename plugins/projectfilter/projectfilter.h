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

#ifndef KDEVPLATFORM_PLUGIN_PROJECTFILTER_H
#define KDEVPLATFORM_PLUGIN_PROJECTFILTER_H

#include <interfaces/iplugin.h>
#include <project/interfaces/iprojectfilter.h>

#include <QVariantList>
#include <QVector>

namespace KDevelop {

struct Filters {
    QVector<QRegExp> include;
    QVector<QRegExp> exclude;
};

class ProjectFilter: public IPlugin, IProjectFilter
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IProjectFilter )

public:
    explicit ProjectFilter( QObject* parent = 0, const QVariantList& args = QVariantList() );

    virtual bool includeInProject(const KUrl& path, bool isFolder, IProject* project) const;

private slots:
    void updateFiltersForProject(KDevelop::IProject*);
    void projectClosing(KDevelop::IProject*);

private:
    mutable QHash<KDevelop::IProject*, Filters> m_filters;
};

}

Q_DECLARE_TYPEINFO(KDevelop::Filters, Q_MOVABLE_TYPE);

#endif // KDEVPLATFORM_PLUGIN_PROJECTFILTER_H
