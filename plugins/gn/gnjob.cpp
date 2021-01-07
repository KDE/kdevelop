/* This file is part of KDevelop
    Copyright 2018 Daniel Mensinger <daniel@mensinger-ka.de>
    Copyright 2021 BogDan Vatra <bogdan@kde.org>

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

#include "gnjob.h"

#include "gnconfig.h"

#include <interfaces/iproject.h>
#include <project/projectmodel.h>

#include <KLocalizedString>

using namespace KDevelop;

GNJob::GNJob(const GN::BuildDir& buildDir, IProject* project, QObject* parent)
    : OutputExecuteJob(parent)
    , m_project(project)
{
    Q_ASSERT(m_project);

    setToolTitle(i18n("GN"));
    setCapabilities(Killable);
    setStandardToolView(KDevelop::IOutputView::BuildView);
    setBehaviours(KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll);
    setProperties(NeedWorkingDirectory | PortableMessages | DisplayStdout | DisplayStderr);

    *this << buildDir.gnExecutable.toLocalFile()
          << QStringLiteral("-ide=json")
          << QProcess::splitCommand(buildDir.gnArgs)
          << QStringLiteral("gen") << buildDir.buildDir.toLocalFile();
}

QUrl GNJob::workingDirectory() const
{
    return m_project->path().toUrl();
}
