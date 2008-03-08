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

#ifndef CMAKEBUILDDIRCREATOR_H
#define CMAKEBUILDDIRCREATOR_H

#include <QDialog>
#include <KUrl>
#include <KProcess>

namespace Ui {
    class CMakeBuildDirCreator;
}

class CMakeBuildDirCreator : public QDialog
{
    Q_OBJECT
    public:
        explicit CMakeBuildDirCreator(const KUrl& srcDir, QWidget* parent = 0, Qt::WindowFlags f=0);
        ~CMakeBuildDirCreator() {}

        KUrl cmakeBinary() const;
        KUrl installPrefix() const;
        KUrl buildFolder() const;
        QString buildType() const;

        void setCMakeBinary(const KUrl&);
        void setInstallPrefix(const KUrl&);
        void setBuildFolder(const KUrl&);
        void setBuildType(const QString&);

    private slots:
        void runBegin();
        void runEnd();

        void addError();
        void addOutput();

        void cmakeCommandDone(int exitCode, QProcess::ExitStatus exitStatus);
        void updated();
    private:
        static QString buildDirProject(const KUrl& buildDir);

        static QString executeProcess(const QString& execName, const QStringList& args=QStringList());
        Ui::CMakeBuildDirCreator* m_creatorUi;
        KProcess m_proc;
        KUrl m_srcFolder;
};


#endif
