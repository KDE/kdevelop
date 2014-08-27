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

#include <KDialog>
#include <QFlags>
#include <kurl.h>

#include "cmakecommonexport.h"

namespace Ui {
    class CMakeBuildDirChooser;
}

class KDEVCMAKECOMMON_EXPORT CMakeBuildDirChooser : public KDialog
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

        explicit CMakeBuildDirChooser(QWidget* parent = 0);
        ~CMakeBuildDirChooser();

        KUrl cmakeBinary() const;
        KUrl installPrefix() const;
        KUrl buildFolder() const;
        QString buildType() const;
        QString extraArguments() const;

        void setCMakeBinary(const KUrl&);
        void setInstallPrefix(const KUrl&);
        void setBuildFolder(const KUrl&);
        void setBuildType(const QString&);
        void setSourceFolder( const KUrl& srcFolder );
        void setAlreadyUsed(const QStringList& used);
        void setStatus(const QString& message, bool canApply);
        void setExtraArguments(const QString& args);

    private slots:
        void updated();
    private:
        QStringList m_alreadyUsed;
        static QString buildDirProject(const KUrl& buildDir);
        QStringList extraArgumentsHistory() const;

        Ui::CMakeBuildDirChooser* m_chooserUi;
        KUrl m_srcFolder;
};
Q_DECLARE_OPERATORS_FOR_FLAGS( CMakeBuildDirChooser::StatusTypes )


#endif
