/*
    SPDX-FileCopyrightText: 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_VCSCHANGESVIEW_H
#define KDEVPLATFORM_PLUGIN_VCSCHANGESVIEW_H

#include <QTreeView>

#include <memory>

class VcsProjectIntegrationPlugin;

namespace KDevelop {
class ProjectChangesModel;
}

class VcsChangesView : public QTreeView
{
    Q_OBJECT
    public:
        explicit VcsChangesView(VcsProjectIntegrationPlugin* plugin, QWidget* parent = nullptr);

    public Q_SLOTS:
        void popupContextMenu( const QPoint &pos );
        void selectCurrentDocument();
        void openSelected(const QModelIndex& idx);

private:
    const std::shared_ptr<KDevelop::ProjectChangesModel> m_model;
};

#endif // KDEVPLATFORM_PLUGIN_VCSCHANGESVIEW_H
