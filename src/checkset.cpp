/*
 * This file is part of KDevelop
 *
 * Copyright 2016 Carlos Nihelton <carlosnsoliveira@gmail.com>
 * Copyright 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
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

#include "checkset.h"

// plugin
#include "debug.h"
// KF
#include <KProcess>

namespace ClangTidy
{

void CheckSet::setClangTidyPath(const QString& path)
{
    if (m_clangTidyPath == path) {
        return;
    }

    m_clangTidyPath = path;

    m_allChecks.clear();

    if (m_clangTidyPath.isEmpty()) {
        return;
    }

    // TODO: make this async
    KProcess tidy;
    tidy << m_clangTidyPath << QStringLiteral("-checks=*") << QStringLiteral("--list-checks");
    tidy.setOutputChannelMode(KProcess::OnlyStdoutChannel);
    tidy.start();

    if (!tidy.waitForStarted()) {
        qCDebug(KDEV_CLANGTIDY) << "Unable to execute clang-tidy.";
        return;
    }

    tidy.closeWriteChannel();
    if (!tidy.waitForFinished()) {
        qCDebug(KDEV_CLANGTIDY) << "Failed during clang-tidy execution.";
        return;
    }

    QTextStream ios(&tidy);
    QString each;
    while (ios.readLineInto(&each)) {
        m_allChecks.append(each.trimmed());
    }
    // Drop leading "Enabled checks:" line and trailing empty line
    if (m_allChecks.size() > 3) {
        m_allChecks.removeAt(m_allChecks.length() - 1);
        m_allChecks.removeAt(0);
    }

    m_allChecks.removeDuplicates();
}

QStringList CheckSet::defaults() const
{
    // TODO: read this from clang-tidy, to pick up global settings
    const QStringList defaultChecks{
        QStringLiteral("clang-analyzer-*"),
    };

    return defaultChecks;
}

}
