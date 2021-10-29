/*
    SPDX-FileCopyrightText: 2018 Anton Anikin <anton@anikin.xyz>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "jobparameters.h"

// plugin
#include "debug.h"
#include "globalsettings.h"
#include "utils.h"
// KF
#include <KLocalizedString>
// Qt
#include <QDir>
#include <QStandardPaths>

using namespace KDevelop;

namespace Clazy
{

JobGlobalParameters::JobGlobalParameters()
    : JobGlobalParameters(GlobalSettings::executablePath(), GlobalSettings::docsPath())
{
}

JobGlobalParameters::JobGlobalParameters(const QUrl& executablePath, const QUrl& docsPath)
{
    m_executablePath = executablePath.toLocalFile();
    m_docsPath = docsPath.toLocalFile();

    QFileInfo info;

    if (m_executablePath.isEmpty()) {
        if (defaultExecutablePath().toLocalFile().isEmpty()) {
            m_error = i18n(
                "clazy-standalone path cannot be detected. "
                "Set the path manually if Clazy is already installed.");
        } else {
            m_error = i18n("clazy-standalone path is empty.");
        }
        return;
    }

    info.setFile(m_executablePath);

    if (!info.exists()) {
        m_error = i18n("clazy-standalone path '%1' does not exists.", m_executablePath);
        return;
    }

    if (!info.isFile() || !info.isExecutable()) {
        m_error = i18n("clazy-standalone path '%1' is not an executable.", m_executablePath);
        return;
    }

    // =============================================================================================

    if (m_docsPath.isEmpty()) {
        if (defaultDocsPath().toLocalFile().isEmpty()) {
            m_error = i18n(
                "Clazy documentation path cannot be detected. "
                "Set the path manually if Clazy is already installed.");
        } else {
            m_error = i18n("Clazy documentation path is empty.");
        }
        return;
    }

    info.setFile(m_docsPath);

    if (!info.exists()) {
        m_error = i18n("Clazy documentation path '%1' does not exists.", m_docsPath);
        return;
    }

    if (!info.isDir()) {
        m_error = i18n("Clazy documentation path '%1' is not a directory.", m_docsPath);
        return;
    }

    m_error.clear();
}

QUrl JobGlobalParameters::defaultExecutablePath()
{
    return QUrl::fromLocalFile(QStandardPaths::findExecutable(QStringLiteral("clazy-standalone")));
}

QUrl JobGlobalParameters::defaultDocsPath()
{
    const QString subPathsCandidates[2]{
        // since clazy 1.4
        QStringLiteral("doc/clazy"),
        // before
        QStringLiteral("clazy/doc"),
    };
    for (auto subPath : subPathsCandidates) {
        const auto docsPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, subPath, QStandardPaths::LocateDirectory);

        if (!docsPath.isEmpty()) {
            return QUrl::fromLocalFile(QDir(docsPath).canonicalPath());
        }
    }

    return {};
}

bool JobGlobalParameters::isValid() const
{
    return m_error.isEmpty();
}

QString JobGlobalParameters::error() const
{
    return m_error;
}

}
