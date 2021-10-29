/*
    SPDX-FileCopyrightText: 2018 Daniel Mensinger <daniel@mensinger-ka.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <outputview/outputexecutejob.h>

namespace Meson
{
struct BuildDir;
}

namespace KDevelop
{
class IProject;
}

class MesonJob : public KDevelop::OutputExecuteJob
{
    Q_OBJECT
public:
    enum CommandType { CONFIGURE, RE_CONFIGURE, SET_CONFIG };

public:
    MesonJob(const Meson::BuildDir& buildDir, KDevelop::IProject* project, CommandType commandType,
             const QStringList& arguments, QObject* parent);

    QUrl workingDirectory() const override;

private:
    KDevelop::IProject* m_project = nullptr;
    CommandType m_commandType;
    QStringList m_arguments;
};
