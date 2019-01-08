/* This file is part of KDevelop
    Copyright 2018 Daniel Mensinger <daniel@mensinger-ka.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
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
    MesonJob(Meson::BuildDir const& buildDir, KDevelop::IProject* project, CommandType commandType,
             QStringList const& arguments, QObject* parent);

    QUrl workingDirectory() const override;

private:
    KDevelop::IProject* m_project = nullptr;
    CommandType m_commandType;
    QStringList m_arguments;
};
