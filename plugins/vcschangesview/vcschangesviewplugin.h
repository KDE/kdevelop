/*
    SPDX-FileCopyrightText: 2010 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_VCSCHANGESVIEWPLUGIN_H
#define KDEVPLATFORM_PLUGIN_VCSCHANGESVIEWPLUGIN_H

#include <interfaces/iplugin.h>

class VCSProjectToolViewFactory;

class QModelIndex;

namespace KDevelop
{
    class ProjectChangesModel;
}

class VcsProjectIntegrationPlugin : public KDevelop::IPlugin
{
    Q_OBJECT
    public:
        VcsProjectIntegrationPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList& args);
        void unload() override;
        KDevelop::ProjectChangesModel* model();
        
    private Q_SLOTS:
        void activated(const QModelIndex& idx);
        
    private:
        VCSProjectToolViewFactory* const m_factory;
        KDevelop::ProjectChangesModel* m_model;

        friend class VCSProjectToolViewFactory; // to access activated() slot
};

#endif
