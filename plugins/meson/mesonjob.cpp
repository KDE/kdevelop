/*
    SPDX-FileCopyrightText: 2018 Daniel Mensinger <daniel@mensinger-ka.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mesonjob.h"

#include "mesonconfig.h"

#include <interfaces/iproject.h>
#include <project/projectmodel.h>

#include <KLocalizedString>

using namespace KDevelop;

MesonJob::MesonJob(const Meson::BuildDir& buildDir, IProject* project, MesonJob::CommandType commandType,
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
        *this << QStringLiteral("--backend") << buildDir.mesonBackend;
        break;
    case RE_CONFIGURE:
        *this << QStringLiteral("--reconfigure");
        break;
    case SET_CONFIG:
        *this << QStringLiteral("configure");
        break;
    }

    *this << m_arguments;

    for (auto i : buildDir.mesonArgs.split(QLatin1Char(' '))) {
        if (!i.isEmpty()) {
            *this << i;
        }
    }

    *this << buildDir.buildDir.toLocalFile();
}

QUrl MesonJob::workingDirectory() const
{
    return m_project->path().toUrl();
}

#include "moc_mesonjob.cpp"
