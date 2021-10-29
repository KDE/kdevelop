/*
    SPDX-FileCopyrightText: 2016 Carlos Nihelton <carlosnsoliveira@gmail.com>
    SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
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
