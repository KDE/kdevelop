/*
    SPDX-FileCopyrightText: 2017 Anton Anikin <anton.anikin@htower.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QString>

class QWidget;

namespace Heaptrack
{
    QString findExecutable(const QString& fallbackExecutablePath);

    QWidget* activeMainWindow();
}
