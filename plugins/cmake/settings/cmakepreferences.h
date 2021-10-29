/*
    SPDX-FileCopyrightText: 2006 Matt Rogers <mattr@kde.org>
    SPDX-FileCopyrightText: 2007 Aleix Pol <aleixpol@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CMAKEPREFERENCES_H
#define CMAKEPREFERENCES_H

#include "cmakecachemodel.h"
#include "cmakeextraargumentshistory.h"

#include <project/projectconfigpage.h>
#include <util/path.h>

class CMakeSettings;

namespace Ui { class CMakeBuildSettings; }

/**
 * @author Matt Rogers <mattr@kde.org>
 * @author Aleix Pol <aleixpol@gmail.com>
 */
class CMakePreferences : public KDevelop::ConfigPage
{
    Q_OBJECT
    public:
        explicit CMakePreferences(KDevelop::IPlugin* plugin, const KDevelop::ProjectConfigOptions& options, QWidget* parent = nullptr);
        ~CMakePreferences() override;

        QString name() const override;
        QString fullName() const override;
        QIcon icon() const override;

        void apply() override;
        void reset() override;
        void defaults() override;

    private Q_SLOTS:
        void listSelectionChanged ( const QModelIndex& current, const QModelIndex& );
        void showInternal(int state);
        void cacheEdited(QStandardItem * ) { emit changed(); }
        void buildDirChanged(int index);
        void cacheUpdated();
        void createBuildDir();
        void removeBuildDir();
        void showAdvanced(bool v);
        void configureCacheView();
    private:
        void configure();
        void initAdvanced();
        void setBuildType(const QString& buildType);

        KDevelop::IProject* m_project;
        KDevelop::Path m_srcFolder;
        KDevelop::Path m_subprojFolder;
        void updateCache( const KDevelop::Path & );
        Ui::CMakeBuildSettings* m_prefsUi;
        CMakeCacheModel* m_currentModel;
        CMakeExtraArgumentsHistory* m_extraArgumentsHistory;
};

#endif
