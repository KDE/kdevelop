/*
    SPDX-FileCopyrightText: 2006-2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2008 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CMAKEJOB_H
#define CMAKEJOB_H

#include <outputview/outputexecutejob.h>

#include <util/path.h>

namespace KDevelop {
class IProject;
class ProjectBaseItem;
}

class CMakeJob: public KDevelop::OutputExecuteJob
{
    Q_OBJECT

public:
    enum ErrorTypes
    {
        NoProjectError = UserDefinedError,
        FailedError
    };

    explicit CMakeJob(QObject* parent = nullptr);
    
    void setProject(KDevelop::IProject* project);

    void start() override;

    // This returns the build directory for registered item.
    QUrl workingDirectory() const override;

    // This returns the "cmake" command line.
    QStringList commandLine() const override;

    // This returns the configured global environment profile.
    QString environmentProfile() const override;

private:
    QStringList cmakeArguments( KDevelop::IProject* project );
    KDevelop::Path buildDir( KDevelop::IProject* project );

    KDevelop::IProject* m_project = nullptr;
};

#endif // CMAKEJOB_H
