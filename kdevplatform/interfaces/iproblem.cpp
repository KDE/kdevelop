/*
    SPDX-FileCopyrightText: 2015 Laszlo Kis-Adam

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "iproblem.h"

// Qt
#include <QIcon>
#include <QString>

using namespace KDevelop;

QIcon IProblem::iconForSeverity(IProblem::Severity severity)
{
    switch (severity) {
    case IProblem::Hint:
        return QIcon::fromTheme(QStringLiteral("data-information"), QIcon::fromTheme(QStringLiteral("dialog-information")));
    case IProblem::Warning:
        return QIcon::fromTheme(QStringLiteral("data-warning"), QIcon::fromTheme(QStringLiteral("dialog-warning")));
    case IProblem::Error:
        return QIcon::fromTheme(QStringLiteral("data-error"), QIcon::fromTheme(QStringLiteral("dialog-error")));
    case IProblem::NoSeverity:
        return {};
    }
    Q_UNREACHABLE();
    return {};
}

IProblem::IProblem()
{
}

IProblem::~IProblem() = default;
