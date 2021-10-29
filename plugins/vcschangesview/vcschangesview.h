/*
    SPDX-FileCopyrightText: 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_VCSCHANGESVIEW_H
#define KDEVPLATFORM_PLUGIN_VCSCHANGESVIEW_H

#include <QTreeView>
#include <QList>
#include <QUrl>

class VcsProjectIntegrationPlugin;
namespace KDevelop { class IProject; }

class VcsChangesView : public QTreeView
{
    Q_OBJECT
    public:
        explicit VcsChangesView(VcsProjectIntegrationPlugin* plugin, QWidget* parent = nullptr);
        
        void setModel(QAbstractItemModel* model) override;
        
    public Q_SLOTS:
        void popupContextMenu( const QPoint &pos );
        void selectCurrentDocument();
        void openSelected(const QModelIndex& idx);
        
    Q_SIGNALS:
        void reload(const QList<KDevelop::IProject*>& p);
        void reload(const QList<QUrl>& p);
};

#endif // KDEVPLATFORM_PLUGIN_VCSCHANGESVIEW_H
