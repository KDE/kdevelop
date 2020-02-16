/*
 * Copyright 2015 Laszlo Kis-Adam
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
