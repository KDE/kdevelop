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
#include <KSharedConfig>
#include <KCModule>

namespace KDevelop { class IProject; }
namespace Ui { class CMakeBuildSettings; }
class KJob;

/**
 * @author Matt Rogers <mattr@kde.org>
 * @author Aleix Pol <aleixpol@gmail.com>
 */
class CMakePreferences : public KCModule
{
    Q_OBJECT
    public:
        explicit CMakePreferences(QWidget* parent = 0, const QVariantList& args = QVariantList());
        ~CMakePreferences();

    private slots:
        void load();
        void save();
        void listSelectionChanged ( const QModelIndex& current, const QModelIndex& );
        void showInternal(int state);
        void cacheEdited(QStandardItem * ) { emit changed(true); }
        void buildDirChanged(const QString &);
        void createBuildDir();
        void removeBuildDir();
        void showAdvanced(bool v);
        void configureFinished(KJob*);
    private:
        void configure();

        struct BuildFolderSettings
        {
            KUrl installDir;
            QString buildType;
            KUrl cmakeBinary;
            QString extraArguments;
        };

        QHash<KUrl, BuildFolderSettings> m_newBuildFolderSettings;
        KUrl m_srcFolder;
        KUrl m_subprojFolder;
        KUrl m_configureJobFolder;
        KDevelop::IProject *m_project;
        void updateCache( const KUrl & );
        Ui::CMakeBuildSettings* m_prefsUi;
        CMakeCacheModel* m_currentModel;
};

#endif
