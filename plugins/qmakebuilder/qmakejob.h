/*
    SPDX-FileCopyrightText: 2006-2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2008 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QMAKEJOB_H
#define QMAKEJOB_H

#include <outputview/outputexecutejob.h>

namespace KDevelop{
    class CommandExecutor;
    class IProject;
}

/**
@author Andreas Pakulat
@author Hamish Rodda (KJob porting)
*/
class QMakeJob : public KDevelop::OutputExecuteJob
{
    Q_OBJECT

public:
    explicit QMakeJob(QObject *parent = nullptr);

    enum ErrorTypes {
        NoProjectError = UserDefinedError,
    };

    void setProject(KDevelop::IProject* project);
    
    void start() override;

    QUrl workingDirectory() const override;
    QStringList commandLine() const override;

protected:
    bool doKill() override;

private:
    KDevelop::IProject* m_project = nullptr;
    KDevelop::CommandExecutor* m_cmd = nullptr;
    bool m_killed = false;
};

#endif // QMAKEJOB_H

