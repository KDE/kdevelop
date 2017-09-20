/* KDevelop CMake Support
 *
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

#ifndef CMAKEBUILDDIRCHOOSER_H
#define CMAKEBUILDDIRCHOOSER_H

#include <QDialog>
#include <QFlags>

#include <util/path.h>

#include "cmakeextraargumentshistory.h"
#include "cmakecommonexport.h"

class QDialogButtonBox;

namespace Ui {
    class CMakeBuildDirChooser;
}
namespace KDevelop {
    class IProject;
}

class KDEVCMAKECOMMON_EXPORT CMakeBuildDirChooser : public QDialog
{
    Q_OBJECT
    public:
        enum StatusType 
        { 
            BuildDirCreated = 1, 
            CorrectProject = 2, 
            BuildFolderEmpty = 4, 
            HaveCMake = 8,
            CorrectBuildDir = 16,
            DirAlreadyCreated = 32 //Error message in case it's already configured
        };
        Q_DECLARE_FLAGS( StatusTypes, StatusType )

        explicit CMakeBuildDirChooser(QWidget* parent = nullptr);
        ~CMakeBuildDirChooser() override;

        KDevelop::Path cmakeExecutable() const;
        KDevelop::Path installPrefix() const;
        KDevelop::Path buildFolder() const;
        QString buildType() const;
        QString extraArguments() const;
        int alreadyUsedIndex() const;
        bool reuseBuilddir();

        void setCMakeExecutable(const KDevelop::Path& path);
        void setInstallPrefix(const KDevelop::Path& path);
        void setBuildFolder(const KDevelop::Path& path);
        void setBuildType(const QString& buildType);
        void setProject( KDevelop::IProject* project );
        void setSourceFolder(const KDevelop::Path &path) { m_srcFolder = path; }
        void setAlreadyUsed(const QStringList& used);
        void setStatus(const QString& message, bool canApply);
        void setExtraArguments(const QString& args);
        void setShowAvailableBuildDirs(bool show);

    private Q_SLOTS:
        void updated();
    private:
        void adoptPreviousBuildDirectory(int index);
        void buildDirSettings(
            const KDevelop::Path& buildDir,
            QString& srcDir,
            QString& installDir,
            QString& buildType);

        QStringList m_alreadyUsed;
        CMakeExtraArgumentsHistory* m_extraArgumentsHistory;

        Ui::CMakeBuildDirChooser* m_chooserUi;
        QDialogButtonBox* m_buttonBox;

        KDevelop::IProject* m_project;
        KDevelop::Path m_srcFolder;
};
Q_DECLARE_OPERATORS_FOR_FLAGS( CMakeBuildDirChooser::StatusTypes )


#endif
