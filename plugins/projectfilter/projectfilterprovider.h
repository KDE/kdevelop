/*
    SPDX-FileCopyrightText: 2013 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
    explicit ProjectFilterProvider(QObject* parent, const KPluginMetaData& metaData,
                                   const QVariantList& args = QVariantList());

    QSharedPointer<IProjectFilter> createFilter(IProject* project) const override;

    ContextMenuExtension contextMenuExtension(Context* context, QWidget* parent) override;

    int perProjectConfigPages() const override;
    ConfigPage* perProjectConfigPage(int number, const ProjectConfigOptions& options, QWidget* parent) override;

Q_SIGNALS:
    void filterChanged(KDevelop::IProjectFilterProvider*, KDevelop::IProject*);

public Q_SLOTS:
    void updateProjectFilters(KDevelop::IProject* project);

private Q_SLOTS:
    void projectClosing(KDevelop::IProject*);
    void projectAboutToBeOpened(KDevelop::IProject*);

    void addFilterFromContextMenu();

private:
    QHash<KDevelop::IProject*, QVector<Filter> > m_filters;
};

}

#endif // KDEVPLATFORM_PLUGIN_PROJECTFILTERPROVIDER_H
