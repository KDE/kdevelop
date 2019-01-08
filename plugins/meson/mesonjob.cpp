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

#include "mesonjob.h"
#include "mesonconfig.h"
#include <interfaces/iproject.h>
#include <klocalizedstring.h>
#include <project/projectmodel.h>

using namespace KDevelop;

MesonJob::MesonJob(Meson::BuildDir const& buildDir, IProject* project, MesonJob::CommandType commandType,
                   const QStringList& arguments, QObject* parent)
    : OutputExecuteJob(parent)
    , m_project(project)
    , m_commandType(commandType)
    , m_arguments(arguments)
{
    Q_ASSERT(m_project);

    setToolTitle(i18n("Meson"));
    setCapabilities(Killable);
    setStandardToolView(KDevelop::IOutputView::BuildView);
    setBehaviours(KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll);
    setProperties(NeedWorkingDirectory | PortableMessages | DisplayStdout | DisplayStderr);

    *this << buildDir.mesonExecutable.toLocalFile();

    switch (m_commandType) {
    case CONFIGURE:
        if (!buildDir.installPrefix.isEmpty()) {
            *this << QStringLiteral("--prefix") << buildDir.installPrefix.toLocalFile();
        }

        *this << QStringLiteral("--backend") << buildDir.mesonBackend;
        *this << QStringLiteral("--buildtype") << buildDir.buildType;

        for (auto const& i : buildDir.mesonArgs) {
            *this << i;
        }

        break;
    case RE_CONFIGURE:
        *this << QStringLiteral("--reconfigure");
        break;
    case SET_CONFIG:
        *this << QStringLiteral("configure");
        if (!buildDir.installPrefix.isEmpty()) {
            *this << QStringLiteral("-Dprefix=") + buildDir.installPrefix.toLocalFile();
        }
        *this << QStringLiteral("-Dbuildtype=") + buildDir.buildType;
    }

    *this << m_arguments;
    *this << buildDir.buildDir.toLocalFile();
}

QUrl MesonJob::workingDirectory() const
{
    return m_project->path().toUrl();
}
