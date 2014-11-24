/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
 * Copyright 2007 Aleix Pol <aleixpol@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef CMAKEPREFERENCES_H
#define CMAKEPREFERENCES_H

#include "cmakecachemodel.h"

#include <project/projectconfigpage.h>
#include <util/path.h>

class QItemSelection;
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
        explicit CMakePreferences(KDevelop::IPlugin* plugin, const KDevelop::ProjectConfigOptions& options, QWidget* parent = 0);
        ~CMakePreferences();

        virtual QString name() const override;
        virtual QString fullName() const override;
        virtual QIcon icon() const override;

        virtual void apply() override;
        virtual void reset() override;
        virtual void defaults() override;

    private slots:
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
        
        KDevelop::IProject* m_project;
        KDevelop::Path m_srcFolder;
        KDevelop::Path m_subprojFolder;
        void updateCache( const KDevelop::Path & );
        Ui::CMakeBuildSettings* m_prefsUi;
        CMakeCacheModel* m_currentModel;
};

#endif
